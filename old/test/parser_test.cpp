/*
parser_test.cpp

*/


// to run: g++ -Wall -std=c++20 -g -O0 -Isimpletest -mconsole -o BIN/parser_test Mathly/test/parser_test.cpp

#include <vector>
#include <variant>

#include "..\parser.hpp"
#include "..\lexer.hpp"
#include "..\..\simpletest\simpletest.h"


using u64 = uint64_t;

int checkParserErrors(const Parser& p);
bool testNumber(std::unique_ptr<ExpressionNode>& exp, u64 num, const char* tokLiteral);
bool testExpression(std::unique_ptr<ExpressionNode>& exp, const std::variant<u64, std::string>& expectedValue, const std::string& tokLiteral);


DEFINE_TEST(TestNumberExpression) {
    std::string input = "1928378412$";

    Lexer lexer = Lexer(input);
    Parser parser = Parser(lexer);
    std::unique_ptr<ExpressionNode> parsedExpr = parser.parseLoop();
    NumberExpressionNode* convertedparsedExpr = dynamic_cast<NumberExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
    int errorCode = checkParserErrors(parser);
    TEST_EQ(errorCode, 0);
    
    
    // Test if the VariableExpressionNode AST node has a value of "area"
    TEST( convertedparsedExpr->Value == 1928378412U );

    // Test if the Tok.Literal attribute is "area"
    TEST( parsedExpr->TokenLiteral() == "1928378412" );
}

DEFINE_TEST(TestVariableExpression) {
    std::string input = "area$";

    Lexer lexer = Lexer(input);
    Parser parser = Parser(lexer);
    std::unique_ptr<ExpressionNode> parsedExpr = parser.parseLoop();
    VariableExpressionNode* convertedparsedExpr = dynamic_cast<VariableExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
    int errorCode = checkParserErrors(parser);
    TEST_EQ(errorCode, 0);
    

    // Test if the VariableExpressionNode AST node has a value of "area"
    TEST( convertedparsedExpr->Value == "area" );

    // Test if the Tok.Literal attribute is "area"
    TEST( parsedExpr->TokenLiteral() == "area" );
}

DEFINE_TEST(TestPrefixExpression) {
    std::string input = "-20$";

    Lexer lexer = Lexer(input);
    Parser parser = Parser(lexer);
    std::unique_ptr<ExpressionNode> parsedExpr = parser.parseLoop();
    PrefixExpressionNode* convertedparsedExpr = dynamic_cast<PrefixExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
    int errorCode = checkParserErrors(parser);
    TEST_EQ(errorCode, 0);
    
    // Test "negative" prefix operator
    TEST( convertedparsedExpr->Operator == '-' );
    
    // Test number
    TEST( testNumber(convertedparsedExpr->Right, 20U, "20") );

    // Test tokenLiteral correctly parsed and stored, which is the prefix operator
    TEST( parsedExpr->TokenLiteral() == "-" );
}

DEFINE_TEST(TestParsingInfixExpressions) {

    struct testInfix_s {
        std::string input;
        std::variant<u64, std::string> leftValue;
        std::string op;
        std::variant<u64, std::string> rightValue;
    };


    std::vector<testInfix_s> testTable = {
        {"5 + 5$", 5U, "+", 5U},
        {"5 - 5$", 5U, "-", 5U},
        {"5 * 5$", 5U, "*", 5U},
        {"5 / 5$", 5U, "/", 5U},
        {"a + 5$", "a", "+", 5U},
        {"5 + b$", 5U, "+", "b"},
        {"x - y$", "x", "-", "y"},
        {"num * 10$", "num", "*", 10U},
    };
    

    for (size_t i=0; i < testTable.size(); i++) {
        const testInfix_s test = testTable[i];
        std::string input = test.input;

        Lexer lexer = Lexer(input);
        Parser parser = Parser(lexer);
        std::unique_ptr<ExpressionNode> parsedExpr = parser.parseLoop();

        InfixExpressionNode* convertedparsedExpr = dynamic_cast<InfixExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
        int errorCode = checkParserErrors(parser);
        TEST_EQ(errorCode, 0);


        // Test left operand
        std::string leftLiteral = std::holds_alternative<u64>(test.leftValue) 
                                ? std::to_string(std::get<u64>(test.leftValue)) 
                                : std::get<std::string>(test.leftValue);
        

        // Test right operand
        std::string rightLiteral = std::holds_alternative<u64>(test.rightValue) 
                                ? std::to_string(std::get<u64>(test.rightValue)) 
                                : std::get<std::string>(test.rightValue);

        if (parsedExpr->getKind() == InfixKind::DIFFERENCE || parsedExpr->getKind() == InfixKind::FRACTION) {

            TEST(testExpression(convertedparsedExpr->Left, test.leftValue, leftLiteral));
            
            // Test operator
            TEST_EQ(convertedparsedExpr->Operator, *(test.op.c_str()));
            TEST(testExpression(convertedparsedExpr->Right, test.rightValue, rightLiteral));   
        } else if (parsedExpr->getKind() == InfixKind::PLUS || parsedExpr->getKind() == InfixKind::MULTIPLY) {
            auto convertedparsedExpr = dynamic_cast<NaryExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
            TEST(testExpression(convertedparsedExpr->Operands[0], test.leftValue, leftLiteral));
            
            // Test operator
            TEST_EQ(convertedparsedExpr->Operator, *(test.op.c_str()));
            TEST(testExpression(convertedparsedExpr->Operands[1], test.rightValue, rightLiteral));   
        }

    }    

}

DEFINE_TEST(TestOperatorPrecedence) {
    struct testOpPrec_s {
        std::string input;
        std::string expected;
    };

    // TODO: write more tests for this, and remember to enclose whole expression in brackets for expected string
    std::vector<testOpPrec_s> testTable = {
        {"1 * (2 + 2)$", "(1 * (2 + 2))"},
        {"2(2 + 2x)$", "(2 * (2 + (2 * x)))"},
        {"5a + b + c$", "(((5 * a) + b) + c)"},
        {"1 * 2 * 3a$", "((1 * 2) * (3 * a))"},
        {"1 * 2a * 3$", "((1 * (2 * a)) * 3)"},
        {"2x + -3x * 4x / 2 * -2x$", "((2 * x) + (((((-3) * x) * (4 * x)) / 2) * ((-2) * x)))"},
        {"2 + 3 * 4$", "(2 + (3 * 4))"},
        {"-a * b$", "((-a) * b)"},
        {"(a + b) * c$", "((a + b) * c)"},
        {"a + b + c + d$", "(((a + b) + c) + d)"},
        {"(((a + b) + c) + d)$", "(((a + b) + c) + d)"},
        {"a + b * c + d / 2$", "((a + (b * c)) + (d / 2))"}
        
        
        
    };

    for (size_t i=0; i < testTable.size(); i++) {
        const testOpPrec_s test = testTable[i];
        std::string input = test.input;

        Lexer lexer = Lexer(input);
        Parser parser = Parser(lexer);
        std::unique_ptr<ExpressionNode> parsedExpr = parser.parseLoop();
        ExpressionNode* convertedparsedExpr = dynamic_cast<ExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
        int errorCode = checkParserErrors(parser);
        TEST_EQ(errorCode, 0);
        
        if (parsedExpr->getKind() == InfixKind::DIFFERENCE || parsedExpr->getKind() == InfixKind::FRACTION) {
            convertedparsedExpr = dynamic_cast<InfixExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
        } else if (parsedExpr->getKind() == InfixKind::MULTIPLY || parsedExpr->getKind() == InfixKind::PLUS) {
            convertedparsedExpr = dynamic_cast<NaryExpressionNode*>(parsedExpr.get());  // dynamic casting to access class members
        }

        auto actual = convertedparsedExpr->String();
        
        TEST_EQ(actual, test.expected);

}
}

bool testExpression(std::unique_ptr<ExpressionNode>& exp, const std::variant<u64, std::string>& expectedValue, const std::string& tokLiteral) {
        if (std::holds_alternative<u64>(expectedValue)) {
            // Check for NumberExpressionNode
            NumberExpressionNode* numbExp = dynamic_cast<NumberExpressionNode*>(exp.get());
            if (!numbExp || numbExp->Value != std::get<u64>(expectedValue) || numbExp->TokenLiteral() != tokLiteral) {
                return false;
            }
        } else if (std::holds_alternative<std::string>(expectedValue)) {
            // Check for VariableExpressionNode
            VariableExpressionNode* varExp = dynamic_cast<VariableExpressionNode*>(exp.get());
            if (!varExp || varExp->Value != std::get<std::string>(expectedValue) || varExp->TokenLiteral() != tokLiteral) {
                return false;
            }
        }
    return true;
}


bool testNumber(std::unique_ptr<ExpressionNode>& exp, u64 num, const char* tokLiteral) {
    NumberExpressionNode* numbExp = dynamic_cast<NumberExpressionNode*>(exp.get());

    if (numbExp->Value == num && numbExp->TokenLiteral() == tokLiteral)
        return true;
    
    return false;
}

int checkParserErrors(const Parser& p) {
    std::vector<std::string> errors = p.errors;

    if (errors.size() == 0)
        return 0;
    
    std::cerr << "\n\nparser has " << errors.size() << " error[s]" << std::endl;

    for (const auto& i : errors) {
        std::cerr << "\nparser error: " << i;
    }
    std::cerr << std::endl;

    return 1;

}

int main() {
    
    bool allTestsPassed = true;

    // Execute all tests
    allTestsPassed &= TestFixture::ExecuteAllTests(TestFixture::Verbose);

    return allTestsPassed ? 0 : 1;
}
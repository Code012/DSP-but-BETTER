#include <iostream>
#include "..\parser.hpp"
#include "..\lexer.hpp"
#include "..\simplifier.hpp"
#include "..\..\simpletest\simpletest.h"


int checkParserErrors(const Parser& p);
// g++ -Wall -std=c++20 -g -O0 -Isimpletest -mconsole -o BIN/simp_test Mathly/test/simplifier_test.cpp Mathly/ast.cpp Mathly/visitors.cpp Mathly/simplifier.cpp
// Helper function to create a number node

DEFINE_TEST(TestLinearCoefficientExpressions) {
    struct testExpr_s {
        std::string input;
        std::string expected;
    };

    std::vector<testExpr_s> testTable = {
        // ✅ Valid linear combinations
        // {"x + x$", "(2 * x)"},
        // {"2x + 3x$", "(5 * x)"},
        // {"-3x + x$", "(-2 * x)"},
        // {"x/2 + 3x/5$", "((11 / 10) * x)"},
        {"-(1/2)x + (1/2)x$", "0"}, //(-1/2)
        {"(-1/2)x + (1/2)x$", "0"}, // (-1/2)
        {"x(-1/2) + x(1/2)$", "0"}, // 
        {"3x + -3x$", "0"},
        {"2x/3 + 4x/3$", "(2 * x)"},
        {"0x + 3x$", "(3 * x)"},
        {"0 + 3x$", "(3 * x)"},
        {"1 + x$", "(1 + x)"},
        {"1/2 + 1x/2$", "((1 / 2) + ((1 / 2) * x))"},
        {"-2x + 4x/3$", "((-2 / 3) * x)"},
        {"1x/2 + 1x/4 + 1x/4$", "x"},
        {"-5x + 3x + 2x$", "0"},

        // // ✅ Subtractive combinations
        // {"x - x$", "(x - x)"},
        // {"2x - 3x$", "((2 * x) - (3 * x))"},
        // {"-3x - x$", "(((-3) * x) - x)"},
        // {"3/2x - 1/2x$", "(((3/2) * x) - ((1/2) * x))"},
        // {"1/2x - 3/2x$", "(((1/2) * x) - ((3/2) * x))"},
        // {"0x - 3x$", "((0 * x) - (3 * x))"},
        // {"1 - x$", "(1 - x)"},
        // {"1/2x - 1/4x - 1/4x$", "(((1/2) * x) - (((1/4) * x) + ((1/4) * x)))"},
    };

    for (const auto& test : testTable) {
        std::string input = test.input;
        Lexer lexer = Lexer(input);
        Parser parser = Parser(lexer);
        std::unique_ptr<ExpressionNode> parsedExpr = parser.parseLoop();
        int errorCode = checkParserErrors(parser);
        TEST_EQ(errorCode, 0);

        SimplifyVisitor visitor;
        parsedExpr->accept(visitor);
        auto simplified = visitor.getResult();

        TEST(simplified->String() == test.expected);

        simplified = visitor.expand_tree(simplified);

        std::cout << simplified->String() << "\n";

        

        
      }
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
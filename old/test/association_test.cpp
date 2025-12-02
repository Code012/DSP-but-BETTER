/*
23/2/25
*/
// g++ -Wall -std=c++20 -g -O0 -Isimpletest -mconsole -o BIN/assoc_test Mathly/test/association_test.cpp Mathly/ast.cpp Mathly/visitors.cpp
#include <vector>

#include "..\visitors.hpp"
#include "..\parser.hpp"
#include "..\..\simpletest\simpletest.h"

bool checkAssociativity(ExpressionNode* node);
int checkParserErrors(const Parser& p);



DEFINE_TEST(TestBasicAssociativeOperator) {
    struct testOpPrec_s {
        std::string input;
        std::string expected;
    };

    // TODO: write more tests for this, and remember to enclose whole expression in brackets for expected string
    std::vector<testOpPrec_s> testTable = {
        {"((1 + 3) + (4 * 4 * (3 * 2 + (3 + 2))))$", "(1 + 3 + (4 * 4 * ((3 * 2) + 3 + 2)))"},
        {"((1 + 1) + 1)$", "(1 + 1 + 1)"},
        {"((a + a) + a)$", "(a + a + a)"},
        {"((a + a) + 2)$", "(a + a + 2)"},
        {"((a + a) * a)$", "((a + a) * a)"},
        
    };

    for (size_t i=0; i < testTable.size(); i++) {
        const testOpPrec_s test = testTable[i];
        std::string input = test.input;

        Lexer lexer = Lexer(input);
        Parser parser = Parser(lexer);
        std::unique_ptr<ExpressionNode> rootNode = parser.parseLoop();
        ExpressionNode* convertedrootNode = dynamic_cast<ExpressionNode*>(rootNode.get());  // dynamic casting to access class members
        int errorCode = checkParserErrors(parser);
        TEST_EQ(errorCode, 0);

        AssociativeTransformationVisitor visitor;
        convertedrootNode->accept(visitor);
    
        if (convertedrootNode && (convertedrootNode->getKind() == InfixKind::MULTIPLY || convertedrootNode->getKind() == InfixKind::PLUS)) {
            bool isValid = checkAssociativity(convertedrootNode);
            TEST_EQ(isValid, true);
        }

        auto actual = convertedrootNode->String();
        TEST_EQ(actual, test.expected);
    }
}

// Recursive function to check associativity
bool checkAssociativity(ExpressionNode* node) {
    if (!node) return true;  // Base case

    NaryExpressionNode* naryNode = dynamic_cast<NaryExpressionNode*>(node);
    if (!naryNode) return true;

    InfixKind kind = naryNode->getKind();
    
    for (size_t i = 0; i < naryNode->Operands.size(); i++) {
        ExpressionNode* child = naryNode->Operands[i].get();  // Get operand

        if (child && child->getKind() == kind) {
            return false;  // Found a nested operation of the same type
        }

        // Recursively check all children
        if (!checkAssociativity(child)) return false;
    }
    
    return true;
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
/*
lexer_test.cpp
Author: Shahbaz
Date: 20/02/2025
*/

// to run: g++ -Wall -std=c++20 -g -O0 -Isimpletest -mconsole -o BIN/lexer_test Mathly/test/lexer_test.cpp Mathly/lexer.cpp  simpletest/simpletest.cpp

#include <vector>

#include "..\src\lexer.hpp"
#include "..\..\simpletest\simpletest.h"




    DEFINE_TEST(TestNextToken) {
        std::string input = "(501341324 + 10234) * 32 + x / area$";

        struct testToken {
            TokenType expectedType;
            std::string expectedLiteral;
        };

        std::vector<testToken> testTable;

        testTable.push_back(testToken{token::LPAREN, "("});
        testTable.push_back(testToken{token::INT, "501341324"});
        testTable.push_back(testToken{token::PLUS, "+"});
        testTable.push_back(testToken{token::INT, "10234"});
        testTable.push_back(testToken{token::RPAREN, ")"});
        testTable.push_back(testToken{token::MULT, "*"});
        testTable.push_back(testToken{token::INT, "32"});
        testTable.push_back(testToken{token::PLUS, "+"});
        testTable.push_back(testToken{token::VAR, "x"});
        testTable.push_back(testToken{token::DIV, "/"});
        testTable.push_back(testToken{token::VAR, "area"});
        testTable.push_back(testToken{token::EOL, "$"});

        Lexer lexer = Lexer(input);
        std::vector<Token> tokens = lexer.lex();

        for (size_t i=0; i < testTable.size(); i++) {
            const Token tok = tokens[i];
            const testToken test = testTable[i];
            
             TEST_MESSAGE(
                tok.Type == test.expectedType, 
                "test table row %d -- token type wrong. expected=%s, got=%s",
                i, test.expectedType.c_str(), tok.Type.c_str()
            ); 

            TEST_MESSAGE(
                tok.Literal == test.expectedLiteral, 
                "test table row %d -- token literal wrong. expected=%s, got=%s",
                i, test.expectedLiteral.c_str(), tok.Literal.c_str()
            ); 
            
        }  



    }


int main() {
    
    bool allTestsPassed = true;

    // Execute all tests
    allTestsPassed &= TestFixture::ExecuteAllTests(TestFixture::Verbose);

    return allTestsPassed ? 0 : 1;

}
/*
ast.cpp

Author: Shahbaz
Date 26/01/2025
*/
// for making ast: https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html
// See: https://lesleylai.info/en/ast-in-cpp-part-1-variant/ for visitor apttern 

#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <sstream>

#include "token.hpp"
#include "visitors_fwd.hpp"


enum class ExprKind {PLUS, MULTIPLY, DIFFERENCE, QUOTIENT, FRACTION, NUM, VAR, PRE_UNARY_MINUS};


// Base class for all expressions
class ExpressionNode {
    public: 
    
        virtual ~ExpressionNode() = default;
        
        virtual std::string TokenLiteral() const = 0;
        virtual std::string String() const = 0;
        virtual ExprKind getKind() const = 0;

        virtual void accept(ExprVisitor& visitor) const  = 0;
        virtual void accept(ExprMutableVisitor& visitor) = 0;
};


class NumberExpressionNode : public ExpressionNode {
    public:
        Token Tok;   
        int Value;  //TODO:change this to a double when rewriting
        ExprKind Kind;

        NumberExpressionNode(Token tok, int Val, ExprKind Kind);
        
        std::string TokenLiteral() const override;
        std::string String() const override;
        ExprKind getKind() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};

class VariableExpressionNode : public ExpressionNode {
    public: 
        std::string Value;
        Token Tok;    // token::VAR
        ExprKind Kind;

        VariableExpressionNode(const std::string &Name, Token &tok, ExprKind Kind);
        
        virtual std::string TokenLiteral() const override;
        virtual std::string String() const override;
        ExprKind getKind() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};


class PrefixExpressionNode : public ExpressionNode { 
    public:
        char Operator;
        Token Tok;        // prefix token, e.g. - for negative numbers    
        ExprKind Kind;
        std::unique_ptr<ExpressionNode> Right;

        PrefixExpressionNode(char Op, Token &tok, ExprKind Kind, std::unique_ptr<ExpressionNode> Right = nullptr);

        std::string TokenLiteral() const override;
        ExprKind getKind() const override;
        std::string String() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};

class BinaryExpressionNode : public ExpressionNode { 
    public:
        Token Tok;        // operator token, e.g. +, *, -, /
        char Operator;
        ExprKind Kind;
        std::unique_ptr<ExpressionNode> Left, Right;
        
        BinaryExpressionNode(Token &tok, char Op, ExprKind Kind, std::unique_ptr<ExpressionNode> Left, std::unique_ptr<ExpressionNode> Right = nullptr);

        std::string TokenLiteral() const override;
        ExprKind getKind() const override;
        std::string String() const override;


        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
        };


class NaryExpressionNode : public ExpressionNode {
    public:
        Token Tok;      // +, *
        char Operator;
        ExprKind Kind;
        std::vector<std::unique_ptr<ExpressionNode>> Operands;  //TODO: a vector of unique pointers is not very good, all those pointers are allocated in random areas in memory, better to use an area allocator. Use areas when re writing everything for report

        NaryExpressionNode(Token &tok, char Op, ExprKind Kind, std::vector<std::unique_ptr<ExpressionNode>> ops);

        std::string TokenLiteral() const override;
        ExprKind getKind() const override;
        std::string String() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};



#endif
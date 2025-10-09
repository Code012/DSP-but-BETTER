/*
ast.cpp

Author: Shahbaz
Date 26/01/2025
*/
// for making ast: https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html
// See: https://lesleylai.info/en/ast-in-cpp-part-1-variant/ for visitor apttern 

#ifndef AST_HPP
#define AST_HPP



// class ExpressionNode;
// class NumberExpressionNode;
// class VariableExpressionNode;
// class PrefixExpressionNode;
// class InfixExpressionNode;
// class NaryExpressionNode;
// enum class InfixKind;

#include <iostream>
#include <vector>
#include <memory>
#include <sstream>

#include "token.hpp"
#include "visitors_fwd.hpp"

enum class InfixKind {PLUS, MULTIPLY, POWER, DIFFERENCE, DIVIDE, FRACTION, NUM, VAR, PRE_MINUS};

// Base class for all expressions
class ExpressionNode {
    public: 
        int id;
        bool is_root = false;
        
        static int nextId;
    
        virtual ~ExpressionNode() = default;
        
        virtual std::string TokenLiteral() const = 0;
        virtual std::string String() const = 0;
        virtual InfixKind getKind() const = 0;

        virtual void accept(ExprVisitor& visitor) const  = 0;
        virtual void accept(ExprMutableVisitor& visitor) = 0;

        ExpressionNode() : id(nextId++) {}
};


class NumberExpressionNode : public ExpressionNode {
    public:
        Token Tok;   
        int Value;  //TODO:change this to a double when rewriting
        InfixKind Kind;

        NumberExpressionNode(Token tok, int Val, InfixKind Kind);
        
        std::string TokenLiteral() const override;
        std::string String() const override;
        InfixKind getKind() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;


};

class VariableExpressionNode : public ExpressionNode {
    public: 
        std::string Value;
        Token Tok;    // token::VAR
        InfixKind Kind;

        VariableExpressionNode(const std::string &Name, Token &tok, InfixKind Kind);
        
        virtual std::string TokenLiteral() const override;
        virtual std::string String() const override;
        InfixKind getKind() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};


class PrefixExpressionNode : public ExpressionNode {    //* RENAME TO UNARY
    public:
        char Operator;
        Token Tok;        // prefix token, e.g. - for negative numbers    
        InfixKind Kind;
        std::unique_ptr<ExpressionNode> Right;

        PrefixExpressionNode(char Op, Token &tok, InfixKind Kind, std::unique_ptr<ExpressionNode> Right = nullptr);

        std::string TokenLiteral() const override;
        InfixKind getKind() const override;
        std::string String() const override;
        
        

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};

class InfixExpressionNode : public ExpressionNode { //* RENAME TO BINARY WHEN YOU REDO EVERYTHING FOR DISSERATTION
    public:
        Token Tok;        // operator token, e.g. +, *, -, /
        char Operator;
        InfixKind Kind;
        std::unique_ptr<ExpressionNode> Left, Right;
        

        std::string TokenLiteral() const override;
        InfixKind getKind() const override;
        std::string String() const override;

         InfixExpressionNode(Token &tok, char Op, InfixKind Kind, std::unique_ptr<ExpressionNode> Left, std::unique_ptr<ExpressionNode> Right = nullptr);
        
        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};


class NaryExpressionNode : public ExpressionNode {
    public:
        Token Tok;      // +, *
        char Operator;
        InfixKind Kind;
        std::vector<std::unique_ptr<ExpressionNode>> Operands;  //TODO: a vector of unique pointers is not very good, all those pointers are allocated in random areas in memory, better to use an area allocator. Use areas when re writing everything for report

        NaryExpressionNode(Token &tok, char Op, InfixKind Kind, std::vector<std::unique_ptr<ExpressionNode>> ops);

        std::string TokenLiteral() const override;
        InfixKind getKind() const override;
        std::string String() const override;

        void accept(ExprVisitor& visitor) const override;
        void accept(ExprMutableVisitor& visitor) override;
};



#endif
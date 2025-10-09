/*
ast.cpp

Author: Shahbaz
Date 26/01/2025
*/
//for making ast: https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html
// See: https://lesleylai.info/en/ast-in-cpp-part-1-variant/ for visitor apttern 

#ifndef AST_CPP
#define AST_CPP

#include "ast.hpp"
#include "visitors.hpp"


int ExpressionNode::nextId = 0;

// Number Node
NumberExpressionNode::NumberExpressionNode(Token tok, int Val, InfixKind Kind) : Tok(tok), Value(Val), Kind(Kind) {}

std::string NumberExpressionNode::TokenLiteral() const { return Tok.Literal; }
std::string NumberExpressionNode::String() const { return Tok.Literal; }


InfixKind NumberExpressionNode::getKind() const { return Kind; }

void NumberExpressionNode::accept(ExprVisitor& visitor) const {visitor.visit(*this);}
void NumberExpressionNode::accept(ExprMutableVisitor& visitor) {visitor.visit(*this);}


// Variable Node
VariableExpressionNode::VariableExpressionNode(const std::string &Name, Token &tok, InfixKind Kind) : Value(Name), Tok(tok), Kind(Kind)  {}

std::string VariableExpressionNode::TokenLiteral() const { return Tok.Literal; };
std::string VariableExpressionNode::String() const { return Value; };

InfixKind VariableExpressionNode::getKind() const { return Kind; };

void VariableExpressionNode::accept(ExprVisitor& visitor) const {visitor.visit(*this);}
void VariableExpressionNode::accept(ExprMutableVisitor& visitor) {visitor.visit(*this);}



// UNARY/PREFIX Node
PrefixExpressionNode:: PrefixExpressionNode(char Op, Token &tok, InfixKind Kind, std::unique_ptr<ExpressionNode> Right) 
        : Operator(Op), Tok(tok), Kind(Kind), Right(std::move(Right)) {}

std::string PrefixExpressionNode :: TokenLiteral() const { return Tok.Literal; };
InfixKind PrefixExpressionNode :: getKind() const  { return Kind; };
std::string PrefixExpressionNode :: String() const { 
    std::ostringstream oss;

    oss << "(";
    oss << Operator;
    oss << Right->String();
    oss << ")";

    std::string result = oss.str();

    return result;
};


                        // kinda like the look of spacing the scope operator
void PrefixExpressionNode :: accept(ExprVisitor& visitor) const {visitor.visit(*this);}
void PrefixExpressionNode :: accept(ExprMutableVisitor& visitor) {visitor.visit(*this);}


// INFIX NODE

std::string InfixExpressionNode :: TokenLiteral() const { return Tok.Literal; };
InfixKind  InfixExpressionNode :: getKind() const { return Kind; };
std::string InfixExpressionNode :: String() const { 
    std::ostringstream oss;

    oss << "(";
    oss << Left->String();
    oss << " " << Operator << " ";
    oss << Right->String();
    oss << ")";

    std::string result = oss.str();

    return result;
    };


InfixExpressionNode :: InfixExpressionNode(Token &tok, char Op, InfixKind Kind, std::unique_ptr<ExpressionNode> Left, std::unique_ptr<ExpressionNode> Right) 
        : Tok(tok), Operator(Op), Kind(Kind), Left(std::move(Left)), Right(std::move(Right)) {}

void InfixExpressionNode :: accept(ExprVisitor& visitor) const  {visitor.visit(*this);}
void InfixExpressionNode :: accept(ExprMutableVisitor& visitor)  {visitor.visit(*this);}




// Nary expression node
NaryExpressionNode :: NaryExpressionNode(Token &tok, char Op, InfixKind Kind, std::vector<std::unique_ptr<ExpressionNode>> ops)
: Tok(tok), Operator(Op), Kind(Kind), Operands(std::move(ops)) {}

std::string NaryExpressionNode :: TokenLiteral() const { return Tok.Literal; }
InfixKind NaryExpressionNode :: getKind() const { return Kind; }


std::string NaryExpressionNode :: String() const {
    std::ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < Operands.size(); i++) {
        oss << Operands[i]->String();
        if (i != Operands.size() - 1) oss << " " << Operator << " ";
    }
    oss << ")";
    return oss.str();
}

void NaryExpressionNode :: accept(ExprVisitor& visitor) const  {visitor.visit(*this);}
void NaryExpressionNode :: accept(ExprMutableVisitor& visitor)  {visitor.visit(*this);}





#endif
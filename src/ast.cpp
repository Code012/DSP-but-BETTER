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




// Number Node
NumberExpressionNode::NumberExpressionNode(Token tok, int Val, ExprKind Kind) : Tok(tok), Value(Val), Kind(Kind) {}

std::string NumberExpressionNode::TokenLiteral() const { return Tok.Literal; }
std::string NumberExpressionNode::String() const { return Tok.Literal; }


ExprKind NumberExpressionNode::getKind() const { return Kind; }

void NumberExpressionNode::accept(ExprVisitor& visitor) const {visitor.visit(*this);}
void NumberExpressionNode::accept(ExprMutableVisitor& visitor) {visitor.visit(*this);}


// Variable Node
VariableExpressionNode::VariableExpressionNode(const std::string &Name, Token &tok, ExprKind Kind) : Value(Name), Tok(tok), Kind(Kind)  {}

std::string VariableExpressionNode::TokenLiteral() const { return Tok.Literal; };
std::string VariableExpressionNode::String() const { return Value; };

ExprKind VariableExpressionNode::getKind() const { return Kind; };

void VariableExpressionNode::accept(ExprVisitor& visitor) const {visitor.visit(*this);}
void VariableExpressionNode::accept(ExprMutableVisitor& visitor) {visitor.visit(*this);}



// UNARY/PREFIX Node
PrefixExpressionNode:: PrefixExpressionNode(char Op, Token &tok, ExprKind Kind, std::unique_ptr<ExpressionNode> Right) 
        : Operator(Op), Tok(tok), Kind(Kind), Right(std::move(Right)) {}

std::string PrefixExpressionNode :: TokenLiteral() const { return Tok.Literal; };
ExprKind PrefixExpressionNode :: getKind() const  { return Kind; };
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


// Binary NODE

std::string BinaryExpressionNode :: TokenLiteral() const { return Tok.Literal; };
ExprKind  BinaryExpressionNode :: getKind() const { return Kind; };
std::string BinaryExpressionNode :: String() const { 
    std::ostringstream oss;

    oss << "(";
    oss << Left->String();
    oss << " " << Operator << " ";
    oss << Right->String();
    oss << ")";

    std::string result = oss.str();

    return result;
    };


BinaryExpressionNode :: BinaryExpressionNode(Token &tok, char Op, ExprKind Kind, std::unique_ptr<ExpressionNode> Left, std::unique_ptr<ExpressionNode> Right) 
        : Tok(tok), Operator(Op), Kind(Kind), Left(std::move(Left)), Right(std::move(Right)) {}

void BinaryExpressionNode :: accept(ExprVisitor& visitor) const  {visitor.visit(*this);}
void BinaryExpressionNode :: accept(ExprMutableVisitor& visitor)  {visitor.visit(*this);}




// Nary expression node
NaryExpressionNode :: NaryExpressionNode(Token &tok, char Op, ExprKind Kind, std::vector<std::unique_ptr<ExpressionNode>> ops)
: Tok(tok), Operator(Op), Kind(Kind), Operands(std::move(ops)) {}

std::string NaryExpressionNode :: TokenLiteral() const { return Tok.Literal; }
ExprKind NaryExpressionNode :: getKind() const { return Kind; }


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
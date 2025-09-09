/*
18/2/25
*/

#ifndef VISITOR_CPP
#define VISITOR_CPP

#include "ast_fwd.hpp"




/**
 * @brief Base class for mutable AST visitors.
 * 
 * This visitor is used for transformations, particularly during simplification.
 * Each visit method corresponds to a specific AST node type.
 */
struct ExprMutableVisitor {
    ExprMutableVisitor() = default;
    virtual ~ExprMutableVisitor() = default;

    virtual void visit(NumberExpressionNode&) = 0;
    virtual void visit(VariableExpressionNode&) = 0;
    virtual void visit(PrefixExpressionNode&) = 0;
    virtual void visit(BinaryExpressionNode&) = 0;
    virtual void visit(NaryExpressionNode&) = 0;
};

/**
 * @brief Base class for read-only AST visitors.
 * 
 * This visitor is used when traversing the AST without modifying it.
 * Each visit method corresponds to a specific AST node type.
 */
struct ExprVisitor {
    ExprVisitor() = default;
    virtual ~ExprVisitor() = default;

    virtual void visit(const NumberExpressionNode&) = 0;
    virtual void visit(const VariableExpressionNode&) = 0;
    virtual void visit(const PrefixExpressionNode&) = 0;
    virtual void visit(const BinaryExpressionNode &) = 0;
    virtual void visit(const NaryExpressionNode &) = 0;
};

#endif        
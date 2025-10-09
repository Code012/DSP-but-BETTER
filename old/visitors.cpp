
// #include "visitors.hpp"
// // #define NDEBUG
// #include <cassert>

// void AssociativeTransformationVisitor :: visit(NumberExpressionNode& node) { };
// void AssociativeTransformationVisitor :: visit(VariableExpressionNode& node) { };
// void AssociativeTransformationVisitor :: visit(PrefixExpressionNode& node) { };
// void AssociativeTransformationVisitor :: visit(InfixExpressionNode &node)  { };
    


// void AssociativeTransformationVisitor :: visit(NaryExpressionNode& node) { 
//     // check for + or *, because associativity is a property of these operators
//     if (node.getKind() == InfixKind::PLUS || node.getKind() == InfixKind::MULTIPLY) {
//         // operand list
//         std::vector<std::unique_ptr<ExpressionNode>> flattenedOperands;

//         // Flatten operands
//         for (size_t i = 0; i < node.Operands.size(); i++) {
//             flattenOperands(node.Operands[i], node.getKind(), flattenedOperands);
//         }

//         if (!flattenedOperands.empty()) {
//             node.Operands = std::move(flattenedOperands);
//             // Recurse to flatten children of current node (root)
//             for (size_t i = 0; i < node.Operands.size(); i++) {
//                 node.Operands[i]->accept(*this);
//             }
//         }



//     }
// }


// void AssociativeTransformationVisitor :: flattenOperands(std::unique_ptr<ExpressionNode>& node, InfixKind kind, std::vector<std::unique_ptr<ExpressionNode>>& flattenedOperands) {
//     if (!node) return;
    
//     // If the node is an infix node and has the same operator as the parent node (*, +), recurse
//     if (kind == InfixKind::PLUS || kind == InfixKind::MULTIPLY) {

//         if ( node->getKind() == kind) {
//             auto naryNode = dynamic_cast<NaryExpressionNode*>(node.get());
//             if (naryNode) {
                
//                 for (size_t i = 0; i < naryNode->Operands.size(); i++) {
//                     flattenOperands(naryNode->Operands[i], naryNode->getKind(), flattenedOperands);
//                 }
//                 return;
//             }
//         } 
//     }

//     // if the operator is different, append it as an opperand
//     flattenedOperands.push_back(std::move(node));
    
// }
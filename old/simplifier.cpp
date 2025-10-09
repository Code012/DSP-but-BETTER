#ifndef SIMPLIFIER_CPP
#define SIMPLIFIER_CPP

#include "simplifier.hpp"


// // Main automatic simplify function
// std::unique_ptr<ExpressionNode> automatic_simplify(std::unique_ptr<ExpressionNode> expr) {
//     SimplifyVisitor visitor;
//     expr->accept(visitor);
//     return visitor.getResult();
// }

// Visitor implementations
void SimplifyVisitor::visit(NumberExpressionNode& node) {
    // Integer/constant nodes are already simplified
    result = std::make_unique<NumberExpressionNode>(node.Tok, node.Value, node.Kind);
}

void SimplifyVisitor::visit(VariableExpressionNode& node) {
    // Symbol nodes are already simplified
    result = std::make_unique<VariableExpressionNode>(node.Value, node.Tok, node.Kind);
}

void SimplifyVisitor::visit(PrefixExpressionNode& node) { // pg 106
    // Simplify the operand
    auto simplified_right = automatic_simplify(std::move(node.Right), *this);

    // u - v --> u + (-1) . v
    // this way we can utilise our sum and product simplification functions, 
    // and as a result, they're associative properties
    std::vector<std::unique_ptr<ExpressionNode>> sum_operands;

    // Check if simplified operand is a number
    if (simplified_right->getKind() == InfixKind::NUM) {
        // If so, just negate the value directly
        auto right_cast = dynamic_cast<NumberExpressionNode*>(simplified_right.get());
        auto num = createNumber(-1 * right_cast->Value);
        sum_operands.push_back(std::move(num));
    } else {
        // Otherwise, rewrite the expression as a product, (-1) . v
        std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
        prod_operands.push_back(createNumber(-1));
        prod_operands.push_back(std::move(simplified_right));
        // Create and simplify product
        auto prod = createProduct(std::move(prod_operands));               // (-1) . v
        auto simplified_prod = simplify_product(std::move(prod));
        // Add as operands for + node
        sum_operands.push_back(std::move(simplified_prod));                // u + (-1) . v
    } 

    // Create a new NaryExpressionNode representing the sum: u + (-1) * v
    Token sumToken{token::PLUS, "+"};
    auto sum = std::make_unique<NaryExpressionNode>(
        sumToken,
        '+', 
        InfixKind::PLUS, 
        std::move(sum_operands)
    );

    result = simplify_sum(std::move(sum));
}

void SimplifyVisitor::visit(InfixExpressionNode& node) {
    // Simplify the operands

    auto simplified_left = automatic_simplify(std::move(node.Left), *this);
    auto simplified_right = automatic_simplify(std::move(node.Right), *this);
    
    // Handle DIVIDE operation by transforming into FRACTION (rational number expression) 
    if (node.Kind == InfixKind::DIVIDE) {
        // Check if both operands are integers 
        bool left_is_integer = isInteger(simplified_left.get());
        bool right_is_integer = isInteger(simplified_right.get());

        // Check denomicator for division by zero
        if (isZero(simplified_right.get())) {
            // Return an Undefined node
            Token undefinedTok{token::VAR, "Undefined"};
            result = std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
            return;
        }

        if (left_is_integer && right_is_integer) {
            // Fraction c/d, where c and d are integers
            int numerator = getintValue(simplified_left.get());
            int denominator = getintValue(simplified_right.get());

            // Create a simplified fraction
            result = simplify_rational_number(createFraction(numerator, denominator));
        } else if ((simplified_left->getKind() == InfixKind::FRACTION) && (simplified_right->getKind() == InfixKind::FRACTION)) {
            // Quotient (a/b) / (c/d)
            result = simplify_rne(createQuotient(std::move(simplified_left), std::move(simplified_right)));
        } else if ((simplified_left->getKind() == InfixKind::MULTIPLY) && (simplified_right->getKind() == InfixKind::NUM)) {
            // (Num * Var) / Num, if numerator is MULTIPLY then its definitely a coeff-var pair. Very hacky but im 8 days away from deadline. book does this a different way but im trying to save time.
            auto* left = dynamic_cast<NaryExpressionNode*>(simplified_left.get());
            int numerator = getintValue(left->Operands[0].get());
            int denominator = getintValue(simplified_right.get());
            auto rne = simplify_rational_number(createFraction(numerator, denominator));
        

            std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
            prod_operands.push_back(std::move(rne));
            prod_operands.push_back(std::move(left->Operands[1]));

            Token multToken{token::MULT, "*"};
            auto prod = std::make_unique<NaryExpressionNode>(
                multToken,
                '*', 
                InfixKind::MULTIPLY, 
                std::move(prod_operands)
            );
            
            result = simplify_product(std::move(prod));
        } else if ((simplified_left->getKind() == InfixKind::VAR) && (simplified_right->getKind() == InfixKind::NUM)) { //* new
                // e.g. x/4 -> (1/4) * x
                int denominator = getintValue(simplified_right.get());
                auto rne = simplify_rational_number(createFraction(1, denominator));

                std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
                prod_operands.push_back(std::move(rne));
                prod_operands.push_back(std::move(simplified_left));

                Token multToken{token::MULT, "*"};
                auto prod = std::make_unique<NaryExpressionNode>(
                    multToken,
                    '*', 
                    InfixKind::MULTIPLY, 
                    std::move(prod_operands)
                );

                result = simplify_product(std::move(prod));

        } else {

            if (simplified_left->getKind() == InfixKind::VAR && simplified_right->getKind() == InfixKind::NUM) {
                // x/1 -> x
                auto* left = dynamic_cast<VariableExpressionNode*>(simplified_left.get());
                if (isOne(simplified_right.get())) {
                    result = std::make_unique<VariableExpressionNode>(
                        left->Value,
                        left->Tok,
                        left->Kind
                    );
                    return;
                }
            }

            result = std::make_unique<InfixExpressionNode>(
                node.Tok,
                node.Operator,
                node.Kind,
                std::move(simplified_left),
                std::move(simplified_right)
            );
        }

    } else if (node.Kind == InfixKind::DIFFERENCE) {
        // if ((simplified_left->getKind() == InfixKind::FRACTION) && (simplified_right->getKind() == InfixKind::FRACTION)) {

        //     result = simplify_rne(createDifference(std::move(simplified_left), std::move(simplified_right)));

            

        // } else {

        //     result = std::make_unique<InfixExpressionNode>(
        //         node.Tok,
        //         node.Operator,
        //         node.Kind,
        //         std::move(simplified_left),
        //         std::move(simplified_right)
        //     );
        // }

        // u - v --> u + (-1) . v
        // this way we can utilise our sum and product simplification functions, and the associative properties of these two operators
        std::vector<std::unique_ptr<ExpressionNode>> sum_operands;
    
        // std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
        // prod_operands.push_back(createNumber(-1));
        // prod_operands.push_back(std::move(simplified_right));
        // auto prod = createProduct(std::move(prod_operands));    // (-1) . v 

        if (simplified_right->getKind() == InfixKind::NUM) {
            auto right_cast = dynamic_cast<NumberExpressionNode*>(simplified_right.get());
            auto num = createNumber(-1 * right_cast->Value);
            sum_operands.push_back(std::move(simplified_left));
            sum_operands.push_back(std::move(num));
        } else {
            std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
            prod_operands.push_back(createNumber(-1));
            prod_operands.push_back(std::move(simplified_right));
            auto prod = createProduct(std::move(prod_operands));    // (-1) . v
            auto simplified_prod = simplify_product(std::move(prod));
            

            sum_operands.push_back(std::move(simplified_left));
            sum_operands.push_back(std::move(simplified_prod));                // u + (-1) . v
        } 


        

        Token sumToken{token::PLUS, "+"};

        auto sum = std::make_unique<NaryExpressionNode>(
            sumToken,
            '+', 
            InfixKind::PLUS, 
            std::move(sum_operands)
        );

        result = simplify_sum(std::move(sum));
    }

}

std::unique_ptr<ExpressionNode> SimplifyVisitor::simplify_rational_number(std::unique_ptr<ExpressionNode> expr) {
    if (!expr) return expr;

    // Already in standard form if its an integer
    if (expr->getKind() == InfixKind::NUM) return expr;

    // If its a fraction, convert to standard form
    if (expr->getKind() == InfixKind::FRACTION) {
        const InfixExpressionNode* frac = dynamic_cast<InfixExpressionNode*>(expr.get());
        if(!frac || !frac->Left || !frac->Right) return expr;

        int numerator = getintValue(frac->Left.get());
        int denominator = getintValue(frac->Right.get());

        // Check for division by zero
        if (denominator == 0) {
            Token undefinedTok{token::VAR, "Undefined"};
            return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
        }

        // If numerator is exactly divisible by denominator, return integer quotient of numerator and denomiator
        if (static_cast<int>(numerator) % static_cast<int>(denominator) == 0) {
            return  createNumber(numerator / denominator);
            // std::stringstream explanation;
            // explanation << numerator << "/" << denominator<< "\t" << numerator << " is divisible by " << denominator << "\n";
            // explanation << res->String();
            // recordStep("Simplifying fractions",explanation.str(), "", "");
        }

        // Calculate GCD to simplify the fraction
        int g = gcd(static_cast<int>(numerator), static_cast<int>(denominator));

        // If positive
        if (denominator > 0) {
            return createFraction(numerator / g, denominator / g);
        } else {    // if negative, (already check for denominator == 0 above)
            return createFraction(-numerator / g, -denominator / g);
        }

    }

}

// Helper Functions for RNE Simplification

// Create a fraction node
std::unique_ptr<InfixExpressionNode> SimplifyVisitor::createFraction(int numerator, int denominator) {
    Token fracTok{token::DIV, "/"};
    auto num_node = createNumber(numerator);
    auto denom_node = createNumber(denominator);

    
    return std::make_unique<InfixExpressionNode>(
        fracTok,
        '/',
        InfixKind::FRACTION, // Special kind for fractions, Defnition 2.26 , p.g. 38
        std::move(num_node),
        std::move(denom_node)
    );
}



// Check if node is an integer
bool SimplifyVisitor::isInteger(const ExpressionNode* node) {
    if (!node) return false;
    return node->getKind() == InfixKind::NUM;

}

// Get integer value from a node
int SimplifyVisitor::getintValue(const ExpressionNode* node) const {
    if (!node || node->getKind() != InfixKind::NUM) {
        return 0; // Default, should handle errors better in real code
    }   
    
    const NumberExpressionNode* num_node = dynamic_cast<const NumberExpressionNode*>(node);
    if (num_node) {
        return num_node->Value;
    }
    return 0;
}

bool SimplifyVisitor::isPlusNodeChild(const std::vector<std::unique_ptr<ExpressionNode>>&  operands) {
    for (const auto& op : operands) {
        if (op->getKind() == InfixKind::PLUS) {
            return true;
        }
    }

    return false;
}



// Get numerator of a fraction or integer
int SimplifyVisitor::getNumerator(const ExpressionNode* node) const {
    if (!node) return 0;
    
    if (node->getKind() == InfixKind::NUM) {
        return getintValue(node);
    }
    else if (node->getKind() == InfixKind::FRACTION) {
        const InfixExpressionNode* frac_node = dynamic_cast<const InfixExpressionNode*>(node);
        if (frac_node && frac_node->Left) {
            return getintValue(frac_node->Left.get());
        }
    }
    return 0;
}

// Get denominator of a fraction or integer
int SimplifyVisitor::getDenominator(const ExpressionNode* node) const {
    if (!node) return 1;
    
    if (node->getKind() == InfixKind::NUM) {
        return 1; // Denominator of an integer is 1
    }
    else if (node->getKind() == InfixKind::FRACTION) {
        const InfixExpressionNode* frac_node = dynamic_cast<const InfixExpressionNode*>(node);
        if (frac_node && frac_node->Right) {
            return getintValue(frac_node->Right.get());
        }
    }
    return 1;
}

// Calculate GCD
int SimplifyVisitor::gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::evaluate_product(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
    int num_left = getNumerator(left.get());
    int denom_left = getDenominator(left.get());

    int num_right = getNumerator(right.get());
    int denom_right = getDenominator(right.get());

    // Division by zero, denom is 0
    if (denom_left == 0 || denom_right == 0) {
        Token undefinedTok{token::VAR, "Undefined"};
        recordStep("Undefined", "Cannot divide by 0, undefined behaviour.", "", "");
        return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
    }

    // (a/b) * (c/d) = (a*c)/(b*d)
    int new_num = num_left * num_right;
    int new_denom = denom_left * denom_right;

    // std::stringstream explanation;
    // explanation << left->String() << " * " << right->String() <<"\n"; 
    // explanation << "(" << num_left << " * " << num_right << ") / (" << denom_left << " * " << denom_right << ")" << " Multiply numerators by numerators, and denominators by denominators." << "\n";
    // explanation << "Equalling " << new_num << "/" << new_denom;
    // recordStep("Multiplying Fractions", explanation.str(), "", "");

    return createFraction(new_num, new_denom);
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::evaluate_quotient(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
    int num_left = getNumerator(left.get());
    int denom_left = getDenominator(left.get());

    int num_right = getNumerator(right.get());
    int denom_right = getDenominator(right.get());

    // Division by zero, denom is 0
    if (num_right == 0) {
        Token undefinedTok{token::VAR, "Undefined"};
        return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
    }

    // (a/b) / (c/d) = (a*d)/(b*c)
    int new_num = num_left * denom_right;
    int new_denom = denom_left * num_right;

    return createFraction(new_num, new_denom);
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::evaluate_sum(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
    int num_left = getNumerator(left.get());
    int denom_left = getDenominator(left.get());

    int num_right = getNumerator(right.get());
    int denom_right = getDenominator(right.get());

    // Division by zero, denom is 0
    if (denom_left == 0 || denom_right == 0) {
        Token undefinedTok{token::VAR, "Undefined"};
        return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
    }

    // (a/b) + (c/d) = (a*d + b*c)/(b*d) (cross multiplication)
    int new_num = (num_left * denom_right) + (denom_left * num_right);
    int new_denom = denom_left * denom_right;

    return createFraction(new_num, new_denom);
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::evaluate_difference(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
    int num_left = getNumerator(left.get());
    int denom_left = getDenominator(left.get());

    int num_right = getNumerator(right.get());
    int denom_right = getDenominator(right.get());

    // Division by zero, denom is 0
    if (denom_left == 0 || denom_right == 0) {
        Token undefinedTok{token::VAR, "Undefined"};
        return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
    }

    // (a/b) - (c/d) = (a*d - b*c)/(b*d) (cross multiplication)
    int new_num = (num_left * denom_right) - (denom_left * num_right);
    int new_denom = denom_left * denom_right;

    return createFraction(new_num, new_denom);
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::simplify_rne(std::unique_ptr<ExpressionNode> expr) {
    auto result = simplify_rne_rec(std::move(expr));

    if (isUndefined(result.get())) {
        return result;
    } else {
        return simplify_rational_number(std::move(result));
    }
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::simplify_rne_rec(std::unique_ptr<ExpressionNode> expr) {
    if (!expr) {
        Token undefinedTok{token::VAR, "Undefined"};
        return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
    }


    // Handle different node types
    if ( expr->getKind() == InfixKind::NUM) return expr;

    if ( expr->getKind() == InfixKind::VAR) return expr;
        
    
    if (expr->getKind() == InfixKind::FRACTION) {
        // Check for division by zero, denomiator is zero
        const InfixExpressionNode* frac = dynamic_cast<InfixExpressionNode*>(expr.get());
        if (frac && isZero(frac->Right.get())) {
            Token undefinedTok{token::VAR, "Undefined"};
            return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
        }
        return expr;
    }

    // Unary expressions, +RNE, -RNE
    //TODO: do this when implementing sum and difference

    // Binary expressions
    if (expr->getKind() == InfixKind::MULTIPLY || expr->getKind() == InfixKind::PLUS) {
        const auto u = dynamic_cast<NaryExpressionNode*>(expr.get());

        if (u->Operands.size() != 2) return nullptr;

        
        auto v = simplify_rne_rec(std::move(u->Operands[0]));
        auto w = simplify_rne_rec(std::move(u->Operands[1]));

        if (isUndefined(v.get()) || isUndefined(w.get())) {
            Token undefinedTok{token::VAR, "Undefined"};
            return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
        } else {
            switch (expr->getKind()) {
                case InfixKind::MULTIPLY: return evaluate_product(std::move(v), std::move(w));
                case InfixKind::PLUS: return evaluate_sum(std::move(v), std::move(w));
            }
        }

    } 
    else if (expr->getKind() == InfixKind::DIFFERENCE || expr->getKind() == InfixKind::DIVIDE) {
        const auto u = dynamic_cast<InfixExpressionNode*>(expr.get());

        
        auto v = simplify_rne_rec(std::move(u->Left));
        auto w = simplify_rne_rec(std::move(u->Right));

        if (isUndefined(v.get()) || isUndefined(w.get())) {
            Token undefinedTok{token::VAR, "Undefined"};
            return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
        } else {
            switch (expr->getKind()) {
                case InfixKind::DIVIDE: return evaluate_quotient(std::move(v), std::move(w));
                case InfixKind::DIFFERENCE: return evaluate_difference(std::move(v), std::move(w));
            }
        }

    }



}

//==============================================================

void SimplifyVisitor::visit(NaryExpressionNode& node) {

    // Create a vector for simplified operands
    std::vector<std::unique_ptr<ExpressionNode>> simplified_operands;
    // std::string before = node.String();
    
        // root_steps.push_back(node.String());
    // Simplify each operand
    for (auto& operand : node.Operands) {
        simplified_operands.push_back(automatic_simplify(std::move(operand), *this));
    }

    
    // Based on the kind of operation, simplify accordingly
    if (node.Kind == InfixKind::MULTIPLY) {
        auto product = std::make_unique<NaryExpressionNode>(
            node.Tok, 
            node.Operator, 
            node.Kind, 
            std::move(simplified_operands) 
        );
        // if (node.is_root) {
        //     std::stringstream explanation;
        //     explanation << "That simplifies " << before << " to " << product->String();
        //     recordStep("",  explanation.str(), before, product->String());
        //     before = product->String();
        //     // root_steps.push_back(product->String());
        // }

        result = simplify_product(std::move(product));
        

    } else if (node.Kind == InfixKind::PLUS) {
        auto sum = std::make_unique<NaryExpressionNode>(
            node.Tok,
            node.Operator, 
            node.Kind, 
            std::move(simplified_operands)
        );
        result = simplify_sum(std::move(sum));
    }
    // if (node.is_root) {
    //     std::stringstream explanation;
    //     explanation << "Now fully simplified to: " << result->String();
    //     recordStep("",  explanation.str(), "", result->String());
    //     // root_steps.push_back(result->String());
    // }
}

// Helper method implementations
std::unique_ptr<ExpressionNode> SimplifyVisitor::automatic_simplify(std::unique_ptr<ExpressionNode> expr, SimplifyVisitor& visitor) {
    // Apply the visitor to the expression
    // SimplifyVisitor visitor;
    expr->accept(visitor);
    
    return visitor.getResult();
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::simplify_sum(std::unique_ptr<NaryExpressionNode> sum) {
    std::string originalExpr = sum->String();

    // std::cout << printExpression() << std::endl;
    // Get the operands
    std::vector<std::unique_ptr<ExpressionNode>> operands;
    for (auto& op : sum->Operands) {
        operands.push_back(std::move(op));
    }

    // SSUM-1: If any operand is Undefined, return Undefined
    for (const auto& op : operands) {
        if (isUndefined(op.get())) {
            Token undefinedTok{token::VAR, "Undefined"};
            recordStep("Undefined", "Undefined Behaviour.", originalExpr, "Undefined");
            return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
        }
    }

    // SSUM-2 : Unary sum simplifies to its operand.    sum = [u1] -> u1, +u -> u
    if (operands.size() == 1) {
        return std::move(operands[0]);
    }

    //SSUM-3 : If first two rules do not apply
    auto simplified_operands = simplify_sum_rec(operands);

    // Handle the three cases from SSUM-4
    if (simplified_operands.empty()) {
        // SSUM-4-3: If empty, return 0
        return createNumber(0);
    } else if (simplified_operands.size() == 1) {
        // SSUM-4-1: If one operand, return it
        return std::move(simplified_operands[0]);
    } else {
        // SSUM-4-2: If multiple operands, construct sum, usually the case will be a symbol + RNE|NUM
        return std::make_unique<NaryExpressionNode>(
            sum->Tok, 
            sum->Operator, 
            sum->Kind, 
            std::move(simplified_operands)
        );
    }
}

std::vector<std::unique_ptr<ExpressionNode>> SimplifyVisitor::simplify_sum_rec(std::vector<std::unique_ptr<ExpressionNode>>& operands) {
    //SSUMREC-1 : Two operands, and neither is a sum
    if (operands.size() == 2 && !isSum(operands[0].get()) && !isSum(operands[1].get())) {
        auto& u1 = operands[0];
        auto& u2 = operands[1];

        // Recording original xepressions for step tracking
        std::string u1_str = u1->String();
        std::string u2_str = u2->String();

        if ((u1->getKind() == InfixKind::NUM || u1->getKind() == InfixKind::FRACTION) && (u2->getKind() == InfixKind::NUM || u2->getKind() == InfixKind::FRACTION)) {
            std::vector<std::unique_ptr<ExpressionNode>> operands;
            operands.push_back(std::move(u1));
            operands.push_back(std::move(u2));
            auto sum = simplify_rne(createSum(std::move(operands)));
            std::stringstream before;
            before << u1_str + " + " + u2_str;
            recordStep("Combine Like Terms", "", before.str(), sum->String());

            // If sum = 0, then, (sum identity u + 0 = u)
            if (sum->getKind() == InfixKind::NUM) {
                auto P = dynamic_cast<NumberExpressionNode*>(sum.get());

                if ( P->Value == 0) {
                    // std::cout << P->Value << std::endl;
                    return {};
                } 
                // and if sum != 0, then
                else {
                    std::vector<std::unique_ptr<ExpressionNode>> result;
                    result.push_back(createNumber(P->Value));
                    return result;
                }
            
            } else if (sum->getKind() == InfixKind::FRACTION) {
                std::vector<std::unique_ptr<ExpressionNode>> result;
                result.push_back(std::move(sum));
                return result;
            }

        }


        // SSUMREC-1-2: Handle identity element 1
        if (isZero(u1.get())) {
            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(u2));
            return result;
        }
        if (isZero(u2.get())) {
            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(u1));
            return result;
        }
        
        // SSUMREC-1-3: Handle like terms
        // x + x -> 2*x
        // 2*x + x -> 3*x, and vice versa
        // 2*x + 2*x -> 4*x
        if ((u1->getKind() == InfixKind::VAR && u2->getKind() == InfixKind::VAR) || 
            (u1->getKind() == InfixKind::MULTIPLY && u2->getKind() == InfixKind::VAR) ||
            (u1->getKind() == InfixKind::VAR && u2->getKind() == InfixKind::MULTIPLY) ||
            (u1->getKind() == InfixKind::MULTIPLY && u2->getKind() == InfixKind::MULTIPLY)) {

            bool combine = true;
            bool u1_has_plus = false;
            bool u2_has_plus = false;
            
            
            // if ((u1->getKind() == InfixKind::MULTIPLY && u2->getKind() == InfixKind::MULTIPLY)) {
            //     const auto& u1_cast = dynamic_cast<const NaryExpressionNode*>(operands[0].get());
            //     const auto& u2_cast = dynamic_cast<const NaryExpressionNode*>(operands[1].get());

            //     combine = !(isPlusNodeChild(u1_cast->Operands) || isPlusNodeChild(u2_cast->Operands));   // if either has plus then false, if neither then true
            // }

            if (u1->getKind() == InfixKind::MULTIPLY) {
                const auto& u1_cast = dynamic_cast<const NaryExpressionNode*>(operands[0].get());
                u1_has_plus = isPlusNodeChild(u1_cast->Operands);
            }
            if ( u2->getKind() == InfixKind::MULTIPLY) {
                const auto& u2_cast = dynamic_cast<const NaryExpressionNode*>(operands[1].get());
                u2_has_plus = isPlusNodeChild(u2_cast->Operands);
            }

            combine = !(u1_has_plus || u2_has_plus); // if either has plus then false, if neither then true
            
            if ( combine ) {
                // Extract the base and coefficient of each term
                std::unique_ptr<ExpressionNode> base1 = nullptr;
                std::unique_ptr<ExpressionNode> coef1 = nullptr;
                extractBaseAndCoefficient(u1.get(), base1, coef1);

                std::unique_ptr<ExpressionNode> base2 = nullptr;
                std::unique_ptr<ExpressionNode> coef2 = nullptr;
                extractBaseAndCoefficient(u2.get(), base2, coef2);



                // If the bases are the same, combine the coefficients
                if (base1 && base2 && (base1->String() == base2->String())) {
                    // Create a sum of the coefficients
                    std::vector<std::unique_ptr<ExpressionNode>> sumOperands;
                    
                    // If coefficients exist, use them, otherwise use 1
                    if (coef1) {
                        sumOperands.push_back(std::move(coef1));
                    } else {
                        sumOperands.push_back(createNumber(1));
                    }
                    
                    if (coef2) {
                        sumOperands.push_back(std::move(coef2));
                    } else {
                        sumOperands.push_back(createNumber(1));
                    }
                    
                    // Simplify the sum of coefficients
                    auto simplifiedSum = simplify_sum(createSum(std::move(sumOperands)));
                    
                    // // Check if the sum of coefficients is zero
                    // if (isZero(simplifiedSum.get())) {
                    //     return {}; // Return empty vector (zero)
                    // }
                    // 
                    // Create the product of the coefficient and the base
                    std::vector<std::unique_ptr<ExpressionNode>> productOperands;
                    productOperands.push_back(std::move(simplifiedSum));
                    productOperands.push_back(std::move(base1));
                    
                    std::vector<std::unique_ptr<ExpressionNode>> result;
                    auto prod = simplify_product(createProduct(std::move(productOperands)));
                    std::string prod_str = prod->String();
                    result.push_back(std::move(prod));
                    if (!simplifying_coefficient) {     // avoids recording steps for the coefficients adding calculations
                        std::stringstream before;
                        before << u1_str + " + " + u2_str;
                        std::stringstream explanation;
                        explanation << before.str() << "\t" << "Add " << u1_str << " and " << u2_str << " to get " << prod_str; 
                        recordStep("Combine Like Terms", explanation.str(), before.str(), prod_str);
                    }

                    return result;
                }
                
                
                return std::move(operands);
                
            }
        }
        
        
        // SSUMREC-1-4: Check ordering
        if (operandLessThan(u2.get(), u1.get())) {
            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(u2));
            result.push_back(std::move(u1));
            return result;
        }
        
        // SSUMREC-1-5: Return as is
        std::vector<std::unique_ptr<ExpressionNode>> result;
        result.push_back(std::move(u1));
        result.push_back(std::move(u2));
        return result;
    }

    // SSUMREC-2: Two operands, at least one is a sum
    if (operands.size() == 2) {
        auto& u1 = operands[0];
        auto& u2 = operands[1];
        
        if (isSum(u1.get()) && isSum(u2.get())) {
            // SSUMREC-2-1: Both are sums
            auto* p1 = dynamic_cast<NaryExpressionNode*>(u1.get());
            auto* p2 = dynamic_cast<NaryExpressionNode*>(u2.get());
            
            auto p1_operands = getNaryOperands(p1);
            auto p2_operands = getNaryOperands(p2);
            
            return merge_sums(p1_operands, p2_operands);
        } else if (isSum(u1.get())) {
            // SSUMREC-2-2: First is a sum
            auto* p1 = dynamic_cast<NaryExpressionNode*>(u1.get());
            
            auto p1_operands = getNaryOperands(p1);
            std::vector<std::unique_ptr<ExpressionNode>> u2_vec;
            u2_vec.push_back(std::move(u2));
            
            return merge_sums(p1_operands, u2_vec);
        } else if (isSum(u2.get())) {
            // SSUMREC-2-3: Second is a sum
            auto* p2 = dynamic_cast<NaryExpressionNode*>(u2.get());
            
            std::vector<std::unique_ptr<ExpressionNode>> u1_vec;
            u1_vec.push_back(std::move(u1));
            auto p2_operands = getNaryOperands(p2);
            
            return merge_sums(u1_vec, p2_operands);
        }
    }

    // SSUMREC-3: More than two operands
    auto& u1 = operands[0];
    
    // Create a new vector with all but the first operand
    std::vector<std::unique_ptr<ExpressionNode>> rest;
    for (size_t i = 1; i < operands.size(); i++) {
        rest.push_back(std::move(operands[i]));
    }
    
    // Apply simplify_sum_rec to rest
    auto w = simplify_sum_rec(rest);
    
    if (isSum(u1.get())) {
        // SSUMREC-3-1: First is a sum
        auto* p1 = dynamic_cast<NaryExpressionNode*>(u1.get());
        auto p1_operands = getNaryOperands(p1);
        
        return merge_sums(p1_operands, w);
    } else {
        // SSUMREC-3-2: First is not a sum
        std::vector<std::unique_ptr<ExpressionNode>> u1_vec;
        u1_vec.push_back(std::move(u1));
        
        return merge_sums(u1_vec, w);
    }

}

std::vector<std::unique_ptr<ExpressionNode>> SimplifyVisitor::merge_sums( std::vector<std::unique_ptr<ExpressionNode>>& p, std::vector<std::unique_ptr<ExpressionNode>>& q) {
    // MSUM-1: q is empty
    if (q.empty()) {
        return std::move(p);
    }
    
    // MSUM-2: p is empty
    if (p.empty()) {
        return std::move(q);
    }
    
    // MSUM-3: Both non-empty
    auto& p1 = p[0];
    auto& q1 = q[0];
    
    // Create operands for simplify_sum_rec
    std::vector<std::unique_ptr<ExpressionNode>> pair;
    
    // We need to clone p1 and q1 to avoid losing them because of std::moveING unique pointers
    // For p1
    //! Add more cloning cases as you implement functionality for more nodes
    std::unique_ptr<ExpressionNode> p1_clone;
    p1_clone = clone_expr(p1);
    if (!p1_clone) {
        std::cerr << "p1_clone didn't clone" << std::endl;
        return {};
    }
    
    // For q1
    //! Add more cloning cases as you implement functionality for more nodes
    std::unique_ptr<ExpressionNode> q1_clone;
    q1_clone = clone_expr(q1);
    if (!q1_clone) {
        std::cerr << "q1_clone didnt' clone" << std::endl;
        return {};
    }

    pair.push_back(std::move(p1_clone));
    pair.push_back(std::move(q1_clone));

    // check if its in the correct order
    bool isOrderedBefore = operandLessThan(pair[0].get(), pair[1].get());

    // Apply simplify_sum_rec
    auto h = simplify_sum_rec(pair);

    bool isOrderedAfter;
    
    if (h.size() == 2) {
        isOrderedAfter = operandLessThan(h[0].get(), h[1].get()); // should always be true
    }
    
    if (h.empty()) {
        // MSUM-3-1: Result is empty (p1_clone+q1_clone = 0)
        auto p_rest = rest(p);
        auto q_rest = rest(q);
        return merge_sums(p_rest, q_rest);
    } else if (h.size() == 1) {
        // MSUM-3-2: Result is a single operand
        auto p_rest = rest(p);
        auto q_rest = rest(q);
        auto result = merge_sums(p_rest, q_rest);
        result.insert(result.begin(), std::move(h[0]));
        return result;
    } else if (h.size() == 2 && (isOrderedBefore == isOrderedAfter)) {
        // MSUM-3-3: Result is [p1, q1] (original order)
        auto p_rest = rest(p);
        auto result = merge_sums(p_rest, q);
        result.insert(result.begin(), std::move(p1));
        p.erase(p.begin());  // Remove p1
        return result;
    } else if (h.size() == 2) {
        // MSUM-3-4: Result is [q1, p1] (reversed order)
        auto q_rest = rest(q);
        auto result = merge_sums(p, q_rest);
        result.insert(result.begin(), std::move(q1));
        q.erase(q.begin());  // Remove q1
        return result;
    }
    
    // This should not happen based on the algorithm
    return {};

}



std::unique_ptr<ExpressionNode> SimplifyVisitor::simplify_product(std::unique_ptr<NaryExpressionNode> product) {

    std::string originalExpr = product->String();

    // Get the operands
    // std::cout << printExpression() << std::endl;
    std::vector<std::unique_ptr<ExpressionNode>> operands;
    for (auto& op : product->Operands) {
        operands.push_back(std::move(op));
        
    }
    
    // SPRD-1: If any operand is Undefined, return Undefined
    for (const auto& op : operands) {
        if (isUndefined(op.get())) {
            // Create an Undefined node (using a variable node as a placeholder)
            Token undefinedTok{token::VAR, "Undefined"};
            recordStep("Undefined", "Undefined Behaviour.", originalExpr, "Undefined");
            return std::make_unique<VariableExpressionNode>("Undefined", undefinedTok, InfixKind::VAR);
        }
    }

    // SPRD-2: If any operarnd is 0, return 0
    for (const auto& op : operands) {
        if (op && isZero(op.get())) {
            auto res =  createNumber(0);

            recordStep("", "Multiplying by 0 is 0.", originalExpr, res->String());

            return res;
        }
    }
    
    // SPRD-3: If only one operand, return it
    if (operands.size() == 1) {
        return std::move(operands[0]);
    }
        
    // SPRD-4: Apply simplify_product_rec
    auto simplified_operands = simplify_product_rec(operands);
    
    // Handle the three cases from SPRD-4
    if (simplified_operands.empty()) {
        // SPRD-4-3: If empty, return 1
        return createNumber(1);
    } else if (simplified_operands.size() == 1) {
        // SPRD-4-1: If one operand, return it
        return std::move(simplified_operands[0]);
    } else {
        // SPRD-4-2: If multiple operands, construct product
        return std::make_unique<NaryExpressionNode>(
            product->Tok, 
            product->Operator, 
            product->Kind, 
            std::move(simplified_operands)
        );
    }
}

std::vector<std::unique_ptr<ExpressionNode>> SimplifyVisitor::simplify_product_rec(
    std::vector<std::unique_ptr<ExpressionNode>>& operands) {
    
    
    // SPRDREC-1: Two operands, neither is a product
    if (operands.size() == 2 && !isProduct(operands[0].get()) && !isProduct(operands[1].get())) {
        auto& u1 = operands[0];
        auto& u2 = operands[1];
        
        // Recording original xepressions for step tracking
        std::string u1_str = u1->String();
        std::string u2_str = u2->String();

        
        // then check if they are either num or fractions
        // if ((u1->getKind() == InfixKind::NUM || u1_is_fraction) && (u2->getKind() == InfixKind::NUM || u2_is_fraction)) {
        if ((u1->getKind() == InfixKind::NUM || u1->getKind() == InfixKind::FRACTION) && (u2->getKind() == InfixKind::NUM || u2->getKind() == InfixKind::FRACTION)) {
            // Combining u1 and u2: u1 * u2 
            // to simplify them both 
            std::vector<std::unique_ptr<ExpressionNode>> operands;
            operands.push_back(std::move(u1));
            operands.push_back(std::move(u2));
            auto before = createProduct(std::move(operands));
            std::string before_string = before->String();
            auto product = simplify_rne(std::move(before));

            // If product = 1, then
            if (product->getKind() == InfixKind::NUM) {
                auto P = dynamic_cast<NumberExpressionNode*>(product.get());

                if ( P->Value == 1) {
                    return {};
                } 
                
                // and if product != 1, then
                else {
                    std::vector<std::unique_ptr<ExpressionNode>> result;
                    result.push_back(createNumber(P->Value));
                    return result;
                }
                // fraction can be anything we don't really care, 1/1 will evaluate to a NumberNode equalling 1 so it will trigger the if statement above
            } else if (product->getKind() == InfixKind::FRACTION) {
                std::vector<std::unique_ptr<ExpressionNode>> result;
                result.push_back(std::move(product));
                return result;
            }

        }
        
        // SPRDREC-1-2: Handle identity element 1
        if (isOne(u1.get())) {
            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(u2));
            return result;
        }
        if (isOne(u2.get())) {
            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(u1));
            return result;
        }
        
        // SPRDREC-1-3: Handle powers with same base
        // (Powers not implemented yet)
        // : Handle expanding (distribution)
        // RNE|NUM * (RNE|NUM + x)
        if ((u1->getKind() == InfixKind::NUM || u1->getKind() == InfixKind::FRACTION) && // u1 NUM|RNE
            (u2->getKind() == InfixKind::PLUS)) {
            auto* u2_cast = dynamic_cast<NaryExpressionNode*>(u2.get());  // need this for printing in step solutions
            std::string u2_0_operand = u2_cast->Operands[0]->String();  // need this for printing in step solutions
            std::string u2_1_operand = u2_cast->Operands[1]->String();// need this for printing in step solutions

            // need to combine u1 and u2: u1 * u2
            // so i can expand them using function
            std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
            prod_operands.push_back(std::move(u1));
            prod_operands.push_back(std::move(u2));
            
            auto prod = createProduct(std::move(prod_operands));
            std::string before = prod->String();    
            auto expanded_expr = expand_tree(std::move(prod));

             // Record transformation
            
            std::stringstream explanation;
            explanation << "(" << u1_str << ")(" << u2_0_operand << ") + (" << u1_str << ")(" << u2_1_operand << ")" << "\t"; // e.g. 2(x+2) -> (2)(x) + (2)(2)
            explanation << "Multiply " << u1_str << " by each term in " << u2_str;
            recordStep("Distribution", explanation.str(), before, expanded_expr->String());

            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(expanded_expr));
            return result;
            }


        // SPRDREC-1-4: Check ordering
        if (operandLessThan(u2.get(), u1.get())) {
            std::vector<std::unique_ptr<ExpressionNode>> result;
            result.push_back(std::move(u2));
            result.push_back(std::move(u1));
            return result;
        }
        
        // SPRDREC-1-5: Return as is
        std::vector<std::unique_ptr<ExpressionNode>> result;
        result.push_back(std::move(u1));
        result.push_back(std::move(u2));
        return result;
    }
    
    // SPRDREC-2: Two operands, at least one is a product
    if (operands.size() == 2) {
        auto& u1 = operands[0];
        auto& u2 = operands[1];
        
        if (isProduct(u1.get()) && isProduct(u2.get())) {
            // SPRDREC-2-1: Both are products
            auto* p1 = dynamic_cast<NaryExpressionNode*>(u1.get());
            auto* p2 = dynamic_cast<NaryExpressionNode*>(u2.get());
            
            auto p1_operands = getNaryOperands(p1);
            auto p2_operands = getNaryOperands(p2);
            
            return merge_products(p1_operands, p2_operands);
        } else if (isProduct(u1.get())) {
            // SPRDREC-2-2: First is a product
            auto* p1 = dynamic_cast<NaryExpressionNode*>(u1.get());
            
            auto p1_operands = getNaryOperands(p1);
            std::vector<std::unique_ptr<ExpressionNode>> u2_vec;
            u2_vec.push_back(std::move(u2));
            
            return merge_products(p1_operands, u2_vec);
        } else if (isProduct(u2.get())) {
            // SPRDREC-2-3: Second is a product
            auto* p2 = dynamic_cast<NaryExpressionNode*>(u2.get());
            
            std::vector<std::unique_ptr<ExpressionNode>> u1_vec;
            u1_vec.push_back(std::move(u1));
            auto p2_operands = getNaryOperands(p2);
            
            return merge_products(u1_vec, p2_operands);
        }
    }
    
    // SPRDREC-3: More than two operands
    auto& u1 = operands[0];
    
    // Create a new vector with all but the first operand
    std::vector<std::unique_ptr<ExpressionNode>> rest;
    for (size_t i = 1; i < operands.size(); i++) {
        rest.push_back(std::move(operands[i]));
    }
    
    // Apply simplify_product_rec to rest
    auto w = simplify_product_rec(rest);
    
    if (isProduct(u1.get())) {
        // SPRDREC-3-1: First is a product
        auto* p1 = dynamic_cast<NaryExpressionNode*>(u1.get());
        auto p1_operands = getNaryOperands(p1);
        
        return merge_products(p1_operands, w);
    } else {
        // SPRDREC-3-2: First is not a product
        std::vector<std::unique_ptr<ExpressionNode>> u1_vec;
        u1_vec.push_back(std::move(u1));
        
        return merge_products(u1_vec, w);
    }
}

std::vector<std::unique_ptr<ExpressionNode>> SimplifyVisitor::merge_products(
    std::vector<std::unique_ptr<ExpressionNode>>& p, 
    std::vector<std::unique_ptr<ExpressionNode>>& q) {
    
    // MPRD-1: q is empty
    if (q.empty()) {
        return std::move(p);
    }
    
    // MPRD-2: p is empty
    if (p.empty()) {
        return std::move(q);
    }
    
    // MPRD-3: Both non-empty
    auto& p1 = p[0];
    auto& q1 = q[0];
    
    // Create operands for simplify_product_rec
    std::vector<std::unique_ptr<ExpressionNode>> pair;
    
    // We need to clone p1 and q1 to avoid losing them
    // For p1
    //! Add more cloning cases as you implement functionality for more nodes
    std::unique_ptr<ExpressionNode> p1_clone;
    p1_clone = clone_expr(p1);
    if (!p1_clone) {
        std::cerr << "p1_clone didn't clone" << std::endl;
        return {};
    }
    
    
    // For q1
    //! Add more cloning cases as you implement functionality for more nodes
    std::unique_ptr<ExpressionNode> q1_clone;
    q1_clone = clone_expr(q1);
    if (!q1_clone) {
        std::cerr << "q1_clone didnt' clone" << std::endl;
        return {};
    }

    pair.push_back(std::move(p1_clone));
    pair.push_back(std::move(q1_clone));


    // check if its in the correct order
    bool isOrderedBefore = operandLessThan(pair[0].get(), pair[1].get());

    // Apply simplify_product_rec
    auto h = simplify_product_rec(pair);


    bool isOrderedAfter;
    
    if (h.size() == 2) {
        isOrderedAfter = operandLessThan(h[0].get(), h[1].get()); // should always be true
    }
    
    if (h.empty()) {
        // MPRD-3-1: Result is empty (p1*q1 = 1)
        auto p_rest = rest(p);
        auto q_rest = rest(q);
        return merge_products(p_rest, q_rest);
    } else if (h.size() == 1) {
        // MPRD-3-2: Result is a single operand
        auto p_rest = rest(p);
        auto q_rest = rest(q);
        auto result = merge_products(p_rest, q_rest);
        result.insert(result.begin(), std::move(h[0]));
        return result;
    } else if (h.size() == 2 && (isOrderedBefore == isOrderedAfter)) {
        // MPRD-3-3: Result is [p1, q1] (original order)
        auto p_rest = rest(p);
        auto result = merge_products(p_rest, q);
        result.insert(result.begin(), std::move(p1));
        p.erase(p.begin());  // Remove p1
        return result;
    } else if (h.size() == 2) {
        // MPRD-3-4: Result is [q1, p1] (reversed order)
        auto q_rest = rest(q);
        auto result = merge_products(p, q_rest);
        result.insert(result.begin(), std::move(q1));
        q.erase(q.begin());  // Remove q1
        return result;
    }
    
    // This should not happen based on the algorithm
    return {};
}

std::vector<std::unique_ptr<ExpressionNode>> SimplifyVisitor::rest(std::vector<std::unique_ptr<ExpressionNode>>& a) {
    std::vector<std::unique_ptr<ExpressionNode>> rest;
    for (size_t i = 1; i < a.size(); i++) {
        rest.push_back(std::move(a[i]));
    }

    return rest;
}

// Helper method implementations
bool SimplifyVisitor::isUndefined(const ExpressionNode* node) const {

    if (node->getKind() == InfixKind::VAR) {
        auto* var = dynamic_cast<const VariableExpressionNode*>(node);
        return var && var->Value == "Undefined";
    }
    return false;
}

bool SimplifyVisitor::isZero(const ExpressionNode* node) const {
    if (node->getKind() == InfixKind::NUM) {
        auto* num = dynamic_cast<const NumberExpressionNode*>(node);
        return num && num->Value == 0;
    }
    return false;
}

bool SimplifyVisitor::isOne(const ExpressionNode* node) const {
    if (node->getKind() == InfixKind::NUM) {
        auto* num = dynamic_cast<const NumberExpressionNode*>(node);
        return num && num->Value == 1;
    }
    return false;
}

std::unique_ptr<NaryExpressionNode> SimplifyVisitor::createProduct(std::vector<std::unique_ptr<ExpressionNode>> operands) const {
    Token prodToken{token::MULT, "*"};

    return std::make_unique<NaryExpressionNode>(
        prodToken,
        '*',
        InfixKind::MULTIPLY,
        std::move(operands)
    );
}

std::unique_ptr<InfixExpressionNode> SimplifyVisitor::createQuotient(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
    Token quotToken{token::DIV, "/"};

    return std::make_unique<InfixExpressionNode>(
        quotToken,
        '/',
        InfixKind::DIVIDE,
        std::move(left),
        std::move(right)
    );
}

std::unique_ptr<NaryExpressionNode> SimplifyVisitor::createSum(std::vector<std::unique_ptr<ExpressionNode>> operands) const {
    Token sumToken{token::PLUS, "+"};

    return std::make_unique<NaryExpressionNode>(
        sumToken,
        '+',
        InfixKind::PLUS,
        std::move(operands)
    );
}

std::unique_ptr<InfixExpressionNode> SimplifyVisitor::createDifference(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
    Token diffToken{token::MINUS, "-"};

    return std::make_unique<InfixExpressionNode>(
        diffToken,
        '/',
        InfixKind::DIFFERENCE,
        std::move(left),
        std::move(right)
    );
}

std::unique_ptr<PrefixExpressionNode> SimplifyVisitor::createPrefix(std::unique_ptr<ExpressionNode> right) {
    Token preMinusToken{token::PRE_MINUS, "-"};

    return std::make_unique<PrefixExpressionNode>(
        '-',
        preMinusToken,
        InfixKind::PRE_MINUS,
        std::move(right)
    );
}


std::unique_ptr<NumberExpressionNode> SimplifyVisitor::createNumber(int value) const{
    Token tok{token::INT, std::to_string(value)};
    return std::make_unique<NumberExpressionNode>(tok, value, InfixKind::NUM);
}

std::unique_ptr<VariableExpressionNode> SimplifyVisitor::createVariable(const std::string& name) const {
    Token tok{token::VAR, name};
    
    return std::make_unique<VariableExpressionNode>(name, tok, InfixKind::VAR);
}

bool SimplifyVisitor::isProduct(const ExpressionNode* node) const {
    if (!node) return false;
    return dynamic_cast<const NaryExpressionNode*>(node) && node->getKind() == InfixKind::MULTIPLY;
}

bool SimplifyVisitor::isSum(const ExpressionNode* node) const {
    if (!node) return false;
    return dynamic_cast<const NaryExpressionNode*>(node) && node->getKind() == InfixKind::PLUS;
}

std::vector<std::unique_ptr<ExpressionNode>> SimplifyVisitor::getNaryOperands(NaryExpressionNode* expr) {
    if (!expr || (expr->getKind() != InfixKind::MULTIPLY && expr->getKind() != InfixKind::PLUS)) {
        return {};
    }
    
    std::vector<std::unique_ptr<ExpressionNode>> result;
    for (auto& op : expr->Operands) {
        result.push_back(std::move(op));
    }
    return result;
}

bool SimplifyVisitor::operandLessThan(const ExpressionNode* u, const ExpressionNode* v) const {
    // Based on the ◁ operator described in the book

    // O-1 : when both u and v are constants (integers or fractions), then order in ascending order, return u < v
    if ((u->getKind() == InfixKind::NUM || u->getKind() == InfixKind::FRACTION) && 
        (v->getKind() == InfixKind::NUM || v->getKind() == InfixKind::FRACTION)) {

        int u_value, v_value;

        // Extract numeric value for u
        if (u->getKind() == InfixKind::NUM) {
            const auto* u_cast = dynamic_cast<const NumberExpressionNode*>(u);
            if (!u_cast) return false; 
            u_value = u_cast->Value;
        } else { 
            u_value = getNumerator(u) / getDenominator(u);
        }
    
        // Extract numeric value for v
        if (v->getKind() == InfixKind::NUM) {
            const auto* v_cast = dynamic_cast<const NumberExpressionNode*>(v);
            if (!v_cast) return false; 
            v_value = v_cast->Value;
        } else { 
            v_value = getNumerator(v) / getDenominator(v);
        }
    
        return u_value < v_value;
    }

    // 0-2 : when both u and v are symbols, then use lexicographical order. 0, 1,..., 9, A, B, . . . , Z, a, b, . . . , z
    if (u->getKind() == InfixKind::VAR && v->getKind() == InfixKind::VAR) {
        return u->String() < v->String();
    }

    // O-3 : when u and v are either both products or sums with operands, u1, u2, ..., um and v1, v1, ..., vn
    if ((u->getKind() == InfixKind::MULTIPLY && v->getKind() == InfixKind::MULTIPLY) || (u->getKind() == InfixKind::PLUS && v->getKind() == InfixKind::PLUS)) {
        const auto* u_cast = dynamic_cast<const NaryExpressionNode*>(u);
        const auto* v_cast = dynamic_cast<const NaryExpressionNode*>(v);
        int m  = u_cast->Operands.size();
        int n = v_cast->Operands.size();
        int min_size = std::min(m, n);
        
        // O-3-2 : if the last operands of u and v are not equal then move left     e.g. a + c + d < b + c + d
        for (int j = 0; j < min_size; j++) {
            if (u_cast->Operands[m - 1 - j]->String() != v_cast->Operands[n - 1 - j]->String()) {
                // O-3-1 : if the last operands of u and v are not equal then recurse the last operands, which will determine the order.    e.g. a + b < a + c
                return operandLessThan(u_cast->Operands[m - 1 - j].get(), v_cast->Operands[n - 1 - j].get());
            }
        }

        // O-3-3 : if all operands are equal, then compare length       e.g. c + d < b + c + d
        return m < n;
    }

    // O-7 : when u is an integer or fraction and v is of any other type, then u always comes before v.
    if ((u->getKind() == InfixKind::NUM || u->getKind() == InfixKind::FRACTION) && !(v->getKind() == InfixKind::NUM || v->getKind() == InfixKind::FRACTION)) {
        return true;
    }
    // O-8 : when u is a product and v is a sum or symbol // *new
    if ((u->getKind() == InfixKind::MULTIPLY && (v->getKind() == InfixKind::PLUS || v->getKind() == InfixKind::VAR))) {
        std::vector<std::unique_ptr<ExpressionNode>> operands;
        if (v->getKind() == InfixKind::PLUS) {
            auto& sum_ops_ref = const_cast<NaryExpressionNode*>(dynamic_cast<const NaryExpressionNode*>(v))->Operands;
            auto sum = createSum(std::move(sum_ops_ref));
            operands.push_back(std::move(sum));
        } else {
            operands.push_back(createVariable(v->TokenLiteral()));
        
        const auto v_prod = createProduct(std::move(operands));

        return operandLessThan(u, v_prod.get());
        }
    }
    // O-10 : when u is a sum, and v is a symbol
    if ((u->getKind() == InfixKind::PLUS) && (v->getKind() == InfixKind::VAR)) {
        std::vector<std::unique_ptr<ExpressionNode>> operands;
        operands.push_back(createVariable(v->TokenLiteral()));
        auto v_sum = createSum(std::move(operands));

        return operandLessThan(u, v_sum.get());
    }

    // O-13 : if none of the rules above are satisfied, then we simply flip u and v
    return !operandLessThan(v, u);
}


std::unique_ptr<ExpressionNode> SimplifyVisitor::clone_expr(const std::unique_ptr<ExpressionNode>& expr) const {

   
    switch (expr->getKind()) {
        case InfixKind::NUM: {
            auto* num = dynamic_cast<NumberExpressionNode*>(expr.get());
            if (num) {
                return std::make_unique<NumberExpressionNode>(num->Tok, num->Value, num->Kind);
            }
            break;
        }

        case InfixKind::VAR: {
            auto* var = dynamic_cast<VariableExpressionNode*>(expr.get());
            if (var) {   
                return std::make_unique<VariableExpressionNode>(var->Value, var->Tok, var->Kind);
            }
            break;
        }

        case InfixKind::FRACTION: {
            auto* frac = dynamic_cast<InfixExpressionNode*>(expr.get());
            auto* frac_numerator = dynamic_cast<NumberExpressionNode*>(frac->Left.get());
            auto* frac_denominator = dynamic_cast<NumberExpressionNode*>(frac->Right.get());
            if (frac || frac_numerator || frac_denominator) {
                return std::make_unique<InfixExpressionNode>(frac->Tok,'/', frac->Kind,  createNumber(frac_numerator->Value), createNumber(frac_denominator->Value));
            }
            break;
        }

        case InfixKind::DIVIDE: {
            auto* div = dynamic_cast<InfixExpressionNode*>(expr.get());
            if (div) {
                return std::make_unique<InfixExpressionNode>(
                    div->Tok,
                    div->Operator,
                    div->Kind,
                    clone_expr(div->Left),
                    clone_expr(div->Right)
                );
            }
        }

        case InfixKind::MULTIPLY: {
            auto* prod = dynamic_cast<NaryExpressionNode*>(expr.get());

            std::vector<std::unique_ptr<ExpressionNode>> operands;

            for (auto& op : prod->Operands) {
                operands.push_back(clone_expr(op));
            }

            if (prod) {
                return std::make_unique<NaryExpressionNode>(
                    prod->Tok,
                    prod->Operator,
                    prod->Kind,
                    std::move(operands)
                );
            }
        }

        case InfixKind::PLUS: {
            auto* sum = dynamic_cast<NaryExpressionNode*>(expr.get());

            std::vector<std::unique_ptr<ExpressionNode>> operands;

            for (auto& op : sum->Operands) {
                operands.push_back(clone_expr(op));
            }

            if (sum) {
                return std::make_unique<NaryExpressionNode>(
                    sum->Tok,
                    sum->Operator,
                    sum->Kind,
                    std::move(operands)
                );
            }
        }
        
 
    }

    return nullptr;

        }

void SimplifyVisitor::extractBaseAndCoefficient(ExpressionNode* expr, std::unique_ptr<ExpressionNode>& base_out, std::unique_ptr<ExpressionNode>& coef_out) {
    if (expr->getKind() == InfixKind::VAR) {
        // For a variable, base is the variable itself and coefficient is 1 (implicit)
        base_out = createVariable(expr->TokenLiteral());
        coef_out = nullptr; // Indicates coefficient of 1
    } else if (expr->getKind() == InfixKind::MULTIPLY) {
        auto* prod = dynamic_cast<NaryExpressionNode*>(expr);
        auto operands = getNaryOperands(prod);
        
        // For a product, check if the first operand is a number/fraction (RNE) and the second is a variable (they will be ordered this way because of simplify_product)
        if (operands.size() >= 2) { //TODO: loop through and multiply all the numbers together if there are more than one, and pass that in 
            if ((operands[0]->getKind() == InfixKind::NUM || 
                 operands[0]->getKind() == InfixKind::FRACTION) && 
                operands[1]->getKind() == InfixKind::VAR) {

                
                // if (operands.size() > 2) {
                    // if (operands[0]->getKind() == InfixKind::NUM) {
                    //     auto num = dynamic_cast<NumberExpressionNode*>(operands[0].get());
                    //     int value = -1 * num->Value;
                    //     operands[0] = createNumber(value);
                    // } 
                    //else {
                    //     auto frac = dynamic_cast<InfixExpressionNode*>(operands[0].get());
                    //     int num = -1 * frac->L;
                    //     int denom = 
                    //     operands[0] = createNumber(value);
                    // }
                    
                //     }
                
                // Create copies of the coefficient and base
                if (operands[0]->getKind() == InfixKind::NUM) {
                    // auto num = dynamic_cast<NumberExpressionNode*>(operands[0].get());
                    // coef_out = createNumber(num->Value);
                    coef_out = clone_expr(operands[0]);
                } else {
                    // Handle fraction case by creating a new fraction
                    // auto frac = dynamic_cast<InfixExpressionNode*>(operands[0].get());
                    // coef_out = createFraction(frac->Left->Value, frac->Right);
                    coef_out = clone_expr(operands[0]);
                }
                
                // base_out = createVariable(operands[1]->TokenLiteral());
                base_out = clone_expr(operands[1]);
                } else {
                    std::cerr << "Not NUM||FRAC/VAR" << std::endl;
                    return;
                
                // // For now, just create a copy of the whole expression as the base
                // // This is a simplified approach
                // std::vector<std::unique_ptr<ExpressionNode>> clonedOperands;
                // for (const auto& op : operands) {
                //     clonedOperands.push_back(clone_expr(const_cast<std::unique_ptr<ExpressionNode>&>(op)));
                // }
                // base_out = createProduct(std::move(clonedOperands));
                // coef_out = nullptr;
            }
        } else {
            std::cout << "Functionality for this to be added with time but not needed here, only RNE*Var pairs exist." << std::endl;
            // // Create a copy of the whole expression as the base
            // std::vector<std::unique_ptr<ExpressionNode>> clonedOperands;
            // for (const auto& op : operands) {
            //     clonedOperands.push_back(clone_expr(const_cast<std::unique_ptr<ExpressionNode>&>(op)));
            // }
            // base_out = createProduct(std::move(clonedOperands));
            // coef_out = nullptr;
        }
    } else {
        std::cerr << "Can't extract base and coefficient" <<std::endl;
    }

}
// Expand linear equation: multiplication node with operands, num, and plus node
std::unique_ptr<ExpressionNode> SimplifyVisitor::expand_tree(std::unique_ptr<ExpressionNode>&& expr) {
    switch (expr->getKind()) {
        case InfixKind::MULTIPLY : {
            auto prod = dynamic_cast<NaryExpressionNode*>(expr.get());
            if ((prod->Operands[0]->getKind() == InfixKind::NUM || prod->Operands[0]->getKind() == InfixKind::FRACTION) && prod->Operands[1]->getKind() == InfixKind::PLUS) {
                
                auto plus_node = dynamic_cast<NaryExpressionNode*>(prod->Operands[1].get());


                std::vector<std::unique_ptr<ExpressionNode>> sum_operands;
                int a;
                int a_numtr;
                int a_denom;
                bool is_num = false;
                if (prod->Operands[0]->getKind() == InfixKind::NUM) {
                    a = dynamic_cast<NumberExpressionNode*>(prod->Operands[0].get())->Value;
                    is_num = true;
                }
                else {
                    a_numtr = getNumerator(prod->Operands[0].get());
                    a_denom = getDenominator(prod->Operands[0].get());
                }

                // auto b = plus_node->Operands[0].get();
                
                
                if (plus_node->Operands[0]->getKind() == InfixKind::NUM || plus_node->Operands[0]->getKind() == InfixKind::FRACTION ) {
                    std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
                    prod_operands.push_back(std::move( prod->Operands[0] ));
                    prod_operands.push_back(std::move( plus_node->Operands[0] ));
                        
                    auto op1 = simplify_product(createProduct(std::move(prod_operands)));

                    if (plus_node->Operands[1]->getKind() != InfixKind::MULTIPLY) {

                        is_num ? prod_operands.push_back(createNumber(a)) : prod_operands.push_back(createFraction(a_numtr, a_denom));
                        prod_operands.push_back(std::move( plus_node->Operands[1] ));
                        
                        sum_operands.push_back(std::move( op1 ));
                        sum_operands.push_back( createProduct(std::move( prod_operands )) );
                        
                        
                        
                    } else {
                        auto& child_operands = dynamic_cast<NaryExpressionNode*>(plus_node->Operands[1].get())->Operands;
                        is_num ? child_operands.push_back(createNumber(a)) : child_operands.push_back(createFraction(a_numtr, a_denom));

                        auto op2 = simplify_product(createProduct(std::move(child_operands)));

                        sum_operands.push_back(std::move(op1));
                        sum_operands.push_back(std::move(op2));
                    }

                    return createSum(std::move(sum_operands));

                } 
            }
        }
    }

    return std::move(expr);
}

std::unique_ptr<ExpressionNode> SimplifyVisitor::negate_expression(std::unique_ptr<ExpressionNode> expr) {
    std::unique_ptr<ExpressionNode> negated_expr;
    
    switch (expr->getKind()) {
     
        case (InfixKind::NUM) : {

            auto expr_cast = dynamic_cast<NumberExpressionNode*>(expr.get());
            return createNumber(-1 * expr_cast->Value);
        }

        case (InfixKind::FRACTION) : {

            int numerator = getNumerator(expr.get());
            int denomincator = getDenominator(expr.get());

            return createFraction(-1*numerator, denomincator);
        }

        case (InfixKind::VAR) : {//
            std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
            prod_operands.push_back(createNumber(-1));
            prod_operands.push_back(std::move(expr));

            return createProduct(std::move(prod_operands));
        }

        case (InfixKind::MULTIPLY) : { // RNE|NUM * SYMBOL, e.g. (3/4) * x or 2 * x
            // getting the coefficient and negating it 
            auto* expr_cast = dynamic_cast<NaryExpressionNode*>(expr.get());
            std::vector<std::unique_ptr<ExpressionNode>> prod_operands;
            std::unique_ptr<ExpressionNode> base1 = nullptr; // not using this, but needed for function
            std::unique_ptr<ExpressionNode> coef1 = nullptr;

            auto expr_clone = clone_expr(expr);
            extractBaseAndCoefficient(expr_clone.get(), base1, coef1);
            int numer = getNumerator(coef1.get());
            int denom = getDenominator(coef1.get());

            prod_operands.push_back(createFraction(-1*numer, denom));
            prod_operands.push_back(std::move(expr_cast->Operands[1]));

            return createProduct(std::move(prod_operands));


        }
    }
}

// (-3/2)x + (5/4)(x + (3/4)) + 4(x + (12/13))
// 12x + x(12/4) + (5/4)(5x + 12) + (6/5)((3/2)x+23))

void SimplifyVisitor::rearrange_left(std::unique_ptr<ExpressionNode>& left, std::unique_ptr<ExpressionNode>& right) {
    
    if (left->getKind() == InfixKind::NUM || left->getKind() == InfixKind::FRACTION) {
        if (right->getKind() == InfixKind::PLUS) {
            auto right_cast = dynamic_cast<NaryExpressionNode*>(right.get());
            auto negated_left = negate_expression(std::move(left));
            right_cast->Operands.push_back(std::move( negated_left ));
            right = std::move(simplify_sum(createSum(std::move(right_cast->Operands))));
        } else {
            std::vector<std::unique_ptr<ExpressionNode>> sum_operands;
            auto negated_left = negate_expression(std::move(left));
            sum_operands.push_back(std::move(negated_left));
            sum_operands.push_back(std::move(right));
            right = std::move(simplify_sum(createSum(std::move(sum_operands))));
        }
    } else if (left->getKind() == InfixKind::PLUS) {
        auto left_cast = dynamic_cast<NaryExpressionNode*>(left.get());

        if (left_cast->Operands[0]->getKind() == InfixKind::VAR && left_cast->Operands[1]->getKind() == InfixKind::MULTIPLY) { // edge case (x + (2 * (2 + x))) we will just expand 2(2+x)
            std::vector<std::unique_ptr<ExpressionNode>> sum_operands;

            sum_operands.push_back(std::move(left_cast->Operands[0])); //move var

            //expand at left_cast->Operands[1], e.g. 2(2+x)
            auto expanded = expand_tree(std::move(left_cast->Operands[1]));

            sum_operands.push_back(std::move(expanded));

            left = std::move(simplify_sum(createSum(std::move(sum_operands))));

        } else {
   
            rearrange_left(left_cast->Operands[0], right);
            left = std::move(simplify_sum(createSum(std::move(left_cast->Operands))));
        }
    
    } else if (left->getKind() == InfixKind::MULTIPLY) {
        std::vector<std::unique_ptr<ExpressionNode>> prod_operands;

        auto* left_cast = dynamic_cast<NaryExpressionNode*>(left.get());
        if (left_cast->Operands[0]->getKind() == InfixKind::NUM) {
            int num_to_rearrange = dynamic_cast<NumberExpressionNode*>(left_cast->Operands[0].get())->Value;
            auto frac = createFraction(1, num_to_rearrange);
            prod_operands.push_back(std::move(frac));
        } else { // its a fraction
            
            // Extract the coefficient (and base but i dont need that here) 
            std::unique_ptr<ExpressionNode> base1 = nullptr;
            std::unique_ptr<ExpressionNode> coef1 = nullptr;
            auto left_clone = clone_expr(left);       
            extractBaseAndCoefficient(left_clone.get(), base1, coef1);
            int numer = getNumerator(coef1.get());
            int denom = getDenominator(coef1.get());
            auto frac = createFraction(denom, numer); // fractino flips when moved to other side
            prod_operands.push_back(std::move(frac));

        }


        
        prod_operands.push_back(std::move(right));
        
        left_cast->Operands.erase(left_cast->Operands.begin());
        left = std::move(simplify_product(createProduct(std::move(left_cast->Operands))));
        right = std::move(simplify_product(createProduct(std::move(prod_operands))));
    
    }

 
    if (!left) {
        left = createNumber(0);
    } 
    if (!right) {
        right = createNumber(0);
    }
}

void SimplifyVisitor::rearrange_right(std::unique_ptr<ExpressionNode>& left, std::unique_ptr<ExpressionNode>& right) {
    if (right->getKind() == InfixKind::VAR || right->getKind() == InfixKind::MULTIPLY) {
        if (left->getKind() == InfixKind::PLUS) {
            auto left_cast = dynamic_cast<NaryExpressionNode*>(left.get());
            auto negated_right = negate_expression(std::move(right));
            left_cast->Operands.push_back(std::move( negated_right ));
            left = std::move(simplify_sum(createSum(std::move(left_cast->Operands))));
        } else {
            std::vector<std::unique_ptr<ExpressionNode>> sum_operands;
            auto negated_right = negate_expression(std::move(right));
            sum_operands.push_back(std::move(negated_right));
            sum_operands.push_back(std::move(left));
            left = std::move(simplify_sum(createSum(std::move(sum_operands))));
        }
    } else if (right->getKind() == InfixKind::PLUS) {
        auto right_cast = dynamic_cast<NaryExpressionNode*>(right.get());

        if (right_cast->Operands[0]->getKind() == InfixKind::VAR && right_cast->Operands[1]->getKind() == InfixKind::MULTIPLY) { // edge case (x + (2 * (2 + x))) we will just expand 2(2+x)
            std::vector<std::unique_ptr<ExpressionNode>> sum_operands;

            sum_operands.push_back(std::move(right_cast->Operands[0])); //move var

            //expand at left_cast->Operands[1], e.g. 2(2+x)
            auto expanded = expand_tree(std::move(right_cast->Operands[1]));

            sum_operands.push_back(std::move(expanded));

            right = std::move(simplify_sum(createSum(std::move(sum_operands))));

        } else {
            rearrange_right(left, right_cast->Operands[1]);
            right = std::move(simplify_sum(createSum(std::move(right_cast->Operands))));
        }
    }

 
    if (!left) {
        left = createNumber(0);
    } 
    if (!right) {
        right = createNumber(0);
    }

   
}

void SimplifyVisitor::solve_x(std::unique_ptr<ExpressionNode>& left, std::unique_ptr<ExpressionNode>& right) {
    if (left->getKind() == InfixKind::VAR) {
        return;
    } else if( left->getKind() == InfixKind::MULTIPLY) {
        auto left_cast = dynamic_cast<NaryExpressionNode*>(left.get());
        int left_coeff = dynamic_cast<NumberExpressionNode*>(left_cast->Operands[0].get())->Value;
        right = simplify_rne(createQuotient(std::move(right), createNumber(left_coeff)));
        left = std::move(createVariable(left_cast->Operands[1]->TokenLiteral()));
        return;
    } else if (left->getKind() == InfixKind::PLUS) {
        auto left_cast = dynamic_cast<NaryExpressionNode*>(left.get());
        int left_coeff = dynamic_cast<NumberExpressionNode*>(left_cast->Operands[0].get())->Value;
        right = simplify_rne(createQuotient(std::move(right), createNumber(left_coeff)));
        left = std::move(createVariable(left_cast->Operands[1]->TokenLiteral()));
        return;
    }
}


#endif
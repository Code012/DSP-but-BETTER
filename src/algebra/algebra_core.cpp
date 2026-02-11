/*  date = January 12th 2026 02:23 AM */



namespace algebra {

global StepList step_list{};

///////////////////////////////
//- Algebraic Simplification 

internal Result
SimplifyWithSteps(parse::Node* root)
{
	Result result{};
	Kind flags{};

	flags = AlgebraKindFromNodeKind(root);
	result.root = AutomaticSimplify(root, flags);


	return result;
}

// page 92. [Figure 3.10.]
internal parse::Node* 
AutomaticSimplify(parse::Node* u, Kind flags)
{

	Kind operand_flags{};

	//- 1. Integers and Symbols are already in simplified form
	if (HasFlag<Kind>(flags, Kind::Integer_B) || HasFlag<Kind>(flags, Kind::Symbol_B))		// leaf nodes, base condition
	{
		return u;
	}
	//- 2. For fractions, simplified form is obtained with SimplifyRationalNumber
	else if (u->kind == parse::NodeKind::BinaryOp && HasFlag<Kind>(flags, Kind::FracOp_B))
	{
		// return SimplifyRationalNumber(u);
	}
	//- 3. For other compound expressions: 
	else
	{
		// 3.1 First, simplify each operand recursively in depth-first post-order (children first)
		// AlgebraKindFromNodeKind is only valid to use here before any simplification for the node is applied. See comment above algebra::Kind in src/algebra/algebra_core.h.
		switch (u->kind)
		{
			case parse::NodeKind::UnaryOp:
			{
				operand_flags = AlgebraKindFromNodeKind(u->unary_child);
				u->unary_child = AutomaticSimplify(u->unary_child, operand_flags);		// my dudes don't freak out at the lack of nullptr checks, I'm trying something out trust (https://www.rfleury.com/p/the-easiest-way-to-handle-errors)
			} 
			case parse::NodeKind::BinaryOp:
			{
				operand_flags = AlgebraKindFromNodeKind(u->bin_left);
				u->bin_left = AutomaticSimplify(u->bin_left, operand_flags);

				operand_flags = AlgebraKindFromNodeKind(u->bin_right);
				u->bin_right = AutomaticSimplify(u->bin_right, operand_flags);
			}
			case parse::NodeKind::NaryOp:
			{
				for(S64 i = 0; i < u->num_operands; i++)
				{
					operand_flags = AlgebraKindFromNodeKind(u->nary_next); 
					u->nary_next = AutomaticSimplify(u->nary_next, operand_flags);
				}
			} 
		}

		// 3.2 Then, apply the appropriate simplificaiton operator
		switch (flags)
		{
			// commented out cases not written yet
			// case Kind::PowUp:
			// {
			// 	return SimplifyPower(u);
			// }
			case Kind::ProdOp_B:
			{
				return SimplifyProduct(u);
			}
			// case Kind::SumOp:
			// {
			// 	return SimplifySum(u);
			// }
			// case Kind::QuotOp:
			// {
			// 	return SimplifyQuotient(u);
			// }
			// case Kind::DiffOp:
			// {
			// 	return SimplifyDifference(u);
			// }
			// case Kind::FactOp:
			// {
			// 	return SimplifyFactorial(u);
			// }
			// default:
			// {
			// 	return SimplifyFunction(u);
			// }
		}
	}
}

internal parse::Node* 
SimplifyProduct(parse::Node* u)
{
	parse::Node* result{};

	
	
	return result;
}

///////////////////////////////
//- Helpers

// parse::NodeKind -> algebra::Kind
internal Kind 
AlgebraKindFromNodeKind(parse::Node* expr)
{

	using enum parse::NodeKind;	// for switch-case enumerators

	Kind operand_flags{};

	switch(expr->kind)
	{
		// Leaf: numeric literal
		case Number:			
		{
			operand_flags |= Kind::Integer_B;
		} break;

		// Leaf: identifier
		case Variable:			
		{
			operand_flags |= Kind::Symbol_B;
		} break;

		// Internal: *, +
		case NaryOp:	        
		{
			if (expr->nary_ops == parse::NaryOpKind::Plus)
			{
				operand_flags |= Kind::SumOp_B;
			}
			else if (expr->nary_ops == parse::NaryOpKind::Multiply)
			{
				operand_flags |= Kind::ProdOp_B;
			}
		} break;

		// Internal: -, /, FRACTION, ^
		case BinaryOp:			
		{
			if (expr->bin_ops == parse::BinOpKind::Minus)
			{
				operand_flags |= Kind::DiffOp_B;
				operand_flags |= Kind::BinaryDiffOp_S;
			}
			else if (expr->bin_ops == parse::BinOpKind::Power)
			{
				operand_flags |= Kind::PowOp_B;
			}
			// Fraction operation if its operands a and b != 0 are integers, however we will allow 0 at this time as it will be checked later and return undefined [Section 2.2]
			// Quotient operation if its operands a and b are rational number expressions (RNE) [Definition 2.26]
			else if (expr->bin_ops == parse::BinOpKind::Divide)
			{
				if (IsInteger(expr->bin_left) && IsInteger(expr->bin_right))
					operand_flags |= Kind::FracOp_B;
				else
					operand_flags |= Kind::QuotOp_B;
			}
		} break;

		// Internal: -{expression}, +{expression}
		case UnaryOp:			
		{
			if (expr->un_ops == parse::UnOpKind::Negate)
			{

				operand_flags |= Kind::DiffOp_B;
				operand_flags |= Kind::UnaryDiffOp_S;
			}
			else if (expr->un_ops == parse::UnOpKind::Positive)
			{
				operand_flags |= Kind::SumOp_B;
				operand_flags |= Kind::UnarySumOp_S;
			}
		} break;

		// Internal: sin(x), sqrt(x)
		case FunctionCall:		
		{} break;
	}

	return operand_flags;
}

internal B32 IsInteger(parse::Node* expr)
{
	return (expr->kind == parse::NodeKind::Number) ? 1 : 0;
}

} // namespace algebra
/*  date = January 12th 2026 02:23 AM */



namespace algebra {

///////////////////////////////
//- Algebraic Simplification 
Node* AutomaticSimplify(Node* u, Kind flags)
{

	Kind operand_flags{};

	if (HasFlag<Kind>(flags, Kind::Integer) || HasFlag<Kind>(flags, Kind::Symbol))		// leaves
	{
		return u;
	}
	else if (u.kind == parse::NodeKind::BinaryOp && HasFlag<Kind>(flags, Kind::FracOp))
	{
		return SimplifyRationalNumber(u);
	}
	else
	{
		// Simplify each operand in depth-first post-order recursively (children first)
		switch (u->kind)
		{
			case parse::NodeKind::UnaryOp:
			{
				operand_flags = DetermineOperandFlags(u->unary_child);
				u->unary_child = AutomaticSimplify(u->unary_child, operand_flags);		// my dudes don't freak out at the lack of nullptr checks, I'm trying something out trust (https://www.rfleury.com/p/the-easiest-way-to-handle-errors)
			} 
			case parse::NodeKind::BinaryOp: 
			{
				operand_flags = DetermineOperandFlags(u->left);
				u->left = AutomaticSimplify(u->left, operand_flags);

				operand_flags = DetermineOperandFlags(u->right);
				u->right = AutomaticSimplify(u->right, operand_flags);
			}
			case parse::NodeKind::NaryOp:
			{
				for(S64 i = 0; i < u->num_operands; i++)
				{
					operand_flags = DetermineOperandFlags(u->nary_next); 
					u->nary_next = AutomaticSimplify(u->nary_next);
				}
			} 
		}


		switch (flags)
		{
			case Kind::PowUp:
			{
				return SimplifyPower(u);
			}
			case Kind::ProdUp:
			{
				return SimplifyProduct(u);
			}
			case Kind::SumOp:
			{
				return SimplifySum(u);
			}
			case Kind::QuotOp:
			{
				return SimplifyQuotient(u);
			}
			case Kind::DiffOp:
			{
				return SimplifyDifference(u);
			}
			case Kind::FactOp:
			{
				return SimplifyFactorial(u);
			}
			default:
			{
				return SimplifyFunction(u);
			}
		}
	}
}

} // namespace algebra
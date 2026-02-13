/*  date = February 11th 2026 04:52 PM  */



namespace expr {

global StepList step_list{};

///////////////////////////////
//- Algebraic Simplification 

internal Result
SimplifyWithSteps(Node* root)
{
	Result result{};

	result.root = AutomaticSimplify(root);


	return result;
}

// page 92. [Figure 3.10.]
internal Node* 
AutomaticSimplify(Node* u)
{

	//- 1. Integers and Symbols are already in simplified form
	if (HasFlag<NodeFlags>(u->flags, NodeFlags::Integer_B) || HasFlag<NodeFlags>(u->flags, NodeFlags::Symbol_B))		// leaf nodes, base condition
	{
		return u;
	}
	//- 2. For fractions, simplified form is obtained with SimplifyRationalNumber
	else if (u->kind == NodeKind::BinaryOp && HasFlag<NodeFlags>(u->flags, NodeFlags::FracOp_B))
	{
		// return SimplifyRationalNumber(u);
	}
	//- 3. For other compound expressions: 
	else
	{
		// 3.1 First, simplify each operand recursively in depth-first post-order (children first)
		switch (u->kind)
		{
			case NodeKind::UnaryOp:
			{
				u->unary_child = AutomaticSimplify(u->unary_child);		// my dudes don't freak out at the lack of nullptr checks, I'm trying something out trust (https://www.rfleury.com/p/the-easiest-way-to-handle-errors)
			} 
			case NodeKind::BinaryOp:
			{
				u->bin_left = AutomaticSimplify(u->bin_left);

				u->bin_right = AutomaticSimplify(u->bin_right);
			}
			case NodeKind::NaryOp:
			{
				for(S64 i = 0; i < u->num_operands; i++)
				{
					u->nary_next = AutomaticSimplify(u->nary_next);
				}
			} 
		}

		// 3.2 Then, apply the appropriate simplificaiton operator
		// commented out cases not written yet
		// case Kind::PowUp:
		// {
		// 	return SimplifyPower(u);
		// }
		if (HasFlag<NodeFlags>(u->flags, NodeFlags::ProdOp_B))
		{
			return SimplifyProduct(u);
		}
		// case NodeFlags::SumOp:
		// {
		// 	return SimplifySum(u);
		// }
		// case NodeFlags::QuotOp:
		// {
		// 	return SimplifyQuotient(u);
		// }
		// case NodeFlags::DiffOp:
		// {
		// 	return SimplifyDifference(u);
		// }
		// case NodeFlags::FactOp:
		// {
		// 	return SimplifyFactorial(u);
		// }
		// default:
		// {
		// 	return SimplifyFunction(u);
		// }
	}
}

internal Node* 
SimplifyProduct(Node* u)
{
	Node* result{};
	result->bin_left = result->bin_right = result->unary_child = result->nary_first = result->nary_next = &nil_node;

	// let L = [u1,...,un] be the list of the operands of u.

	// SPRD-1. If Undefined ∈ L, then return Undefined
	for (Node* it = u->nary_first; !NodeIsNil(it); it = it->next)
	{
		if (it->flags == NodeFlags::Undefined)
		{
			result->flags = NodeFlags::Undefined;
			return result;
		}
	}

	// SPRD-2: if 0 ∈ L, then return 0
	for (Node* it = u->nary_first; !NodeIsNil(it); it = it->next)
	{
		if (IsInteger(it) && IsZero(it))
		{
			result->kind = NodeKind::Number;
			result->flags = NodeFlags::Integer_B;
			result->id = it->id;
			result->number = 0;
			// YOU WERE HERE
			// SOME IDEAS: have create int function that takes a node id as well to encode what changed into what
			return result;
		}
	}


	return result;
}

///////////////////////////////
//- Helpers
internal B32 
IsZero(Node* node)
{
	return (node->number == 0.0 ? 1 : 0);
}



}	// namespace expr


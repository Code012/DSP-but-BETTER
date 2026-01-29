/*  date = January 12th 2026 02:22 AM */ 

#ifndef ALGEBRA_CORE_H
#define ALGEBRA_CORE_H

/*
parser -> Expression Tree -> simplifier -> Modified Expression Tree
parser stores expression tree on arena-backed free list stored in app state
simplifier uses same arena

*/

namespace g
{
	///////////////////////////////
	//- Algebra Simplifications Rules

	global read_only String8 rules[] = {
		Str8Lit("Distributed Rule")
	};
} // namespace global

namespace algebra
{

	enum class Kind: U32
	{
		Nil 		= (1u << 0),
		Integer 	= (1u << 1),
		Symbol 		= (1u << 2),
		FracOp		= (1u << 3),		// define these terms
		PowUp		= (1u << 4),
		ProdUp		= (1u << 5),
		SumOp		= (1u << 6),
		QuotOp		= (1u << 7),
		DiffOp		= (1u << 8),
		FactOp		= (1u << 9),
		COUNT,
	};
	IMPLEMENT_ENUM_CLASS_BITMASK(Kind, U32);


	///////////////////////////////
	//- Solutions Steps Tracker
	struct StepNode
	{
		StepNode* next;
		String8 rule;						// e.g. "Distributed Rule"
		String8 description;				// Human-readable explanation
		String8 before;						// Full expression before applying rule
		String8 after;						// Full expression after applying rule
		std::map<U32, String8> highlights;	// Node ids mapped to highlight part of expression in UI
	};

	struct StepList
	{
		StepNode* first;
		StepNode* last;
		U64 count;
	};

	///////////////////////////////
	//-
	struct Simplifier
	{
		StepList steps;
		B32 simplifying_coefficient{0};
		
	}

	///////////////////////////////
	//- Algebraic Simplification 
	Node* AutomaticSimplify(Node* u);

} // namespace algebra
#endif // ALGEBRA_CORE_H
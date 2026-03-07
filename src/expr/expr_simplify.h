/*  date = February 11th 2026 04:53 PM  */


/*  date = February 11th 2026 04:52 PM  */



#ifndef EXPR_SIMPLIFY_H
#define EXPR_SIMPLIFY_H

namespace g
{
	///////////////////////////////
	//- Algebra Simplifications Rules

	global read_only String8 rules[] = {
		Str8Lit("Distributed Rule")
	};
} // namespace g

namespace expr {

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

	struct Result
	{
		Node* root;
		StepList steps;
	};


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//- Algebraic Simplification 
	internal Node* AutomaticSimplify(Arena* arena, Node* u);	// page 92. [Figure 3.10.]

	//- Fraction Simplification
	// SimplifyRationalNumber			// Not written
	//- Power Simplification
	// internal void SimplifyPower(Node* u) {};		// Not written
	//- Product Simplification
	internal Node* SimplifyProduct(Arena* arena, Node* u);		// Not written
	//- Sum Simplification
	// internal void SimplifySum(Node* u) {};			// Not written
	//- Quotient Simplification
	// internal void SimplifyQuotient(Node* u) {};	// Not written
	//- Difference Simplification
	// internal void SimplifyDifference(Node* u) {};	// Not written
	//- Factorial Simplification
	// internal void SimplifyFactorial(Node* u) {};	// Not written
	//- Function Simplification
	// internal void SimplifyFunction(Node* u) {};	// Not written

	///////////////////////////////
	//- Helpers
	internal B32 IsZero(Node* node);
	

}	// namespace expr

#endif	// EXPR_SIMPLIFY_H
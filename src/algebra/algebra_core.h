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

	// For parse::NodeKind -> algebra::Kind transformation. Note that the algebra::Kind enumarator is not meant to be "maintained" over the course of simplification which is why there is no Unary specifier for Product Operations; Unary ProdOps and SumOps will appear over the course of simplification and the algorithm will handle those cases. It is only meant to encode what algebra::Kind it is fresh from the parsing stage, before any simplification is done to make life easy, in the function DetermineOperandFlags. Therefore is not meant to be "maintained".
	// E.g. a UnarySumOp Specifier is defined because it is valid user input, 2+(+2) the +2 denotes positive 2. A UnaryProdOp Specifier is not because it is not valid user input 2+(*2). However unary versions of both will appear over the course of simplification and that's fine.
	enum class Kind: U32
	{
		// _B represent "base classification"
		// _S represent "specific classification"
		// _B enumerators will always be set, so that its _S enumerators can be checked
		Nil 		= (1u << 0),
		//- Base Enumerations
		Integer_B 	= (1u << 1),
		Symbol_B	= (1u << 2),
		FracOp_B	= (1u << 3),		// define these terms (BinaryOp)
		PowUp_B   	= (1u << 4),		// (UnaryOp)
		ProdUp_B	= (1u << 5),		// 
		SumOp_B		= (1u << 6),
		QuotOp_B	= (1u << 7),
		FactOp_B	= (1u << 8),
		DiffOp_B 	= (1u << 9),

		//- Specific Enumerations
		// SumOp_B
		UnarySumOp_S 	= (1u << 10),	// +x -> x [Definition 3.16]
		// DiffOp_B
		UnaryDiffOp_S 	= (1u << 11),	// Each unary difference is replaced by the product [Definition 3.12.1]
		BinaryDiffOp_S	= (1u << 12),	// Each binary difference is replaced by the sum [Definition 3.12.2]
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

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//- Algebraic Simplification 
	internal Node* AutomaticSimplify(Node* u, Kind flags);	// page 92. [Figure 3.10.]

	//- Power Simplification
	// internal void SimplifyPower(Node* u) {};		// Not written
	//- Product Simplification
	internal Node* SimplifyProduct(Node* u);		// Not written
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

	// parse::NodeKind -> algebra::Kind
	internal Kind DetermineOperandFlags(Node* expr);
	internal B32 IsInteger(Node* expr);

} // namespace algebra
#endif // ALGEBRA_CORE_H
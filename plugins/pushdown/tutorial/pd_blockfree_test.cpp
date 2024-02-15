/** @file pd_blockfree_test.cpp

 This file tests the blockfree algorithm.


 @ingroup Tutorials

 @include pd_blockfree_test.cpp

 */

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

/**
 * Tests the algorithm to ensure non blocking.
 * In this case the algorithm is adapted to the given pushdown generator (pd_blocked)
 * General algorithm see PushdownBlockfree() in file pd_alg_main.cpp
 */
PushdownGenerator make_blockfree(const PushdownGenerator& pd){

		//take a copy of the old generator
		PushdownGenerator rPd(pd);

		//create SDPDA
		rPd = SPDA(rPd);
		FAUDES_TEST_DUMP("Spda", rPd)

		//remove double acceptance
		rPd = Nda(rPd);
		PushdownTrim(rPd,0);
		FAUDES_TEST_DUMP("Nda", rPd)

		//convert to grammar
		Grammar gr = Sp2Lr(rPd);
		FAUDES_TEST_DUMP("CFG has 12 productions", (gr.GrammarProductions().size() == 12))

		//remove unreachable productions
		gr = Rup(gr);
		FAUDES_TEST_DUMP("9 productions after Rup", (gr.GrammarProductions().size() == 9))

		//augment grammar
		//create temporary automaton to create a new event for the augment terminal
		//and a new stack symbol for the augment nonterminal
		PushdownGenerator tempPd = rPd;

		//create fresh terminal
		Terminal t(tempPd.InsEvent("augSymbol"));

		std::vector<Idx> v;
		v.push_back(tempPd.InsStackSymbol(StackSymbol("augSymbol")));
		Nonterminal nt(0, v);

		//Augments the grammar such that a new grammar production  will be inserted
		Grammar augGr = Aug(gr, nt, t);

		//construct the goto generator for the augmented grammar
		GotoGenerator gotoGen = Lrm(augGr, 1);
		FAUDES_TEST_DUMP("goto generator", gotoGen)

		//construct a parser for the grammar
		Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, t);
		FAUDES_TEST_DUMP("Lr(1)-Parser has 13 actions", (parser.Actions().size() == 13))

		//detach augment symbol
		DetachAugSymbol(parser);

		// Transform actions
		parser = TransformParserAction(parser);
		FAUDES_TEST_DUMP("12 actions after transform", (parser.mActions.size() == 12))

		//convert parser to pushdown generator
		rPd = LrParser2EPDA(parser);
		FAUDES_TEST_DUMP("Epda", rPd)

		//merge adjacent transitions
		MergeAdjacentTransitions(rPd);
		FAUDES_TEST_DUMP("Merge adjacent transitions", rPd)

		//removes transitions that pop lambda
		rPd = RemoveLambdaPop(rPd);
		FAUDES_TEST_DUMP("No lambda pop", rPd)

		//remove transitions popping more then one symbol
		rPd = RemoveMultPop(rPd);
		FAUDES_TEST_DUMP("No multiple pop", rPd)

		//removes transitions that pop lambda
		rPd = RemoveLambdaPop(rPd);
		FAUDES_TEST_DUMP("Pda", rPd)

		PushdownTrim(rPd,2);
		FAUDES_TEST_DUMP("Pda trim with 2 as lookahead", rPd)

		//merge adjacent transitions
		MergeAdjacentTransitions(rPd);
		FAUDES_TEST_DUMP("Merge adjacent transitions again", rPd)

		PushdownTrim(rPd,0);

		return rPd;
}

/** Run the blockfree test */
int main() {

	//*******************************
	//Get pushdown generator from DOT-file
	//The generator can be seen in data/graph/pd_blockfree_test.png
	//State 's3' will lead to a blocking issue
	//*******************************
	PushdownGenerator pd("data/graph/pd_blockfree_test.dot");


	//*******************************
	//create blockfree generator
	//*******************************
	PushdownGenerator pd_blockfree = make_blockfree(pd);


	//*******************************
	// Record test case
	//*******************************

	FAUDES_TEST_DUMP("blocked pushdown generator", pd)
	FAUDES_TEST_DUMP("blockfree pushdown generator", pd_blockfree)

	// Validate result
	FAUDES_TEST_DIFF()

	// done
	return 0;
}


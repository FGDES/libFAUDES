/** @file pd_controllable_test.cpp

 This file tests the algorithm to ensure
 the controllability of a controller.


 @ingroup Tutorials

 @include pd_controllable_test.cpp


 */

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

/**
 * Tests the algorithm to ensure controllability.
 * In this case the algorithm is adapted to the given pushdown generator (pd_ctest_contr)
 * and system (pd_ctest_plant)
 * General algorithm see ConstructControllerLoop() in file pd_alg_main.cpp
 */
PushdownGenerator make_controllable(const PushdownGenerator& cont, const System& sys){

		//generator to return
		PushdownGenerator rPd;

		//create product generator
		rPd = Times(sys, cont);
		FAUDES_TEST_DUMP("product generator", rPd);

		//trim the generator
		PushdownTrim(rPd, 0);
		FAUDES_TEST_DUMP("trim generator", rPd);

		//split states into heads and ears
		rPd = Split(rPd);
		FAUDES_TEST_DUMP("split generator", rPd);

		//remove inaccessible states
		PushdownAccessible(rPd,rPd);
		FAUDES_TEST_DUMP("remove inaccessible states", rPd);

		//save number of states
		uint numberOfStates = rPd.Size();

		//set lambda event controllable
		rPd.SetControllable(FAUDES_PD_LAMBDA);

		//remove non-controllable ears
		rPd = Rnce(rPd, sys);

		FAUDES_TEST_DUMP("remove non-controllable ears", rPd);

		FAUDES_TEST_DUMP("states before > states now", numberOfStates > rPd.Size());

		return rPd;
}

/** Run the blockfree test */
int main() {

	//*******************************
	//Get pushdown generator from DOT-file
	//The generator and plant can be seen in
	//data/graph/pd_ctest_contr.png and data/graph/pd_ctest_plant.png
	//State 's2' will prevent the uncontrollable event 'u'
	//*******************************
	PushdownGenerator cont("data/graph/pd_ctest_contr.dot");
	System plant = SystemFromDot("data/graph/pd_ctest_plant.dot");

	//*******************************
	//create controllable generator
	//*******************************
	PushdownGenerator rPd;
	rPd = make_controllable(cont, plant);

	//*******************************
	// Record test case
	//*******************************

	FAUDES_TEST_DUMP("plant", plant);
	FAUDES_TEST_DUMP("controller with controllability problems", plant);
	FAUDES_TEST_DUMP("controllable controller", rPd);

	// Validate result
	FAUDES_TEST_DIFF()

	// done
	return 0;
}


/** @file mtc_3_observer.cpp 

Tutorial,  MtcSystem observer computation

@ingroup Tutorials 

@include mtc_3_observer.cpp
*/

// This tutotial is BROKEN. Need to go back to version 2.13a and FIX. Sorry. 

#include "libfaudes.h"
#include "mtc_include.h"


// Make namespaces available to our program (lazy)
using namespace faudes;
using namespace std;


/////////////////
// main program
/////////////////

int main() {



  MtcSystem cb4_0, cb4_0_spec1, cb4_0_spec2, cb4_0_spec, cb4_0_sup, cb4_1,
               mh1d1_0_sup, mh1d1_1,
               cb4_1_spec, cb11_1_spec, mh1d1_1_spec,
               rt1_1_spec, rt1_1_spec1, rt1_1_spec2,
               cb4mh1d1_1, cb4mh1d1_1_spec, cb4mh1d1_1_sup, cb4mh1d1_2, cb4mh1d1_2_orig;

// We explain the relevant methods using a manufacturing system example that is 
// a part of the example in 
// K. Schmidt and J.E.R. Cury, "Redundant Tasks in Multitasking Control of Discrete Event Systems", Workshop on Dependable Control of Discrete Event Systems, 2009.


// First a synthesis without removing redundant colors is performed
// Read Plant generators
  // cb4[0]
    cb4_0.Read("data/cb4[0].gen");
    cb4_0.GraphWrite("tmp_mtc_cb4[0].png");
  // mh1d1[0]_sup
    mh1d1_0_sup.Read("data/mh1d1[0]_sup.gen");
    mh1d1_0_sup.GraphWrite("tmp_mtc_mh1d1[0]_sup.png");  

// Read Specifications
    // cb4[0]_spec1
    cb4_0_spec1.Read("data/cb4[0]_spec1.gen");
    cb4_0_spec1.GraphWrite("tmp_mtc_cb4[0]_spec1.png");
    mtcInvProject(cb4_0_spec1, cb4_0.Alphabet());    
  // cb4[0]_spec2        
    cb4_0_spec2.Read("data/cb4[0]_spec2.gen");
    cb4_0_spec2.GraphWrite("tmp_mtc_cb4[0]_spec2.png");   
 
  // cb4[1]_spec
    cb4_1_spec.Read("data/cb4[1]_spec.gen");
    cb4_1_spec.GraphWrite("tmp_mtc_cb4[1]_spec.png");
  // mh1d1[1]_spec
    mh1d1_1_spec.Read("data/mh1d1[1]_spec.gen");
    mh1d1_1_spec.GraphWrite("tmp_mtc_mh1d1[1]_spec.png");

    // Supervisor Synthesis

  // cb4[0]_sup
  {
    std::cout << std::endl << "MtcParallel(cb4[0]_spec1, cb4[0]_spec2) => cb4[0]_spec" << std::endl;
    mtcParallel(cb4_0_spec1, cb4_0_spec2, cb4_0_spec);
    cb4_0_spec.Write("tmp_mtc_cb4[0]_spec.gen");
    cb4_0_spec.GraphWrite("tmp_mtc_cb4[0]_spec.png");

    std::cout << "MtcSupConNB(cb4[0], cb4[0]_spec) => cb4[0]_sup" << std::endl << std::endl;
    mtcSupConNB(cb4_0, cb4_0_spec, cb4_0_sup);

    cb4_0_sup.Write("tmp_mtc_cb4[0]_sup.gen");
    cb4_0_sup.GraphWrite("tmp_mtc_cb4[0]_sup.png");
  }

/////////////////
//   Level 1
/////////////////

  // cb4[1]
  {
    EventSet cb4_1_alph;
    cb4_1_alph.Insert("cb11-cb4");
    cb4_1_alph.Insert("cb12-cb4");
    cb4_1_alph.Insert("cb4-cb11");
    cb4_1_alph.Insert("cb4-cb12");
    cb4_1_alph.Insert("cb4stp");

    if (IsMtcObs(cb4_0_sup, cb4_1_alph)) {
      std::cout << "Project(cb4[0]_sup) => cb4[1], with alphabet: " << cb4_1_alph.ToString() << std::endl << std::endl;
      mtcProject(cb4_0_sup, cb4_1_alph, cb4_1);
    }
    else std::cout << "cb4[0]_sup: Observer condition is not fulfilled" << std::endl;

    cb4_1.StateNamesEnabled(false);
    cb4_1.Write("tmp_mtc_cb4[1].gen");
    cb4_1.GraphWrite("tmp_mtc_cb4[1].png");
  }
  // mh1d1[1]
  {
    EventSet mh1d1_1_alph;
    mh1d1_1_alph.Insert("mh1start");
    mh1d1_1_alph.Insert("mh1end");

    if (IsMtcObs(mh1d1_0_sup, mh1d1_1_alph)) {
      std::cout << "Project(mh1d1[0]_sup) => mh1d1[1], with alphabet: " << mh1d1_1_alph.ToString() << std::endl << std::endl;
      mtcProject(mh1d1_0_sup, mh1d1_1_alph, mh1d1_1);
    }
    else std::cout << "mh1d1[0]_sup: Observer condition is not fulfilled" << std::endl;

    mh1d1_1.StateNamesEnabled(false);
    mh1d1_1.Write("tmp_mtc_mh1d1[1].gen");
    mh1d1_1.GraphWrite("tmp_mtc_mh1d1[1].png");
  }
  // cb4mh1d1[1]_sup
  {
    std::cout << "MtcParallel(cb4[1], mh1d1[1]) => cb4mh1d1[1]" << std::endl;
    mtcParallel(cb4_1, mh1d1_1, cb4mh1d1_1);
    cb4mh1d1_1.Write("tmp_mtc_cb4mh1d1_1.gen");
    cb4mh1d1_1.GraphWrite("tmp_mtc_cb4mh1d_1.png");
    std::cout << "MtcSupConNB(cb4mh1d1[1], cb4mh1d1[1]_spec) => cb4mh1d1[1]_sup" << std::endl << std::endl;
    mtcInvProject(mh1d1_1_spec, cb4mh1d1_1.Alphabet() );
    //MtcInvProject(cb4mh1d1_1, cb4mh1d1_1.Alphabet()+cb4mh1d1_1_spec.Alphabet());
    mtcSupConNB(cb4mh1d1_1, mh1d1_1_spec, cb4mh1d1_1_sup);

    cb4mh1d1_1_sup.Write("tmp_mtc_cb4mh1d1_1_sup.gen");
    cb4mh1d1_1_sup.GraphWrite("tmp_mtc_cb4mh1d1_1_sup.png");
  }


/////////////////
//   Level 2
/////////////////
  // cb4mh1d1[2]
  {
    EventSet cb4mh1d1_2_alph;
    cb4mh1d1_2_alph.Insert("cb11-cb4");
    cb4mh1d1_2_alph.Insert("cb12-cb4");
    cb4mh1d1_2_alph.Insert("cb4-cb11");
    cb4mh1d1_2_alph.Insert("cb4-cb12");
    cb4mh1d1_2_alph.Insert("mh1end");
    // compute the abstraction with the original colors
    mtcProject(cb4mh1d1_1_sup, cb4mh1d1_2_alph, cb4mh1d1_2_orig);
  // cb4mh1d1[2]
  {
    EventSet cb4mh1d1_2_alph;
    cb4mh1d1_2_alph.Insert("cb11-cb4");
    cb4mh1d1_2_alph.Insert("cb12-cb4");
    cb4mh1d1_2_alph.Insert("cb4-cb11");
    cb4mh1d1_2_alph.Insert("cb4-cb12");
    cb4mh1d1_2_alph.Insert("mh1end");
    // compute the abstraction with the original colors
    mtcProject(cb4mh1d1_1_sup, cb4mh1d1_2_alph, cb4mh1d1_2_orig);
    cb4mh1d1_2_orig.StateNamesEnabled(false);
    cb4mh1d1_2_orig.Write("tmp_mtc_cb4mh1d1_2_orig.gen");
    cb4mh1d1_2_orig.GraphWrite("tmp_mtc_cb4mh1d1_2_orig.png");
  }
    cb4mh1d1_2_orig.StateNamesEnabled(false);
    cb4mh1d1_2_orig.Write("tmp_mtc_cb4mh1d1_2_orig.gen");
    cb4mh1d1_2_orig.GraphWrite("tmp_mtc_cb4mh1d1_2_orig.png");
  }

// ==============================
// computation with color removal
// ==============================

/////////////////
//   Level 2
/////////////////
  // cb4mh1d1[2]
  {
    // reduction of the color set
    ColorSet optimalColors; 
    EventSet cb4mh1d1_2_alph;
    cb4mh1d1_2_alph.Clear();
    cb4mh1d1_2_alph.Insert("cb11-cb4");
    cb4mh1d1_2_alph.Insert("cb12-cb4");
    cb4mh1d1_2_alph.Insert("cb4-cb11");
    cb4mh1d1_2_alph.Insert("cb4-cb12");    
    OptimalColorSet(cb4mh1d1_1_sup,optimalColors, cb4mh1d1_2_alph);
    cout << "these are the optimal colors " << endl;
    optimalColors.Write();
    cout << "this is the optimal alphabet " << endl;
    cb4mh1d1_2_alph.Write();
    
    ColorSet redundantColors = cb4mh1d1_1_sup.Colors() - optimalColors;
    ColorSet::Iterator cIt = redundantColors.Begin();
    for( ; cIt != redundantColors.End(); cIt++){
        cb4mh1d1_1_sup.DelColor(*cIt);
    }
    
    if (IsMtcObs(cb4mh1d1_1_sup, cb4mh1d1_2_alph)) {
      std::cout << "Project(cb4mh1d1[0]_sup) => cb4mh1d1[2], with alphabet: " << cb4mh1d1_2_alph.ToString() << std::endl << std::endl;
      mtcProject(cb4mh1d1_1_sup, cb4mh1d1_2_alph, cb4mh1d1_2);
    }
    else std::cout << "cb4mh1d1[1]_sup: Observer condition is not fulfilled" << std::endl;

    cb4mh1d1_2.StateNamesEnabled(false);
    cb4mh1d1_2.Write("tmp_mtc_cb4mh1d1_2.gen");
    cb4mh1d1_2.GraphWrite("tmp_mtc_cb4mh1d1_2.png");
  }

} // end main()

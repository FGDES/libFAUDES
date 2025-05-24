/** @file mtc_2_functions.cpp 

Tutorial,  MtcSystem functions eg parallel, project etc.


@ingroup Tutorials 

@include mtc_2_functions.cpp
*/


#include "libfaudes.h"


// Make the faudes namespace available to our program
using namespace faudes;


/////////////////
// main program
/////////////////

int main() {
  {
    // create generator which is not accessible and not strongly, but weakly coaccessible
    MtcSystem original, accNotStrCoac, notAccStrCoac, strTrim;

    // the implementation below allows the user to easily change the generators
    // and to find out what the algorithms effect when being applied on them
    Idx st1 = original.InsState("1");
    Idx st2 = original.InsState("2");
    Idx st3 = original.InsState("3");
    Idx st4 = original.InsState("4");
    Idx st5 = original.InsState("5");
    Idx st6 = original.InsColoredState("6", "c6a");

    Idx eva = original.InsEvent("a");
    Idx evb = original.InsEvent("b");
    Idx evc = original.InsEvent("c");
    Idx evd = original.InsEvent("d");

    original.InsColor(st3, "c3");
    original.InsColor(st4, "c4");
    original.InsColor(st6, "c6b");

    original.SetTransition(st1, eva, st2);
    original.SetTransition(st2, evb, st3);
    original.SetTransition(st2, evc, st4);
    original.SetTransition(st4, evc, st6);
    original.SetTransition(st5, eva, st4);
    original.SetTransition(st6, evd, st1);
    original.SetTransition(st3, evd, st3);

    original.SetInitState(st1);

    original.Write("tmp_mtc_functions_1a_not_trim.gen");
    original.GraphWrite("tmp_mtc_functions_1a_not_trim.png");

    ////////////////////////////////////////////////////
    // analyze a generator's accessibility properties
    ////////////////////////////////////////////////////
    if (!original.IsAccessible())
      std::cout << std::endl << "Original generator is not accessible" << std::endl;
    if (!original.IsStronglyCoaccessible())
      std::cout << "Original generator is not strongly coaccessible" << std::endl;
    if (!original.IsStronglyTrim())
      std::cout << "Original generator is not strongly trim" << std::endl << std::endl;

    ////////////////////////////////////////////////////
    // generate accessible version of original generator
    ////////////////////////////////////////////////////
    accNotStrCoac = original;
    accNotStrCoac.Accessible();

    accNotStrCoac.Write("tmp_mtc_functions_1b_acc.gen");
    accNotStrCoac.GraphWrite("tmp_mtc_functions_1b_acc.png");

    if (accNotStrCoac.IsAccessible())
      std::cout << "accNotStrCoac is accessible" << std::endl;
    if (!accNotStrCoac.IsStronglyTrim())
      std::cout << "accNotStrCoac is not strongly trim" << std::endl << std::endl;

    // generate strongly coaccessible version of original generator
    notAccStrCoac = original;
    notAccStrCoac.StronglyCoaccessible();

    notAccStrCoac.Write("tmp_mtc_functions_1c_str_trim.gen");
    notAccStrCoac.GraphWrite("tmp_mtc_functions_1c_str_trim.png");

    if (notAccStrCoac.IsStronglyCoaccessible())
      std::cout << "notAccStrCoac is strongly coaccessible" << std::endl;
    if (!notAccStrCoac.IsStronglyTrim())
      std::cout << "notAccStrCoac is not strongly trim" << std::endl << std::endl;

      // generate trim version of original generator
    strTrim = original;
    strTrim.StronglyTrim();

    strTrim.Write("tmp_mtc_functions_1d_str_trim.gen");
    strTrim.GraphWrite("tmp_mtc_functions_1d_str_trim.png");

    if (strTrim.IsAccessible())
      std::cout << "strTrim is accessible" << std::endl;
    if (strTrim.IsStronglyCoaccessible())
      std::cout << "strTrim is strongly coaccessible" << std::endl;
    if (strTrim.IsStronglyTrim())
      std::cout << "strTrim is strongly trim" << std::endl << std::endl;
  }

  {
    // create nondeterministic generator nondet, deterministic version to compute shall be saved as det
    MtcSystem nondet, det;

    // the implementation below allows the user to easily change the generators
    // and to find out what the algorithms effect when being applied on them
    Idx st1 = nondet.InsState("1");
    Idx st2 = nondet.InsState("2");
    Idx st3 = nondet.InsState("3");
    Idx st4 = nondet.InsState("4");
    Idx st5 = nondet.InsState("5");
    Idx st6 = nondet.InsColoredState("6", "c6a");

    Idx eva = nondet.InsEvent("a");
    Idx evb = nondet.InsEvent("b");
    Idx evc = nondet.InsEvent("c");

    nondet.InsColor(st3, "c3");
    nondet.InsColor(st4, "c4");
    nondet.InsColor(st5, "c5");
    nondet.InsColor(st6, "c6b");

    nondet.SetTransition(st1, eva, st2);
    nondet.SetTransition(st2, evb, st3);
    nondet.SetTransition(st2, evb, st4);
    nondet.SetTransition(st4, evc, st6);
    nondet.SetTransition(st5, eva, st4);

    nondet.SetInitState(st1);
    nondet.SetInitState(st5);

    nondet.Write("tmp_mtc_functions_2a_nondet.gen");
    nondet.GraphWrite("tmp_mtc_functions_2a_nondet.png");

    //////////////////////////////////////////////////
    // make nondeterministic generator deterministic
    //////////////////////////////////////////////////
    mtcDeterministic(nondet, det);

    det.Write("tmp_mtc_functions_2b_det.gen");
    det.GraphWrite("tmp_mtc_functions_2b_det.png");
  }

  {
    EventSet evset;
    evset.Insert("a");
    evset.Insert("b");
    evset.Insert("d");

    MtcSystem original, projected;

    Idx st1 = original.InsState("1");
    Idx st2 = original.InsState("2");
    Idx st3 = original.InsState("3");
    Idx st4 = original.InsState("4");
    Idx st5 = original.InsState("5");

    Idx eva = original.InsEvent("a");
    Idx evb = original.InsEvent("b");
    Idx evc = original.InsEvent("c");
    Idx evd = original.InsEvent("d");

    original.InsColor(st2, "color2");
    original.InsColor(st3, "color3");

    original.SetTransition(st1, eva, st2);
    original.SetTransition(st2, evb, st3);
    original.SetTransition(st2, evc, st4);
    original.SetTransition(st3, evc, st5);
    original.SetTransition(st4, evb, st5);
    original.SetTransition(st5, evd, st1);

    original.SetInitState(st1);

    original.StateNamesEnabled(false);
    original.Write("tmp_mtc_functions_3a_system.gen");
    original.GraphWrite("tmp_mtc_functions_3a_system.png");

    // Write state sets for colored and uncolored states to console
    // original.ColoredStates().Write();
    // original.UncoloredStates().Write();

    //////////////////////////////////////////////////////////////
    // compute projection for alphabet evset and the original
    // generator result is deterministic and saved in projected
    //////////////////////////////////////////////////////////////
    mtcProject(original, evset, projected);

    projected.Write("tmp_mtc_functions_3b_projected.gen");
    projected.GraphWrite("tmp_mtc_functions_3b_projected.png");

    //////////////////////////////////////////////////////////////
    // compute original generator to its projected version for the
    // alphabet evset result is nondeterministic
    //////////////////////////////////////////////////////////////
    mtcProjectNonDet(original, evset);

    original.Write("tmp_mtc_functions_3c_projected_nondet.gen");
    original.GraphWrite("tmp_mtc_functions_3c_projected_nondet.png");
  }

  {
    // create two generators for parallel composition and compose them to gen_ab
    MtcSystem gen_a, gen_b, gen_ab;

    // generator a
    Idx sta1 = gen_a.InsInitState("1");
    Idx sta2 = gen_a.InsState("2");

    gen_a.InsColor(sta2, "Color_1");

    Idx evaa = gen_a.InsEvent("a");
    Idx evab = gen_a.InsEvent("b");

    gen_a.SetTransition(sta1, evaa, sta2);
    gen_a.SetTransition(sta2, evab, sta1);

    gen_a.Write("tmp_mtc_functions_4a_system.gen");
    gen_a.GraphWrite("tmp_mtc_functions_4a_system.png");

    // generator b
    Idx stb1 = gen_b.InsInitState("1");
    Idx stb2 = gen_b.InsState("2");

    gen_b.InsColor(stb1, "Color_1");
    gen_b.InsColor(stb2, "Color_2");

    Idx evba = gen_b.InsEvent("a");
    Idx evbc = gen_b.InsEvent("c");

    gen_b.SetTransition(sta1, evba, sta2);
    gen_b.SetTransition(sta2, evbc, sta1);

    gen_b.Write("tmp_mtc_functions_4b_system.gen");
    gen_b.GraphWrite("tmp_mtc_functions_4b_system.png");

    /////////////////////////////////////////////
    // Compute the parallel composition gen_ab of
    // these two colored marking generators
    /////////////////////////////////////////////
    mtcParallel(gen_a, gen_b, gen_ab);

    gen_ab.Write("tmp_mtc_functions_4c_parallel.gen");
    gen_ab.GraphWrite("tmp_mtc_functions_4c_parallel.png");
  }


  {
    // create model model and specification spec for
    // computing a strongly nonblocking supervisor sup
    MtcSystem model, spec, sup, sup_nb;

    // generator model
    Idx stm1 = model.InsInitState("1");
    Idx stm2 = model.InsState("2");
    Idx stm3 = model.InsState("3");
    Idx stm4 = model.InsState("4");
    Idx stm5 = model.InsState("5");
    Idx stm6 = model.InsState("6");

    model.InsColor(stm1, "Color_1");
    model.InsColor(stm6, "Color_2");

    Idx evma = model.InsControllableEvent("a");
    Idx evmb = model.InsControllableEvent("b");
    Idx evmc = model.InsControllableEvent("c");

    model.SetTransition(stm1, evmb, stm2);
    model.SetTransition(stm2, evma, stm3);
    model.SetTransition(stm2, evmc, stm4);
    model.SetTransition(stm4, evma, stm6);
    model.SetTransition(stm1, evmc, stm5);
    model.SetTransition(stm5, evmb, stm6);
    model.SetTransition(stm6, evma, stm1);

    model.Write("tmp_mtc_functions_5_plant.gen");
    model.GraphWrite("tmp_mtc_functions_5_plant.png");

    // generator spec
    Idx sts1 = spec.InsInitState("1");
    Idx sts2 = spec.InsState("2");

    Idx evsb = spec.InsControllableEvent("b");
    Idx evsc = spec.InsControllableEvent("c");

    spec.SetTransition(sts1, evsb, sts2);
    spec.SetTransition(sts2, evsc, sts1);

    spec.Write("tmp_mtc_functions_5_spec.gen");
    spec.GraphWrite("tmp_mtc_functions_5_spec.png");

    /////////////////////////////////////////////////////////
    // Inverse projection adds missing events by inserting
    // self-loops into all automaton's states
    /////////////////////////////////////////////////////////
    mtcInvProject(spec, model.Alphabet());

    spec.Write("tmp_mtc_functions_5_spec_invpro.gen");
    spec.GraphWrite("tmp_mtc_functions_5_spec_invpro.png");

    /////////////////////////////////////////////////////////
    // Supervisor computation
    /////////////////////////////////////////////////////////
    mtcSupConClosed(model, spec, sup);

    sup.Write("tmp_mtc_functions_5_super.gen");
    sup.GraphWrite("tmp_mtc_functions_5_super.png");

    /////////////////////////////////////////////
    // Nonblocking supervisor computation
    /////////////////////////////////////////////
    mtcSupCon(model, spec, sup_nb);

    sup_nb.Write("tmp_mtc_functions_5_supernb.gen");
    sup_nb.GraphWrite("tmp_mtc_functions_5_supernb.png");
  }

}


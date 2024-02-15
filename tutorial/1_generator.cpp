/** @file 1_generator.cpp 

Tutorial, Generator methods. 

The Generator class implements the 5-tuple automaton G, consisting of
- Set of States Q, 
- Alphabet Sigma, 
- Transition Relation Delta,
- Set of Initial States Qo, and
- Set of Marked States Qm.

This tutorial demonstrates how to insert/erase states, events and transitions. 
It also demonstrates file IO.

@ingroup Tutorials 

@include 1_generator.cpp
*/


#include "libfaudes.h"

// make faudes namespace available
using namespace faudes;



/////////////////
// main program
/////////////////

int main() {

  ////////////////////////////////////////////
  // Constructors (part 1) and filling example
  ////////////////////////////////////////////

  // at first we create an empty Generator object

  Generator g1;

  // do some random "user interaction" stuff with the Generator g1

  g1.InsState("s1");  
  g1.InsState("s2");                  
  g1.InsState("s3");                  

  g1.InsEvent("a");         
  g1.InsEvent("b");

  g1.SetTransition("s1", "a", "s2");  
  g1.SetTransition("s2", "a", "s3");  
  g1.SetTransition("s3", "b", "s1");

  g1.SetInitState("s1");              
  g1.SetMarkedState("s2");
  g1.SetMarkedState("s3");

  // inspect result on console

  std::cout << "################################\n";
  std::cout << "# tutorial, handcraft generator \n";
  g1.Write();
  std::cout << "################################\n";

  // record test case
  FAUDES_TEST_DUMP("g1", g1); 



  ///////////////////////////////////////////////////
  // Constructors (part 2) & Copying and versioning
  ///////////////////////////////////////////////////

  // Create a  1:1 copy of the Generator with the copy constructor ... 

  Generator g_copy(g1);

  // ... with assignment method, or assignement operator

  Generator g2;
  g2.Assign(g1);
  Generator g3=g2;

  // create a Generator copy with versioned events (for testing algorithms):
  // versioning by an integer. E.g. for integer 3 events {"a", "b", "c"}
  // become {"a_3", "b_3", "c_3"}.

  Generator version1;
  g3.Version(3, version1);

  // versioning by a string. "a" -> "a_versionstring"

  Generator version2;
  g3.Version("str", version2);

  // versioning by replacing a pattern  "_str" -> "_machine_3"

  Generator version3;
  version2.Version("_str", "_machine_3",version3);

  // editing by renaming an individual event

  version3.EventRename("a_machine_3","synch");

  // inspect result on console

  std::cout << "################################\n";
  std::cout << "# tutorial, version of generator \n";
  version3.Write();
  std::cout << "################################\n";

  // record test case
  FAUDES_TEST_DUMP("generator", version1); 


  ///////////////////////////////////////////////
  // Methods for Input/Output
  ///////////////////////////////////////////////

  // read a Generator from file 

  g2.Read("data/simplemachine.gen");

  // create a Generator by reading a Generator file

  Generator g4("data/simplemachine.gen");


  // write a Generator to file 

  g4.Write("tmp_simplemachine.gen");

  // write a Generator to file with re-indexed states
  
  g4.ReindexOnWrite(true);
  g4.Write("tmp_simplemachine_ridx.gen");

  // read back files (testing token io)
  g4.Read("tmp_simplemachine.gen");
  g4.Read("tmp_simplemachine_ridx.gen");

  // debug output of Generator to console 

  std::cout << "################################\n";
  std::cout << "# tutorial, debug dump \n";
  g4.DWrite();
  std::cout << "################################\n";

  // create dotfile for further processing by graphviz 
  // (map state indices to begin with 1)

  g4.DotWrite("tmp_simplemachine.dot"); 
  g4.DDotWrite("tmp_simplemachine_debug.dot");

  // there also is a convenience method, that runs graphviz to
  // generate graphical output; requires  "dot" binary in $PATH
  try {
    g4.GraphWrite("tmp_simplemachinie.png");
  } catch(faudes::Exception& exception) {
    std::cout << "1_generator: cannot execute graphviz' dot. " << std::endl;
  } 


  // create a debug string for an event with symbolic name + index

  std::string str_singleevent = g1.EStr(2);

  // create a debug string for a state with symbolic name + index.
  // If there is no symblic name, a symbolic name of the index is constructed.

  std::string str_singlestate = g1.SStr(3);

  // build string of events in the Generator's alphabet

  std::string str_alph =  g1.AlphabetToString();

  // build string of states in the Generator's set of states
  std::string str_states = g1.StatesToString();

  // there also are TransRelToString(), InitStatesToString() and  MarkedStatesToString()  


  //////////////////////////////////////
  // Accessing the Generator's Members
  //////////////////////////////////////

  // get the Generator's name

  std::string str_name = g1.Name();

  // set new name for Generator

  g1.Name("NewName");



  // the core members alphabet, stateset and transitionrelation may be retrieved 
  // as const references; ie. they can be inspected freely, but write access is 
  // exclusively via the provided Generator methods.

  // retrieve a const reference to and copy of the Generator's alphabet

  const EventSet&  eset_ref_alph  = g1.Alphabet();
  EventSet         eset_copy_alph = g1.Alphabet();

  // you cannot alter the alphabet of a generator via an  alphabet method
  // eset_ref_alph.Insert("new_event");  // compile time error!

  // however, the copy can be altered, but with no effect on the original generator

  eset_copy_alph.Insert("new_event"); 
  if(g1.ExistsEvent("new_event")) std::cout << "### THIS CANNOT HAPPEN ###";

  // retrieve a const reference to and copy of the Generator's set of states "mStates"

  const StateSet& sset_ref_states = g1.States();
  StateSet sset_copy_states = g1.States();

  // retrieve a const reference to and a copy of the Generator's transition relation "mTransRel"

  const TransSet& tset_ref_trel = g1.TransRel();
  TransSet tset_copy_trel = g1.TransRel();

  // same with initial states and marked states

  const StateSet& sset_ref_istates = g1.InitStates();
  StateSet sset_copy_istates = g1.InitStates();

  const StateSet& sset_ref_mstates = g1.MarkedStates();
  StateSet sset_copy_mstates = g1.MarkedStates();



  //////////////////////////////////////////////////////////////////////////////
  // Modifying the 5-tuple Generator (X, Sigma, Delta, X0 and Xm)
  //////////////////////////////////////////////////////////////////////////////

  // insert an event by it's symbolic name in the alphabet
  // (if the event is not known so far, a new index for the symbolic name is generated) 
  g1.InsEvent("newevent");


  // insert an existing event into the Generator's alphabet (mAlphabet)
  // (by "existing event" we refer to an event that has been previously inserted to some Generator)

  g1.InsEvent(1);   // of course index 1 is already in the alphabet here...

  // insert a bunch of events (EventSet) and get the integer index if requested

  EventSet eset1;
  eset1.Insert("newevent1");
  Idx idx_tmp = eset1.Insert("newevent2");
  g1.InsEvents(eset1);

  // delete an event from Generator ie delete from alphabet and transition relation

  g1.DelEvent("newevent1"); // by symbolic name
  g1.DelEvent(idx_tmp);     // by index

  // delete a bunch of events
  // g1.DelEvents(eset1); // .. of course we have already deleted them before...

  // insert a new state. The state gets a integer index that is unique within
  // the Generator

  idx_tmp = g1.InsState();            // anonymous state
  idx_tmp = g1.InsState("newstate2"); // named state
  idx_tmp = g1.InsState("77"); // named state

  // insert a new state as initial state

  idx_tmp = g1.InsInitState();
  idx_tmp = g1.InsInitState("newinitstate");

  // ... same for marked states

  idx_tmp = g1.InsMarkedState();
  idx_tmp = g1.InsMarkedState("newmarkedstate");


  // delete single states from Generator ie stateset and transitionrelation

  g1.DelState(idx_tmp); // by index (relatively fast, for algorithms)
  g1.DelState("newinitstate"); // by symbolic name, if name assigned

  // delete a bunch of states 
  // (this should be more efficient than deleting states individually)

  StateSet stateset1;
  stateset1.Insert(1);
  stateset1.Insert(2);
  stateset1.Insert(3);
  g1.DelStates(stateset1);

  // for further proceeding we insert some new states and events...

  Idx idx_s10 = g1.InsState("s10");
  Idx idx_s11 = g1.InsState("s11");
  Idx idx_s12 = g1.InsState("s12");
  Idx idx_e10 = g1.InsEvent("e10");
  Idx idx_e11 = g1.InsEvent("e11");

  // set a state that already exists in Generator as initial state

  g1.SetInitState(idx_s10);

  // set a state that already exists in Generator as marked state

  g1.SetMarkedState(idx_s11);

  // unset an existing state as initial state (does not remove from mStates)

  g1.ClrInitState(idx_s10); 

  // unset an existing state as marked state (does not remove from stateset)

  g1.ClrMarkedState(idx_s10); 

  // clear all initial states (does not remove from stateset)

  //   g1.ClrInitStates();  // we do not really do it here, so it's commented

  // clear all marked states (mStates stays untouched)

  //   g1.ClrMarkedStates();  // we do not really do it here, so it's commented

  // set a transition for existing states and events

  g1.SetTransition(idx_s10, idx_e10, idx_s11); // by indices
  g1.SetTransition("s10", "e11", "s10"); // by symbolic names (slow)


  // report back to console

  std::cout << "################################\n";
  std::cout << "# tutorial, on the way ... \n";
  g1.Write();
  std::cout << "################################\n";


  // clear a transition (does not touch mStates, mInitStates and mMarkedStates)

  g1.ClrTransition(idx_s10, idx_e10, idx_s11); // by index

  // transitions can also be cleared by names (slower) or by an assigned
  // TransSet::Iterator (faster); use ClearTransRel() to remove all transitions


  // clear the symbolic name for a state in the StateSymbolTable

  g1.ClrStateName(idx_s10); 

  // exists event index/name in mAlphabet?

  bool bool_eventexists1 = g1.ExistsEvent("e11"); 
  bool bool_eventexists2 = g1.ExistsEvent(2); 


  // exists state in mStates?

  bool bool_stateexists1 = g1.ExistsState(4); 


  // check if a state is an initial state

  bool bool_initstateexists = g1.ExistsInitState(4); 

  // check if a state is a marked state

  bool bool_ismarkedstate = g1.ExistsMarkedState(4); 

  // look up event name for index in the EventSymbolTable of the event domain

  std::string str_eventname1 = g1.EventName(1);

  // look up event index for name in the EventSymbolTable of the event domain

  Idx idx_eventindex = g1.EventIndex("e11");

  // get symbolic name assigned to state (returns "" if no name assigned).

  std::string str_tmp = g1.StateName(idx_s10);

  // get index for symbolic state name. only possible for state names of states in
  // the Generator 

  idx_tmp = g1.StateIndex("s12");

  // clear Generator (including alphabet)

  g4.Clear();

  // get the number of events in the Generator's alphabet

  Idx idx_eventnum = g1.AlphabetSize();

  // get the number of states

  Idx idx_statenum = g1.Size();

  // get the number of transitions

  Idx idx_transnum = g1.TransRelSize();

  // there also are InitStatesSize(), MarkedStatesSize()

  // is the alphabet of the Generator empty?

  bool bool_alphempty = g1.AlphabetEmpty();

  // is the Generator empty (number of states == 0) ?

  bool bool_isempty = g1.Empty();

  // see also TransRelEmpty, InitStatesEmpty, MarkedStatesEmpty


  // insert a small loop 

  Idx initstate = g1.InsInitState("in");
  Idx markedstate = g1.InsMarkedState("out");
  g1.SetTransition("in","a","out");
  g1.SetTransition("out","a","in");


  // show effect on console 

  std::cout << "################################\n";
  std::cout << "# tutorial, after ins and del \n";
  g1.DWrite();
  std::cout << "################################\n";

  // record test case
  FAUDES_TEST_DUMP("g1, edited", g1); 

  ///////////////////////
  // Iterators
  ///////////////////////

  // since the core members are all implemented as sets, iterators
  // effectively are const_iterators, i.e. you cannot change the 
  // current value of an iterator. instead you may remove the value 
  // and insert the new value.

  // iteration over alphabet indices (member "mAlphabet")

  std::cout << "################################\n";
  std::cout << "# tutorial, iterators 1         \n";
  EventSet::Iterator eit;
  for (eit = g1.AlphabetBegin(); eit != g1.AlphabetEnd(); ++eit) {
    std::cout << "event \"" << g1.EventName(*eit) << "\" with index "<< *eit << std::endl;
  }
  std::cout << "################################\n";

  // iteration over state indices (member "mStates")

  std::cout << "################################\n";
  std::cout << "# tutorial, iterators 2         \n";
  StateSet::Iterator sit;
  for (sit = g1.StatesBegin(); sit != g1.StatesEnd(); ++sit) {
    std::cout << *sit << std::endl;
  }
  std::cout << "################################\n";

  // iteration over complete transition relation (member "mTransRel")

  std::cout << "################################\n";
  std::cout << "# tutorial, iterators 3         \n";
  TransSet::Iterator tit;
  for (tit = g1.TransRelBegin(); tit != g1.TransRelEnd(); ++tit) {
    std::cout << g1.TStr(*tit) << std::endl;
  }
  std::cout << "################################\n";

  // iteration over transitions from a given state; note that we avoid
  // computation of the end of the iteration in every step

  std::cout << "################################\n";
  std::cout << "# tutorial, iterators 4         \n";
  idx_tmp = g1.StateIndex("s1");
  TransSet::Iterator tit_end;
  tit = g1.TransRelBegin(idx_tmp);
  tit_end = g1.TransRelEnd(idx_tmp);
  for (; tit != tit_end; ++tit) {
    std::cout << g1.TStr(*tit) << std::endl;
  }
  std::cout << "################################\n";

  // variations: transitions of given state index + given event index:
  // TransRelBegin(x1, ev) - TransRelEnd(x1, ev)

  // iteration over initial and marked states:
  // InitStatesBegin() - InitStatesEnd()  (member "mInitStates")
  // MarkedStatesBegin() - MarkedStatesEnd() (member "mMarkedStates")


  ////////////////////////////////////////////////////////////
  // retrieve copies of the Generator's transition releation
  // in different sorting orders than X1 -> Ev -> X2
  ////////////////////////////////////////////////////////////

  // note: the availabity of iterator ranges depends on the sorting order;
  // eg iteration with specified x2 requires X2->Ev->X1 or X2->X1->Ev sorting.

  // retrieve a copy that is sorted by X2 -> Ev -> X1 by the binary
  // predicate TransSort::X2EvX1. 
  
  TransSetX2EvX1 tset_x2evx1;
  g1.TransRel(tset_x2evx1);

  // report to console

  std::cout << "################################\n";
  std::cout << "# tutorial, x2-ev-x1 sorting\n";
  TransSetX2EvX1::Iterator tit2;
  for (tit2 = tset_x2evx1.Begin(); tit2 != tset_x2evx1.End(); ++tit2) {
    std::cout << g1.TStr(*tit2) << std::endl;
  }
  std::cout << "################################\n";



  ////////////////////////
  // Convenience Methods
  ////////////////////////

  // remove all events from mAlphabet, that do not have a transition in
  // mTransRel:  g1.MinimizeAlphabet()

  // get an EventSet containing all the events that drive some transition 

  EventSet eset_usedevents = g1.UsedEvents();

  // get an EventSet containing all the events that do not drive any transition

  EventSet eset_unusedevents = g1.UnusedEvents();

  // return the active event set at a given state 

  EventSet eset_activeeventset = g1.ActiveEventSet(idx_s12);

  // return a StateSet containing all the states that are connected by
  // some transition 

  StateSet sset_trel_sspace = g1.TransRelStates();

  // return a StateSet containing all the successor states of a given predecessor
  // state.

  StateSet sset_successors = g1.SuccessorStates(idx_s12);

  // note: if you need predecessor states, use a resorted transition relation

  /////////////////////////////////
  // Symbolic state name handling
  /////////////////////////////////

  // are symbolic state names enabled? depending on this boolean value
  // library functions like Determine or StateMin may create symbolic
  // state names automatically

  bool bool_statenamesenabled = g1.StateNamesEnabled();

  // disable state name creation in resulting generators for functions in
  // the faudes library, that support this feature (nearly all) with
  // "false"; enable state name creation with "true".

  g1.StateNamesEnabled(true); // anyway .. true is the default value

  // clear existing symbolic statenames for states in the Generator

  // g1.ClearStateNames(); 

  // set symbolic names for all states in the generator. the symbolic name becomes
  // the equivalent string representation of the state's integer index. This is
  // only usefull for debugging purposes.

  g1.SetDefaultStateNames();


  // show effect on console 

  std::cout << "################################\n";
  std::cout << "# tutorial, default names \n";
  g1.Write();
  std::cout << "################################\n";


  ///////////////////////////////////
  // Accessible, Coaccessible, Complete, Trim
  ///////////////////////////////////

  // read example generator for reachability analysis
  Generator greach("data/trimness_nottrim.gen");

  std::cout << "################################\n";
  std::cout << "# tutorial, reachability test case  \n";
  greach.Write();
  std::cout << "# tutorial, reachability relevant sets \n";
  StateSet astates = greach.AccessibleSet();
  StateSet cstates = greach.CoaccessibleSet();
  StateSet tstates = greach.TerminalStates();
  astates.Write();
  cstates.Write();
  tstates.Write();   
  std::cout << "# tutorial, reachability analysis \n";
  bool isacc = greach.IsAccessible(); 
  if(isacc) 
    std::cout << "accesibility: ok [error]\n";
  else
    std::cout << "accesibility: failed [expected]\n";
  bool iscoacc = greach.IsCoaccessible(); 
  if(iscoacc) 
    std::cout << "coaccesibility: ok [error]\n";
  else
    std::cout << "coaccesibility: failed [expected]\n";
  bool iscompl = greach.IsComplete(); 
  if(iscompl) 
    std::cout << "completeness: ok [error]\n";
  else
    std::cout << "completeness: failed [expected]\n";
  bool istrim = greach.IsTrim(); 
  if(istrim) 
    std::cout << "trimness: ok [error]\n";
  else
    std::cout << "trimness: failed [expected]\n";
  bool isotrim = greach.IsOmegaTrim(); 
  if(isotrim) 
    std::cout << "w-trimness: ok [error]\n";
  else
    std::cout << "w-trimness: failed [expected]\n";
  std::cout << "################################\n";
  
  // record test case
  FAUDES_TEST_DUMP("acc",astates);
  FAUDES_TEST_DUMP("coacc",cstates);
  FAUDES_TEST_DUMP("term",tstates);

  // Make the Generator accessible by removing transitions and states. 
  // The routine returns true if the generator has an initial state.
  Generator gaccess(greach);
  gaccess.Name("GAccessible");
  bool bool_hasinit = gaccess.Accessible();

  // Make the Generator coaccessible by removing transitions and states.
  // The routine returns true if the generator has a marked state.
  Generator gcoaccess(greach);
  gcoaccess.Name("GCoccessible");
  bool bool_hasmarked = gcoaccess.Coaccessible();

  // Make the Generator complete by removing transitions and states. 
  // The routine returns true if the generator has an initial state.
  Generator gcompl(greach);
  gcompl.Name("GComplete");
  gcompl.Complete();

  // Make the Generator trim by removing transitions and states. 
  // The routine returns true if the generator has an initial state
  // and a marked state.
  Generator gtrim(greach);
  gtrim.Name("GTrim");
  bool bool_isnontrivial = gtrim.Trim();

  // Make the Generator omega-trim by removing transitions and states. 
  // The routine returns true if the generator has an initial state
  // and a marked state.
  Generator gotrim(greach);
  gotrim.Name("GOmegaTrim");
  gotrim.OmegaTrim();

  // show effect on console
  std::cout << "################################\n";
  std::cout << "# tutorial, reachability results \n";
  gaccess.Write();
  gcoaccess.Write();
  gcompl.Write();
  gtrim.Write();
  gotrim.Write();
  std::cout << "################################\n";

  // contribute to html docu
  greach.Write("tmp_greach.gen");
  gaccess.Write("tmp_gaccess.gen");
  gcoaccess.Write("tmp_gcoaccess.gen");
  gcompl.Write("tmp_gcompl.gen");
  gtrim.Write("tmp_gtrim.gen");
  gotrim.Write("tmp_gotrim.gen");

  // Test protocol
  FAUDES_TEST_DUMP("accessible",gaccess);
  FAUDES_TEST_DUMP("coaccessible",gcoaccess);
  FAUDES_TEST_DUMP("complete",gcompl);
  FAUDES_TEST_DUMP("trim",gtrim);
  FAUDES_TEST_DUMP("omega trim",gotrim);
  

  ///////////////////////////////////
  // Generalized Completeness
  ///////////////////////////////////

  // read example generator for generalized completeness
  Generator gsigcomplB("data/trimness_nottrim.gen");
  Generator gsigcomplC("data/trimness_nottrim.gen");
  EventSet sigoB;
  sigoB.FromString("<A> b </A>");
  EventSet sigoC;
  sigoC.FromString("<A> c </A>");

  std::cout << "################################\n";
  std::cout << "# tutorial, sigma_o-completeness test case  \n";
  greach.Write();
  bool issigcompl = gsigcomplB.IsComplete(sigoB); 
  if(iscompl) 
    std::cout << "completeness: ok [error]\n";
  else
    std::cout << "completeness: failed [expected]\n";
  

  // Make the Generator complete by removing transitions and states. 
  // The routine returns true if the generator has an initial state.
  gsigcomplB.Name("GSigoCompleteB");
  gsigcomplB.Complete(sigoB);
  gsigcomplC.Name("GSigoCompleteC");
  gsigcomplC.Complete(sigoC);

  // record test case
  FAUDES_TEST_DUMP("iscmpl",issigcompl);
  FAUDES_TEST_DUMP("gsigcomplB",gsigcomplB);
  FAUDES_TEST_DUMP("gsigcomplC",gsigcomplC);

  // contribute to html docu
  gsigcomplB.Write("tmp_gsigcomplb.gen");
  gsigcomplC.Write("tmp_gsigcomplc.gen");


  ///////////////////////////////////
  // Test case evaluation
  ///////////////////////////////////
  FAUDES_TEST_DIFF();

  return 0;
}




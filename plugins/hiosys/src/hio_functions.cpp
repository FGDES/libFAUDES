/** @file hio_functions.cpp Algorithms for hierarchical discrete event systems with inputs and outputs */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/

// note: deterministic generators only 

// todo: exceptions on invalid input data if FAUDES_CHECKED
// todo: files eg: normal. cycles. hiotest. hiosynth.
// todo: implement routines that check hio properties
// todo: only use statenames if statenamesenabled (eg HioFreeInput)

// todo: other todos below

#include "hio_functions.h"
#include "syn_include.h"


namespace faudes {


// This is Sebastian'y version of SupNorm with a special
// marking of the result. It also uses a different order
// of arguments

//  SupNormSP(rL,rOAlph,rK,rResult)
bool SupNormSP(
  Generator& rL,
  const EventSet& rOAlph,
  const Generator& rK, 
  Generator& rResult)
{
  FD_DF("SupNorm(" << rK.Name() << "," << rL.Name() << "," << rOAlph.Name() << ")");
  
  NormalityConsistencyCheck(rL,rOAlph,rK);
  
  // prepare result
  rResult.Clear();
  
  //extract overall alphabet:
  EventSet allevents;
  allevents=rL.Alphabet();
  
  //extract alphabet of rK:
  EventSet Kevents=rK.Alphabet();
  
  // record name and marked states of rL
  std::string rLname=rL.Name();
  StateSet rLmarked=rL.MarkedStates();
  
  // involved operations from cfl_regular.h operate on the marked
  // languages -> turn generated languages into marked langs
  rL.InjectMarkedStates(rL.States());
  Generator prK=rK;
  prK.InjectMarkedStates(prK.States());
  rResult = prK;
  
  // calculate "L-K" (ToDo: use LanguageDifference):
  LanguageComplement(rResult);
  FD_DF("SupNorm: size of complement(K): "<<rResult.Size()<<",marked: "<<rResult.MarkedStatesSize());
  Generator tempgen;
  LanguageIntersection(rL, rResult, tempgen);
  rResult.Clear();
  FD_DF("SupNorm: sizeof L U complement(K)=L-K: "<<tempgen.Size());
  
  // calculate Pinv(P(L-K)):
  Project(tempgen,rOAlph,rResult);
  tempgen.Clear();
  FD_DF("SupNorm: sizeof p(L-K): "<< rResult.Size());
  InvProject(rResult,allevents);
  FD_DF("SupNorm: sizeof pinv(p(L-K)): "<<rResult.Size());
  
  //  FD_DF("SupNorm: sizeof pinv(p(L-K)): "<<rResult.Size());

  //calculate remaining set difference -> supnorm(K)
  LanguageComplement(rResult);
  LanguageIntersection(prK, rResult, tempgen);
  rResult=tempgen;
  tempgen.Clear();
  FD_DF("SupNorm: sizeof K - pinv(p(L-K)) (result): "<<rResult.Size());
  
  // language MARKED by rResult is the result -> remove blocking states, mark all remaining states.
  rResult.Trim();
  rResult.InjectMarkedStates(rResult.States());
  
  // restore original rL
  rL.Name(rLname);
  rL.InjectMarkedStates(rLmarked);
  
  return !( rResult.InitStatesEmpty() );
}






// CompleteSynth(rPlant,rCAlph,rSpec,rClosedLoop)
bool CompleteSynth(
  const Generator& rPlant,
  const EventSet rCAlph, 
  const Generator& rSpec, 
  Generator& rClosedLoop) { 
  FD_DF("CompleteSynth(" << rPlant.Name() << "," << rSpec.Name()<< ")");

  // prepare result
  rClosedLoop.Clear();
  
  //check for trivial result
	if(rSpec.InitStatesEmpty()){
	    FD_DF("CompleteSynth::empty language specification: empty closed loop.");
		rClosedLoop.Name("CompeleteSynth("+rPlant.Name()+", "+rSpec.Name()+")");
	    return false; 
	}  
	
	// spec as candidate for closed loop
	Generator ClosedLoopCand=rSpec;

	// iterate supcon and dead end elimination
	while(true) {
		// try std synthesis
		FD_DF("CompleteSynth:: spec size " << ClosedLoopCand.Size());
		
		// try SupCon
		SupConClosed(rPlant, rCAlph, ClosedLoopCand, rClosedLoop);
		//std::cout<<std::endl<<"CompleteSynth:: current result after SupCon: tmp_ContLoop.gen";
		FD_DF("CompleteSynth: size of current result after SupCon: "<<rClosedLoop.Size());
		//std::cout<<" (Size: "<<rClosedLoop.Size()<<").";
		
		//check for empty result
		if(rClosedLoop.InitStatesEmpty()) {
			FD_DF("CompleteSynth:: failed: empty language result.");
			rClosedLoop.StateNamesEnabled(false);
			rClosedLoop.Name("CompeleteSynth("+rPlant.Name()+", "+rSpec.Name()+")");
			return false;
		}
		  
		FD_DF("CompleteSynth:: candidate size " << rClosedLoop.Size());
		
		// find dead ends  and return success if no dead end found
		StateSet dead_ends=rClosedLoop.TerminalStates(rClosedLoop.AccessibleSet());
		if(dead_ends.Empty()) {
			rClosedLoop.StateNamesEnabled(false);
			FD_DF("CompleteSynth:: done");
			rClosedLoop.Name("CompeleteSynth("+rPlant.Name()+", "+rSpec.Name()+")");
			return true;
		}
		
		// report to console
		FD_DF("CompleteSynth:: eliminating following " << dead_ends.Size() << " dead ends");
		FD_DF(rClosedLoop.StateSetToString(dead_ends));
		
		// eliminate dead ends.
		rClosedLoop.DelStates(dead_ends);
		// becomes new candidate for SupCon
		ClosedLoopCand=rClosedLoop;
		
		//std::cout<<std::endl<<"CompleteSynth:: current result after Complete: tmp_CompleteLoop.gen";
		FD_DF("CompleteSynth:: size of result after removing dead ends: "<<rClosedLoop.Size());
		//std::cout<<" (Size: "<<rClosedLoop.Size()<<").";

		//check for empty result
		if(rClosedLoop.InitStatesEmpty()) {
			FD_DF("CompleteSynth:: failed: empty language result.");
			rClosedLoop.Name("CompeleteSynth("+rPlant.Name()+", "+rSpec.Name()+")");
			return false;
		}
		rClosedLoop.Clear();
	}
}

// NormalCompleteSynth(rPlant,rCAlph,rOAlph,rSpec,rClosedLoop)
bool NormalCompleteSynth(
  Generator& rPlant, 
  const EventSet& rCAlph, 
  const EventSet& rOAlph,
  const Generator& rSpec,
  Generator& rClosedLoop) 
{ 
  FD_DF("NormalCompleteSynth(" << rPlant.Name() << "," << rSpec.Name()<< ")");
  
  // prepare result
  rClosedLoop.Clear();
  
  // spec as candidate for closed loop
  Generator Spec=rSpec;
  
  while(true) {
	  
	  // try CompleteSynth and check for empty result:
	  if(!CompleteSynth(rPlant,rCAlph,Spec,rClosedLoop)) {
		FD_DF("NormalCompleteSynth:: failed: empty result after CompleteSynth().");
	    std::cout<<std::endl<<"NormalCompleteSynth:: failed: empty result after CompleteSynth().";
		rClosedLoop.Name("NormalCompleteSynth(" + rPlant.Name() + "," + rSpec.Name()+ ")");
	    return false;
	  }
	 
	  // check for normality and return on success
	  if(IsNormal(rPlant,rOAlph,rClosedLoop)) {
	    FD_DF("NormalCompleteSynth:: candidate supervisor is normal. success.");
		rClosedLoop.Name("NormalCompleteSynth(" + rPlant.Name() + "," + rSpec.Name()+ ")");
	    return true;
	  }
	  
	  //  supremal normal sublanguage of rClosedLoop as new spec
	  // return false on empty result
	  FD_DF("NormalCompleteSynth:: candidate supervisor not normal. running SupNorm");
	  if(!SupNormSP(rPlant, rOAlph, rClosedLoop, Spec)) {
	    FD_DF("NormalCompleteSynth:: failed: empty result after SupNorm().");
	    std::cout<<std::endl<<"NormalCompleteSynth:: failed: empty result after SupNorm().";
		rClosedLoop.Clear();
		rClosedLoop.Name("NormalCompleteSynth(" + rPlant.Name() + "," + rSpec.Name()+ ")");
	    return false;
	  }  
	  
	  Spec.StateNamesEnabled(false);
	  //std::cout<<std::endl<<"NormalCompleteSynth:: current result after SupNorm():";
	  //std::cout<<" Size: "<<Spec.Size()<<".";
	  FD_DF("NormalCompleteSynth:: size of result after SupNorm(): "<<Spec.Size());
	}
}


// NormalCompleteSynthNB(rPlant,rCAlph,rOAlph,rSpec,rClosedLoop)
bool NormalCompleteSynthNB(
  Generator& rPlant, 
  const EventSet& rCAlph, 
  const EventSet& rOAlph,
  const Generator& rSpec,
  Generator& rClosedLoop) 
{ 
  FD_DF("NormalCompleteSynth(" << rPlant.Name() << "," << rSpec.Name()<< ")");
  
  // prepare result
  rClosedLoop.Clear();

  // spec as candidate for closed loop
  Generator Spec=rSpec;
  while(true) {
	  
	  // try NormalCompleteSynth and check for empty result:
	  if(!NormalCompleteSynth(rPlant,rCAlph,rOAlph,Spec,rClosedLoop)) {
	    FD_DF("NormalCompleteSynthNB:: failed: empty result after NormalCompleteSynth().");
	    std::cout<<std::endl<<"NormalCompleteSynthNB:: failed: empty result after NormalCompleteSynth().";
		rClosedLoop.Name("NormalCompleteSynthNB(" + rPlant.Name() + "," + rSpec.Name()+ ")");
	    return false;
	  }
	  
		// check for coaccessibility and return on success
	  if(rClosedLoop.IsTrim()) {
	    FD_DF("NormalCompleteSynthNB:: candidate supervisor is trim. success.");
		rClosedLoop.Name("NormalCompleteSynthNB(" + rPlant.Name() + "," + rSpec.Name()+ ")");
	    return true;
	  }
	  
	  // supremal nonblocking sublanguage of rClosedLoop as new Spec
	  // return false on empty result
	  FD_DF("NormalCompleteSynthNB:: candidate supervisor not Trim. running Trim()");
	  Spec=rClosedLoop; 
	  if(!Spec.Trim()) {
			FD_DF("NormalCompleteSynthNB:: failed: empty result after Trim().");
			std::cout<<std::endl<<"NormalCompleteSynthNB:: failed: empty result after Trim().";
			rClosedLoop.Clear(); 
			rClosedLoop.Name("NormalCompleteSynthNB(" + rPlant.Name() + "," + rSpec.Name()+ ")");
			return false;
	    }
	  //std::cout<<std::endl<<"NormalCompleteSynthNB:: current result after Trim():";
	  //std::cout<<" Size: "<<Spec.Size()<<".";
	  FD_DF("NormalCompleteSynthNB:: size of result after Trim():"<<Spec.Size());
    }
}


// HioSortCL(const EventSet& rYc,rUc,rYp,rUP,rYe,rUe)
Generator HioSortCL(
  const EventSet& rYc, 
  const EventSet& rUc,
  const EventSet& rYp, 
  const EventSet& rUp,
  const EventSet& rYe,
  const EventSet& rUe)
{
    FD_DF("HioSortCL(...)");

	#ifdef FAUDES_CHECKED
	// check for nonempty sets
	if (rYc.Empty()||rUc.Empty()||rYp.Empty()||rUp.Empty()||rYe.Empty()||rUe.Empty()){
	    std::stringstream errstr;
	    errstr << "At least one empty parameter.";
	    throw Exception("HioSortCL", errstr.str(), 0);
	}
	#endif
	
	// create resulting alphabet while checking for disjoint sets
	EventSet alphabet,errevents;
	// initialize with rYc
	alphabet.InsertSet(rYc);
	// insert remaining events one by one and check if new
    EventSet::Iterator evit;
	// rUc
    for (evit = rUc.Begin(); evit != rUc.End(); ++evit) {
		if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
    }
	// rYp
    for (evit = rYp.Begin(); evit != rYp.End(); ++evit) {
		if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
    }
	// rUp
    for (evit = rUp.Begin(); evit != rUp.End(); ++evit) {
		if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
    }
	// rYe
    for (evit = rYe.Begin(); evit != rYe.End(); ++evit) {
		if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
    }
	// rUe
    for (evit = rUe.Begin(); evit != rUe.End(); ++evit) {
		if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
    }
	
	#ifdef FAUDES_CHECKED
	// throw exception on non disjoint alphabets
	if (!errevents.Empty()){
		std::stringstream errstr;
		errstr << "Non-disjoint parameters; ambiguous events:\n" <<errevents.ToString();
		throw Exception("HioSortCL", errstr.str(), 0);
	}
	#endif
	
	// create result
    Generator ResGen;
	
    ResGen.Name("HioSortCL("+rYc.Name()+","+rUc.Name()+","+rYp.Name()+","+rUp.Name()+","+rYe.Name()+","+rUe.Name()+")");
	
    ResGen.InjectAlphabet(alphabet);
	alphabet.Clear();
	
    // 5 marked states with initial state 1:
    ResGen.InsInitState("Yp_or_Ye");
    ResGen.SetMarkedState("Yp_or_Ye");
    ResGen.InsMarkedState("Yc_or_Up");
    ResGen.InsMarkedState("Uc");
    ResGen.InsMarkedState("Up");
    ResGen.InsMarkedState("Ue");
    
    // set transitions according to desired ioSorting structure:
    // all Yp-events lead from state 1 to state 2:
    for (evit = rYp.Begin(); evit != rYp.End(); ++evit) {
      ResGen.SetTransition("Yp_or_Ye",rYp.SymbolicName(*evit),"Yc_or_Up");
    }
    // all Yc-events lead from state 2 to state 3:
    for (evit = rYc.Begin(); evit != rYc.End(); ++evit) {
      ResGen.SetTransition("Yc_or_Up",rYc.SymbolicName(*evit),"Uc");
    }
    // all Uc-events lead from state 3 to state 4:
    for (evit = rUc.Begin(); evit != rUc.End(); ++evit) {
      ResGen.SetTransition("Uc",rUc.SymbolicName(*evit),"Up");
    }
    // all Up-events lead from state 2 and 4 to state 1:
    for (evit = rUp.Begin(); evit != rUp.End(); ++evit) {
      ResGen.SetTransition("Yc_or_Up",rUp.SymbolicName(*evit),"Yp_or_Ye");
      ResGen.SetTransition("Up",rUp.SymbolicName(*evit),"Yp_or_Ye");
    }
    // all Ye-events lead from state 1 to state 5:
    for (evit = rYe.Begin(); evit != rYe.End(); ++evit) {
      ResGen.SetTransition("Yp_or_Ye",rYe.SymbolicName(*evit),"Ue");
    }
    // all Ue-events lead from state 5 to state 1:
    for (evit = rUe.Begin(); evit != rUe.End(); ++evit) {
      ResGen.SetTransition("Ue",rUe.SymbolicName(*evit),"Yp_or_Ye");
    }
    return ResGen;
} 

// HioFreeInput(rGen,rInput,rOutput,rResGen,rErrState1,rErrState2,rErrState1Idx,rErrState2Idx) 
void HioFreeInput(
  const Generator& rGen, 
  const EventSet& rInput, 
  const EventSet& rOutput,
  Generator& rResGen,
  const std::string& rErrState1, 
  const std::string& rErrState2,
  Idx& rErrState1Idx,
  Idx& rErrState2Idx) {
  FD_DF("HioFreeInput( [generator] " << rGen.Name() << ")");

  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen) {
    pResGen= rResGen.New();
  }  
 
  #ifdef FAUDES_CHECKED
  // exception on empty Input-alphabet or non-disjoint Input and Output
  if(rInput.Empty() || !((rInput*rOutput).Empty())) {
	    std::stringstream errstr;
	    errstr << "Empty Input-alphabet or non-disjoint Input and Output.\n";
	    errstr << "Input: "<< rInput.ToString()<<"\n";
	    errstr << "Output: "<< rOutput.ToString()<<"\n";
	    errstr << "Input*Output: "<< (rInput*rOutput).ToString()<<"\n";
	    throw Exception("HioFreeInput", errstr.str(), 0);	
    }
  #endif
  
  // check for enabled state names:
  bool StateNamesEnabled = pResGen->StateNamesEnabled();
  // Warning on inconsistency with error-state names
  if(!StateNamesEnabled && (!rErrState1.empty()||!rErrState2.empty())) {
	  FD_WARN("HioFreeInput: state names disabled in rResGen: error state names ignored.");
	}
  
  // prepare result, preserve StateNamesEnabled of rGen
  *pResGen=rGen;
  pResGen->StateNamesEnabled(StateNamesEnabled);
  pResGen->Name("HioFreeInput("+rGen.Name()+")");
  
  //create first error state, where inserted Input-events shall lead to:
  std::string errstate1;
  if(StateNamesEnabled) {
    errstate1=pResGen->UniqueStateName(rErrState1);	
    rErrState1Idx=pResGen->InsMarkedState(errstate1);
  }
  else {
    rErrState1Idx=pResGen->InsMarkedState();
  }

  //find Input-states
  StateSet::Iterator sit=pResGen->StatesBegin();
  EventSet::Iterator evit;
  bool InputInserted = false;
  for(;sit!=pResGen->StatesEnd();sit++){
    EventSet disabledInput;
    disabledInput = rInput - pResGen->ActiveEventSet(*sit);
    //if at least one Input-event is enabled AND at least one Input-event is disabled:
    //insert missing Input-transitions to error state
    if( (disabledInput!=rInput) && !disabledInput.Empty()) {
      FD_DF("HioFreeInput: " << *sit << " disabledInput: " << disabledInput.ToString()); 
      InputInserted = true;
      for (evit = disabledInput.Begin(); evit != disabledInput.End(); ++evit) {
        pResGen->SetTransition(*sit,*evit,rErrState1Idx);
      }       
    }
  }//for-loop through states

  //insert err_Input-state and connect both err-states with Input-Output-loop in case missing Input-events were inserted:
  if(InputInserted) {
	  if(StateNamesEnabled) {
		 FD_DF("HioFreeInput(...): inserted error state 1: " << errstate1 << " with Idx: " << rErrState1Idx);
		}
	  else {
		 FD_DF("HioFreeInput(...): inserted error state 1 with Idx: " << rErrState1Idx);
		}
	  if(!rOutput.Empty()) { // this 'if' and the 'if' before are seperated because of the 'else' below !
		   //create second error state
		   std::string errstate2;
		   if(StateNamesEnabled) {
			   errstate2=pResGen->UniqueStateName(rErrState2);
			   rErrState2Idx=pResGen->InsMarkedState(errstate2);
			   FD_DF("HioFreeInput(...): inserted error state 2: " << errstate2);
			}
		    else {
			    rErrState2Idx=pResGen->InsMarkedState();
			    FD_DF("HioFreeInput(...): inserted error state 2 with Idx: " << rErrState2Idx);
			}
			   
			   //insert Input-events:
			   for (evit = rInput.Begin(); evit != rInput.End(); ++evit) {
			      pResGen->SetTransition(rErrState2Idx,*evit,rErrState1Idx);
			   }
			   //insert output-events:
			   for (evit = rOutput.Begin(); evit != rOutput.End(); ++evit) {
			     pResGen->SetTransition(rErrState1Idx,*evit,rErrState2Idx);
			   }
		}
	}
   //else delete err_Output-state
   else {
     pResGen->DelState(rErrState1Idx);
    }
   // if necessary, move pResGen to rResGen
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

// HioFreeInput(rGen,rInput,rOutput,rResGen,rErrState1,rErrState2) 
void HioFreeInput(
  const Generator& rGen, 
  const EventSet& rInput, 
  const EventSet& rOutput,
  Generator& rResGen,
  const std::string& rErrState1, 
  const std::string& rErrState2) {
  Idx errstate1,errstate2;
  HioFreeInput(rGen,rInput,rOutput,rResGen,rErrState1,rErrState2,errstate1,errstate2);
}

// HioFreeInput(rGen,rInput,rOutput,rResGen) 
void HioFreeInput(
  const Generator& rGen, 
  const EventSet& rInput, 
  const EventSet& rOutput,
  Generator& rResGen)
{
  //Call HioFreeInput with empty names for error states
  std::string ErrState1,ErrState2;
  HioFreeInput(rGen,rInput,rOutput,rResGen,ErrState1,ErrState2);
}

// HioFreeInput(HioPlant,HioPlant) 
void HioFreeInput(
  const HioPlant& rPlant,
  HioPlant& rResPlant) 
{

  FD_DF("HioFreeInput( [plant] " << rPlant.Name() << ")");

  // Call HioFreeInput with certain names for error state,
  // set Err-flag
  Idx errstateUp,errstateUe,errstate2;
  std::string ErrState1,ErrState2;
  
  EventSet NoOutput,uP,uE,yP,yE;  
  uP=rPlant.UpEvents();
  uE=rPlant.UeEvents();
  yP=rPlant.YpEvents();
  yE=rPlant.YeEvents();
  
  ErrState1="UpError";
  HioFreeInput(rPlant,uP,NoOutput,rResPlant,ErrState1,ErrState2,errstateUp,errstate2);
  
  ErrState1="UeError";
  HioFreeInput(rResPlant,uE,NoOutput,rResPlant,ErrState1,ErrState2,errstateUe,errstate2);
  
  // set HioStateFlag Err
  if(rResPlant.ExistsState(errstateUp)) rResPlant.SetErr(errstateUp);
  if(rResPlant.ExistsState(errstateUe)) rResPlant.SetErr(errstateUe);
  
  // restore event attributes
  rResPlant.SetYp(yP);
  rResPlant.SetUp(uP);
  rResPlant.SetYe(yE);
  rResPlant.SetUe(uE);
  
}

// HioFreeInput(HioController,HioController)   
void HioFreeInput(
  const HioController& rController,
  HioController& rResController)  
{
  FD_DF("HioFreeInput( [controller] " << rController.Name() << ")");

  HioController* pResController = &rResController;
  if(&rResController== &rController) {
    pResController=rResController.New();
  }  
  
  // check for enabled state names:
  bool StateNamesEnabled = pResController->StateNamesEnabled();
  
  // prepare error state names
  std::string errStr1, errStr2;
  
  // prepare error state indeces
  Idx errIdx1, errIdx2;
  
  // prepare result, preserve StateNamesEnabled of rGen
  pResController->Assign(rController);
  //*pResController=rController; // didn't work (should work ... check!!)
  //pResController=&rController; // didn't work (cannot work anyway)
  pResController->StateNamesEnabled(StateNamesEnabled);
  pResController->Name("HioFreeInput("+rController.Name()+")");
  
  // figure input alphabets
  EventSet UcEvents = rController.UcEvents();
  EventSet YpEvents = rController.YpEvents();
  EventSet YcEvents = rController.YcEvents();
  EventSet UpEvents = rController.UpEvents();
  
	//create first error state, where inserted Uc- or Yp-events shall lead to:
  if(StateNamesEnabled) {
	errStr1=pResController->UniqueStateName("UcYpError");	
	errIdx1=pResController->InsMarkedState(errStr1);
  }
  else {
  errIdx1=pResController->InsMarkedState();
  }
  // set HioStateFlag Err
  pResController->SetErr(errIdx1);
  
  // set state attribute QUp
  pResController->SetQUp(errIdx1);
  
  //find Input-states
  StateSet::Iterator sit=pResController->StatesBegin();
  EventSet::Iterator evit;
  bool InputInserted = false;
  for(;sit!=pResController->StatesEnd();sit++){
    EventSet disabledInput, active;
	active = pResController->ActiveEventSet(*sit);
	// first treat Uc-events
    disabledInput = UcEvents - active;
    //if at least one Input-event is enabled AND at least one Input-event is disabled:
    //insert missing Input-transitions to error state
    if( (disabledInput!=UcEvents) && !disabledInput.Empty()) {
      // FD_DF("HioFreeInput: " << *sit << " disabledInput: " << disabledInput.ToString());
      InputInserted = true;
      for (evit = disabledInput.Begin(); evit != disabledInput.End(); ++evit) {
        pResController->SetTransition(*sit,*evit,errIdx1);
      }       
    }
	// then treat Yp-events
	disabledInput = YpEvents - active;
    //if at least one Input-event is enabled AND at least one Input-event is disabled:
    //insert missing Input-transitions to error state
    if( (disabledInput!=YpEvents) && !disabledInput.Empty()) {
      // FD_DF("HioFreeInput: " << *sit << " disabledInput: " << disabledInput.ToString());
      InputInserted = true;
      for (evit = disabledInput.Begin(); evit != disabledInput.End(); ++evit) {
        pResController->SetTransition(*sit,*evit,errIdx1);
      }       
    }
  }//for-loop through states

  //insert err_Input-state and connect both err-states with Input-Output-loop in case missing Input-events were inserted:
  if(InputInserted) {
     //create second error state
	 std::string errstate2;
	 if(StateNamesEnabled) {
		 FD_DF("HioFreeInput(...): inserted error state 1: " << errStr1);
		 errStr2=pResController->UniqueStateName("ErrorQYp");	
		 errIdx2=pResController->InsMarkedState(errStr2);
		 FD_DF("HioFreeInput(...): inserted error state 2: " << errStr2);
		}
	 else {
		 FD_DF("HioFreeInput(...): inserted error state 1 with Idx: " << errIdx1);
		 errIdx2=pResController->InsMarkedState();
		 FD_DF("HioFreeInput(...): inserted error state 2 with Idx: " << errIdx2);
		}
	 
	 // set HioStateFlag Err
	 pResController->SetErr(errIdx2);
	 // set HioStateFlag QYp
	 pResController->SetQYp(errIdx2);
	 
     //insert Up-events:
     for (evit = UpEvents.Begin(); evit != UpEvents.End(); ++evit) {
		 pResController->SetTransition(errIdx1,*evit,errIdx2);
		}
	 //insert Yp-events:
     for (evit = YpEvents.Begin(); evit != YpEvents.End(); ++evit) {
		 pResController->SetTransition(errIdx2,*evit,errIdx1);
		}
	}
	
   //else delete error-state 1
   else {
     pResController->DelState(errIdx1);
    }

   // if necessary, move pResGen to rResGen
  if(pResController != &rResController) {
    pResController->Move(rResController);
    delete pResController;
  }	
   // restore event attributes
   rResController.SetYc(YcEvents);
   rResController.SetUc(UcEvents);
   rResController.SetYp(YpEvents);
   rResController.SetUp(UpEvents);
}
  
// HioFreeInput(HioEnvironment,HioEnvironment)  
void HioFreeInput(
  const HioEnvironment& rEnvironment,
  HioEnvironment& rResEnvironment) 
{
  FD_DF("HioFreeInput( [environment] " << rEnvironment.Name() << ")");

  HioEnvironment* pResEnvironment = &rResEnvironment;
  if(&rResEnvironment== &rEnvironment) {
    pResEnvironment=rResEnvironment.New();
  }  
  
  // check for enabled state names:
  bool StateNamesEnabled = pResEnvironment->StateNamesEnabled();
  
  // prepare error state names
  std::string errStr1, errStr2;
  
  // prepare error state indeces
  Idx errIdx1, errIdx2;
  
  // prepare result, preserve StateNamesEnabled of rGen
  pResEnvironment->Assign(rEnvironment);
  //*pResEnvironment=rEnvironment; // didn't work
  //pResEnvironment=&rEnvironment; // didn't work
  pResEnvironment->StateNamesEnabled(StateNamesEnabled);
  pResEnvironment->Name("HioFreeInput("+rEnvironment.Name()+")");
  
  // figure alphabets
  EventSet UlEvents = rEnvironment.UlEvents();
  EventSet YeEvents = rEnvironment.YeEvents();
  EventSet YlEvents = rEnvironment.YlEvents();
  EventSet UeEvents = rEnvironment.UeEvents();
  
	//create first error state, where inserted Uc- or Yp-events shall lead to:
  if(StateNamesEnabled) {
	errStr1=pResEnvironment->UniqueStateName("UlYeError");	
	errIdx1=pResEnvironment->InsMarkedState(errStr1);
  }
  else {
  errIdx1=pResEnvironment->InsMarkedState();
  }
  
  // set HioStateFlag Err
  pResEnvironment->SetErr(errIdx1);
  // set HioStateFlag QUe
  pResEnvironment->SetQUe(errIdx1);
  
  //find Input-states
  StateSet::Iterator sit=pResEnvironment->StatesBegin();
  EventSet::Iterator evit;
  bool InputInserted = false;
  for(;sit!=pResEnvironment->StatesEnd();sit++){
    EventSet disabledInput, active;
	active = pResEnvironment->ActiveEventSet(*sit);
	// first treat Uc-events
    disabledInput = UlEvents - active;
    //if at least one Input-event is enabled AND at least one Input-event is disabled:
    //insert missing Input-transitions to error state
    if( (disabledInput!=UlEvents) && !disabledInput.Empty()) {
      // FD_DF("HioFreeInput: " << *sit << " disabledInput: " << disabledInput.ToString());
      InputInserted = true;
      for (evit = disabledInput.Begin(); evit != disabledInput.End(); ++evit) {
        pResEnvironment->SetTransition(*sit,*evit,errIdx1);
      }       
    }
	// then treat Yp-events
	disabledInput = YeEvents - active;
    //if at least one Input-event is enabled AND at least one Input-event is disabled:
    //insert missing Input-transitions to error state
    if( (disabledInput!=YeEvents) && !disabledInput.Empty()) {
      // FD_DF("HioFreeInput: " << *sit << " disabledInput: " << disabledInput.ToString());
      InputInserted = true;
      for (evit = disabledInput.Begin(); evit != disabledInput.End(); ++evit) {
        pResEnvironment->SetTransition(*sit,*evit,errIdx1);
      }       
    }
  }//for-loop through states

  //insert err_Input-state and connect both err-states with Input-Output-loop in case missing Input-events were inserted:
  if(InputInserted) {
     //create second error state
	 std::string errstate2;
	 if(StateNamesEnabled) {
		 FD_DF("HioFreeInput(...): inserted error state 1: " << errStr1);
		 errStr2=pResEnvironment->UniqueStateName("ErrorQYe");	
		 errIdx2=pResEnvironment->InsMarkedState(errStr2);
		 FD_DF("HioFreeInput(...): inserted error state 2: " << errStr2);
		}
	 else {
		 FD_DF("HioFreeInput(...): inserted error state 1 with Idx: " << errIdx1);
		 errIdx2=pResEnvironment->InsMarkedState();
		 FD_DF("HioFreeInput(...): inserted error state 2 with Idx: " << errIdx2);
		}

	 // set HioStateFlag Err
	 pResEnvironment->SetErr(errIdx2);
	 // set state attribute QYe
	 pResEnvironment->SetQYe(errIdx2);
	 
     //insert Ue-events:
     for (evit = UeEvents.Begin(); evit != UeEvents.End(); ++evit) {
		 pResEnvironment->SetTransition(errIdx1,*evit,errIdx2);
		}
	 //insert Ye-events:
     for (evit = YeEvents.Begin(); evit != YeEvents.End(); ++evit) {
		 pResEnvironment->SetTransition(errIdx2,*evit,errIdx1);
		}
	}
	
   //else delete error-state 1
   else {
     pResEnvironment->DelState(errIdx1);
    }
   // if necessary, move pResGen to rResGen
  if(pResEnvironment != &rResEnvironment) {
    pResEnvironment->Move(rResEnvironment);
    delete pResEnvironment;
  }
     // restore event attributes
   rResEnvironment.SetYl(YlEvents);
   rResEnvironment.SetUl(UlEvents);
   rResEnvironment.SetYe(YeEvents);
   rResEnvironment.SetUe(UeEvents);
}

// HioFreeInput(HioConstraint,HioConstraint)   
void HioFreeInput(
  const HioConstraint& rConstraint,
  HioConstraint& rResConstraint) 
{
  // Call HioFreeInput with certain names for error states,
  // set Err-flags
  Idx errstate1,errstate2;
  std::string ErrState1="Y-Error";
  std::string ErrState2="ErrorQY";
  EventSet y,u;
  y=rConstraint.YEvents();
  u=rConstraint.UEvents();
  HioFreeInput(rConstraint,y,u,rResConstraint,ErrState1,ErrState2,errstate1,errstate2);
  // set HioStateFlag Err
  rResConstraint.SetErr(errstate1);
  rResConstraint.SetErr(errstate2);
  // restore event attributes
  rResConstraint.SetY(y);
  rResConstraint.SetU(u);
}

// HioFreeInput(HioPlant) 
void HioFreeInput(HioPlant& rPlant) {
  FD_DF("HioFreeInput( [single arg plant] " << rPlant.Name() << ")");
  HioFreeInput(rPlant,rPlant);
}

// HioFreeInput(HioController) 
void HioFreeInput(HioController& rController) {
  HioFreeInput(rController,rController);
}

// HioFreeInput(HioEnvironment) 
void HioFreeInput(HioEnvironment& rEnvironment) {
  HioFreeInput(rEnvironment,rEnvironment);
}

// HioFreeInput(HioConstraint) 
void HioFreeInput(HioConstraint& rConstraint) {
  HioFreeInput(rConstraint,rConstraint);
}



//**********************************************************
//******************** Completeness ************************
//**********************************************************



//**********************************************************
//******************** I/O-shuffle ************************
//**********************************************************

//******************** current version: marked parameters + marking of alternation ************************

// MarkHioShuffle(rGen1,rGen2,rMapOrigToResult,rShuffle)
void MarkHioShuffle(const Generator& rGen1,const Generator& rGen2,
	const std::map< std::pair<Idx,Idx>, Idx >& rMapOrigToResult,
	Generator& rShuffle) {
	
	// invert mapOrigToResult (possible, as map is expected to be from parallel composition and thus must be bijective)
	std::map< Idx, std::pair<Idx,Idx> > mapResultToOrig;
	std::map< std::pair<Idx,Idx>, Idx >::const_iterator iter;	
    for( iter = rMapOrigToResult.begin(); iter != rMapOrigToResult.end(); ++iter ) {
      mapResultToOrig.insert(std::make_pair( iter->second, iter->first ));
    }
	
	// (I) duplicate all non-marked states, whose corresponding state in rGen1 or rGen2 is marked.
	StateSet nonmarked=rShuffle.States()-rShuffle.MarkedStates();
	StateSet::Iterator sit;
	std::map< Idx, Idx > rGen1Clones,rGen2Clones;
	for(sit = nonmarked.Begin(); sit != nonmarked.End(); sit ++) {
		// marked in rGen1?
		if(rGen1.ExistsMarkedState(mapResultToOrig[*sit].first)) {
			rGen1Clones[*sit]=rShuffle.InsMarkedState();
		}
		// marked in rGen2?
		if(rGen2.ExistsMarkedState(mapResultToOrig[*sit].first)) {
			rGen2Clones[*sit]=rShuffle.InsMarkedState();
		}
	}
	
	// (II) MOVE HEAD OF rGen1-transitions whose X2 has a rGen1-clone, and head of rGen2-transitions whose
	//        X2 has a rGen2-clone
	TransSet TransToClear, TransToSet;
    TransSet::Iterator tit=rShuffle.TransRelBegin();
	TransSet::Iterator tit_end=rShuffle.TransRelEnd();
	std::map<Idx,Idx>::iterator cloneit;
	bool x1hasGen1Clone,x1hasGen2Clone, x2hasClone;
	for(;tit!=tit_end;tit++) {
		
		x1hasGen1Clone=false;
		x1hasGen2Clone=false;
		x2hasClone=false;
	
		//##################################################
		// CASE (I) - X2 has clone: move head of transition, also duplicate transition if X1 has clone
		//##################################################
		
		// preliminarily check if X1 has clone
		cloneit=rGen1Clones.find(tit->X1);
		if(cloneit!=rGen1Clones.end()) x1hasGen1Clone=true;
		cloneit=rGen2Clones.find(tit->X1);
		if(cloneit!=rGen2Clones.end()) x1hasGen2Clone=true;
		
		
		// distinguish rGen1-events and rGen2-events
		if(rGen1.Alphabet().Exists(tit->Ev)) {
			// exists rGen1-clone?
			cloneit=rGen1Clones.find(tit->X2);
			if(cloneit!=rGen1Clones.end()) {
				x2hasClone=true;
				// move head to clone: delete original trans, insert moved trans
				TransToClear.Insert(*tit);
				TransToSet.Insert(tit->X1,tit->Ev,rGen1Clones[tit->X2]);
				// if X1 has a clone make copy starting from clone of X1
				if(x1hasGen1Clone) {
						TransToSet.Insert(rGen1Clones[tit->X1],tit->Ev,rGen1Clones[tit->X2]);
					}
				if(x1hasGen2Clone) {
						TransToSet.Insert(rGen2Clones[tit->X1],tit->Ev,rGen1Clones[tit->X2]);
					}
			}
		}
		
		//  else, tit is rGen2-transition
		else {
			// exists rGen2-clone?
			cloneit=rGen2Clones.find(tit->X2);
			if(cloneit!=rGen2Clones.end()) {
				x2hasClone=true;
				// move head to clone: delete original trans, insert moved trans
				TransToClear.Insert(*tit);
				TransToSet.Insert(tit->X1,tit->Ev,cloneit->second);
				// if X1 has a clone make copy starting from clone of X1
				if(x1hasGen1Clone) {
						TransToSet.Insert(rGen1Clones[tit->X1],tit->Ev,rGen2Clones[tit->X2]);
					}
				if(x1hasGen2Clone) {
						TransToSet.Insert(rGen2Clones[tit->X1],tit->Ev,rGen2Clones[tit->X2]);
					}
			}
		}
		
		//###################################################
		// CASE (II) - (only) X1 has clone: duplicate transition (see Case I for case also X2 has clone)
		//###################################################
		
		if( (x1hasGen1Clone||x1hasGen2Clone) && !x2hasClone ) {
			if(x1hasGen1Clone) TransToSet.Insert(rGen1Clones[tit->X1],tit->Ev,tit->X2);
			if(x1hasGen2Clone) TransToSet.Insert(rGen2Clones[tit->X1],tit->Ev,tit->X2);
		}
	}
	
	// clear invalid transitions
	tit=TransToClear.Begin();
	tit_end=TransToClear.End();
	for(;tit!=tit_end;tit++) rShuffle.ClrTransition(*tit);
	
	// set updated transitions
	tit=TransToSet.Begin();
	tit_end=TransToSet.End();
	for(;tit!=tit_end;tit++) rShuffle.SetTransition(*tit);
	
	// cosmetics
	rShuffle.Accessible();
	return;
}

// MarkAlternationAB(Aset,Bset,AltAB)
void MarkAlternationAB(
  const EventSet rAset, 
  const EventSet rBset,
  Generator& rAltAB)
{
	FD_DF("MarkAlternationAB()");
	
	#ifdef FAUDES_CHECKED
	// validate parameters
	if (rAset.Empty()){
		std::stringstream errstr;
		errstr << "Empty parameter rAset.";
		throw Exception("CheapAltAnB", errstr.str(), 0);
	}	
	if (rBset.Empty()){
	    std::stringstream errstr;
	    errstr << "Empty parameter rBset.";
	    throw Exception("CheapAltAnB", errstr.str(), 0);
	}
	#endif
	
    rAltAB.Clear();
    rAltAB.InjectAlphabet(rAset+rBset);
    
    Idx s1,s2,s3,s4,s5;
    
    s1=rAltAB.InsInitState();
    rAltAB.SetMarkedState(s1);
    s2=rAltAB.InsMarkedState();
    s3=rAltAB.InsMarkedState();
    s4=rAltAB.InsState();
    s5=rAltAB.InsState();
    EventSet::Iterator evit=rAset.Begin();
    EventSet::Iterator evit_end=rAset.End();
    for(;evit!=evit_end;++evit) {
        rAltAB.SetTransition(s1,*evit,s2);
        rAltAB.SetTransition(s2,*evit,s4);
        rAltAB.SetTransition(s3,*evit,s2);
        rAltAB.SetTransition(s4,*evit,s4);
        rAltAB.SetTransition(s5,*evit,s2);
    }
    evit=rBset.Begin();
    evit_end=rBset.End();
    for(;evit!=evit_end;++evit) {
        rAltAB.SetTransition(s1,*evit,s3);
        rAltAB.SetTransition(s2,*evit,s3);
        rAltAB.SetTransition(s3,*evit,s5);
        rAltAB.SetTransition(s4,*evit,s3);
        rAltAB.SetTransition(s5,*evit,s5);
    }
}

// HioShuffleUnchecked(rPlantA,rPlantB,rYp,rUp,rYe,rUe,rIOShuffAB) 
void HioShuffleUnchecked(
  const Generator& rPlantA, 
  const Generator& rPlantB, 
  const EventSet& rYp,
  const EventSet& rUp, 
  const EventSet& rYe, 
  const EventSet& rUe,
  Generator& rIOShuffAB) 
{ 
  FD_DF("HioShuffle()");

  //Extract alphabets:             
  EventSet A,B,SigmaP;
  A = rPlantA.Alphabet();
  B = rPlantB.Alphabet();

  // parallel composition of plantA and plantB (i.e. shuffle as there are no shared events)
  Generator parallel1, parallel2;
  std::map< std::pair<Idx,Idx>, Idx > MapOrigToResult;
  Parallel(rPlantA,rPlantB,MapOrigToResult,parallel1);
  MarkHioShuffle(rPlantA,rPlantB,MapOrigToResult,parallel1);

  // restrict to event pairs [(SigA SigA)*+(SigB SigB)*]*, which results
  // in correct I/O sorting L_IO.
  Generator EventPairs; // generator of above sorting language
  EventPairs.InjectAlphabet(A + B);
  Idx state1,state2,state3;
  state1=EventPairs.InsInitState("1");
  EventPairs.SetMarkedState(state1);
  state2=EventPairs.InsMarkedState("2");
  state3=EventPairs.InsMarkedState("3");
  EventSet::Iterator evit;
  for (evit = A.Begin(); evit != A.End(); ++evit) {
    EventPairs.SetTransition(state1,*evit,state2);
    EventPairs.SetTransition(state2,*evit,state1);
  }
  for (evit = B.Begin(); evit != B.End(); ++evit) {
    EventPairs.SetTransition(state1,*evit,state3);
    EventPairs.SetTransition(state3,*evit,state1);
  }
  Parallel(parallel1,EventPairs,parallel2);
  parallel1 = parallel2;

// //#####################################################################################
// //#####################################################################################
// //#####################################################################################
//  //restrict to IO-sorting structure with forced alternation:
//
//  // Old Vers before Dez 4 2006: Alternation of whole alphabets
//  A = rPlantA.Alphabet();
//  B = rPlantB.Alphabet();
//
//
////  // New Vers: Alternation of YP Events only
////  // -> currently leads to normality problems during superposed controller synthesis
////  SigmaP.InsertSet(rYp);
////  SigmaP.InsertSet(rUp);
////  A.RestrictSet(SigmaP);
////  B.RestrictSet(SigmaP);
//
//
//
//  Generator AltAB;
//  A.Name("A");
//  B.Name("B");
//  CheapAltAB(A,B,Depth,AltAB);
//  // Alt_AB.Write("tmp_Alternation_AB.gen");
//  Parallel(parallel2, AltAB, parallel1);
//  //parallel2.Write("tmp_IOS3.gen");
//
//
//

  
  // bugfix to resolve normality problem: whenever plant A
	// can send a yp-event and at the same time plant B can
	// send a ye-event, then let the ye-event be sent first:
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //*** Environment-first policy: If, in some state both, Yp- and Ye events are
  //*** active, then cancel all Yp-transitions in this state, as environment has
  //*** to be updated first.
  
  FD_DF("------ HioShuffle with Environment-First-Policy! -----");
  
  // loop all states
  StateSet::Iterator sit = parallel1.StatesBegin();
  StateSet::Iterator sit_end = parallel1.StatesEnd();
  
  for(; sit != sit_end; sit++)
  {
        // figure active Yp-events and active Ye-events
        //std::cout << "End: " << *sit_end <<"...done: " << *sit << std::endl;
        EventSet ActiveYe;
        EventSet ActiveYp = parallel1.TransRel().ActiveEvents(*sit);
        ActiveYe=ActiveYp;
        ActiveYe.RestrictSet(rYe);
        ActiveYp.RestrictSet(rYp);
        if (!(ActiveYe.Empty())&& !(ActiveYp.Empty()))
        {   
            TransSet TransToClear;
            TransSet::Iterator tit=parallel1.TransRelBegin(*sit);
            TransSet::Iterator tit_end=parallel1.TransRelEnd(*sit);
            for(;tit!=tit_end;tit++)
                {
                //std::cout << "finding Yp-events to delete... "; 
                if(ActiveYp.Exists(tit->Ev))
                    {
                    TransToClear.Insert(*tit);
                    }
                }
            tit=TransToClear.Begin();
            tit_end=TransToClear.End();
            for(;tit!=tit_end;tit++)
            {
            //std::cout<<" ...deleting" <<std::endl;
            parallel1.ClrTransition(*tit);
            }
        } 
  }  
  
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//~ ////
//~ // //#####################################################################################
//~ // //#####################################################################################
//~ // //#####################################################################################


  // Insert Error Behaviour, i.e. make UP and UE free in (YP,UP)-port of result:
  // free UP:
  HioFreeInput(parallel1, rUp, rYp, parallel2, "errYp_Ye", "errUp");
  //parallel1.Write("tmp_IOS4.gen");
  // free UE:
  HioFreeInput(parallel2, rUe, rYe, parallel1, "errYp_Ye", "errUe");
  //parallel2.Write("tmp_IOS5.gen");
  
  // Mark Alternation of YP-events
  A.RestrictSet(rYp);
  B.RestrictSet(rYp);
  MarkAlternationAB(A,B,parallel2);
  Parallel(parallel2,parallel1,parallel1);

  rIOShuffAB = parallel1;
  rIOShuffAB.Name("HioShuffle("+rPlantA.Name()+rPlantB.Name()+")");
  return;
}


// HioShuffle(rPlantA,rPlantB,rYp,rUp,rYe,rUe,rIOShuffAB) 
void HioShuffle(
  const Generator& rPlantA, 
  const Generator& rPlantB, 
  const EventSet& rYp,
  const EventSet& rUp, 
  const EventSet& rYe, 
  const EventSet& rUe,
  Generator& rIOShuffAB) 
{ 
  rIOShuffAB.Clear();
  #ifdef FAUDES_CHECKED
  // exception on empty alphabets
  if (rYp.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Yp alphabet\n";
	 throw Exception("HioShuffle", errstr.str(), 0);
	}
  if (rUp.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Up alphabet\n";
	 throw Exception("HioShuffle", errstr.str(), 0);
	}
  if (rYe.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Ye alphabet\n";
	 throw Exception("HioShuffle", errstr.str(), 0);
	}
  if (rUe.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Ue alphabet\n";
	 throw Exception("HioShuffle", errstr.str(), 0);
	}
	
  // create resulting alphabet while checking for disjoint sets
  EventSet alphabet,alphabetA,alphabetB,errevents;
  alphabetA=rPlantA.Alphabet();
  alphabetB=rPlantB.Alphabet();
  // initialize with rYp
  alphabet.InsertSet(rYp);
  // insert remaining events one by one and check if new
  EventSet::Iterator evit;
  // rUp
  for (evit = rUp.Begin(); evit != rUp.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // rYe
  for (evit = rYe.Begin(); evit != rYe.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // rUe
  for (evit = rUe.Begin(); evit != rUe.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // throw exception on non disjoint alphabets
  if (!errevents.Empty()){
  	std::stringstream errstr;
  	errstr << "Non-disjoint parameters; ambiguous events:\n" <<errevents.ToString();
	throw Exception("HioShuffle", errstr.str(), 0);
  }
  
  // check alphabet match with plant A and B-alphabets:
  // The union of plant A and B alphabets need not equal but must be included in the union of EventSet parameters.
  
  errevents=alphabetA+alphabetB-alphabet;
  if (!errevents.Empty()){
  	std::stringstream errstr;
  	errstr << "Plant A- or plant B-events missing in Yp,Up,Ye or Ue:\n" <<errevents.ToString();
	throw Exception("HioShuffle", errstr.str(), 0);
  }
  
  // plant A and plant B must be in HioPlantForm
  HioPlant hioPlantA=HioPlant(rPlantA,rYp*alphabetA,rUp*alphabetA,rYe*alphabetA,rUe*alphabetA);
  std::string report;
  if(!IsHioPlantForm(hioPlantA,report)){
  	std::stringstream errstr;
  	errstr << "plant A not in HioPlantForm:\n" << report;
	throw Exception("HioShuffle", errstr.str(), 0);
  }
  
  HioPlant hioPlantB=HioPlant(rPlantB,rYp*alphabetB,rUp*alphabetB,rYe*alphabetB,rUe*alphabetB);
  report.clear();
  if(!IsHioPlantForm(hioPlantB,report)){
  	std::stringstream errstr;
  	errstr << "plant B not in HioPlantForm:\n" << report;
	throw Exception("HioShuffle", errstr.str(), 0);
  }
  #endif
  
  // compute I/O-shuffle 
  HioShuffleUnchecked(rPlantA, rPlantB, rYp, rUp, rYe, rUe, rIOShuffAB);
  
}

void HioShuffle(
  const HioPlant& rPlantA, 
  const HioPlant& rPlantB,
  HioPlant& rIOShuffAB) {
  
  //prepare result
  rIOShuffAB.Clear();
  
  EventSet yp = rPlantA.YpEvents()+rPlantB.YpEvents();
  EventSet up = rPlantA.UpEvents()+rPlantB.UpEvents();
  EventSet ye = rPlantA.YeEvents()+rPlantB.YeEvents();
  EventSet ue = rPlantA.UeEvents()+rPlantB.UeEvents();
  
  // compute I/O-shuffle
  HioShuffle(rPlantA, rPlantB, yp, up, ye, ue, rIOShuffAB);
  
  // set event and state attributes
  rIOShuffAB.SetYp(yp);
  rIOShuffAB.SetUp(up);
  rIOShuffAB.SetYe(ye);
  rIOShuffAB.SetUe(ue);
  
  IsHioPlantForm(rIOShuffAB);
}
//******************** old version: no marking, forced alternation ************************

// CheapAltAnB(rAset,rBset,Depth,rAltAnB)
void CheapAltAnB(
  const EventSet rAset, 
  const EventSet rBset,
  const int Depth,
  Generator& rAltAnB) 
{ 
  FD_DF("CheapAltAnB()");
  
  // validate parameters
	if (Depth<1){
	    std::stringstream errstr;
	    errstr << "Parameter Depth must be 1 or greater.";
	    throw Exception("CheapAltAnB", errstr.str(), 0);
	}
	#ifdef FAUDES_CHECKED
	if (rAset.Empty()){
	    std::stringstream errstr;
	    errstr << "Empty parameter rAset.";
	    throw Exception("CheapAltAnB", errstr.str(), 0);
	}	
	if (rBset.Empty()){
	    std::stringstream errstr;
	    errstr << "Empty parameter rBset.";
	    throw Exception("CheapAltAnB", errstr.str(), 0);
	}
	#endif
	
  // prepare result
  rAltAnB.Clear();
  rAltAnB.Name("CheapAltAnB(" + rAset.Name() + "," + rBset.Name() + "," + ToStringInteger(Depth) + ")");

  // create initial state
  Idx init = rAltAnB.InsInitState("alt_"+rAset.Name()+"_init");
  rAltAnB.SetMarkedState(init);
  
  Idx last = init;
  rAltAnB.InjectAlphabet(rAset + rBset);

  // selfloop initial state with Bset:
  EventSet::Iterator evit;
  for (evit = rBset.Begin(); evit != rBset.End(); ++evit) {
      rAltAnB.SetTransition(last,*evit,last);
  }

  //create n concatenated pathes A->|a|A->|b| and transitions B from |b| to initial state.
  for (int i=0; i!=Depth; ++i) {
    Idx a = rAltAnB.InsMarkedState("alt_"+rAset.Name()+"_U"+ToStringInteger(i+1));
    Idx b = rAltAnB.InsMarkedState("alt_"+rAset.Name()+"_Y"+ToStringInteger(i+2));
    //connect last,a and b with A-transitions.
    for (evit = rAset.Begin(); evit != rAset.End(); ++evit) {
      rAltAnB.SetTransition(last,*evit,a);
      rAltAnB.SetTransition(a,*evit,b);
    }
    //insert B-transitions from b to init
    for (evit = rBset.Begin(); evit != rBset.End(); ++evit) {
      rAltAnB.SetTransition(b,*evit,init);
    }
    last = b;
  }
}

// CheapAltAB(rAset,rBset,Depth,rAltAnB)
void CheapAltAB(
  const EventSet rAset, 
  const EventSet rBset,  
  const int Depth,
  Generator& rAltAB) 
{ 
  FD_DF("CheapAltAB()");
  // prepare result
  rAltAB.Clear();
  
  Generator AnB, BnA;
  CheapAltAnB(rAset,rBset,Depth,AnB);
  CheapAltAnB(rBset,rAset,Depth,BnA);
  Parallel(AnB,BnA,rAltAB);
  rAltAB.Name("CheapAltAB("+rAset.Name()+","+rBset.Name()+","+ToStringInteger(Depth)+")");
}

// HioShuffleTU(rPlantA,rPlantB,rYp,rUp,rYe,rUe,Depth,rIOShuffAB) 
void HioShuffleTU(
  const Generator& rPlantA, 
  const Generator& rPlantB, 
  const EventSet& rUp, 
  const EventSet& rYp,
  const EventSet& rYe, 
  const EventSet& rUe,
  const int Depth,
  Generator& rIOShuffAB) 
{ 
  FD_DF("HioShuffle()");

  //Extract alphabets:             
  EventSet A,B,SigmaP;
  A = rPlantA.Alphabet();
  B = rPlantB.Alphabet();

  // parallel composition of plantA and plantB (i.e. shuffle as there are no shared events)
  Generator parallel1, parallel2;
  Parallel(rPlantA,rPlantB,parallel1);

  // restrict to event pairs [(SigA SigA)*+(SigB SigB)*]*, which results
  // in correct I/O sorting L_IO.
  Generator EventPairs; // generator of above sorting language
  EventPairs.InjectAlphabet(A + B);
  Idx state1,state2,state3;
  state1=EventPairs.InsInitState("1");
  EventPairs.SetMarkedState(state1);
  state2=EventPairs.InsMarkedState("2");
  state3=EventPairs.InsMarkedState("3");
  EventSet::Iterator evit;
  for (evit = A.Begin(); evit != A.End(); ++evit) {
    EventPairs.SetTransition(state1,*evit,state2);
    EventPairs.SetTransition(state2,*evit,state1);
  }
  for (evit = B.Begin(); evit != B.End(); ++evit) {
    EventPairs.SetTransition(state1,*evit,state3);
    EventPairs.SetTransition(state3,*evit,state1);
  }
  Parallel(parallel1,EventPairs,parallel2);

  //restrict to IO-sorting structure with forced alternation:

////////  // Old Vers before Dez 4 2006: Alternation of whole alphabets
////////  A = rPlantA.Alphabet();
////////  B = rPlantB.Alphabet();
//////
//////  // New Vers: Alternation of YP Events only
//////  // -> currently leads to normality problems during superposed controller synthesis
//////  SigmaP.InsertSet(rYp);
//////  SigmaP.InsertSet(rUp);
//////  A.RestrictSet(SigmaP);
//////  B.RestrictSet(SigmaP);
//////
//////
//////  Generator AltAB;
//////  A.Name("A");
//////  B.Name("B");
//////  CheapAltAB(A,B,Depth,AltAB);
//////  // Alt_AB.Write("tmp_Alternation_AB.gen");
//////  Parallel(parallel2, AltAB, parallel1);
//////  //parallel2.Write("tmp_IOS3.gen");

  // Old Vers before Dez 4 2006: Alternation of whole alphabets
  EventSet Ap, Bp, Ae, Be, SigmaE;
  
  Ap = rPlantA.Alphabet();
  Bp = rPlantB.Alphabet();
  
  Ae = rPlantA.Alphabet();
  Be = rPlantB.Alphabet();
  
  

  //*************************************************************************
  //// New Vers: Alternation of YP Events only
  //// -> currently leads to normality problems during superposed controller synthesis
  SigmaP.InsertSet(rYp);
  SigmaP.InsertSet(rUp);
  Ap.RestrictSet(SigmaP);
  Bp.RestrictSet(SigmaP);
  
  SigmaE.InsertSet(rYe);
  SigmaE.InsertSet(rUe);
  Ae.RestrictSet(SigmaE);
  Be.RestrictSet(SigmaE);
  
  
  //*************************************************************************
  Generator AltAB;
  Ap.Name("A");
  Bp.Name("B");
  CheapAltAB(Ap,Bp,Depth,AltAB);
  // Alt_AB.Write("tmp_Alternation_AB.gen");
  Parallel(parallel2, AltAB, parallel1);
  
/*   // bugfix to resolve normality problem: whenever plant A
 * 	// can send a yp-event and at the same time plant B can
 * 	// send a ye-event, then let the ye-event be sent first:
 *   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *   // Environment-first policy: If, in some state both, Yp- and Ye events are
 *   // active, then cancel all Yp-transitions in this state, as environment has
 *   // to be updated first.
 *   
 *   // loop all states
 *   StateSet::Iterator sit = parallel1.StatesBegin();
 *   StateSet::Iterator sit_end = parallel1.StatesEnd();
 *   
 *   for(; sit != sit_end; sit++)
 *   {
 *         // figure active Yp-events and active Ye-events
 *         //std::cout << "End: " << *sit_end <<"...done: " << *sit << std::endl;
 *         EventSet ActiveYe;
 *         EventSet ActiveYp = parallel1.TransRel().ActiveEvents(*sit);
 *         ActiveYe=ActiveYp;
 *         ActiveYe.RestrictSet(rYe);
 *         ActiveYp.RestrictSet(rYp);
 *         if (!(ActiveYe.Empty())&& !(ActiveYp.Empty()))
 *         {   
 *             TransSet TransToClear;
 *             TransSet::Iterator tit=parallel1.TransRelBegin(*sit);
 *             TransSet::Iterator tit_end=parallel1.TransRelEnd(*sit);
 *             for(;tit!=tit_end;tit++)
 *                 {
 *                 //std::cout << "finding Yp-events to delete... "; 
 *                 if(ActiveYp.Exists(tit->Ev))
 *                     {
 *                     TransToClear.Insert(*tit);
 *                     }
 *                 }
 *             tit=TransToClear.Begin();
 *             tit_end=TransToClear.End();
 *             for(;tit!=tit_end;tit++)
 *             {
 *             //std::cout<<" ...deleting" <<std::endl;
 *             parallel1.ClrTransition(*tit);
 *             }
 *         } 
 *   }  
 *   
 *   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 * 
 */


  // Insert Error Behaviour, i.e. make UP and UE free in (YP,UP)-port of result:
  // free UP:
  HioFreeInput(parallel1, rUp, rYp, parallel2, "errYp_Ye", "errUp");
  //parallel1.Write("tmp_IOS4.gen");
  // free UE:
  HioFreeInput(parallel2, rUe, rYe, parallel1, "errYp_Ye", "errUe");
  //parallel2.Write("tmp_IOS5.gen");

  rIOShuffAB = parallel1;
}

//**********************************************************
//******************** Cycles ************************
//**********************************************************


// ***Search for strongly connected ycless components (SCC)***
// This function partitions the stateset of a generator into equivalent classes
// such that states x1 and x2 are equivalent iff there is a ycless path from x1
// to x2 AND a ycless path from x2 to x1.
// This function implements the algorithm based on a depth first search
// presented in:
// -Aho, Hopcroft, Ullman: The Design and Analysis of Computer Algorithms-
//
// Most of the comments in this function have been literally taken from
// this book!
//
// Parameters (an api with generator and Yc-events as input and SCCs as output
// is provided right below this method.)
// -state: State, from which the current recursion is started.
// -rcount: denotes the current depth of the recursion.
// -Yc: set of Yc-events
// -UnMarkedOnly: if set true, being unmarked is an additional condition for equivalence of states
// -NewStates: Set of states that up to now were not found by the depth first search
// -STACK: stack of state indeces
// -StackStates: set of states whose indeces are on STACK.
// -DFN: map assigning to each state its Depth-First Number
// -LOWLINK: map assigning to each state its LOWLINK Number
// -SCCset: result - the set of strongly connected components
// -UnMarkedSCCset: result - the set of strongly connected components consisting exclusively of marked states
// -Roots: result - the set of states that each are root of some SCC.
//
// ToDo: handle exceptions.
//

void SearchYclessScc(
  const Idx state, 
  int& rcount,      // why is this a ref?
  const Generator& rGen,
  const EventSet& rYc,
  const bool UnMarkedOnly,
  StateSet&  rNewStates,
  std::stack<Idx>& rSTACK, 
  StateSet& rStackStates,
  std::map<const Idx, int>& rDFN,
  std::map<const Idx, int>& rLOWLINK,
  std::set<StateSet>& rSccSet,
  StateSet& rRoots)
{
  FD_DF("SerchYclessScc: -- search from state "<< state << "--");

  // mark state "old";
  rNewStates.Erase(state);

  // DFNUMBER[state] <- count;
  rDFN[state]=rcount;

  // count <- count+1;
  rcount++;

  // LOWLINK[v] <- DFNUMBER[v];
  rLOWLINK[state]=rDFN[state];
  //cout<<"count: "<<rcount<<" state: "<<state<<" DFN: "<<rDFN[state]<<endl;

  // push state on STACK;
  rSTACK.push(state);
  rStackStates.Insert(state);

  //<<create set "L[state]" of successor states of state, without states reached via a Yc-event:
  StateSet SuccStates = StateSet();
  TransSet::Iterator it = rGen.TransRelBegin(state);
  TransSet::Iterator it_end = rGen.TransRelEnd(state);
  for (; it != it_end; ++it) {
    if(!rYc.Exists(it->Ev)||(UnMarkedOnly &! rGen.ExistsMarkedState(it->X2)))
      {
        SuccStates.Insert(it->X2);
      }
  }

  // for each vertex *sit on L[state] do
  StateSet::Iterator sit = SuccStates.Begin();
  StateSet::Iterator sit_end = SuccStates.End();
  for (; sit != sit_end; ++sit)
    {
      // if *sit is marked "new" then
      if(rNewStates.Exists(*sit))
        {// begin
        
	  // SearchC(*sit);
	  SearchYclessScc(*sit, rcount, rGen, rYc, UnMarkedOnly, rNewStates, rSTACK, rStackStates, rDFN, rLOWLINK, rSccSet, rRoots);
	  // LOWLINK[state] <- MIN(LOWLINK[state],LOWLINK[*sit]);
	  if(rLOWLINK[*sit]<rLOWLINK[state])
	    {
	      rLOWLINK[state]=rLOWLINK[*sit];
	    }
        }//end
        
      else
        {
	  // if DFNUMBER[*sit]<DFNUMBER[state] and [*sit] is on STACK then
	  if((rDFN[*sit]<rDFN[state])&&(rStackStates.Exists(*sit)))
	    {
	      // LOWLINK[state]<-MIN(DFNUMBER[*sit],LOWLINK[state]);
	      if(rDFN[*sit]<rLOWLINK[state])
		{
		  rLOWLINK[state]=rDFN[*sit];
		}
	    }                
        }
    }//end for
  // if LOWLINK[state]=DFNUMBER[state] (i.e. state is root of a SCC) then
  if(rLOWLINK[state]==rDFN[state])
    {
      // per def., each state in a graph is element of a SCC, thus:
      // check if size of SCC is>1 or (else) SCC contains (non-{epsilon,yc}-)selfloops
      // to avoid print of trivial SCCs
      bool realscc=rSTACK.top()!=state; //size of SCC>1 ? if not, check for selfloops:
      if(!realscc)
        {
	  TransSet::Iterator it = rGen.TransRelBegin(state);
	  TransSet::Iterator it_end = rGen.TransRelEnd(state);
	  for (; it != it_end; ++it)
            {
	      if((it->X2==state)&&(!rYc.Exists(it->Ev)))
                {
		  realscc=true; //at least one yc-less selfloop found -> SCC is nontrivial
		  break;
                }
            }
        }
        
      //~ if(realscc) // be aware: "else" statement is NOT correct at this point, as realscc-value is changed in the above if-clause
        //~ {
	  //~ // cout<<endl<<"found ycless SCC with root "<<state<<", consisting of the following states:"<<endl<<"begin"<<endl;
	  //~ rRoots.Insert(state);
        //~ }
        
      //create SCC
      StateSet Scc;
      bool purelyUnMarked=true;
      // begin
      // repeat
      while(true)
        {// begin
	  // pop x from top of STACK and print x;
	  Idx top=rSTACK.top();
	  if(realscc) // things outside the "if(realscc)"-clause have to be done in any case!
            {
	      // cout<<top;
	      Scc.Insert(top);
          if(rGen.ExistsMarkedState(top)) purelyUnMarked=false;
            }
	  rStackStates.Erase(top);
	  rSTACK.pop();
        
	  // until x=state;
	  if(top==state)
	    {
	      // print "end of SCC", insert SCC into SCCset;
	      if(realscc)
		{
		  // cout<<endl<<"end"<<endl;
          if(!purelyUnMarked || UnMarkedOnly)
            {
              rSccSet.insert(Scc);
              rRoots.Insert(state);
            }
		}
	      //system("pause");
	      break;
	    }
	  // cout<<", "<<endl;
        } //end while
    } // end if
}

// YclessScc(Generator,Yc-events, SccSet, Roots)
// -> api using SearchYclessScc() from above.
bool YclessScc(
  const Generator& rGen,
  const EventSet& rYc,
  std::set<StateSet>& rSccSet,
  StateSet& rRoots)
{
  FD_DF("YclessScc(" << rGen.Name() << ")");

  // inititalize results:
  rRoots.Clear();
  rSccSet.clear();
  // initialize counter for depthfirstnumbers(DFN):
  int count=1;

  // initialize stack of states (state indeces):
  std::stack<Idx> STACK;
  // due to lack of STACK-iterator: initialize set of states that are on STACK:
  StateSet StackStates = StateSet();

  // initialize set of states to be examined:
  StateSet  NewStates=rGen.AccessibleSet();

  // initialize map of state indeces to their DFN:
  std::map<const Idx, int> DFN;
  // initialize map of state indeces to their LOWLINK:
  std::map<const Idx, int> LOWLINK;

  // figure initial state:
  Idx InitState = *rGen.InitStatesBegin();
  // start recursive depth-first search for Scc's:
  while(!NewStates.Empty()) {
    // the following 'if' isn't necessary, but provokes search begin at initial state of rGen
    if(NewStates.Exists(InitState)) {
      SearchYclessScc(InitState, count, rGen, rYc, false, NewStates, STACK, StackStates, DFN, LOWLINK, rSccSet, rRoots);
    } else {
      SearchYclessScc(*NewStates.Begin(), count, rGen, rYc, false, NewStates, STACK, StackStates, DFN, LOWLINK, rSccSet, rRoots);
    }
  }
  
  return !rSccSet.empty();
}

// YclessUnMarkedScc(Generator,Yc-events, SccSet, Roots)
// -> api using SearchYclessScc() from above.
bool YclessUnmarkedScc(
  const Generator& rGen,
  const EventSet& rYc,
  std::set<StateSet>& rSccSet,
  StateSet& rRoots)
{
  FD_DF("YclessScc(" << rGen.Name() << ")");

  // inititalize results:
  rRoots.Clear();
  rSccSet.clear();
  // initialize counter for depthfirstnumbers(DFN):
  int count=1;

  // initialize stack of states (state indeces):
  std::stack<Idx> STACK;
  // due to lack of STACK-iterator: initialize set of states that are on STACK:
  StateSet StackStates = StateSet();

  // initialize set of states to be examined:
  StateSet  NewStates=rGen.AccessibleSet()-rGen.MarkedStates();

  // initialize map of state indeces to their DFN:
  std::map<const Idx, int> DFN;
  // initialize map of state indeces to their LOWLINK:
  std::map<const Idx, int> LOWLINK;

  // figure initial state:
  Idx InitState = *rGen.InitStatesBegin();
  // start recursive depth-first search for Scc's:
  while(!NewStates.Empty()) {
    // the following 'if' isn't necessary, but provokes search begin at initial state of rGen
    if(NewStates.Exists(InitState)) {
      SearchYclessScc(InitState, count, rGen, rYc, true, NewStates, STACK, StackStates, DFN, LOWLINK, rSccSet, rRoots);
    } else {
      SearchYclessScc(*NewStates.Begin(), count, rGen, rYc, true, NewStates, STACK, StackStates, DFN, LOWLINK, rSccSet, rRoots);
    }
  }
  
  return !rSccSet.empty();
}


// YclessScc(rGen,rYc,rSccSet)
// -> api using YclessScc() from above.
bool YclessScc(
  const Generator& rGen,
  const EventSet& rYc,
  std::set<StateSet>& rSccSet)
{
  FD_DF("YclessScc(" << rGen.Name() << ")");

  // inititalize result:
  rSccSet.clear();
  
  StateSet Roots;
  return YclessScc(rGen,rYc,rSccSet,Roots);
}

// IsYcLive(rGen,rYc)
bool IsYcLive(
  const Generator& rGen,
  const EventSet& rYc)
{
  FD_DF("IsYcLive(" << rGen.Name() << ")");

  std::set<StateSet> rSccSet;
  
  // Generator is YcLive if no YcLessScc is found
  YclessScc(rGen,rYc,rSccSet);
  return rSccSet.size()==0;
}

// WriteStateSets(rStateSets)
// Write set of StateSet's to console.
void WriteStateSets(
  const std::set<StateSet>& rStateSets)
{
  FD_DF("WriteStateSets()");
  std::cout<<std::endl;
  if(rStateSets.empty()) {
    std::cout<<"WriteStateSets: Set of state sets is empty."<<std::endl;
    FD_DF("WriteStateSets: Set of state sets is empty.");
    return;
  }
  std::cout<<">WriteStateSets: Set of state sets begin:"<< std::endl;
  std::set<StateSet>::iterator StateSetit = rStateSets.begin();
  std::set<StateSet>::iterator StateSetit_end = rStateSets.end();
  for (; StateSetit != StateSetit_end; ++StateSetit) {
    std::cout<<std::endl<<"  >> State set begin:"<< std::endl;
    StateSet::Iterator sit = StateSetit->Begin();
    StateSet::Iterator sit_end = StateSetit->End();
    for (; sit != sit_end; ++sit) {
      std::cout<<"  >> "<<*sit<<std::endl;
    }
    std::cout<<"  >> State set end."<< std::endl;
  }  
  std::cout<<std::endl<<">WriteStateSets: Set of state sets end."<<std::endl;
}

// WriteStateSets(rGen,rStateSets)
// Write set of StateSet's to console.
// Use rGen for symbolic state names
void WriteStateSets(
  const Generator& rGen,
  const std::set<StateSet>& rStateSets)
{
  FD_DF("WriteStateSets()");
  std::cout<<std::endl;
  if(rStateSets.empty()) {
    std::cout<<"WriteStateSets: Set of state sets is empty."<<std::endl;
    FD_DF("WriteStateSets: Set of state sets is empty.");
    return;
  }
  std::cout<<">WriteStateSets: Set of state sets begin:"<< std::endl;
  std::set<StateSet>::iterator StateSetit = rStateSets.begin();
  std::set<StateSet>::iterator StateSetit_end = rStateSets.end();
  for (; StateSetit != StateSetit_end; ++StateSetit) {
    std::cout<<std::endl<<"  >> State set begin:"<< std::endl;
    std::cout<<"  "<<rGen.StateSetToString(*StateSetit)<<std::endl;
    std::cout<<"  >> State set end."<< std::endl;
  }  
  std::cout<<std::endl<<">WriteStateSets: Set of state sets end."<<std::endl;
}

// SccEntries(rGen,rSccSet,rEntryStates,rEntryTransSet)
void SccEntries(
  const Generator& rGen,
  const std::set<StateSet>& rSccSet,
  StateSet& rEntryStates,
  TransSetX2EvX1& rEntryTransSet) {
  				  
  FD_DF("SccEntries(...)");

  // prepare results:
  rEntryStates.Clear();
  rEntryTransSet.Clear();
  
  //extract all transitions sorted by target state X2
  TransSetX2EvX1 trel;
  rGen.TransRel(trel);
  FD_DF("SccEntries: Entry states of all SCCs:");

  //loop through all SCCs:
  std::set<StateSet>::iterator sccit = rSccSet.begin();
  std::set<StateSet>::iterator sccit_end = rSccSet.end();
  for (int i=0; sccit != sccit_end; ++sccit, i++) {
    FD_DF("SccEntries: SCC " << i << " begin:");
    //loop through all states:
    StateSet::Iterator sit = sccit->Begin();
    StateSet::Iterator sit_end = sccit->End();
    for (; sit != sit_end; ++sit) {
      //check if sit is initial state:
      if(rGen.ExistsInitState(*sit)) {
        rEntryStates.Insert(*sit);
      }        
      //loop through all transitions with target state X2=sit:
      TransSetX2EvX1::Iterator tit=trel.BeginByX2(*sit);
      TransSetX2EvX1::Iterator tit_end=trel.EndByX2(*sit);
      for(;tit!=tit_end;tit++) {
        //if sit is successor of a state X1 that is element of a SCC different
        //from the current SCC "StateSetit" and if sit has not been added to
        //rEntryStates up to now, then add sit to EntryStates:
        if(!sccit->Exists(tit->X1)) {
          // insert entry transition:
          rEntryTransSet.Insert(*tit);
          // if new, insert entry state:
          if(!rEntryStates.Exists(*sit)) {
            // Perk: check if Insert returns false anyway if element already
            // existed before. if so, then write:
            // if(rEntryStates.Insert(*sit)) FD_DF("SccEntries: Entry state:" <<*sit);
            FD_DF("SccEntries: Entry state:" <<*sit);
            rEntryStates.Insert(*sit);
          }
        }
      }
    }
    FD_DF("SccEntries: SCC " << i << " end:");
  }
  FD_DF("SccEntries: done");
}

// CloneScc(rGen,rScc,rSccSet,rEntryState,rEntryStates,rEntryTransSet)
void CloneScc(
  Generator& rGen, 
  const StateSet& rScc, 
  std::set<StateSet>& rSccSet,
  const Idx EntryState, 
  StateSet& rEntryStates, 
  TransSetX2EvX1& rEntryTransSet)
{
  FD_DF("CloneScc(...)");
  // initialize map from original SCC-states to their clones:
  std::map<const Idx, Idx> clone;
  // initialize clone-SCC:
  StateSet CloneScc;
  // clone SCC-states:
  StateSet::Iterator sit=rScc.Begin();
  StateSet::Iterator sit_end=rScc.End();
  for(;sit!=sit_end;sit++) {
    // if sit is marked, also mark its clone
    if(rGen.ExistsMarkedState(*sit)) { 
      clone[*sit]=rGen.InsMarkedState();
      CloneScc.Insert(clone[*sit]);
      // cout<<"marked state "<<clone[*sit]<<" of "<<*sit<<" inserted.";
    } 
    // or else insert unmarked state
    else {
      clone[*sit]=rGen.InsState();
      CloneScc.Insert(clone[*sit]);
      // cout<<"unmarked state "<<clone[*sit]<<" of "<<*sit<<" inserted.";
    }
  }
  // add clone of SCC to set of all SCCs:
  rSccSet.insert(CloneScc);
  // clone all transitions:
  sit=rScc.Begin();
  for(;sit!=sit_end;sit++) {
    TransSet::Iterator tit=rGen.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rGen.TransRelEnd(*sit);
    for(;tit!=tit_end;tit++) {
      // clone all transitions within SCC:
      if(rScc.Exists(tit->X2)) {
        rGen.SetTransition(clone[*sit],tit->Ev,clone[tit->X2]);
      }
      //...and transitions leaving the SC: 
      else {
        rGen.SetTransition(clone[*sit],tit->Ev,tit->X2);
        //**** added 06.09.2007:
        // If this cloned transition leads to an entry state of
        // some other SCC, then  it is a new entry transition:
        if(rEntryStates.Exists(tit->X2)) {
          rEntryTransSet.Insert(clone[*sit],tit->Ev,tit->X2);
        }
        //****
      }
    }
  }
  // move the head of all entry transitions leading to EntryState to the clone of EntryState:
  TransSet TransToClear, EntryTransToInsert;
  TransSetX2EvX1::Iterator tit=rEntryTransSet.BeginByX2(EntryState);
  TransSetX2EvX1::Iterator tit_end=rEntryTransSet.EndByX2(EntryState);
  for(;tit!=tit_end;tit++) {
  //    cout<<endl<<"Moving entryTransition "<<"("<<tit->X1<<","<<rGen.EventName(tit->Ev)<<","<<tit->X2<<")";
  //    cout<<" to target state "<<clone[EntryState];
    rGen.SetTransition(tit->X1,tit->Ev,clone[EntryState]);
    //prebook new moved transition to be inserted in rEntryTransSet:
    EntryTransToInsert.Insert(tit->X1,tit->Ev,clone[EntryState]);
    //prebook transitions to former entry state to clear in rGen and rEntryTransSet:
    TransToClear.Insert(*tit);
  }
  //clear former transitions in rGen and rEntryTransSet:
  TransSet::Iterator tit2=TransToClear.Begin();
  TransSet::Iterator tit2_end=TransToClear.End();
  for(;tit2!=tit2_end;tit2++) {
    rGen.ClrTransition(*tit2);
    //erase transitions to former entry state from rEntryTransSet:
    rEntryTransSet.Erase(*tit2);
  }
  // update rEntryTransSet with new moved entry transitions:
  TransSet::Iterator tit3=EntryTransToInsert.Begin();
  TransSet::Iterator tit3_end=EntryTransToInsert.End();
  for(;tit3!=tit3_end;tit3++) {
    //insert new moved transitions in rEntryTransSet:
    rEntryTransSet.Insert(*tit3);
  }
  //update rEntryStateSet:
  rEntryStates.Erase(EntryState); // erase former ambiguous entry state
  rEntryStates.Insert(clone[EntryState]); // insert unique entry state of cloned SCC
}

// CloneUnMarkedScc(rGen,rScc,rSccSet,rEntryState,rEntryStates,rEntryTransSet)
void CloneUnMarkedScc(
  Generator& rGen, 
  const StateSet& rScc,
  const Idx EntryState, 
  const StateSet& rEntryStates, 
  TransSetX2EvX1& rEntryTransSet)
{
  FD_DF("CloneUnMarkedScc(...)");
  // initialize map from original SCC-states to their clones:
  std::map<const Idx, Idx> clone;
  // initialize clone-SCC:
  StateSet CloneScc;
  // clone all SCC-states but EntryState:
  StateSet::Iterator sit=rScc.Begin();
  StateSet::Iterator sit_end=rScc.End();
  for(;sit!=sit_end;sit++) {
      if(*sit != EntryState) {
         clone[*sit]=rGen.InsState();
         CloneScc.Insert(clone[*sit]);
        }
      else clone[*sit]=*sit; // entry state is not cloned
      
    }
  
  //*** Think about this:
  // // add clone of SCC to set of all SCCs:
  // rSccSet.insert(CloneScc);
  //***
    
  // clone all transitions:
  TransSet TransToClear;
  sit=rScc.Begin();
  bool isEntryState=false;
  TransSet::Iterator tit,tit_end;
  for(;sit!=sit_end;sit++) {
    isEntryState=(*sit==EntryState);
    tit=rGen.TransRelBegin(*sit);
    tit_end=rGen.TransRelEnd(*sit);
    for(;tit!=tit_end;tit++) {
      // clone all transitions within SCC:
      if(rScc.Exists(tit->X2)) {
        rGen.SetTransition(clone[*sit],tit->Ev,clone[tit->X2]);
        // if transition starts at entry state, only keep the clone transition
        if(isEntryState) {
          // prebook this transition to be cleared
          TransToClear.Insert(*tit);
        }
      }
      //...and transitions leaving the SCC:
      else {
        rGen.SetTransition(clone[*sit],tit->Ev,tit->X2);
        //**** added 06.09.2007:
        // If this cloned transition leads to an entry state of
        // some other SCC, then  it is a new entry transition:
        if(rEntryStates.Exists(tit->X2)) {
          rEntryTransSet.Insert(clone[*sit],tit->Ev,tit->X2);
        }
        //****
      }
    }
  }
  
  // clear prebooked transitions
  tit=TransToClear.Begin();
  tit_end=TransToClear.End();
  for(;tit!=tit_end;tit++) {
    rGen.ClrTransition(*tit);
  }

}
//============================



// YcAcyclic(rGen,rYc,rResGen)
void YcAcyclic(
  const Generator& rGen,
  const EventSet& rYc,
  Generator& rResGen)
{
  FD_DF("YcAcyclic: Statesize before: "<<rGen.Size()<<" - starting...");
  // initialize set of ycless strongly connected components (SCCs):
  std::set<StateSet> SccSet,UnMarkedSccSet;
  // initialize set of all states that are member or root or entry state of some SCC:
  StateSet Scc,Roots,EntryStates,notRoots,UnMarkedRoots;
  // initialize set of transitions that enter some SCC from a different SCC,
  // sorted by target states X2 (which are entry states):
  TransSetX2EvX1 EntryTransSet;
  // take rGen as first candidate:
  rResGen=rGen;
  // counter for below while loop:
  // int i=0;

  while(true) {
    //++i;
    // check for SCCs and return if there are none -> ResGen is the result.
    // cout<<endl<<"Round "<<i<<": Searching for remaining Yc-less SCCs...";
    if(!YclessScc(rResGen,rYc,SccSet,Roots)) {
       //time_t *t2;
       //time(t2);
       //cout<<"...finished in "<<difftime(*t2, *t1)<<" seconds.";
        //rResGen.DotWrite("tmp_YcAcyclic_res");
      FD_DF("YcAcyclic: Statesize of result: "<<rResGen.Size());
//        cout<<". Statemin...";
//        Generator tmp;
//        StateMin(rResGen,tmp);
//        cout<<"done. New Statesize:"<<tmp.Size();
//        tmp.ClrStateNames();
//        rResGen=tmp;
//        tmp.Clear();
        //rResGen.Write("tmp_YcAcyclic.gen");
          return;
        }
    //cout<<endl<<"...found "<<SCCset.size()<<" Yc-less SCCs: computing entry states...";
//    cout<<"Found following Yc-less SCCs:"<<endl;
//    WriteStateSets(SCCset);
//    system("pause");
    // else figure entry states and entry transitions of the SCCs
    SccEntries(rResGen,SccSet,EntryStates,EntryTransSet);
    
    // root states are kept as entry states of the original SCCs, for all other
    // entry states clone SCCs:
    notRoots=EntryStates - Roots;
    //cout<<"...found "<<notRoots.Size()<<" entry states that are not roots. Start cloning SCCs..."<<endl;
//    cout<<endl<<"EntryStates before cloning:"<<endl;
//    EntryStates.DWrite();
//    cout<<endl<<"Roots before cloning:"<<endl;
//    Roots.DWrite();
//    cout<<endl<<"notRoots before cloning:"<<endl;
//    notRoots.DWrite();

    StateSet::Iterator sit=notRoots.Begin();
    StateSet::Iterator sit_end=notRoots.End();
    for(;sit!=sit_end;sit++)
        {
        // figure SCC with entry state sit:
	  std::set<StateSet>::iterator sccit = SccSet.begin();
	  std::set<StateSet>::iterator sccit_end = SccSet.end();
        for (; sccit != sccit_end; ++sccit)
            {
            if(sccit->Exists(*sit))
                {
                Scc=*sccit;
                break;
                }
            }
        // clone this SCC (and add clone to SCCset):
//        cout<<"cloning SCCC:"<<endl;
//        SCC.DWrite();
        CloneScc(rResGen,Scc,SccSet,*sit,EntryStates,EntryTransSet);
//        cout<<endl<<"New set of SCCs:"<<endl;
//        WriteStateSets(SccSet);
        }
    //cout<<endl<<"...done. Identifying unique entry states...";
//    rResGen.Write("tmp_YcAcyclic_step.gen");
//    rResGen.DotWrite("tmp_YcAcyclic_step");
//    ++i;
//    cout<<endl<<"Printed intermediate result "<<i<<"."<<endl;
//    system("pause");

    // Now, all SCCs have one unique entry state.
    // If some SCC contains a sub-SCC that a) includes the entry
    // state of the whole SCC and b) consists of only unmarked
    // states, then this sub-SCC has to be copied to resolve the
    // ambiguity.
    if( !(  ((rResGen.States()-rResGen.MarkedStates())*EntryStates).Empty()  ) ) {
        // find possible unmarked sub-SCCs including EntryState
        YclessUnmarkedScc(rResGen,rYc,UnMarkedSccSet,UnMarkedRoots);
        std::set<StateSet>::iterator sccit = UnMarkedSccSet.begin();
        std::set<StateSet>::iterator sccit_end = UnMarkedSccSet.end();
        for (; sccit != sccit_end; ++sccit) {
            // check if this unmarked sub-SCC includes an EntryState; if so: clone it
            if(  !( ((*sccit)*EntryStates).Empty() )  ) {
                 Idx EntryState=*(((*sccit)*EntryStates).Begin());
                 CloneUnMarkedScc(rResGen,*sccit,EntryState,EntryStates,EntryTransSet);
                }
        }
    }
        
    
    
    //"Back transitions" leading
    // from a SCC to its own entry state have to be cancelled,
    // if they are not Yc-transitions:

    // 2) loop through all SCCs and find Ycless back transitions:
    TransSet TransToClear;
    std::set<StateSet>::iterator sccit = SccSet.begin();
    std::set<StateSet>::iterator sccit_end = SccSet.end();
    //cout<<endl<<"Determining non-yc-backtransitions...";
    for (; sccit != sccit_end; ++sccit)
        {
        //loop through all states:
        StateSet::Iterator sit = sccit->Begin();
        StateSet::Iterator sit_end = sccit->End();
        for (; sit != sit_end; ++sit)
            {
            //cout<<endl<<"looping state "<<*sit<<endl;
            // loop through all transitions:
            TransSet::Iterator tit=rResGen.TransRelBegin(*sit);
            TransSet::Iterator tit_end=rResGen.TransRelEnd(*sit);
            for(;tit!=tit_end;tit++)
                {
                // Check if tit leads back to entry state of current SCC
                // and if it is not a Yc-transition:
                if(sccit->Exists(tit->X2) && EntryStates.Exists(tit->X2)
                   &&!rYc.Exists(tit->Ev))
                    {
                    //cout<<" - Trans to clear: "<<tit->Ev<<endl;
                    TransToClear.Insert(*tit);
                    }
                }
            }
        }
    //cout<<endl<<"...found "<<TransToClear.Size()<<" transitions. Deleting these transitions...";
    // 3) Clear back transitions:
    TransSet::Iterator tit=TransToClear.Begin();
    TransSet::Iterator tit_end=TransToClear.End();
    for(;tit!=tit_end;tit++)
        {
        //cout<<"clearing transition ("<<tit->X1<<","<<rResGen.EventName(tit->Ev)<<","<<tit->X2<<")"<<endl;
        rResGen.ClrTransition(*tit);
        }
        
//    ++i;
//    rResGen.Write("tmp_YcAcyclic_step.gen");
//    rResGen.DotWrite("tmp_YcAcyclic_step");
//    cout<<endl<<"Printed intermediate result "<<i<<"."<<endl;
//    system("pause");
//    cout<<endl<<"...transitions deletetd, next round!";
//     cout<<endl<<"state size of current intermediate result: "<<rResGen.Size();
     //cout<<endl<<". Press enter to start statemin."<<endl;
     //string anykey;
     //getline( cin, anykey );

//     cout<<". Statemin...";
//     Generator tmp;
//     StateMin(rResGen,tmp);
//     cout<<"done. New Statesize:"<<tmp.Size();
//     rResGen=tmp;
//     tmp.Clear();

//    system("pause");
    } //end while(true): resulting generator might contain SCCs within the
      // SCCs that have been "broken" in this step -> continue while-loop.
}

//**********************************************************
//******************** hio synthesis ************************
//**********************************************************

// ConstrSynth_Beta(rPlant,rYp,rUp,rOpConstr)
void ConstrSynth_Beta(
        Generator& rPlant,
  const EventSet& rYp,
  const EventSet& rUp,
  const Generator& rLocConstr,
  Generator& rOpConstraint) {
  
  	FD_DF("ConstrSynth_Beta(...)");
	
	// helper
	Generator tmpGen;
	EventSet sigmap=rYp+rUp;
	
	//prepare rsult:
	rOpConstraint.Clear();
	
	// respect optional local constraints
	Parallel(rPlant,rLocConstr,rOpConstraint);
	
	// calculate YpAcyclic sublanguage:
	YcAcyclic(rOpConstraint,rYp,tmpGen);
	rOpConstraint.Clear();
	
	// compute normal, complete, controllable and nonblocking sublanguage
	// note: tmpGen has role of spec
	NormalCompleteSynthNB(rPlant,rUp,sigmap,tmpGen,rOpConstraint);
	tmpGen.Clear();
	
	// project to P-Alphabet
	Project(rOpConstraint,sigmap,tmpGen);
	rOpConstraint.Clear();
	
	// minimize state space
	StateMin(tmpGen,tmpGen);
	tmpGen.StateNamesEnabled(false);
	
	// eneable symbolic state names for error states
	tmpGen.StateNamesEnabled(true);
	rOpConstraint.StateNamesEnabled(true);
	// make YP free in OpConstr
	HioFreeInput(tmpGen, rYp, rUp, rOpConstraint, "errUp","errYp");

	rOpConstraint.Name("ConstrSynth_Beta("+rPlant.Name()+")");
	
	return;
  }

// HioSynthUnchecked(rPlant,rSpec,rConstr,rLocConstr,rYc,rUc,rYp,rUp,rYel,rUel,rController)
void HioSynthUnchecked(
  const Generator& rPlant,
  const Generator& rSpec,
  const Generator& rExtConstr,
  const Generator& rLocConstr,
  const EventSet& rYc,
  const EventSet& rUc, 
  const EventSet& rYp,
  const EventSet& rUp, 
  const EventSet& rYel,
  const EventSet& rUel,
  Generator& rController)
{ 
	FD_DF("HioSynth(...)");
	//prepare rsult:
	rController.Clear();
	//make copy of plant, spec for modifications
	Generator plant,spec;
	plant = rPlant;
	spec = rSpec;
	
	//create necessary eventsets:
	EventSet sigmacp;
	sigmacp.InsertSet(rUc + rYc + rUp + rYp);
	EventSet sigmace;
	sigmace.InsertSet(rUc + rYc + rUel + rYel);
	
	EventSet allsigma;
	allsigma.InsertSet(rPlant.Alphabet());
	allsigma.InsertSet(rSpec.Alphabet());
	
	EventSet controllable, wait;
	controllable.InsertSet(rUp);
	controllable.InsertSet(rYc);

	//make alphabets of plant and spec match:
	InvProject(plant,allsigma);
	InvProject(spec,allsigma);
	
  // FD_DF("******************** actual plant: composition with external constraints");
	
	// generate plant under constraint:
	Parallel(plant,rExtConstr,plant);
	//plant.Write("tmp_cplant.gen");
	
	//FD_DF("******************** mininmal hio structure, composition with plant");
	Generator minhio;
	minhio = HioSortCL(rYc,rUc,rYp,rUp,rYel,rUel);
	//minhio.Write("tmp_minhio.gen");
	
	// generate plant in hio structure:
	Parallel(plant,minhio,plant);
	plant.StateNamesEnabled(false);
	//plant.Write("tmp_hiocplant.gen");
	
	//FD_DF("******************** composition plant-specification: tmp_plantspec.gen ");
	
	// composition of plant, I/O sorting, constraint and specification:
	Generator plantspec;
	Parallel(plant,spec,plantspec);
	//plantspec.Write("tmp_plantspec.gen");
	plantspec.StateNamesEnabled(false);
	FD_DF("Size of plantspec: "<<plantspec.Size()<<" - Statemin...");
	StateMin(plantspec,plantspec);
	FD_DF("done. New Statesize: "<<plantspec.Size()<<".");
	plantspec.StateNamesEnabled(false);
    //plantspec.Write("results/tmp_plantspec.gen");
  FD_DF("******************** computing YcAcyclic(plantspec):");
	
	// calculate YcAcyclic sublanguage of plantspec:
	Generator yclifeplantspec;
	YcAcyclic(plantspec,rYc,yclifeplantspec);
	yclifeplantspec.StateNamesEnabled(false);
	//yclifeplantspec.Write("tmp_YcAcyclic.gen");
	
	//FD_DF("******************** actual spec: composition yclifeplantspec||minhio||locconstr (tmp_actualspec.gen)");
	//generate plantspec with closed-loop I/O sorting:
	Parallel(yclifeplantspec,minhio,plantspec);
	yclifeplantspec.Clear();
	//add local constraints to plantspec: the result is the actual spec for synthesis
	Parallel(plantspec,rLocConstr,plantspec);
	plantspec.StateNamesEnabled(false);
  plantspec.Name("actualspec");
	//plantspec.Write("tmp_actualspec.gen");
	
	//FD_DF("******************** closed loop synthesis: tmp_closedloop.gen/.dot");
	
	// compute normal, complete and controllable sublangugae of actualspec w.r.t. plant under constraints (cplant):
	Generator loop;
	NormalCompleteSynthNB(plant,controllable,sigmacp,plantspec,loop);
	plantspec.Clear();
	//loop.Write("tmp_closedloop.gen");
	
	FD_DF("size of closed loop before StateMin: "<<loop.Size()<<", starting StateMin...");
	StateMin(loop,loop);
	FD_DF("...size of closed loop after StateMin (tmp_closedloop.gen): "<<loop.Size());
    //std::cout<<"ClosedLoop.IsTrim: "<<loop.IsTrim();
	loop.StateNamesEnabled(false);
	// loop.Write("tmp_closedloop.gen");
	FD_DF("********* IsYcLive(closed loop): "<<IsYcLive(loop,rYc));
  plant.Clear();
  
	//calculate external closedloop:
	FD_DF("******************** external closed loop: tmp_extloop.gen");
	Generator extloop;
	Project(loop,sigmace,extloop);
  FD_DF("size of external closed loop: "<<extloop.Size());
	
	// // recheck if specification is met:
	// if(LanguageInclusion(extloop,rSpec)) {
	  // FD_DF("External closed loop meets specification.");
	    // } // Perk: change below to LanguageInclusion with reverse order of parameters
	    // // and move into above bracket.
	// if(LanguageEquality(extloop,rSpec)) {
	  // FD_DF("External closed loop EQUALS specification.");
	    // }
	
	extloop.StateNamesEnabled(false);
	//extloop.Write("tmp_extloop.gen");
	
	FD_DF("******************** project closed loop: tmp_controller.gen");
	
	Generator controller;
	Project(loop,sigmacp,controller);
	loop.Clear();
	controller.StateNamesEnabled(false);
	//controller.Write("tmp_controller.gen");
	
	// extend to io-controller: result
  HioFreeInput(controller, rYp, rUp, rController, "errUp","errYp");
  controller.Clear();
  FD_DF("size of marked IO controller: "<<rController.Size()<<". PrefixClosure, StateMin...");
  
  // the following procedure erases all marking information to reduce the result's state space! This step is unproblematic as long as the specification
  // is a prefix-closed system, because then, all marking information comes from the plant and is restored in the closed loop. Otherwise, care has to be taken
  // wether this step is correct or should be omitted .
  if(!(rSpec.AccessibleSet()<=rSpec.MarkedStates())) {
  FD_WARN("Careful: marking information of the specification has been erased from the resulting controller.");
  }
  PrefixClosure(rController);
	StateMin(rController,rController);
  FD_DF("...size of prefix-closed IO controller after StateMin: "<<rController.Size());
	rController.StateNamesEnabled(false);
	rController.Name("HioSynth("+rPlant.Name()+","+rSpec.Name()+")");
  FD_DF("IO controller synthesis done. ******************** " );
}

// HioSynth(rPlant,rSpec,rConstr,rLocConstr,rYc,rUc,rYp,rUp,rYel,rUel,rController)
void HioSynth(
  const Generator& rPlant,
  const Generator& rSpec,
  const Generator& rExtConstr,
  const Generator& rLocConstr,
  const EventSet& rYc,
  const EventSet& rUc, 
  const EventSet& rYp,
  const EventSet& rUp, 
  const EventSet& rYel,
  const EventSet& rUel,
  Generator& rController)
{ 
  
  rController.Clear();
  
  #ifdef FAUDES_CHECKED
  // exception on empty alphabets
  if (rYc.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Yc alphabet\n";
	 throw Exception("HioSynth", errstr.str(), 0);
	}
  if (rUc.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Uc alphabet\n";
	 throw Exception("HioSynth", errstr.str(), 0);
	}
  if (rYp.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Yp alphabet\n";
	 throw Exception("HioSynth", errstr.str(), 0);
	}
  if (rUp.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Up alphabet\n";
	 throw Exception("HioSynth", errstr.str(), 0);
	}
  if (rYel.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Ye alphabet\n";
	 throw Exception("HioSynth", errstr.str(), 0);
	}
  if (rUel.Empty()){
	 std::stringstream errstr;
	 errstr << "Empty Ue alphabet\n";
	 throw Exception("HioSynth", errstr.str(), 0);
	}
	
  // create resulting alphabet while checking for disjoint sets
  EventSet alphabet,errevents;
  // initialize with rYc
  alphabet.InsertSet(rYc);
  // insert remaining events one by one and check if new
  EventSet::Iterator evit;
  // rUc
  for (evit = rUc.Begin(); evit != rUc.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // rYp
  for (evit = rYp.Begin(); evit != rYp.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // rUp
  for (evit = rUp.Begin(); evit != rUp.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // rYe
  for (evit = rYel.Begin(); evit != rYel.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // rUe
  for (evit = rUel.Begin(); evit != rUel.End(); ++evit) {
  	if(!(alphabet.Insert(*evit))) errevents.Insert(*evit);
  }
  // throw exception on non disjoint alphabets
  if (!errevents.Empty()){
  	 std::stringstream errstr;
  	 errstr << "Non-disjoint parameters; ambiguous events:\n" <<errevents.ToString();
	 throw Exception("HioSynth", errstr.str(), 0);
	}
  // check alphabet matches:
  EventSet plantEvents, specEvents;
  plantEvents=rPlant.Alphabet();
  specEvents=rSpec.Alphabet();
  // plant and spec must share environment alphabet
  if (!(plantEvents*specEvents==rYel+rUel)){
  	 std::stringstream errstr;
  	 errstr << "Alphabet mismatch between plant and spec:\n";
  	 errstr << "plant: " << plantEvents.ToString() << "\n";
  	 errstr << "spec: " << specEvents.ToString() << "\n";
  	 errstr << "Yel+Uel:   " << (rYel+rUel).ToString() << "\n";
	 throw Exception("HioSynth", errstr.str(), 0);  
	}
  // ExtConstr must relate to spec alphabet
  if (!(rExtConstr.Alphabet()<=specEvents)){
  	std::stringstream errstr;
  	errstr << "Alphabet mismatch between external constraints and spec.\n";
	throw Exception("HioSynth", errstr.str(), 0);
  }
  // LocConstr must relate to plant alphabet
  if (!(rLocConstr.Alphabet()<=plantEvents)){
  	std::stringstream errstr;
  	errstr << "Alphabet mismatch between internal constraints and plant.\n";
	throw Exception("HioSynth", errstr.str(), 0);
  }
  
  // check I/O plant form of spec (note: plant may be composed group and thus need not be in I/o plant form)
  HioPlant spec(rSpec,rYc,rUc,rYel,rUel);
  std::string report;
  if(!IsHioPlantForm(spec,report)){
  	std::stringstream errstr;
  	errstr << "Spec not in HioPlantForm:\n" << report;
	throw Exception("HioSynth", errstr.str(), 0);
  }
  report.clear();
  #endif
  
  // do hio synthesis
  HioSynthUnchecked(rPlant,rSpec,rExtConstr,rLocConstr,rYc,rUc,rYp,rUp,rYel,rUel,rController);
}

// HioSynthMonolithic(rPlant,rSpec,rSc,rSp,rSe,rController)
void HioSynthMonolithic(
  const HioPlant& rPlant,
  const HioPlant& rSpec,
  const HioConstraint& rSc,
  const HioConstraint& rSp,
  const HioConstraint& rSe,
  HioController& rController) {
  
  rController.Clear();
  
  #ifdef FAUDES_CHECKED
  // check I/O plant form of rPlant (further parameter checks in HioSynth())
  // tmp non-const copy of const rPlant
  HioPlant plant=rPlant;
  std::string report;
  if(!IsHioPlantForm(plant,report)){
  	std::stringstream errstr;
  	errstr << "Plant not in HioPlantForm:\n" << report;
	throw Exception("HioSynthMonolithic", errstr.str(), 0);
  }
  plant.Clear();
  report.clear();
  #endif
  
  // extract alphabets
  EventSet yc,uc,yp,up,ye,ue;
  yc=rSpec.YpEvents();
  uc=rSpec.UpEvents();
  yp=rPlant.YpEvents();
  up=rPlant.UpEvents();
  ye=rSpec.YeEvents();
  ue=rSpec.UeEvents();
  
  // generator for composed external constraints
  Generator extConstr;
  Parallel(rSc,rSe,extConstr);
  
  // run HioSynth procedure
  HioSynth(rPlant,rSpec,extConstr,rSp,yc,uc,yp,up,ye,ue,rController);
  
  // set event and state attributes
  rController.SetYc(yc);
  rController.SetUc(uc);
  rController.SetYp(yp);
  rController.SetUp(up);
  
  IsHioControllerForm(rController);
}

void HioSynthHierarchical(
  const HioPlant& rHioShuffle,
  const HioEnvironment& rEnvironment,
  const HioPlant& rSpec,
  const Generator& rIntConstr,
  const HioConstraint& rSc,
  const HioConstraint& rSl,
  HioController& rController) {
  
  rController.Clear();
  
  #ifdef FAUDES_CHECKED
  // check I/O plant form of rPlant (further parameter checks in HioSynth())
  // tmp non-const copy of const rPlant
  HioPlant plant=rHioShuffle;
  std::string report;
  if(!IsHioPlantForm(plant,report)){
  	std::stringstream errstr;
  	errstr << "HioShuffle not in HioPlantForm:\n" << report;
	throw Exception("HioSynthHierarchical", errstr.str(), 0);
  }
  plant.Clear();
  report.clear();
  
  // check I/O environment form
  // tmp non-const copy of const rEnvironment
  HioEnvironment env=rEnvironment;
  if(!IsHioEnvironmentForm(env,report)){
  	std::stringstream errstr;
  	errstr << "Environment not in HioEnvironmentForm:\n" << report;
	throw Exception("HioSynthHierarchical", errstr.str(), 0);
  }
  env.Clear();
  report.clear();
  
  // check for matching alphabets between ioshuffle, environment and spec
  if (!(rHioShuffle.EEvents()==rEnvironment.EEvents())){
  	std::stringstream errstr;
  	errstr << "Alphabet mismatch between I/O-shuffle and environment.\n";
	throw Exception("HioSynthHierarchical", errstr.str(), 0);
  }  
  // check for matching alphabets between ioshuffle, environment and spec
  if (!(rSpec.EEvents()==rEnvironment.LEvents())){
  	std::stringstream errstr;
  	errstr << "Alphabet mismatch between specification and environment.\n";
	throw Exception("HioSynthHierarchical", errstr.str(), 0);
  }
  
   //(further parameter checks in HioSynth())
   
  #endif
  // compose ioshuffle and environment
  Generator ioShuffParEnv;
  Parallel(rHioShuffle,rEnvironment,ioShuffParEnv);
  
  // extract alphabets
  EventSet yc,uc,yp,up,ye,ue;
  yc=rSpec.YpEvents();
  uc=rSpec.UpEvents();
  yp=rHioShuffle.YpEvents();
  up=rHioShuffle.UpEvents();
  ye=rSpec.YeEvents();
  ue=rSpec.UeEvents();
  
  // generator for composed external constraints
  Generator extConstr;
  Parallel(rSc,rSl,extConstr);
  
  // run HioSynth procedure
  HioSynth(ioShuffParEnv,rSpec,extConstr,rIntConstr,yc,uc,yp,up,ye,ue,rController); 
  
  // set event and state attributes
  rController.SetYc(yc);
  rController.SetUc(uc);
  rController.SetYp(yp);
  rController.SetUp(up);
  
  IsHioControllerForm(rController);
}

// end of hio_functions - below just archive 

//######################################################
//############ Special versions for HioModule - will soon be obsolete

//HioShuffle_Musunoi() - Version with hio-parameters
void HioShuffle_Musunoi(
  const HioPlant& rPlantA, 
  const HioPlant& rPlantB, 
  int depth, 
  Generator& rIOShuffAB) 
{ 
  FD_DF("HioShuffle()_Musunoi");

  //Extract alphabets:             
  EventSet A,B,SigmaP, Yp, Up, Ye, Ue;
  Yp = rPlantA.YpEvents()+rPlantB.YpEvents();
  Ye = rPlantA.YeEvents()+rPlantB.YeEvents();
  Up = rPlantA.UpEvents()+rPlantB.UpEvents();
  Ue = rPlantA.UeEvents()+rPlantB.UeEvents();
  A = rPlantA.Alphabet();
  B = rPlantB.Alphabet();
  

  // parallel composition of plantA and plantB (i.e. shuffle as there are no shared events)
  Generator parallel1, parallel2;
  Parallel(rPlantA,rPlantB,parallel1);

  // restrict to event pairs [(SigA SigA)*+(SigB SigB)*]*, which results
  // in correct I/O sorting L_IO.
  Generator EventPairs; // generator of above sorting language
  EventPairs.InjectAlphabet(A + B);
  Idx state1,state2,state3;
  state1=EventPairs.InsInitState("1");
  EventPairs.SetMarkedState(state1);
  state2=EventPairs.InsMarkedState("2");
  state3=EventPairs.InsMarkedState("3");
  EventSet::Iterator evit;
  for (evit = A.Begin(); evit != A.End(); ++evit) {
    EventPairs.SetTransition(state1,*evit,state2);
    EventPairs.SetTransition(state2,*evit,state1);
  }
  for (evit = B.Begin(); evit != B.End(); ++evit) {
    EventPairs.SetTransition(state1,*evit,state3);
    EventPairs.SetTransition(state3,*evit,state1);
  }
  Parallel(parallel1,EventPairs,parallel2);

  //restrict to IO-sorting structure with forced alternation:

  // Old Vers before Dez 4 2006: + of whole alphabets
  EventSet Ap, Bp, Ae, Be, SigmaE;
  
  Ap = rPlantA.Alphabet();
  Bp = rPlantB.Alphabet();
  
  Ae = rPlantA.Alphabet();
  Be = rPlantB.Alphabet();
  
  

  //*************************************************************************
  //// New Vers: Alternation of YP Events only
  //// -> currently leads to normality problems during superposed controller synthesis
  SigmaP.InsertSet(Yp);
  SigmaP.InsertSet(Up);
  Ap.RestrictSet(SigmaP);
  Bp.RestrictSet(SigmaP);
  
  SigmaE.InsertSet(Ye);
  SigmaE.InsertSet(Ue);
  Ae.RestrictSet(SigmaE);
  Be.RestrictSet(SigmaE);
  
  
  //*************************************************************************
  Generator AltAB;
  Ap.Name("A");
  Bp.Name("B");
  CheapAltAB(Ap,Bp,depth,AltAB);
  // Alt_AB.Write("tmp_Alternation_AB.gen");
  Parallel(parallel2, AltAB, parallel1);
  //parallel2.Write("tmp_IOS3.gen");
  
  
  
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //*** Environment-first policy: If, in some state both, Yp- and Ye events are
  //*** active, then cancel all Yp-transitions in this state, as environment has
  //*** to be updated first.

  // loop all states
  StateSet::Iterator sit = parallel1.StatesBegin();
  StateSet::Iterator sit_end = parallel1.StatesEnd();
  
  for(; sit != sit_end; sit++)
  {
        // figure active Yp-events and active Ye-events
        //std::cout << "End: " << *sit_end <<"...done: " << *sit << std::endl;
        EventSet ActiveYe;
        EventSet ActiveYp = parallel1.TransRel().ActiveEvents(*sit);
        ActiveYe=ActiveYp;
        ActiveYe.RestrictSet(Ye);
        ActiveYp.RestrictSet(Yp);
            
        if (!(ActiveYe.Empty())&& !(ActiveYp.Empty()))
        {   
            TransSet TransToClear;
            TransSet::Iterator tit=parallel1.TransRelBegin(*sit);
            TransSet::Iterator tit_end=parallel1.TransRelEnd(*sit);
            for(;tit!=tit_end;tit++)
                {
                //std::cout << "finding Yp-events to delete... "; 
                if(ActiveYp.Exists(tit->Ev))
                    {
                    TransToClear.Insert(*tit);
                    }
                }
            tit=TransToClear.Begin();
            tit_end=TransToClear.End();
            for(;tit!=tit_end;tit++)
            {
            //std::cout<<" ...deleting" <<std::endl;
            parallel1.ClrTransition(*tit);
            }
        } 
  }  
  
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  
  
  
  // Insert Error Behaviour, i.e. make UP and UE free in (YP,UP)-port of result:
  // free UP:
          
  HioFreeInput(parallel1, Up, Yp, parallel2, "errYp_Ye", "errUp");
  //parallel1.Write("tmp_IOS4.gen");
  // free UE:
  HioFreeInput(parallel2, Ue, Ye, parallel1, "errYp_Ye", "errUe");
  //parallel2.Write("tmp_IOS5.gen");

  rIOShuffAB = parallel1;
}

// HioSynth_Musunoi()
void HioSynth_Musunoi(const Generator& rPlant, const HioPlant& rSpec,
    const Generator& rConstr, const Generator& rLocConstr,
    const EventSet& rYp,  const EventSet& rUp, 
    Generator& rController)
    {
EventSet rYc, rUc, rYe, rUe;
rYc = (rSpec.YpEvents());
rUc = (rSpec.UpEvents());
rYe = (rSpec.YeEvents());
rUe = (rSpec.UeEvents());


//make copy of plant, spec, constr and alphabets
Generator plant,spec,constr,locconstr;
plant = rPlant;
spec = rSpec;
constr = rConstr;
locconstr = rLocConstr;

EventSet yc,uc,yp,up,ye,ue;
yc = rYc;
uc = rUc;
yp = rYp;
up = rUp;
ye = rYe;
ue = rUe;

//create necessary eventsets:
EventSet sigmacp;
sigmacp.InsertSet(rUc + rYc + rUp + rYp);
EventSet sigmace;
sigmace.InsertSet(rUc + rYc + rUe + rYe);

EventSet allsigma;
allsigma.InsertSet(rPlant.Alphabet());
allsigma.InsertSet(rSpec.Alphabet());

EventSet controllable;
controllable.InsertSet(up);
controllable.InsertSet(yc);

//make alphabets of plant and spec match:
InvProject(plant,allsigma);
InvProject(spec,allsigma);

 FD_DF("******************** actual plant: composition with environment constraint: tmp_cplant.gen ");

// generate plant under constraint:
Generator cplant;
Parallel(plant,constr,cplant);
//cplant.Write("tmp_cplant.gen");

 FD_DF("******************** mininmal hio structure: tmp_minhio.gen, composition with plant: tmp_hioplant.gen ");
Generator minhio;
minhio = HioSortCL(yc,uc,yp,up,ye,ue);
//minhio.Write("tmp_minhio.gen");

// generate plant in hio structure:
Generator hiocplant;
Parallel(cplant,minhio,hiocplant);
//hiocplant.Write("tmp_hiocplant.gen");

 FD_DF("******************** composition plant-specification: tmp_plantspec.gen ");

// composition of plant, I/O sorting, constraint and specification:
Generator plantspec,plantspecMin;
Parallel(hiocplant,spec,plantspec);
//plantspec.Write("tmp_plantspec.gen");
plantspec.StateNamesEnabled(false);
// plantspec.DotWrite("tmp_plantspec");
 FD_DF("Size of plantspec: "<<plantspec.Size()<<" - Statemin...");
//string anykey;
//getline(cin,anykey);
StateMin(plantspec,plantspecMin);
 FD_DF("done. New Statesize: "<<plantspecMin.Size()<<".");
plantspec=plantspecMin;
plantspecMin.Clear();
plantspec.StateNamesEnabled(false);
 FD_DF("******************** computing YcAcyclic(plantspec):");

// calculate YcAcyclic sublanguage of plantspec:
Generator yclifeplantspec;
YcAcyclic(plantspec,yc,yclifeplantspec);
Generator yclifeplantspecMin;
//cout <<endl<< "                     StateMin(yclifeplantspec)..."  << endl;
//StateMin(yclifeplantspec,yclifeplantspecMin);
//cout<<endl<<"Statesize of yclifeplantspec after StateMin (tmp_YcAcyclic_Min.gen): "<<yclifeplantspecMin.Size();
yclifeplantspecMin=yclifeplantspec;
yclifeplantspecMin.StateNamesEnabled(false);
//yclifeplantspecMin.Write("tmp_YcAcyclic_Min.gen");

 FD_DF("******************** actual spec: composition yclifeplantspec||minhio||locconstr (tmp_actualspec.gen)");
//generate plantspec with closed-loop I/O sorting:
Parallel(yclifeplantspecMin,minhio,plantspec);

//add local constraints to plantspec: the result is the actual spec for synthesis
Generator actualspec;
Parallel(plantspec,locconstr,actualspec);
//actualspec.Write("tmp_actualspec.gen");

 FD_DF("******************** closed loop synthesis: tmp_closedloop.gen/.dot");

//std::cout<<"Starting NormalCompleteSynth....";
FD_DF("Starting NormalCompleteSynth....");

// compute normal, complete and controllable sublangugae of actualspec w.r.t. plant under constraints (cplant):
Generator loop;
 NormalCompleteSynth(hiocplant,controllable,yc+uc+yp+up,actualspec,loop);
//loop.Write("tmp_closedloop.gen");
//loop.DotWrite("tmp_closedloop");

 FD_DF("size of closed loop before StateMin: "<<loop.Size()<<", starting StateMin...");
 
//std::cout<<"Starting NormalCompleteSynth....done... starting StateMin";
FD_DF("Starting NormalCompleteSynth....done... starting StateMin");
 
Generator minloop;
StateMin(loop,minloop);
 FD_DF("...size of closed loop after StateMin (tmp_closedloop.gen): "<<minloop.Size());
//minloop=loop;
minloop.StateNamesEnabled(false);
//minloop.Write("tmp_closedloop.gen");
// test: recheck for YCless SCCs:
Generator minlooptest;
minlooptest=minloop;
 std::set<StateSet> SccSet;
YclessScc(minlooptest,yc,SccSet);
 FD_DF("");
 FD_DF("*********number of Yc-less strongly connencted components in closed loop (should be zero): "
       <<SccSet.size());

// minloop.DotWrite("tmp_minclosedloop");

//calculate external closedloop:
 FD_DF("******************** external closed loop: tmp_extloop.gen/.dot");
Generator extloop;
Project(minloop,sigmace,extloop);
 FD_DF("size of external closed loop: "<<extloop.Size());;

// recheck if specification is met:
if(LanguageInclusion(extloop,rSpec)) {
  FD_DF("External closed loop meets specification.");
    }
if(LanguageEquality(extloop,rSpec)) {
  FD_DF("External closed loop EQUALS specification.");
    }

extloop.StateNamesEnabled(false);
//extloop.Write("tmp_extloop.gen");
//extloop.DotWrite("tmp_extloop");

 FD_DF("******************** project closed loop: tmp_controller.gen/.dot ");

Generator controller;
Project(minloop,sigmacp,controller);
controller.StateNamesEnabled(false);
//controller.Write("tmp_controller.gen");
// controller.DotWrite("tmp_controller");

 FD_DF("******************** extend to IO controller: tmp_IOcontroller.gen/.dot ");

// extend to io-controller
Generator HioController, minIOcontroller;
 HioFreeInput(controller, yp, up, HioController, "errUp","errYp");
//HioController.Write("tmp_IOcontroller.gen");
// HioController.DotWrite("tmp_IOcontroller");
 FD_DF("size of IO controller: "<<HioController.Size()<<", StateMin...");
StateMin(HioController,minIOcontroller);
 FD_DF("...size of IO controller after StateMin: "<<minIOcontroller.Size());
minIOcontroller.StateNamesEnabled(false);
//minIOcontroller.Write("tmp_IOcontroller.gen");

// RESULT:
rController = minIOcontroller;

////*******************************************************************
// // uncomment this, if you use complete_synth, instead of
// // normal_complete_synth, for controller design:

//cout <<endl<< "******************** checking normality... "  << endl;
//
//if(!isnormal(minloop,hiocplant,sigmacp))
//    {
//    cout<<endl<<"isnormal says: false! :("<<endl;
//    }
//else
//    {
//    cout<<endl<<"isnormal says: true! :)"<<endl;
//    }
////*******************************************************************

 FD_DF("IO controller synthesis done. ******************** " );
}

}//namespace faudes

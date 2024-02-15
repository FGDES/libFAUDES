/** @file hio_environment.cpp Generator with I/O-environment attributes */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/
 
#include "hio_environment.h"

namespace faudes {

// IsHioEnvironmentForm()
bool IsHioEnvironmentForm(HioEnvironment& rHioEnvironment,
                StateSet& rQYe,
                StateSet& rQUe,
                StateSet& rQUl,
                StateSet& rQYlUe,
                EventSet& rErrEvSet,
                TransSet& rErrTrSet,
                StateSet& rErrStSet,
                std::string& rReportStr)
    {
    FD_DF("IsHioEnvironmentForm("<< rHioEnvironment.Name() << ",...)");
    
    // prepare results
    rQYe.Clear();
    rQUe.Clear();
    rQUl.Clear();
    rQYlUe.Clear();
    
    rErrEvSet.Clear();
    rErrEvSet.Name("rErrEvSet");
    
    rErrTrSet.Clear();
    rErrTrSet.Name("rErrTrSet");
    
    rErrStSet.Clear();
    rErrStSet.Name("rErrStSet");

	// used to locally store error states/transitions on each condition
	StateSet locErrStSet;
	TransSet locErrTrSet;
    
    rReportStr.clear();
    
    // meant to be set false on violation of any condition:
    bool finalResult = true;
    // used to locally store result on each condition
    bool localResult = true;
    
    // helpers
    
    EventSet ye = rHioEnvironment.YeEvents();
    EventSet ue = rHioEnvironment.UeEvents();
    EventSet yl = rHioEnvironment.YlEvents();
    EventSet ul = rHioEnvironment.UlEvents();
    
    StateSet initStates = rHioEnvironment.InitStates();
    StateSet accessibleStates = rHioEnvironment.AccessibleSet();
    StateSet deadEnds;

    EventSet::Iterator evit;
    StateSet::Iterator sit;
    TransSet::Iterator tit;
    
    // Info string header
    rReportStr.append("#########################################################\n");
    rReportStr.append("########## IsHioEnvironmentForm("+rHioEnvironment.Name()+",...) - test results:\n");

    /**************************** Precondition: determinism ***********************/
    // HioEnvironment must be deterministic
    if(!rHioEnvironment.IsDeterministic()){
        rReportStr.append("##### fail: generator is not deterministic!\n");
        if(initStates.Size()>1) {
			 rErrStSet = initStates;
			 rReportStr.append("##### (amongst others, there is more than one initial state)\n");
			}
        finalResult = false;
    }


    rReportStr.append("#####\n");
    
    // test all conditions verifying I/O-environment form:
    
    /**************************** Condition (i) ***********************/
    localResult = true;
    rReportStr.append("########## Condition (i):\n");
    
    //YE, UE, YL, UL nonempty?
    if (ye.Empty()) {
       rReportStr.append("##### fail: empty YE alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (ue.Empty()) {
       rReportStr.append("##### fail: empty UE alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (yl.Empty()) {
       rReportStr.append("##### fail: empty YL alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (ul.Empty()) {
       rReportStr.append("##### fail: empty UL alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    
    // check for disjoint eventsets YE, YL, UE, UL and for
    // YE u YL u UE u UL == Sigma, ie unique HioEventFlags.
    // note: testing disjoint E- and L-Alphabet is sufficient
    //       as properties U and Y are exclusive by construltion.
        
    rErrEvSet=(rHioEnvironment.EEvents()*rHioEnvironment.LEvents()) + (rHioEnvironment.Alphabet()-rHioEnvironment.EEvents()-rHioEnvironment.LEvents());
    
    // In case of failing condition (i) further inspection is omitted, as too many consecutive faults are expected.
    if(!rErrEvSet.Empty()){
            rReportStr.append("##### fail: found events with missing or ambiguous attribute:\n");
            rReportStr.append(rErrEvSet.ToString()+"\n");
            rReportStr.append("##### Condition (i) failed.\n");
            rReportStr.append("########## Termination due to crucial failure. ##########\n");
            rReportStr.append("#########################################################\n");
            return false;
        }
    if(localResult) rReportStr.append("##### Condition (i) passed.\n");
    else rReportStr.append("##### Condition (i) failed.\n");
    rReportStr.append("#####\n");
    /*************************** Condition (i) finished *****************************/
    
    
    /*************************** Condition (ii) ***********************/ 
    localResult = true;
    rReportStr.append("########## Condition (ii):\n");
    
    // Check if in states QUl, QYlUe, QUe and QYe only UL-, YL-/UE-, UE and YE-events are active, respectively.
    // Also, dead ends are stored for condition (xi)
    for(sit = accessibleStates.Begin(); sit != accessibleStates.End(); ++sit) {
         
            bool isUl  = false;
            bool isYlUe = false;
            bool isUe = false;
            bool isYe = false;
            bool goodState = true;
               
            EventSet activeEv = rHioEnvironment.ActiveEventSet(*sit);
            
            if(activeEv.Empty()) {
                //dead ends violate condition (xi)
                deadEnds.Insert(*sit);
            }
            else {
                
                // get attribute of first event and compare with remaining events
                evit = activeEv.Begin();
                isUl  = rHioEnvironment.IsUl(*evit);
                isYlUe = rHioEnvironment.IsYl(*evit); // YlEvents can only be active in YlUe states
                isUe = rHioEnvironment.IsUe(*evit); // is reset (and YlUe is set) in case YlEvent is found in activeEv
                isYe = rHioEnvironment.IsYe(*evit);
                
                for(; evit != activeEv.End(); evit++) {
                    if( (isUl && !rHioEnvironment.IsUl(*evit)) ||						
						((isYlUe||isUe) && (!rHioEnvironment.IsYl(*evit) && (!rHioEnvironment.IsUe(*evit)))) || 
                        (isYe &&  !rHioEnvironment.IsYe(*evit)) ){
                        goodState = false;
                        localResult = false;
                        finalResult = false;
                        // add state to error set, go to next state
                        locErrStSet.Insert(*sit);
                        rErrStSet.Insert(*sit);
                        break; // leave loop over active events
                    }
                    if(isUe && rHioEnvironment.IsYl(*evit)) {
                        isYlUe = true;
                        isUe   = false;
                    }
                }
                
                activeEv.Clear();
                
                if(!goodState) continue; // if undecidable go on with next state
                
                // set state attribute
                if(isUl) {
                   rQUl.Insert(*sit);
                   rHioEnvironment.SetQUl(*sit);
                }
                else if(isYlUe) {
                   rQYlUe.Insert(*sit);
                   rHioEnvironment.SetQYlUe(*sit);
                }
                else if(isUe) {
                   rQUe.Insert(*sit);
                   rHioEnvironment.SetQUe(*sit);
                }
                else if(isYe){
                    rQYe.Insert(*sit);
                    rHioEnvironment.SetQYe(*sit);
                }
            }
        }
            
    if(localResult) rReportStr.append("##### Condition (ii) passed.\n");
     // In case of failing condition (ii) further inspection is omitted, as too many consecutive faults are expected.
    else {
        rReportStr.append("##### fail: found states with undecidable attribute:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (ii) failed.\n");
        rReportStr.append("########## Termination due to crulial failure. ##########\n");
        rReportStr.append("###################### No sulcess. ######################\n");
        rReportStr.append("#########################################################\n");
        return false;
    }
    rReportStr.append("#####\n");
    /*************************** Condition (ii) finished ****************************/  
    
    
    /*************************** Condition (iii) **********************/
    localResult = true;
    rReportStr.append("########## Condition (iii):\n");
    
    //check if the initial state is a QYe-state
    if(!(initStates <= rQYe)) {
        rReportStr.append("##### fail: some init state(s) is (are) not a QYe-state:\n");
		locErrStSet=initStates-rQYe;
		rReportStr.append(locErrStSet.ToString()+"\n");
        rErrStSet.InsertSet(locErrStSet);
		locErrStSet.Clear();
        localResult = false;
        finalResult = false;
    }
    if(localResult) rReportStr.append("##### Condition (iii) passed.\n");
    else rReportStr.append("##### Condition (iii) failed.\n");
    rReportStr.append("#####\n");
    
    /*************************** Condition (iii) finished ***************************/
    
    
    /*************************** Condition (iv) ***********************/
    localResult = true; 
    rReportStr.append("########## Condition (iv):\n");
    
    // YE-events have to lead to a QYlUe or a QUe-state
    for(sit = rQYe.Begin(); sit != rQYe.End(); ++sit) {
        for(tit = rHioEnvironment.TransRelBegin(*sit); tit != rHioEnvironment.TransRelEnd(*sit); ++tit) {
            // YE-event to QYlUe or QUe-state
            if ( !( rQYlUe.Exists(tit->X2) || rQUe.Exists(tit->X2) ) ) {
                // add transition to error transition set
                rErrTrSet.Insert(*tit);
                locErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            }
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (iv) passed.\n");
    else {
        rReportStr.append("##### fail: found YE-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (iv) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (iv) finished ****************************/
    
    
    /*************************** Condition (v) ************************/
    localResult = true;
    rReportStr.append("########## Condition (v):\n");
    
    // UE-events have to lead to a QYe-state
    for(sit = rQUe.Begin(); sit != rQUe.End(); ++sit) {
        for(tit = rHioEnvironment.TransRelBegin(*sit); tit != rHioEnvironment.TransRelEnd(*sit); ++tit) {
            if(!rQYe.Exists(tit->X2)) {
                rErrTrSet.Insert(*tit);
                locErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (v) passed.\n");
    else {
        rReportStr.append("##### fail: found UE-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (v) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (v) finished *****************************/
    
    
    /*************************** Condition (vi) ***********************/ 
    localResult = true;
    rReportStr.append("########## Condition (vi):\n");
    
    // From QYlUe states, UE-events have to lead to a QYe-state and YL-events
    // have to lead to a UL-state
    for(sit = rQYlUe.Begin(); sit != rQYlUe.End(); ++sit) {
        for(tit = rHioEnvironment.TransRelBegin(*sit); tit != rHioEnvironment.TransRelEnd(*sit); ++tit) {
            
            if( (rHioEnvironment.IsUe(tit->Ev) && !rQYe.Exists(tit->X2)) ||
                (rHioEnvironment.IsYl(tit->Ev) && !rQUl.Exists(tit->X2)) ){
                locErrTrSet.Insert(*tit);
                rErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            }    
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vi) passed.\n");
    else {
        rReportStr.append("##### fail: found YL- or UE-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (vi) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vi) finished ****************************/
    

    /*************************** Condition (vii) ************************/
    localResult = true;
    rReportStr.append("########## Condition (vii):\n");
    
    // UL-events have to lead to a QUe-state
    for(sit = rQUl.Begin(); sit != rQUl.End(); ++sit) {
        for(tit = rHioEnvironment.TransRelBegin(*sit); tit != rHioEnvironment.TransRelEnd(*sit); ++tit) {
            if(!rQUe.Exists(tit->X2)) {
                locErrTrSet.Insert(*tit);
                rErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vii) passed.\n");
    else {
        rReportStr.append("##### fail: found UL-transitions leading to wrong states, see rErrTrSet:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (vii) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vii) finished *****************************/

    
    /*************************** Condition (viii) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (viii):\n");
    
    // UL must be free in QUl-states
    for(sit = rQUl.Begin(); sit != rQUl.End(); ++sit) {
        
        if(!(ul <= rHioEnvironment.ActiveEventSet(*sit))) {
               locErrStSet.Insert(*sit);
               rErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (viii) passed.\n");
    else {
        rReportStr.append("##### fail: found QUl-states with inactive UL-events:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (viii) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (viii) finished ***************************/
    
    
    /*************************** Condition (ix) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (ix):\n");
    
    // YE must be free in QYe-states
    for(sit = rQYe.Begin(); sit != rQYe.End(); ++sit) {
        
        if(!(ye <= rHioEnvironment.ActiveEventSet(*sit))) {
               locErrStSet.Insert(*sit);
               rErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (ix) passed.\n");
    else {
        rReportStr.append("##### fail: found QYe-states with inactive YE-events:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (ix) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (ix) finished ***************************/
    
    
    /*************************** Condition (x) ***********************/
    localResult = true;
    rReportStr.append("########## Condition (x):\n");
    
    // Qm==Q?
    if(!(accessibleStates<=rHioEnvironment.MarkedStates())) {
        finalResult = false;
        localResult = false;
    }
    
    if(localResult) rReportStr.append("##### Condition (x) passed.\n");
    else {
        rReportStr.append("##### fail: not all accessible states are marked, see rErrStSet:\n");
        locErrStSet= accessibleStates - rHioEnvironment.MarkedStates();
		rErrStSet.InsertSet(locErrStSet);
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (x) failed.\n");
    }
    rReportStr.append("#####\n");       
    /*************************** Condition (x) finished ****************************/
    
    
    /*************************** Condition (xi) ************************/
    localResult = true;
    rReportStr.append("########## Condition (xi):\n");
    
    // found dead ends?
    if(!deadEnds.Empty()) {
        finalResult = false;
        localResult = false;
        rErrStSet.InsertSet(deadEnds);
        rReportStr.append("##### fail: found dead ends:\n");
		rReportStr.append(deadEnds.ToString()+"\n");
        deadEnds.Clear();
        rReportStr.append("##### Condition (xi) failed.\n");
    }
    rReportStr.append("##### Condition (xi) passed.\n");
    /*************************** Condition (xi) finished *****************************/
    
    
    /*************************** Condition (xii) ************************/
    rReportStr.append("########## Condition (xii):\n");
    
    // make accessible if necessary    
    if(!rHioEnvironment.IsAccessible()) {
        rHioEnvironment.Accessible();
        rReportStr.append("##### warning: non-accessible states have been removed.\n");
		rReportStr.append("##### Condition (xii) repaired.\n");
    }
    else rReportStr.append("##### Condition (xii) passed.\n");
    /*************************** Condition (xii) finished *****************************/
    
    
    
    /*************************** Final result ****************************/
    rReportStr.append("##################### Final result: #####################\n");
    if(finalResult) {
        rReportStr.append("######### Generator is in HioEnvironmentForm. #########\n");
        rReportStr.append("#########################################################\n");
        return true;
    }
    else {
        rReportStr.append("########### Generator is NOT in HioEnvironmentForm. ###########\n");
        rReportStr.append("#########################################################\n");
        return false;
    }

}// END OF IsHioEnvironmentForm()


//IsHioEnvironmentForm wrapper functions
bool IsHioEnvironmentForm(HioEnvironment& rHioEnvironment,std::string& rReportStr)
{
     StateSet QYe, QUe, QUl, QYlUe;
     EventSet ErrEvSet;
     TransSet ErrTrSet;
     StateSet ErrStSet;
     
     return IsHioEnvironmentForm(rHioEnvironment, QYe, QUe, QUl, QYlUe, ErrEvSet, ErrTrSet, ErrStSet, rReportStr);
}

// rti function interface
bool IsHioEnvironmentForm(HioEnvironment& rHioEnvironment)
{
     StateSet QYe, QUe, QUl, QYlUe;
     EventSet ErrEvSet;
     TransSet ErrTrSet;
     StateSet ErrStSet;
     std::string ReportStr;
     
     return IsHioEnvironmentForm(rHioEnvironment, QYe, QUe, QUl, QYlUe, ErrEvSet, ErrTrSet, ErrStSet, ReportStr);
}

// rti function interface
void HioStatePartition(HioEnvironment& rHioEnvironment) {
 IsHioEnvironmentForm(rHioEnvironment);
}


} // end namespace

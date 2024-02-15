/** @file hio_controller.cpp Generator with I/O-controller attributes */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/
 
#include "hio_controller.h"

namespace faudes {

// IsHioControllerForm()
bool IsHioControllerForm(HioController& rHioController,
                StateSet& rQUc,
                StateSet& rQYP,
                StateSet& rQUp,
                StateSet& rQYcUp,
                EventSet& rErrEvSet,
                TransSet& rErrTrSet,
                StateSet& rErrStSet,
                std::string& rReportStr)
    {
    FD_DF("IsHioControllerForm("<< rHioController.Name() << ",...)");
    
    // prepare results
    rQUc.Clear();
    rQYP.Clear();
    rQUp.Clear();
    rQYcUp.Clear();
    
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
    
    EventSet yp = rHioController.YpEvents();
    EventSet up = rHioController.UpEvents();
    EventSet yc = rHioController.YcEvents();
    EventSet uc = rHioController.UcEvents();
    
    StateSet initStates = rHioController.InitStates();
    StateSet accessibleStates = rHioController.AccessibleSet();
    StateSet deadEnds;

    EventSet::Iterator evit;
    StateSet::Iterator sit;
    TransSet::Iterator tit;
    
    // Info string header
    rReportStr.append("#########################################################\n");
    rReportStr.append("########## IsHioControllerForm("+rHioController.Name()+",...) - test results:\n");

    /**************************** Precondition: determinism ***********************/
    // HioController must be deterministic
    if(!rHioController.IsDeterministic()){
        rReportStr.append("##### fail: generator is not deterministic!\n");
        if(initStates.Size()>1) {
			 rErrStSet = initStates;
			 rReportStr.append("##### (amongst others, there is more than one initial state)\n");
			}
        finalResult = false;
    }

    rReportStr.append("#####\n");
    
    // test all conditions verifying I/O-controller form:
    
    /**************************** Condition (i) ***********************/
    localResult = true;
    rReportStr.append("########## Condition (i):\n");
    
    //YP, UP, YC, UC nonempty?
    if (yp.Empty()) {
       rReportStr.append("##### fail: empty YP alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (up.Empty()) {
       rReportStr.append("##### fail: empty UP alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (yc.Empty()) {
       rReportStr.append("##### fail: empty YC alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (uc.Empty()) {
       rReportStr.append("##### fail: empty UC alphabet.\n");
       localResult=false;
       finalResult = false;
    }
	
    // check for disjoint eventsets YP, YC, UP, UC and for
    // YP u YC u UP u UC == Sigma, ie unique HioEventFlags.
    // note: testing disjoint C- and P-Alphabet is sufficient
    //       as properties U and Y are exclusive by construction.
        
    rErrEvSet=(rHioController.PEvents()*rHioController.CEvents()) + (rHioController.Alphabet()-rHioController.PEvents()-rHioController.CEvents());
    
    // In case of failing condition (i) further inspection is omitted, as too many consecutive faults are expected.
    if(!rErrEvSet.Empty()){
            rReportStr.append("##### fail: found events with missing or ambiguous attribute, see rErrEvSet:\n");
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
    
    // Check if in states QUc, QYcUp, QUp and QYP only UC-, YC-/UP-, UP and YP-events are active, respectively.
    // Also, dead ends are stored for condition (xi)
    for(sit = accessibleStates.Begin(); sit != accessibleStates.End(); ++sit) {
         
            bool isUc  = false;
            bool isYcUp = false;
            bool isUp = false;
            bool isYp = false;
            bool goodState = true;
               
            EventSet activeEv = rHioController.ActiveEventSet(*sit);
            
            if(activeEv.Empty()) {
                //dead ends violate condition (xi)
                deadEnds.Insert(*sit);
            }
            else {
                
                // get attribute of first event and compare with remaining events
                evit = activeEv.Begin();
                isUc  = rHioController.IsUc(*evit);
                isYcUp = rHioController.IsYc(*evit); // YcEvents can only be active in YcUp states
                isUp = rHioController.IsUp(*evit); // is reset (and YcUp is set) in case YcEvent is found in activeEv
                isYp = rHioController.IsYp(*evit);
                
                for(; evit != activeEv.End(); evit++) {
                    if( (isUc && !rHioController.IsUc(*evit)) ||						
						((isYcUp||isUp) && (!rHioController.IsYc(*evit) && (!rHioController.IsUp(*evit)))) ||
                        (isYp &&  !rHioController.IsYp(*evit)) ){
                        goodState = false;
                        localResult = false;
                        finalResult = false;
                        // add state to error set, go to next state
                        locErrStSet.Insert(*sit);
                        break; // leave loop over active events
                    }
                    if(isUp && rHioController.IsYc(*evit)) {
                        isYcUp = true;
                        isUp   = false;
                    }
                }
                
                activeEv.Clear();
                
                if(!goodState) continue; // if undecidable go on with next state
                
                // set state attribute
                if(isUc) {
                   rQUc.Insert(*sit);
                   rHioController.SetQUc(*sit);
                }
                else if(isYcUp) {
                   rQYcUp.Insert(*sit);
                   rHioController.SetQYcUp(*sit);
                }
                else if(isUp) {
                   rQUp.Insert(*sit);
                   rHioController.SetQUp(*sit);
                }
                else if(isYp){
                    rQYP.Insert(*sit);
                    rHioController.SetQYp(*sit);
                }
            }
        }
            
    if(localResult) rReportStr.append("##### Condition (ii) passed.\n");
     // In case of failing condition (ii) further inspection is omitted, as too many consecutive faults are expected.
    else {
        rReportStr.append("##### fail: found states with undecidable attribute:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
        rReportStr.append("##### Condition (ii) failed.\n");
        rReportStr.append("########## Termination due to crucial failure. ##########\n");
        rReportStr.append("###################### No success. ######################\n");
        rReportStr.append("#########################################################\n");
		rErrStSet.InsertSet(locErrStSet);
		locErrStSet.Clear();
        return false;
    }
    rReportStr.append("#####\n");
    /*************************** Condition (ii) finished ****************************/  
    
    
    /*************************** Condition (iii) **********************/
    localResult = true;
    rReportStr.append("########## Condition (iii):\n");
    
    //check if the initial state is a QYP-state
    if(!(initStates <= rQYP)) {
        rReportStr.append("##### fail: some init state(s) is (are) not a QYP-state:\n");
		locErrStSet=initStates-rQYP;
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
    
    // YP-events have to lead to a QYcUp or a QUp-state
    for(sit = rQYP.Begin(); sit != rQYP.End(); ++sit) {
        for(tit = rHioController.TransRelBegin(*sit); tit != rHioController.TransRelEnd(*sit); ++tit) {
            // YP-event to QYcUp or QUp-state
            if ( !( rQYcUp.Exists(tit->X2) || rQUp.Exists(tit->X2) ) ) {
                // add transition to error transition set
				locErrTrSet.Insert(*tit);
                rErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            }
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (iv) passed.\n");
    else {
        rReportStr.append("##### fail: found YP-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (iv) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (iv) finished ****************************/
    
    
    /*************************** Condition (v) ************************/
    localResult = true;
    rReportStr.append("########## Condition (v):\n");
    
    // UP-events have to lead to a QYP-state
    for(sit = rQUp.Begin(); sit != rQUp.End(); ++sit) {
        for(tit = rHioController.TransRelBegin(*sit); tit != rHioController.TransRelEnd(*sit); ++tit) {
            if(!rQYP.Exists(tit->X2)) {
                locErrTrSet.Insert(*tit);
                rErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (v) passed.\n");
    else {
        rReportStr.append("##### fail: found UP-transitions leading to wrong states, see rErrTrSet:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (v) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (v) finished *****************************/
    
    
    /*************************** Condition (vi) ***********************/ 
    localResult = true;
    rReportStr.append("########## Condition (vi):\n");
    
    // From QYcUp states, UP-events have to lead to a QYP-state and YC-events
    // have to lead to a UC-state
    for(sit = rQYcUp.Begin(); sit != rQYcUp.End(); ++sit) {
        for(tit = rHioController.TransRelBegin(*sit); tit != rHioController.TransRelEnd(*sit); ++tit) {
            
            if( (rHioController.IsUp(tit->Ev) && !rQYP.Exists(tit->X2)) ||
                (rHioController.IsYc(tit->Ev) && !rQUc.Exists(tit->X2)) ){
                rErrTrSet.Insert(*tit);
                locErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            }    
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vi) passed.\n");
    else {
        rReportStr.append("##### fail: found YC- or UP-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (vi) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vi) finished ****************************/
    

    /*************************** Condition (vii) ************************/
    localResult = true;
    rReportStr.append("########## Condition (vii):\n");
    
    // UC-events have to lead to a QUp-state
    for(sit = rQUc.Begin(); sit != rQUc.End(); ++sit) {
        for(tit = rHioController.TransRelBegin(*sit); tit != rHioController.TransRelEnd(*sit); ++tit) {
            if(!rQUp.Exists(tit->X2)) {
                locErrTrSet.Insert(*tit);
                rErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vii) passed.\n");
    else {
        rReportStr.append("##### fail: found UC-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (vii) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vii) finished *****************************/

    
    /*************************** Condition (viii) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (viii):\n");
    
    // UC must be free in QUc-states
    for(sit = rQUc.Begin(); sit != rQUc.End(); ++sit) {
        
        if(!(uc <= rHioController.ActiveEventSet(*sit))) {
               locErrStSet.Insert(*sit);
               rErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (viii) passed.\n");
    else {
        rReportStr.append("##### fail: found QUc-states with inactive UC-events:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (viii) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (viii) finished ***************************/
    
    
    /*************************** Condition (ix) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (ix):\n");
    
    // YP must be free in QYP-states
    for(sit = rQYP.Begin(); sit != rQYP.End(); ++sit) {
        
        if(!(yp <= rHioController.ActiveEventSet(*sit))) {
               locErrStSet.Insert(*sit);
               rErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (ix) passed.\n");
    else {
        rReportStr.append("##### fail: found QYP-states with inactive YP-events:\n");
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
    if(!(accessibleStates<=rHioController.MarkedStates())) {
        finalResult = false;
        localResult = false;
    }
    
    if(localResult) rReportStr.append("##### Condition (x) passed.\n");
    else {
        rReportStr.append("##### fail: not all accessible states are marked:\n");
        locErrStSet = accessibleStates - rHioController.MarkedStates();
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
    if(!rHioController.IsAccessible()) {
        rHioController.Accessible();
        rReportStr.append("##### warning: non-accessible states have been removed.\n");
		rReportStr.append("##### Condition (xii) repaired.\n");
    }
    else rReportStr.append("##### Condition (xii) passed.\n");
    /*************************** Condition (xii) finished *****************************/
    
    
    
    /*************************** Final result ****************************/
    rReportStr.append("##################### Final result: #####################\n");
    if(finalResult) {
        rReportStr.append("######### Generator is in HioControllerForm. #########\n");
        rReportStr.append("#########################################################\n");
        return true;
    }
    else {
        rReportStr.append("########### Generator is NOT in HioControllerForm. ###########\n");
        rReportStr.append("#########################################################\n");
        return false;
    }

}// END OF IsHioControllerForm()

//IsHioControllerForm wrapper functions
bool IsHioControllerForm(HioController& rHioController,std::string& rReportStr)
{
     StateSet QUc, QYP, QUp, QYcUp;
     EventSet ErrEvSet;
     TransSet ErrTrSet;
     StateSet ErrStSet;
     
     return IsHioControllerForm(rHioController, QUc, QYP, QUp, QYcUp, ErrEvSet, ErrTrSet, ErrStSet, rReportStr);
}

bool IsHioControllerForm(HioController& rHioController)
{
     StateSet QUc, QYP, QUp, QYcUp;
     EventSet ErrEvSet;
     TransSet ErrTrSet;
     StateSet ErrStSet;
     std::string ReportStr;
     
     return IsHioControllerForm(rHioController, QUc, QYP, QUp, QYcUp, ErrEvSet, ErrTrSet, ErrStSet, ReportStr);
}

// rti function interface
void HioStatePartition(HioController& rHioController) {
 IsHioControllerForm(rHioController);
}

} // end namespace

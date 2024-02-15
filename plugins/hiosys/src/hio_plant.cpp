/** @file hio_plant.cpp Generator with I/O-plant attributes */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/
     
#include "hio_plant.h"

namespace faudes {

// IsHioPlantForm()
bool IsHioPlantForm(HioPlant& rHioPlant,
            StateSet& rQYpYe,
            StateSet& rQUp,
            StateSet& rQUe,
            EventSet& rErrEvSet,
            TransSet& rErrTrSet,
            StateSet& rErrStSet,
            std::string& rReportStr)
    {
    FD_DF("IsHioPlantForm("<< rHioPlant.Name() << ",...)");
    
    // prepare results
    rQYpYe.Clear();
    rQUp.Clear();
    rQUe.Clear();
    
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
    
    EventSet yp = rHioPlant.YpEvents();
    EventSet up = rHioPlant.UpEvents();
    EventSet ye = rHioPlant.YeEvents();
    EventSet ue = rHioPlant.UeEvents();
    
    StateSet initStates = rHioPlant.InitStates();
    StateSet accessibleStates = rHioPlant.AccessibleSet();

    EventSet::Iterator evit;
    StateSet::Iterator sit;
    TransSet::Iterator tit;
    
    // Info string header
    rReportStr.append("#########################################################\n");
    rReportStr.append("########## IsHioPlantForm("+rHioPlant.Name()+",...) - test results:\n");

    /**************************** Precondition: determinism ***********************/
    // HioPlant must be deterministic
    if(!rHioPlant.IsDeterministic()){
        rReportStr.append("##### fail: generator is not deterministic!\n");
        if(initStates.Size()>1) {
			 rErrStSet = initStates;
			 rReportStr.append("##### (amongst others, there is more than one initial state)\n");
			}
        finalResult = false;
    }

    rReportStr.append("#####\n");
    
    // test all conditions verifying I/O-plant form:
    
    /**************************** Condition (i) ***********************/
    localResult = true;
    rReportStr.append("########## Condition (i):\n");
    
    //YP, UP, YE, UE nonempty?
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
    
    // check for disjoint eventsets YP, YE, UP, UE and for
    // YP u YE u UP u UE == Sigma, ie unique HioEventFlags.
    // note: testing disjoint P- and E-Alphabet is sufficient
    //       as properties U and Y are exclusive by construction.
        
    rErrEvSet=(rHioPlant.PEvents()*rHioPlant.EEvents()) + (rHioPlant.Alphabet()-rHioPlant.PEvents()-rHioPlant.EEvents());
    
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
    
    // check if in states QYpYe, QUp and QUe only Y-, UP- and UE-events are active, respectively.
    for(sit = accessibleStates.Begin(); sit != accessibleStates.End(); ++sit) {
         
            bool isY  = false;
            bool isUp = false;
            bool isUe = false;
            bool goodState = true;
               
            EventSet activeEv = rHioPlant.ActiveEventSet(*sit);
            
            if(activeEv.Empty()) {
                //deadlocks are always QYpYe -states
                rQYpYe.Insert(*sit);
                rHioPlant.SetQYpYe(*sit);
            }
            else {
                
                // get attribute of first event and compare with remaining events
                evit = activeEv.Begin();
                isY  = rHioPlant.IsY(*evit);
                isUp = rHioPlant.IsUp(*evit);
                isUe = rHioPlant.IsUe(*evit);
                
                for(; evit != activeEv.End(); evit++) {
                    if( (isY &&  !rHioPlant.IsY(*evit)) || 
                        (isUp && !rHioPlant.IsUp(*evit)) ||
                        (isUe && !rHioPlant.IsUe(*evit)) ) {
                        goodState = false;
                        localResult = false;
                        finalResult = false;
                        // add state to error set, go to next state
                        locErrStSet.Insert(*sit);
                        rErrStSet.Insert(*sit);
                        break; // leave loop over active events
                    }
                }
                
                activeEv.Clear();
                
                if(!goodState) continue; // if undecidable go on with next state
                
                // set state attribute
                if(isY) {
                   rQYpYe.Insert(*sit);
                   rHioPlant.SetQYpYe(*sit);
                }
                else if(isUp) {
                   rQUp.Insert(*sit);
                   rHioPlant.SetQUp(*sit);
                }
                else if(isUe){
                    rQUe.Insert(*sit);
                    rHioPlant.SetQUe(*sit);
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
        rReportStr.append("########## Termination due to crucial failure. ##########\n");
        rReportStr.append("###################### No success. ######################\n");
        rReportStr.append("#########################################################\n");
        return false;
    }
    rReportStr.append("#####\n");
    /*************************** Condition (ii) finished ****************************/  
    
    
    /*************************** Condition (iii) **********************/
    localResult = true;
    rReportStr.append("########## Condition (iii):\n");
    
    //check if the initial state is a QYpYe-state
    if(!(initStates <= rQYpYe)) {
        rReportStr.append("##### fail: some init state(s) is (are) not a QYpYe-state:\n");
		locErrStSet=initStates-rQYpYe;
        rErrStSet.InsertSet(locErrStSet);
        rReportStr.append(locErrStSet.ToString()+"\n");
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
    
    // YP-events have to lead to a QUp-state, while a YE-events
    // have to lead to a QUe-state.
    for(sit = rQYpYe.Begin(); sit != rQYpYe.End(); ++sit) {
        for(tit = rHioPlant.TransRelBegin(*sit); tit != rHioPlant.TransRelEnd(*sit); ++tit) {
            // YP-event to QUp-state, YE-event to QUe-state
            if( (rHioPlant.IsYp(tit->Ev) && !(rQUp.Exists(tit->X2) || rQUe.Exists(tit->X2)))) {
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
        rReportStr.append("##### fail: found YP- or YE-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (iv) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (iv) finished ****************************/
    
    
    /*************************** Condition (v) ************************/
    localResult = true;
    rReportStr.append("########## Condition (v):\n");
    
    // UP-events have to lead to a QYpYe-state
    for(sit = rQUp.Begin(); sit != rQUp.End(); ++sit) {
        for(tit = rHioPlant.TransRelBegin(*sit); tit != rHioPlant.TransRelEnd(*sit); ++tit) {
            if(!rQYpYe.Exists(tit->X2)) {
                rErrTrSet.Insert(*tit);
                locErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (v) passed.\n");
    else {
        rReportStr.append("##### fail: found UP-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (v) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (v) finished *****************************/
    
    
    /*************************** Condition (vi) ***********************/ 
    localResult = true;
    rReportStr.append("########## Condition (vi):\n");
    
    // UE-events have to lead to a QYpYe-state
    for(sit = rQUe.Begin(); sit != rQUe.End(); ++sit) {
        for(tit = rHioPlant.TransRelBegin(*sit); tit != rHioPlant.TransRelEnd(*sit); ++tit) {
            if(!rQYpYe.Exists(tit->X2)) {
                rErrTrSet.Insert(*tit);
                locErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vi) passed.\n");
    else {
        rReportStr.append("##### fail: found UE-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (vi) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vi) finished ****************************/
    
    
    /*************************** Condition (vii) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (vii):\n");
    
    // UP must be free in QUp-states
    for(sit = rQUp.Begin(); sit != rQUp.End(); ++sit) {
        
        if(!(up <= rHioPlant.ActiveEventSet(*sit))) {
               rErrStSet.Insert(*sit);
               locErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vii) passed.\n");
    else {
        rReportStr.append("##### fail: found QUp-states with inactive UP-events:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (vii) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vii) finished ***************************/
    
    
    /*************************** Condition (viii) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (viii):\n");
    
    // UE must be free in QUe-states
    for(sit = rQUe.Begin(); sit != rQUe.End(); ++sit) {
        
        if(!(ue <= rHioPlant.ActiveEventSet(*sit))) {
               rErrStSet.Insert(*sit);
               locErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (viii) passed.\n");
    else {
        rReportStr.append("##### fail: found QUe-states with inactive UE-events:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (viii) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vii) finished ***************************/
    
    //###### Condition ix is outdated since we introduced marking!!! 
    // /*************************** Condition (ix) ***********************/
    // localResult = true;
    // rReportStr.append("########## Condition (ix):\n");
    
    // // Qm==Q?
    // if(!(accessibleStates<=rHioPlant.MarkedStates())) {
        // finalResult = false;
        // localResult = false;
    // }
    
    // if(localResult) rReportStr.append("##### Condition (ix) passed.\n");
    // else {
        // rReportStr.append("##### fail: not all accessible states are marked:\n");
        // locErrStSet = accessibleStates - rHioPlant.MarkedStates();
		// rErrStSet.InsertSet(locErrStSet);
        // rReportStr.append(locErrStSet.ToString()+"\n");
		// locErrStSet.Clear();
        // rReportStr.append("##### Condition (ix) failed.\n");
    // }
    // rReportStr.append("#####\n");       
    // /*************************** Condition (ix) finished ****************************/
    
    
    /*************************** Condition (x) ************************/
    rReportStr.append("########## Condition (x):\n");
    
    // make accessible if necessary    
    if(!rHioPlant.IsAccessible()) {
        rHioPlant.Accessible();
        rReportStr.append("##### warning: non-accessible states have been removed.\n");
		rReportStr.append("##### Condition (x) repaired.\n");
    }
    else rReportStr.append("##### Condition (x) passed.\n");
    /*************************** Condition (x) finished *****************************/
    
    
    
    /*************************** Final Result ************************/
    
    rReportStr.append("##################### Final result: #####################\n");
    if(finalResult) {
        rReportStr.append("############## Generator is in HioPlantForm. ##############\n");
        rReportStr.append("#########################################################\n");
        return true;
    }
    else {
        rReportStr.append("############ Generator is NOT in HioPlantForm. ###########\n");
        rReportStr.append("#########################################################\n");
        return false;
    }

}// END OF IsHioPlantForm()
    

//IsHioPlantForm wrapper functions
bool IsHioPlantForm(HioPlant& rHioPlant, std::string& rReportStr)
{
     StateSet QYpYe, QUp, QUe;
     EventSet ErrEvSet;
     TransSet ErrTrSet;
     StateSet ErrStSet;
    
    return IsHioPlantForm(rHioPlant, QYpYe, QUp, QUe, ErrEvSet, ErrTrSet, ErrStSet,rReportStr);
}
    
// rti function interface
bool IsHioPlantForm(HioPlant& rHioPlant)
{
     StateSet QYpYe, QUp, QUe;
     EventSet ErrEvSet;
     TransSet ErrTrSet;
     StateSet ErrStSet;
     std::string ReportStr;
     
     return IsHioPlantForm(rHioPlant, QYpYe, QUp, QUe, ErrEvSet, ErrTrSet, ErrStSet,ReportStr);
}

// rti function interface
void HioStatePartition(HioPlant& rHioPlant) {
 IsHioPlantForm(rHioPlant);
}

} // end namespace

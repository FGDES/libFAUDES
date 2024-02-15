/** @file hio_constraint.cpp Generator with I/O-constraint attributes */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/
     
#include "hio_constraint.h"

namespace faudes {

// IsHioConstraintForm()
bool IsHioConstraintForm(HioConstraint& rHioConstraint,
            StateSet& rQY,
            StateSet& rQU,
            EventSet& rErrEvSet,
            TransSet& rErrTrSet,
            StateSet& rErrStSet,
            std::string& rReportStr)
    {
    FD_DF("IsHioConstraintForm("<< rHioConstraint.Name() << ",...)");
    
    // prepare results
    rQY.Clear();
    rQU.Clear();
    
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
    
    EventSet y = rHioConstraint.YEvents();
    EventSet u = rHioConstraint.UEvents();
    
    StateSet initStates = rHioConstraint.InitStates();
    StateSet accessibleStates = rHioConstraint.AccessibleSet();
    StateSet deadEnds;

    EventSet::Iterator evit;
    StateSet::Iterator sit;
    TransSet::Iterator tit;
    
    // Info string header
    rReportStr.append("#########################################################\n");
    rReportStr.append("########## IsHioConstraintForm("+rHioConstraint.Name()+",...) - test results:\n");

    /**************************** Precondition: determinism ***********************/
    // HioConstraint must be deterministic
    if(!rHioConstraint.IsDeterministic()){
        rReportStr.append("##### fail: generator is not deterministic!\n");
        if(initStates.Size()>1) {
			 rErrStSet = initStates;
			 rReportStr.append("##### (amongst others, there is more than one initial state)\n");
			}
        finalResult = false;
    }

    rReportStr.append("#####\n");
    
    // test all conditions verifying I/O-constraint form:
    
    /**************************** Condition (i) ***********************/
    localResult = true;
    rReportStr.append("########## Condition (i):\n");
    
    //YP, UP, YE, UE nonempty?
    if (y.Empty()) {
       rReportStr.append("##### fail: empty Y alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    if (u.Empty()) {
       rReportStr.append("##### fail: empty U alphabet.\n");
       localResult=false;
       finalResult = false;
    }
    
    // check for disjoint eventsets Y and U and for 
    // Y u U == Sigma, ie unique HioEventFlags.
    // note: by construction, any event has the exclusive property U or Y.
	// 	    thus, condition (i) is always met at this point.
        
    rReportStr.append("##### Condition (i) passed.\n");
    rReportStr.append("#####\n");
    /*************************** Condition (i) finished *****************************/
    
    
    /*************************** Condition (ii) ***********************/ 
    localResult = true;
    rReportStr.append("########## Condition (ii):\n");
    
    // check if in states QYpYe, QUp and QUe only Y-, UP- and UE-events are active, respectively.
    for(sit = accessibleStates.Begin(); sit != accessibleStates.End(); ++sit) {
         
            bool isY  = false;
            bool isU = false;
            bool goodState = true;
               
            EventSet activeEv = rHioConstraint.ActiveEventSet(*sit);
            
            if(activeEv.Empty()) {
                //dead ends violate condition (vii)
                deadEnds.Insert(*sit);
            }
            else {
                
                // get attribute of first event and compare with remaining events
                evit = activeEv.Begin();
                isY  = rHioConstraint.IsY(*evit);
                isU  = rHioConstraint.IsU(*evit);
                
                for(; evit != activeEv.End(); evit++) {
                    if( (isY &&  !rHioConstraint.IsY(*evit)) || 
                        (isU && !rHioConstraint.IsU(*evit))) {
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
                   rQY.Insert(*sit);
                   rHioConstraint.SetQY(*sit);
                }
                else if(isU) {
                   rQU.Insert(*sit);
                   rHioConstraint.SetQU(*sit);
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
    
    //check if the initial state is a QY-state
    if(!(initStates <= rQY)) {
        rReportStr.append("##### fail: some init state(s) is (are) not a QY-state:\n");
		locErrStSet=initStates-rQY;
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
    
    // Y-events have to lead to a QU-state
    for(sit = rQY.Begin(); sit != rQY.End(); ++sit) {
        for(tit = rHioConstraint.TransRelBegin(*sit); tit != rHioConstraint.TransRelEnd(*sit); ++tit) {
            // Y-event to QU-state
            if( (rHioConstraint.IsY(tit->Ev) && !rQU.Exists(tit->X2)) ) {
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
        rReportStr.append("##### fail: found Y-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (iv) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (iv) finished ****************************/
    
    
    /*************************** Condition (v) ************************/
    localResult = true;
    rReportStr.append("########## Condition (v):\n");
    
    // U-events have to lead to a QY-state
    for(sit = rQU.Begin(); sit != rQU.End(); ++sit) {
        for(tit = rHioConstraint.TransRelBegin(*sit); tit != rHioConstraint.TransRelEnd(*sit); ++tit) {
            if(!rQY.Exists(tit->X2)) {
                locErrTrSet.Insert(*tit);
                rErrTrSet.Insert(*tit);
                finalResult = false;
                localResult = false;
            } 
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (v) passed.\n");
    else {
        rReportStr.append("##### fail: found U-transitions leading to wrong states:\n");
        rReportStr.append(locErrTrSet.ToString()+"\n");
		locErrTrSet.Clear();
        rReportStr.append("##### Condition (v) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (v) finished *****************************/
    
    
    /*************************** Condition (vi) **********************/    
    localResult = true;
    rReportStr.append("########## Condition (vi):\n");
    
    // Y must be free in QY-states
    for(sit = rQY.Begin(); sit != rQY.End(); ++sit) {
        
        if(!(y <= rHioConstraint.ActiveEventSet(*sit))) {
               locErrStSet.Insert(*sit);
               rErrStSet.Insert(*sit);
               finalResult = false;
               localResult = false;
        }
    }
    
    if(localResult) rReportStr.append("##### Condition (vi) passed.\n");
    else {
        rReportStr.append("##### fail: found QY-states with inactive Y-events:\n");
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (vi) failed.\n");
    }
    rReportStr.append("#####\n");
    /*************************** Condition (vi) finished ***************************/
    
    
    /*************************** Condition (vii) ************************/
    localResult = true;
    rReportStr.append("########## Condition (vii):\n");
    
    // found dead ends?
    if(!deadEnds.Empty()) {
        finalResult = false;
        localResult = false;
        rErrStSet.InsertSet(deadEnds);
        rReportStr.append("##### fail: found dead ends:\n");
		rReportStr.append(deadEnds.ToString()+"\n");
        deadEnds.Clear();
        rReportStr.append("##### Condition (vii) failed.\n");
    }
    rReportStr.append("##### Condition (vii) passed.\n");
    /*************************** Condition (vii) finished *****************************/
    
    
    /*************************** Condition (viii) ***********************/
    localResult = true;
    rReportStr.append("########## Condition (viii):\n");
    
    // Qm==Q?
    if(!(accessibleStates<=rHioConstraint.MarkedStates())) {
        finalResult = false;
        localResult = false;
    }
    
    if(localResult) rReportStr.append("##### Condition (viii) passed.\n");
    else {
        rReportStr.append("##### fail: not all accessible states are marked:\n");
        locErrStSet=accessibleStates - rHioConstraint.MarkedStates();
		rErrStSet.InsertSet(locErrStSet);
        rReportStr.append(locErrStSet.ToString()+"\n");
		locErrStSet.Clear();
        rReportStr.append("##### Condition (viii) failed.\n");
    }
    rReportStr.append("#####\n");       
    /*************************** Condition (viii) finished ****************************/
    
    
    /*************************** Condition (ix) ************************/
    rReportStr.append("########## Condition (ix):\n");
    
    // make accessible if necessary    
    if(!rHioConstraint.IsAccessible()) {
        rHioConstraint.Accessible();
        rReportStr.append("##### warning: non-accessible states have been removed.\n");
		rReportStr.append("##### Condition (ix) repaired.\n");
    }
    else rReportStr.append("##### Condition (ix) passed.\n");
    /*************************** Condition (ix) finished *****************************/
    
    
    
    /*************************** Final Result ************************/
    
    rReportStr.append("##################### Final result: #####################\n");
    if(finalResult) {
        rReportStr.append("############ Generator is in HioConstraintForm. ###########\n");
        rReportStr.append("#########################################################\n");
        return true;
    }
    else {
        rReportStr.append("############## Generator is NOT in HioConstraintForm. ##############\n");
        rReportStr.append("#########################################################\n");
        return false;
    }

}// END OF IsHioConstraintForm()
    
//IsHioConstraintForm wrapper functions
bool IsHioConstraintForm(HioConstraint& rHioConstraint, std::string& rReportStr)
{
     StateSet rQY, rQU;
     EventSet rErrEvSet;
     TransSet rErrTrSet;
     StateSet rErrStSet;
    
    return IsHioConstraintForm(rHioConstraint, rQY, rQU, rErrEvSet, rErrTrSet, rErrStSet,rReportStr);
}
    
bool IsHioConstraintForm(HioConstraint& rHioConstraint)
{
     StateSet rQY, rQU;
     EventSet rErrEvSet;
     TransSet rErrTrSet;
     StateSet rErrStSet;
     std::string rReportStr;
     
     return IsHioConstraintForm(rHioConstraint, rQY, rQU, rErrEvSet, rErrTrSet, rErrStSet, rReportStr);
}

// rti function interface
void HioStatePartition(HioConstraint& rHioConstraint) {
 IsHioConstraintForm(rHioConstraint);
}



} // end namespace

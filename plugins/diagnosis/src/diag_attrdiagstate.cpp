/** @file diag_attrdiagstate.cpp
Implements state estimates for the current status of the generator (as state attributes).
*/

#include "diag_attrdiagstate.h"

using namespace std;

namespace faudes {

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeDiagnoserState,AttributeFlags)

// consruct
AttributeDiagnoserState::AttributeDiagnoserState(void) {
  // our map type is not in the registry, so we need to configure manually
  mDiagnoserStateMap.ElementTag("TargetState"); 
  mDiagnoserStateMap.Name("TargetStateEstimates"); 
  mDiagnoserStateMap.TypeName("TargetStateEstimates"); 
}
  
// destruct
AttributeDiagnoserState::~AttributeDiagnoserState(void) {}


// copy my members
void AttributeDiagnoserState::DoAssign(const AttributeDiagnoserState& rSrcAttr) {
  AttributeFlags::DoAssign(rSrcAttr); 
  mDiagnoserStateMap=rSrcAttr.mDiagnoserStateMap;
}

// IsDefault()
bool AttributeDiagnoserState::IsDefault(void) const {
  return mDiagnoserStateMap.Empty();
}

// Clear()
void AttributeDiagnoserState::Clear(void) {
  mDiagnoserStateMap.Clear();
}

// DiagnoserStateMap()
const TaIndexSet<DiagLabelSet>& AttributeDiagnoserState::DiagnoserStateMap(void) const {
  return mDiagnoserStateMap;
}

// DiagnoserStateMapp()
const TaIndexSet<DiagLabelSet>* AttributeDiagnoserState::DiagnoserStateMapp(void) const {
  return &mDiagnoserStateMap;
}

// DiagnoserStateMap()
void AttributeDiagnoserState::DiagnoserStateMap(const TaIndexSet<DiagLabelSet>& newDiagStateMap) {
  mDiagnoserStateMap = newDiagStateMap;
}

// AddStateLabelMapping()
void AttributeDiagnoserState::AddStateLabelMapping(Idx gstate, Idx label) {
  DiagLabelSet hset;
  
  // if gstate is already mapped from, the corresponding DiagLabelSet is loaded
  if(mDiagnoserStateMap.Exists(gstate)) {
    hset = mDiagnoserStateMap.Attribute(gstate);
  }
  // and the new mapping is added
  hset.mDiagLabels.Insert(label);
  mDiagnoserStateMap.Insert(gstate,hset);
}

// AddStateLabelMap()
void AttributeDiagnoserState::AddStateLabelMap(Idx gstate, const DiagLabelSet& labels) {
  NameSet::Iterator it;
  DiagLabelSet hset;
  
  // if gstate is already mapped from, the corresponding DiagLabelSet is loaded
  if(mDiagnoserStateMap.Exists(gstate)) {
    hset = mDiagnoserStateMap.Attribute(gstate);
  }
  // add labels
  for(it = labels.mDiagLabels.Begin(); it != labels.mDiagLabels.End(); it++) {
    hset.mDiagLabels.Insert(*it);
  }
  // set
  mDiagnoserStateMap.Insert(gstate,hset);
}

// Str()
string AttributeDiagnoserState::Str(void) const {
  TaIndexSet<DiagLabelSet>::Iterator it;
  string str;
  DiagLabelSet label;
  
  for (it = mDiagnoserStateMap.Begin(); it != mDiagnoserStateMap.End(); it++) {
    label = mDiagnoserStateMap.Attribute(*it);
    if (it != mDiagnoserStateMap.Begin()) {
      str += " ";
    }
    str += ToStringInteger(*it);
    str += label.Str();
  }
  return str;
}

// ExistsState()
// not used
bool AttributeDiagnoserState::ExistsState(Idx state) const {
  return mDiagnoserStateMap.Exists(state);
}

// DoWrite()
void AttributeDiagnoserState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mDiagnoserStateMap.Write(rTw,"StateEstimates", pContext);
}

// DoXWrite()
void AttributeDiagnoserState::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mDiagnoserStateMap.XWrite(rTw,"TargetStateEstimates", pContext);
}

//DoRead()
void AttributeDiagnoserState::DoRead(TokenReader &rTr, const std::string &rLabel, const Type *pContext) {
  mDiagnoserStateMap.Clear();
  Token token;
  rTr.Peek(token);
  // native faudes
  if(token.IsBegin("StateEstimates")) 
   mDiagnoserStateMap.Read(rTr,"",pContext);
  // XML format
  if(token.IsBegin("TargetStateEstimates")) 
   mDiagnoserStateMap.Read(rTr,"",pContext);
}

// Equality
bool AttributeDiagnoserState::DoEqual(const AttributeDiagnoserState& rOtherAttribute) const {
  // equal set
  if(mDiagnoserStateMap != rOtherAttribute.mDiagnoserStateMap) return false;
  // equal attributes
  if(!mDiagnoserStateMap.EqualAttributes(rOtherAttribute.mDiagnoserStateMap)) return false;
  // pass
  return true;
}


} // namespace faudes


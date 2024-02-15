/** @file diag_attrfailuretypes.cpp
Failure and indicator partition for a system (used as global attribute).
*/

#include "diag_attrfailuretypes.h"

using namespace std;

namespace faudes {

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeFailureTypeMap,AttributeFlags)

// construct
AttributeFailureTypeMap::AttributeFailureTypeMap(void) : AttributeFlags() {
  // configure unregistered type
  // note: its just my member which is not registered, the attribute is registered
  mFailureTypeMap.TypeName("FailureTypes");
  mFailureTypeMap.XElementTag("FailureType");
  mFailureTypeMap.Name("FailureTypes");
  // always use static symbol table
  mFailureTypeMap.SymbolTablep(DiagLabelSet::StaticLabelSymbolTablep());
}
  
// comnstruct from file
AttributeFailureTypeMap::AttributeFailureTypeMap(const std::string& rFilename)  : AttributeFlags() {
  // configure unregistered type
  // note: its just my member which is not registered, the attribute is registered
  mFailureTypeMap.TypeName("FailureTypes");
  mFailureTypeMap.XElementTag("FailureType");
  mFailureTypeMap.Name("FailureTypes");
  // always use static symbol table
  mFailureTypeMap.SymbolTablep(DiagLabelSet::StaticLabelSymbolTablep());
  // read
  Read(rFilename);
}
  
// copycontruct
AttributeFailureTypeMap::AttributeFailureTypeMap(const AttributeFailureTypeMap& rOtherAttr)  : AttributeFlags(rOtherAttr) {
  // configure unregistered type
  // note: its just my member which is not registered, the attribute is registered
  mFailureTypeMap.TypeName("FailureTypes");
  mFailureTypeMap.XElementTag("FailureType");
  mFailureTypeMap.Name("FailureTypes");
  // always use static symbol table
  mFailureTypeMap.SymbolTablep(DiagLabelSet::StaticLabelSymbolTablep());
  // copy
  DoAssign(rOtherAttr);
}


// copy my members
void AttributeFailureTypeMap::DoAssign(const AttributeFailureTypeMap& rSrcAttr) {
  AttributeFlags::DoAssign(rSrcAttr); 
  mFailureTypeMap = rSrcAttr.mFailureTypeMap; 
}

// Equality
bool AttributeFailureTypeMap::DoEqual(const AttributeFailureTypeMap& rAttr) const {
  // test base
  if(!AttributeFlags::DoEqual(rAttr)) return false;
  // my members
  if(mFailureTypeMap!=rAttr.mFailureTypeMap) return false; 
  // pass
  return true; 
} 


// IsDefault()
bool AttributeFailureTypeMap::IsDefault(void) const{
  return mFailureTypeMap.Empty();
}

// Empty()
bool AttributeFailureTypeMap::Empty(void) const {
  return mFailureTypeMap.Empty();
}

// Clear()
void AttributeFailureTypeMap::Clear(void) {
  mFailureTypeMap.Clear();
}

// AddFailureTypeMapping()
Idx AttributeFailureTypeMap::AddFailureTypeMapping(const std::string& failureType, const EventSet& rfailureEvents) {
  Idx failureLabelIndex;
  SymbolTable* mpLabelSymbolTable = DiagLabelSet::StaticLabelSymbolTablep();
  
  // Insert failureType in msLabelSymbolTable of DiagLabelSet.
  // If failure type name does already exist in LabelSymbolTable, InsEntry returns index of existing entry.
  try {
    failureLabelIndex = mpLabelSymbolTable->InsEntry(failureType);
  }
  catch (faudes::Exception& exception) {
    stringstream errstr;
    errstr << "Have not been able to add failure with Index " << failureType << " to LabelSymbolTable" << endl;
    throw Exception("AttributeFailureTypeMap::AddFailureTypeMapping()", errstr.str(), 300);
  }
  // store failure event set in AttributeFailureEvents
  AttributeFailureEvents fEvents;
  fEvents.mFailureEvents.InsertSet(rfailureEvents);
      
  // write new map entry
  mFailureTypeMap.Insert(failureLabelIndex,fEvents);
  
  return failureLabelIndex;  
}

// AddFailureTypeMap()
void AttributeFailureTypeMap::AddFailureTypeMap(const std::map<std::string,EventSet>& rFailureMap) {
  map<string,EventSet>::const_iterator it;
  
  for(it = rFailureMap.begin(); it != rFailureMap.end(); it++) {
    AddFailureTypeMapping(it->first,it->second);
  }
}

// FailureType()
// not used
Idx AttributeFailureTypeMap::FailureType(Idx failureEvent) const {
  TaNameSet<AttributeFailureEvents>::Iterator it;  

  for(it = mFailureTypeMap.Begin(); it != mFailureTypeMap.End(); it++) {
    if (mFailureTypeMap.Attribute(*it).mFailureEvents.Exists(failureEvent)) {
      return *it;
    }
  }
  return 0;
}

// AllFailureEvents()
EventSet AttributeFailureTypeMap::AllFailureEvents(void) const {
  EventSet failures;
  AttributeFailureEvents failureAttr;
  TaNameSet<AttributeFailureEvents>::Iterator it;  
  
  failures.Clear();
  for(it = mFailureTypeMap.Begin(); it != mFailureTypeMap.End(); it++) {
    failureAttr = mFailureTypeMap.Attribute(*it);
    failures.InsertSet(failureAttr.mFailureEvents);
  }  
  return failures;
}

// DoWrite()
void AttributeFailureTypeMap::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mFailureTypeMap.Write(rTw,"FailureTypes", pContext);
}

// DoXWrite()
void AttributeFailureTypeMap::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mFailureTypeMap.XWrite(rTw,"FailureTypes", pContext);
}

// DoRead()
void AttributeFailureTypeMap::DoRead(TokenReader &rTr, const std::string &rLabel, const Type *pContext) {
  mFailureTypeMap.Clear();
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin("FailureTypes")) return;
  mFailureTypeMap.Read(rTr, "FailureTypes", pContext);
}


} // namespace faudes


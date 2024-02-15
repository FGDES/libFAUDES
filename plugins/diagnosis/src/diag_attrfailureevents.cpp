/** @file diag_attrfailureevents.cpp
    Implements failure and indicator events belonging to a common partition.
*/

#include "diag_attrfailureevents.h"

using namespace std;

namespace faudes {

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeFailureEvents,AttributeFlags)

// Construct
AttributeFailureEvents::AttributeFailureEvents(void) {
  // configure unregistered set type
  mFailureEvents.Name("FailureEvents");
  mFailureEvents.TypeName("FailureEvents");
  mFailureEvents.XElementTag("Event");
  mIndicatorEvents.Name("IndicatorEvents");
  mIndicatorEvents.TypeName("IndicatorEvents");
  mIndicatorEvents.XElementTag("Event");
}


// DoAssign()
void AttributeFailureEvents::DoAssign(const AttributeFailureEvents& rSrcAttr){
  AttributeFlags::DoAssign(rSrcAttr); 
  mFailureEvents = rSrcAttr.mFailureEvents; 
  mIndicatorEvents = rSrcAttr.mIndicatorEvents;
}


// Equality
bool AttributeFailureEvents::DoEuqal(const AttributeFailureEvents& rAttr) const {
  // test base
  if(!AttributeFlags::DoEqual(rAttr)) return false;
  // my members
  if(mFailureEvents!=rAttr.mFailureEvents) return false; 
  if(mIndicatorEvents!=rAttr.mIndicatorEvents) return false;
  // pass
  return true; 
} 

// IsDefault()
bool AttributeFailureEvents::IsDefault(void) const{
  return (mFailureEvents.Empty() && mIndicatorEvents.Empty());
}

// Clear()
void AttributeFailureEvents::Clear(void) {
  mFailureEvents.Clear();
  mIndicatorEvents.Clear();
}

// DoWrite()
void AttributeFailureEvents::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  AttributeFlags::DoWrite(rTw,"",pContext);
  if(!mFailureEvents.Empty())
    mFailureEvents.Write(rTw,"FailureEvents", pContext);
  if(!mIndicatorEvents.Empty())
    mIndicatorEvents.Write(rTw,"IndicatorEvents", pContext);
}

// DoXWrite()
void AttributeFailureEvents::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  AttributeFlags::DoXWrite(rTw,"",pContext);
  if(!mFailureEvents.Empty())
    mFailureEvents.XWrite(rTw,"FailureEvents", pContext);
  if(!mIndicatorEvents.Empty())
    mIndicatorEvents.XWrite(rTw,"IndicatorEvents", pContext);
}



// DoRead()
void AttributeFailureEvents::DoRead(TokenReader &rTr, const std::string &rLabel, const Type *pContext) {
  AttributeFlags::DoRead(rTr,"",pContext);
  Token token;
  while(1) {
    rTr.Peek(token);
    if(token.Type()!=Token::Begin) break;
    if(token.StringValue()=="FailureEvents") {
      mFailureEvents.Read(rTr, "FailureEvents", pContext);
      continue;
    }
    if(token.StringValue()=="IndicatorEvents") {
      mIndicatorEvents.Read(rTr, "IndicatorEvents", pContext);
      continue;
    }
    break;
  }
}


} // namespace faudes


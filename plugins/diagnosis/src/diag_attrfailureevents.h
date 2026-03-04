/** @file diag_attrfailureevents.h Failure and indicator events for a common failure type. */

#ifndef DIAG_ATTRFAILUREEVENTS_H
#define DIAG_ATTRFAILUREEVENTS_H

#include <vector>
#include <map>
#include <set>
#include "corefaudes.h"
#include "diag_attrlabelset.h"
#include "diag_debug.h"

namespace faudes {

/**
 *  Stores the failure and indicator events for a particular failure type.
 *
 *  Note: this is formaly derived from AttributeFlags since some
 *  some applications assume AttributeFlag the minimum attribute class.
 *
 *  @ingroup DiagnosisPlugIn
 */
class FAUDES_API AttributeFailureEvents : public AttributeFlags {

FAUDES_TYPE_DECLARATION(Void,AttributeFailureEvents,AttributeFlags)

 public:
  /** Set of failure events. */
  EventSet mFailureEvents;
  /** Set of indicator events. */
  EventSet mIndicatorEvents;

 public:

  /** Default constructor. */
  AttributeFailureEvents(void);

  /** Copy constructor. */
  AttributeFailureEvents(const AttributeFailureEvents& rSrcAttr);

  /** Default destructor. */
  ~AttributeFailureEvents(void) {};

  /**
   *  Test for default values of failure and indicator events.
   *
   *  @return
   *    True for default value.
   */
  bool IsDefault(void) const;
  
  /**
   * Clears failure and indicator events.
   */
  virtual void Clear(void);

 protected:

  /**
   *  Copy attribute members
   *
   *  @param rSrcAttr 
   *    Source to copy from
   */
  void DoAssign(const AttributeFailureEvents& rSrcAttr); 

  /**
   *  Copy attribute members
   *
   *  @param rSrcAttr 
   *    Source to copy from
   */
  void DoMove(AttributeFailureEvents& rSrcAttr); 

  /**
   *  Test equality
   *
   *  @param rAttr 
   *    Source to copy from
   */
   bool DoEuqal(const AttributeFailureEvents& rAttr) const; 

  /**
   *  Write failure and indicator events.
   *
   *  @param rTw
   *    Reference to TokenWriter
   *  @param rLabel
   *    Section to write.
   *  @param pContext
   *    Write context to provide contextual information
   */  
  void DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const;
  
  /**
   *  Write failure and indicator events.
   *
   *  @param rTw
   *    Reference to TokenWriter
   *  @param rLabel
   *    Section to write.
   *  @param pContext
   *    Write context to provide contextual information
   */  
  void DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const;
  
  /**
   *  Read failure and indicator events.
   *
   *  @param rTr
   *    Reference to TokenReader
   *  @param rLabel
   *    Section to read from.
   *  @param pContext
   *    Read context to provide contextual information
   */  
  void DoRead(TokenReader &rTr, const std::string &rLabel="", const Type *pContext=0);
  
}; // class AttributeFailureEvents


} // namespace faudes

#endif

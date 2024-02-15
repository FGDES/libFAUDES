/** @file pd_merge.h  Attributes for pushdown automata states */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_MERGE_H
#define FAUDES_PD_MERGE_H

#include "corefaudes.h"
#include "pd_basics.h"
namespace faudes {
 
  
//forward declarations
/*template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr> class TpdGenerator;
class AttributePushdownState;
class AttributePushdownGlobal;
class AttributePushdownTransition;
typedef TpdGenerator<AttributePushdownGlobal, AttributePushdownState, AttributeCFlags,AttributePushdownTransition>
  PushdownGenerator;*/

/**
 * Abstract interface class for all kinds of items which can comprise a new state.
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/

class MergeAbstract {

  public:
    
    /**
     * Generator which contains the contained items. In order to print information
     * about the contained items, the class needs a reference to the generator.
     */
    //PushdownGenerator* mpGen;
  
    /**
     * Write Method which is to be implemented by all inheriting classes. Writes the
     * contained items of which the states consists.
     * 
     * @param rTw
     *      TokenWriter
     * @param rLabel
     *      label
     */
    virtual void Write(TokenWriter& rTw, const std::string& rLabel) = 0;
    
    // virtuals destructor for abstract class (tmoor 2016-03)
    virtual ~MergeAbstract() {};
};// class MergeAbstract

/**
 * Merge class which contains only states.
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/
class MergeStates : public MergeAbstract{

  public:
  
  /** the contained states */
  std::vector<Idx> mStates;
  
  /**
   * Constructor.
   * 
   * @param states
   *        Vector of states. The vector must not be empty.
   */
  MergeStates(const std::vector<Idx>& states);
  //MergeStates(std::vector<Idx> states, PushdownGenerator* gen);
  
  /**
   * Writes the contained items.
   * @param rTw
   *        TokenWriter
   * @param rLabel
   *        label
   */
  void Write(TokenWriter& rTw, const std::string& rLabel);
  
  /**
   * Getter for States
   * 
   * @return
   *    mStates
   */
  std::vector<Idx> States() const { return mStates; }
  
//   /**
//    * Set the contained states.
//    * 
//    * @param states
//    *        Vector of states. The vector must not be emtpy.
//    */
//   void Set(std::vector<Idx> states);
};//class MergeStates

/**
 * Merge class which contains one state and a string annotation.
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/
class MergeStateAnnotation : public MergeAbstract{

  public:
  
  /** the contained state */
  Idx mState;
  /** the annotation */
  std::string mAnnotation;
  
  /**
   * Constructor.
   * 
   * @param state
   *        The state, which must not be empty
   * @param annotation
   *        The annotation
   */
  MergeStateAnnotation(const Idx state, const std::string& annotation);
  
  /**
   * Getter for State
   * 
   * @return
   *    mState
   */
  Idx State() const { return mState; }
  
    /**
   * Getter for annotation
   * 
   * @return
   *    mAnnotation
   */
  std::string Annotation() const { return mAnnotation; }
  
  /**
   * Writes the contained items.
   * @param rTw
   *        TokenWriter
   * @param rLabel
   *        label
   */
  void Write(TokenWriter& rTw, const std::string& rLabel);
  
};//class MergeStateAnnotation

/**
 * Merge class which contains one state and one event
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/
class MergeStateEvent : public MergeAbstract{

  public:
  
  /** the contained state */
  Idx mState;
  /** the contained event*/
  Idx mEvent;
  
  /**
   * Constructor.
   * 
   * @param state
   *        The state, which must not be empty
   * @param event
   *        The event
   */
  MergeStateEvent(const Idx state, const Idx event);
  
  /**
   * Getter for State
   * 
   * @return
   *    mState
   */
  Idx State() const { return mState; }
  
    /**
   * Getter for event
   * 
   * @return
   *    mEvent
   */
  Idx Event() const { return mEvent; }
  
  /**
   * Writes the contained items.
   * @param rTw
   *        TokenWriter
   * @param rLabel
   *        label
   */
  void Write(TokenWriter& rTw, const std::string& rLabel);
  
};//class MergeStateEvent


/**
 * Merge class which contains one state, one stack symbol and a string annotation.
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/
class MergeStateSplit : public MergeAbstract{

  public:
  
  /** the contained state */
  Idx mState;
  /** the stack symbol */
  Idx mSymbol;
  /** isHead */
  bool mIsHead;
  
  /**
   * Constructor for heads.
   * 
   * @param state
   *        The state, which must not be empty
   */
  MergeStateSplit(const Idx state);
  
  /**
   * Constructor for ears.
   * 
   * @param state
   *        The state, which must not be empty
   * @param symbol
   *        The stack symbol
   */
  MergeStateSplit(const Idx state, const Idx symbol);
  
  /**
   * Getter for State
   * 
   * @return
   *    mState
   */
  Idx State() const { return mState; }
  
  /**
   * Getter for symbol
   * 
   * @return
   *    mSymbol
   */
  Idx Symbol() const { return mSymbol; }
  
  /**
   * Getter for IsHead
   * 
   * @return
   *    mIsHead
   */
  bool IsHead() const { return mIsHead; }
  
  /**
   * Writes the contained items.
   * @param rTw
   *        TokenWriter
   * @param rLabel
   *        label
   */
  void Write(TokenWriter& rTw, const std::string& rLabel);
  
};//class MergeStateSplit

/**
 * Merge class which contains one transition
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/
class MergeTransition : public MergeAbstract{

  public:
  
  /** the transition's start state, event, end state, pop symbols and push symbols */
  Idx mX1;
  Idx mEv;
  Idx mX2;
  std::vector<Idx> mPop;
  std::vector<Idx> mPush;
  
  /**
   * Constructor for heads.
   * 
   * @param x1
   *    start state
   * @param ev
   *    event
   * @param x2
   *    end state
   * @param pop
   *    pop stack symbols
   * @param push
   *    push stack symbols
   */
  MergeTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& pop, const std::vector<Idx>& push);
  
  /**
   * Constructor for heads.
   *
   * @param trans
   *    Transition
   * @param pop
   *    pop stack symbols
   * @param push
   *    push stack symbols
   */
  MergeTransition(const Transition trans, const std::vector<Idx>& pop, const std::vector<Idx>& push);

  /**
   * Getter for mX1
   * 
   * @return
   *    mX1
   */
  Idx X1() const { return mX1; }
  
  /**
   * Getter for mEv
   * 
   * @return
   *    mEv
   */
  Idx Ev() const { return mEv; }
  
  /**
   * Getter for mX2
   * 
   * @return
   *    mX2
   */
  Idx X2() const { return mX2; }
  
  /**
   * Getter for mPop
   * 
   * @return
   *    mPop
   */
  const std::vector<Idx>& Pop() const { return mPop; }
  
  /**
   * Getter for mPush
   * 
   * @return
   *    mPush
   */
  const std::vector<Idx>& Push() const { return mPush; }
  
  
  /**
   * Writes the contained items.
   * @param rTw
   *        TokenWriter
   * @param rLabel
   *        label
   */
  void Write(TokenWriter& rTw, const std::string& rLabel);
  
};//class MergeTransition

} // namespace faudes

#endif

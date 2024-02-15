/** @file hyb_experiment.h  Exhaustive experments on linear hybris automata */


/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2017  Thomas Moor, Stefan Goetz

*/

#ifndef HYP_EXPERIMENT_H
#define HYP_EXPERIMENT_H

#include "corefaudes.h"
#include "hyb_reachability.h"

namespace faudes {

// forward declaration of dynamics operator
class CompatibleStates;


/**
 * Finite fragment of a computation tree.
 *
 * An Experiment represents a finite fragment of a computation tree of. Formaly, an Experiment 
 * avoids an explicit reference to the
 * underlying state set of the transition system; i.e., each node represents the finite sequence of 
 * events generated when traversing from the root the respective node. 
 * Experiments are used to construct deterministic finite state abstractions.
 * 
 * Although an explicit reference to the underlying state set is avoided, implictly
 * each node is associated with the set of state in which the transition could 
 * possibly reside after execution of the respective finite string. These sets of
 * CompatibleStates can be computed by an iterative forward reachability analysis, where 
 * the choice of available computational procedures depends of the
 * class of transition systems under consideration. 
 *
 * 
 *
 * @ingroup HybridPlugin 
 *
 */
class FAUDES_API Experiment {

  
  public:
    // basic maintenance
    Experiment(const EventSet& alph);   
    ~Experiment(void);
    void InitialStates(CompatibleStates* istates);
    void Clear(void);
    Idx Size(void) const;  
    const EventSet& Alphabet(void) const;
    int Cost(void) const { return mCost;};

    // convenience methods
    void SWrite(void) const;
    void DWrite(void) const;
    void DWrite(Idx nid) const;

    /**
     * Navigate tree 
     * This interface is intended for external use. 
     * All methods throw an exception when accessing a non-existent node.
     */
    Idx Root(void) const;                                                       
    EventSet EnabledEvents(Idx nid) const;
    Idx SuccessorNode(Idx nid, Idx ev) const;  // 0 for has no such succesor
    bool IsLeave(Idx nid) const; // false for nonex
    bool IsLock(Idx nid) const;
    Idx Find(const std::deque< Idx >& seq) const;
    std::deque< Idx > Sequence(Idx nid) const;
    const IndexSet& Leaves() const;

    /**
     * Access compatible states
     * (note: implememtation dependant data type)
     */
    const CompatibleStates* States(Idx nid) const;
     
    // forward reachability analysis
    void RefineAt(Idx idx);
    Idx RefineSequence(const std::deque< Idx >& seq);
    void RefineUniformly(unsigned int depth);                          
         

 protected:

  /** hide Node from intended interface */
  class Node;
    
  /** core experiment data: have root and track all nodes by index */
  Node* mpRoot;
  std::map< Idx , Node* > mNodeMap;
  typedef std::map<Idx, Node*>::const_iterator Iterator;
  IndexSet mLeaves;
  int mCost;

  /** reference to alphabet */
  const faudes::EventSet& rAlphabet;

  /** Node record to form a tree */
  class Node {
  public:
  Node(void) : mId(0), pParent(0), mRecEvent(0), mDepth(0), mpStates(0), mLock(false) {};
    /* unique id */
    Idx mId;
    /* pointer to parent, NULL for root */
    Node* pParent;
    /* event by which we got here */
    Idx mRecEvent;
    /* distance from root */
    Idx mDepth;
    /* children by event */
    std::map<Idx, Node*> mChildren;
    /* payload: set of compatible states */
    CompatibleStates* mpStates;                                  
    bool mLock;
    /* convenience typedef */
    typedef std::map<Idx, Node*>::const_iterator Iterator;
  };


  /** Node related methods, hosted by Experiment for convenience */

  /** tree member access */
  Idx& Index(Node* node) 
    { return node->mId; };
  const Idx& Index(const Node* node) const
    { return node->mId;};
  Node*& Parent(Node* node) 
    { return node->pParent; };
  Node* const & Parent(const Node* node) const 
    { return node->pParent; };
  Idx& RecEvent(Node* node) 
    { return node->mRecEvent; };
  const Idx& RecEvent(const Node* node) const
    { return node->mRecEvent;};
  Idx& Depth(Node* node) 
    { return node->mDepth; };
  const Idx& Depth(const Node* node) const
    { return node->mDepth;};
  std::map< Idx, Node*>& Children(Node* node) 
    { return node->mChildren; };
  const std::map< Idx, Node*>& Children(const Node* node) const 
    { return node->mChildren; };
  bool IsLeave(const Node* node) const 
    { return Children(node).empty(); }
  Node* Child(Node* node, Idx ev) {
    Node::Iterator nit = Children(node).find(ev);
    if(nit==Children(node).end()) return 0;
    return nit->second;
  };

  /** grow tree */
  Node* Insert(Node* parent, Idx ev) {
    if(IsLeave(parent)) mLeaves.Erase(Index(parent));
    Node* node= new Node();
    node->mId = mNodeMap.size()+1; // should use max
    mNodeMap[node->mId]=node;
    Parent(node)=parent;
    RecEvent(node) = ev;
    Depth(node)=Depth(parent)+1;
    Children(parent)[ev]=node;
    mLeaves.Insert(Index(node));
    return node;
  }

  /** payload access */
  CompatibleStates*& States(Node* node) 
   { return node->mpStates; };
  CompatibleStates* const & States(const Node* node) const
   { return node->mpStates; };
  bool& IsLock(Node* node) 
   { return node->mLock; };
  const bool& IsLock(const Node* node) const 
   { return node->mLock; };

  /** get node ptr by index (for API wrappers)*/
  Node* NodePtr(Idx idx) const {
    Node::Iterator mit = mNodeMap.find(idx);
    if(mit==mNodeMap.end()) return 0;
    return mit->second;
  };


};// class


/** 
 * Abstract dynamics operator, i.e., some set of states,
 * that knows where it eveolves when it next triggers an event.
 * Technically, CompatibleStates provide a minimal interface that 
 * allows the Experiment to refine itself.
 */
 
class FAUDES_API CompatibleStates  {
 public:
  CompatibleStates(void) {};
  virtual ~CompatibleStates(void) {};
  virtual void InitialiseFull(void) = 0;
  virtual void InitialiseConstraint(void) = 0;
  virtual void ExecuteTransitions(void) = 0;
  virtual CompatibleStates* TakeByEvent(Idx ev) = 0;
  virtual void DWrite(void) const = 0;
  virtual const int Cost(void) {return 0;};   
}; 

// DES dynamics operator
class FAUDES_API DesCompatibleStates : public CompatibleStates {
public:
  DesCompatibleStates(const Generator& gen);
  virtual ~DesCompatibleStates(void);
  virtual void InitialiseFull();
  virtual void InitialiseConstraint();
  virtual void ExecuteTransitions(void);
  virtual DesCompatibleStates* TakeByEvent(Idx ev);
  virtual void DWrite(void) const;
protected:
  const Generator& rGen;
  StateSet* mpStates;
  std::map<Idx, StateSet*> mReachSets;
};  

// LHA dynamics operator
class FAUDES_API LhaCompatibleStates : public CompatibleStates {
public:
  LhaCompatibleStates(const LinearHybridAutomaton& lha);
  virtual ~LhaCompatibleStates(void);
  virtual void InitialiseFull();
  virtual void InitialiseConstraint();
  virtual void ExecuteTransitions(void);
  virtual LhaCompatibleStates* TakeByEvent(Idx ev);
  virtual void DWrite(void) const;
  virtual const int Cost(void) {return mCnt;};   
  const HybridStateSet* States(void) const { return mpStates; };
protected:
  const LinearHybridAutomaton& rLha;
  HybridStateSet* mpStates;
  std::map<Idx, HybridStateSet*> mHybridReachSets;
  int mCnt;
};  

 
} // namespace
#endif // EXPERIMENT_H

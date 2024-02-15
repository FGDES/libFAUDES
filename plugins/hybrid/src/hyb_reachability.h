/** @file hyp_reachability.h  LHA reachabilty */

/* 
   Hybrid systems plug-in for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2017  Thomas Moor

*/



#ifndef FAUDES_HYB_REACHABILITY_H
#define FAUDES_HYB_REACHABILITY_H


#include "corefaudes.h"
#include "hyb_hgenerator.h"

//#undef FD_DG
//#define FD_DG(message) FAUDES_WRITE_CONSOLE("FAUDES_GENERATOR: " << message)


namespace faudes {



/**
 * Set of states in an hybrid automata
 *
 * Records a list of polyhedra per location.
 */

class FAUDES_API HybridStateSet {

 public:

  /** constructors */

  HybridStateSet(void);
  ~HybridStateSet(void);
  HybridStateSet(const  HybridStateSet& rOther);
  void Assign(const  HybridStateSet& rOther);

  /** access to locations */
  const IndexSet& Locations(void);
  IndexSet::Iterator LocationsBegin(void) const;
  IndexSet::Iterator LocationsEnd(void) const;

  /** access to polyhedra of states */
  typedef std::list< Polyhedron* >::const_iterator Iterator;
  Iterator StatesBegin(Idx q) const;
  Iterator StatesEnd(Idx q) const;

  /** insert / erase (we take owvership of polyhedra) */
  void Insert(Idx q);
  void Insert(Idx q, Polyhedron* states);
  void Erase(Idx q);
  void Clear(void);

  /** test emptyness */
  bool IsEmpty(void) const;

  /** inspect */
  void DWrite(const LinearHybridAutomaton& lha);

protected:
  
  /** payload */
  StateSet mLocations;
  std::map< Idx , std::list< Polyhedron* > > mStates;  

};
 

/** compute sets of reachable state per successor event */
void FAUDES_API LhaReach(
  const LinearHybridAutomaton& lha, 
  const HybridStateSet& states, 
  std::map< Idx, HybridStateSet*  >& ostates,
  int* pCnt=NULL);


} // namespace faudes


#endif


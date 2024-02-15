/** @file hyb_abstraction.h  Abstractions by experiments for linear hybrid automata */


/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2017  Thomas Moor, Stefan Goetz

*/

#ifndef HYP_ABSTRACTION_H
#define HYP_ABSTRACTION_H

#include "corefaudes.h"
#include "hyb_experiment.h"

namespace faudes {

// very pragmatic function interface
void TimeInvariantAbstraction(const Experiment& exp, Generator& res);
void TimeVariantAbstraction(const Experiment& exp, Generator& res);

// abstraction class
class FAUDES_API LbdAbstraction {
  public:
    // construct/destruct
    LbdAbstraction(void);
    ~LbdAbstraction(void);

    // set experiment (must be initialised; we take ownership)
    void Experiment(faudes::Experiment* exp);
    const faudes::Experiment& Experiment(void);

    // pass on refine
    void RefineAt(Idx nid);
    void RefineUniformly(unsigned int depth);

    // get abstraction (we keep ownership)
    const Generator& TivAbstraction(void);
    const Generator& TvAbstraction(void);

 protected:

    // foster experiment
    faudes::Experiment* mpExperiment;
    
    // provide abstraction
    Generator mAbstraction;
    StateSet mLeaves;

    // track whether we need to update
    bool  mExpChanged;   
    bool mTivMode;
    bool mTvMode;
    
    // workers
    void doInitAbstraction(void);
    void doTivAbstraction(void);
    void doTivAbstractionMG(void);
    void doTivAbstractionRY(void);
    void doTvAbstraction(void);

};   
      
            
} // namespace

#endif 

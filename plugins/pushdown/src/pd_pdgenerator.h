/** @file pd_pdgenerator.h  pushdown generator class TpdGenerator */

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14  Stefan Jacobi, Ramon Barakat, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_PDGENERATOR_H
#define FAUDES_PD_PDGENERATOR_H
//#define FAUDES_DEBUG_GENERATOR
//#define FAUDES_DEBUG_CONTAINER

#include "corefaudes.h"
#include "pd_attributes.h"
#include "pd_grammar.h"


namespace faudes {



/**
 * Generator with push down extensions. 
 *
 *
 * @ingroup PushdownPlugin
 * 
 * @section Overview
 *  tratarterteartawrtrae
 * 
 * @section Contents
 *  fvdgdthrthorthtiop
 */

template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class FAUDES_API TpdGenerator : public TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr> {

public:


	/**
	 * Constructor
	 */
	TpdGenerator(void);

	/** 
	 * Copy constructor 
	 *
	 * @param rOtherGen
	 */
	TpdGenerator(const TpdGenerator& rOtherGen);

	/** 
	 * Copy constructor (no attributes)
	 *
	 * @param rOtherGen
	 */
	TpdGenerator(const vGenerator& rOtherGen);

	/**
	 * Assignment operator (uses Assign)
	 * Note: you must reimplement this operator in derived 
	 * classes in order to handle internal pointers correctly
	 *
	 * @param rOtherGen
	 *   Other generator
	 */
	virtual TpdGenerator& operator= (const TpdGenerator& rOtherGen) {return this->Assign(rOtherGen);};
	  
	 /* Assignment method
         *
         * Note: you must reimplement this method in derived 
         * classes in order to handle internal pointers correctly
         *
         * @param rOtherGen
         *   Other generator
         */
        virtual TpdGenerator& Assign(const Type& rSource);

	/**
	 * Construct from file
	 *
	 * @param rFileName
	 *   Name of file
	 *
	 * @exception Exception
	 *   - file format errors (id 1, 50, 51, 52)
	 */
	TpdGenerator(const std::string& rFileName);

	/**
	 * Construct on heap. 
	 * Constructs a TpdGenerator on heap.
	 *
	 * @return 
	 *   new Generator 
	 */
	TpdGenerator* New(void) const;

	/**
	 * Construct copy on heap. 
	 * Constructs a TpdGenerator on heap.
	 *
	 * @return 
	 *   new Generator 
	 */
	TpdGenerator* Copy(void) const;

	/**
	 * Type test.
	 * Uses C++ dynamic cast to test whether the specified object
	 * casts to a PushdownGenerator.
	 *
	 * @return 
	 *   TpdGenerator reference if dynamic cast succeeds, else NULL 
	 */
	virtual const Type* Cast(const Type* pOther) const {
		return dynamic_cast< const TpdGenerator* > (pOther);
	};


	/**
	 * Construct on stack.
	 * Constructs a TpdGenerator on stack.
	 *
	 * @return 
	 *   new Generator 
	 */
	TpdGenerator NewPdGen(void) const;

    /**
     * Get Pointer to global StackSymbolTable. This is
     * a static member of SymbolTable and used as default
     * for all derived generator classes and instantiated objects.
     *
     * @return
     *   Pointer to global StackSymbolTable
     */
    static SymbolTable* GlobalStackSymbolTablep(void);

    /**
     * XXX: NEW !
     * Return pretty printable state name (eg for debugging)
     *
     * @param idx
     *    index
     *
     * @return State name or Idx:<idx>  for nonexistent name
     */
    std::string StateStr(Idx idx) const;

    /**
     * XXX: NEW !
     * Return pretty printable event name (eg for debugging)
     *
     * @param idx
     *    index
     *
     * @return Event name or Idx:<idx>  for nonexistent name
    */
    std::string EventStr(Idx idx) const;

    /**
    * Get Pointer to mpStackSymbolTable.
    *
    * @return Pointer mpStackSymbolTable
    */
    SymbolTable* StackSymbolTablep(void) const;

   /**
    * Set StackSymbolTable.
    *
    * @param pStackSymTab
    *    Pointer SymbolTable
    */
    void StackSymbolTablep(SymbolTable* pStackSymTab);

    /**
     * Return a NameSet with generator's StackSymbolTable
     *
     * @return
     *   New empty StackSymbolSet on stack
     */
     StackSymbolSet NewStackSymbolSet(void) const;

    /**
     * Construct a stack symbol on heap.
     *
     * @return
     *   Pointer to new empty StackSymbolSet on heap
     */
     StackSymbolSet* NewStackSymbolSetp(void) const;

   /**
    * Number of stacks symbols in mStackSymbols
    *
    * @return Number of stack symbols in mStackSymbols
    */
    Idx StackSymbolsSize(void) const;

   /**
    * Get stack symbol set as const reference
    *
    * @return mStackSymbols
    */
    const StackSymbolSet& StackSymbols(void) const;


   /**
    * Get stack symbol set as pointer
    *
    * @return mStackSymbols
    */
    StackSymbolSet* StackSymbolsp(void);

   /**
    * Overwrites mStackSymbols with new stack symbols without consistency check
    *
    * @param newstacksymbols
    *    New stack symbols that are written to mStackSymbols
    */
    void InjectStackSymbols(const StackSymbolSet& newstacksymbols);

   /**
    * Looks up stack symbol name for given index
    *
    * @param index
    *    Stack symbol index
    *
    * @return Stack symbol name
    */
    std::string StackSymbolName(Idx index) const;


   /** XXX:NEW
    * Set new stack symbol name for existing index.
    * FAUDES_CHECKED checks if index exists in NameSet.
    *
    * @param index
    *   Index to edit
    * @param rName
    *   New name
    *
    * @exception Exception
    *   - index not in this set (id 60)
    *   - index not found in SymbolTable (id 42)
    *   - name already associated with another index (id 44)
    */
    void StackSymbolName(Idx index, const std::string& rName) const;

    /**
    * Returns stack symbol for given index
    *
    * @param index
    *    Stack symbol index
    *
    * @return Stack symbol object
    */
    StackSymbol StackSymbolObj(Idx index) const;

    /**
    * Looks up stack symbol index for given name
    *
    * @param rName
    *    Stack symbol name
    *
    * @return Stack symbol index or 0 for nonexistent
    */
    Idx StackSymbolIndex(const std::string& rName) const;

    /**
     * XXX:NEW !
     * Get string of stack symbol for given index
     *
     * @param rName
     *    Stack symbol name
     *
     * @return Stack symbol name or Idx:<idx>  for nonexistent name
    */
    std::string StackSymbolStr(Idx idx) const;

    /**
    * Add named stack symbol to generator. An entry in the mpStackSymbolTable will
    * be made if stack symbol is new.
    *
    * @param rName
    *   Name of the stack symbol to add
    *
    * @return
    *   New unique index
    */
    Idx InsStackSymbol(const std::string& rName);

    /**
    * Add stack symbol to generator. An entry in the mpStackSymbolTable will
    * be made if stack symbol is new.
    *
    * @param rSymbol
    *   The stack symbol to add
    *
    * @return
    *   New unique index
    */
    Idx InsStackSymbol(const StackSymbol& rSymbol);

    /**
    * Add named stack bottom to generator. An entry in the mpStackSymbolTable will
    * be made if stack symbol is new. This will replace any old stack bottom symbol.
    *
    * @param rName
    *   Name of the stack bottom symbol to add
    *
    * @return
    *   Stack bottom symbol index
    */
    Idx SetStackBottom(const std::string& rName);

    /**
    * Add stack bottom to generator. An entry in the mpStackSymbolTable will
    * be made if stack symbol is new. This will replace any old stack bottom symbol.
    *
    * @param rSymbol
    *   The stack bottom symbol to add
    *
    * @return
    *   Stack bottom symbol index
    */
    Idx SetStackBottom(const StackSymbol& rSymbol);

    /**
    * Add stack bottom to generator. An entry in the mpStackSymbolTable will
    * be made if stack symbol is new. This will replace any old stack bottom symbol.
    *
    * @param idx
    *   The index of the stack bottom symbol to add
    *
    * @return
    *   Stack bottom symbol index
    */
    Idx SetStackBottom(const Idx idx);

    /**
     * Get the index of the stack bottom symbol
     *
     * @return
     *      index of the stack bottom symbol
     */
    Idx StackBottom() const;

    /**
    * Add new named stack symbols to generator.
    *
    * @param rStackSymbolSet
    *   StackSymbolSet
    */
    void InsStackSymbols(const StackSymbolSet& rStackSymbolSet);

    /**
    * Delete stack symbol from generator by index.
    *
    * @param index
    *   Index of stack symbol
    * @return
    *   True if stack symbol did exist
    *
    */
    bool DelStackSymbol(Idx index);

    /**
    * Delete stack symbol from generator by name. mpStackSymbolTable stays untouched.
    *
    * @param rName
    *    Name of stack symbol
    * @return
    *    True if stack symbol did exist
    */
    bool DelStackSymbol(const std::string& rName);

    /**
    * Delete a set of stack symbols from generator.
    *
    * @param rStackSymbols
    *   StackSymbolSet containing stack symbols to remove
    */
    void DelStackSymbols(const StackSymbolSet& rStackSymbols);

   /**
    * Test existence of stack symbol in mStackSymbols
    *
    * @param index
    *   Stack symbol index
    *
    * @return
    *   true / false
    */
    bool ExistsStackSymbol(Idx index) const;

   /**
    * Test existence of stack symbol in mStackSymbols
    *
    * @param rName
    *   Stack symbol name
    *
    * @return
    *   True if stack symbol exists
    */
    bool ExistsStackSymbol(const std::string& rName) const;

   /**
    * Returns an iterator to stack symbol index in mStackSymbols
    *
    * @param index
    *   Index to find
    *
    * @return
    *   StackSymbolSet::Iterator to stack symbol index
    */
    StackSymbolSet::Iterator FindStackSymbol(Idx index) const;

   /**
    * Returns an iterator to stack symbol index in mStackSymbols
    *
    * @param rName
    *   Stack symbol name of index to find
    *
    * @return
    *   StackSymbolSet::Iterator to stack symbol index
    */
    StackSymbolSet::Iterator FindStackSymbol(const std::string& rName) const;

    /**
     * XXX NEW
     * Get unique stack symbole name
     *
     * @param rName
     * 		The name prefix
     * @return string
     * 		The unique name
     */
    std::string UniqueStackSymbolName(const std::string& rName) const;

   /**
    * Iterator to Begin() of mStackSymbols
    *
    * @return iterator to begin of mStackSymbols
    */
    StackSymbolSet::Iterator StackSymbolsBegin(void) const;

    /**
    * Iterator to End() of mStackSymbols
    *
    * @return iterator to end of mStackSymbols
    */
    StackSymbolSet::Iterator StackSymbolsEnd(void) const;

    /**
     * Converts a vector of stack symbols to a vector of Idx
     *
     * @param symbols
     *      the vector of stack symbols
     * @return
     *      vector of Idx
     */
    std::vector<Idx> StackSymbolsToIndices(const std::vector<StackSymbol> symbols) const;

    /** XXX:NEW
      * Converts a vector of stack symbol names to a vector of Idx
      *
      * @param symbols
      *      the vector of stack symbol names
      * @return
      *      vector of Idx
      */
    std::vector<Idx> StackSymbolNamesToIndices(const std::vector<std::string> symbolnames) const;

    /**
     * Check if the pushdown generator is deterministic.
     *
     * A pushdown generator is deterministic iff distinct steps from a reachable configuration
     * append elements of the Alphabet to the history variable.
     * Note that this implies that the existence of an outgoing transition, which event is lambda,
     * in state q requiring stack-top A prevents other outgoing transitions in q requiring stack-top A.
     *
     * @ return
     * 		true, if the generator is deterministic
     */
    bool IsDeterministic() const;

    /**
     * Get number of transitions including pop-push-pairs
     *
     * @return
     *      number of transitions
     */
    Idx TransRelSize() const;

	/** 
	 * Add a transition to generator by indices. States and event
	 * must already exist!
	 *
	 * Define FAUDES_CHECKED for consistency checks.
	 *
	 * @param x1 
	 *   Predecessor state index 
	 * @param ev 
	 *   Event index 
	 * @param x2
	 *   Successor state index
	 *
	 * @return
	 *   True, if the transition was new the generator
	 *
	 * @exception Exception
	 *   - state or event not in generator (id 95)
	 */
	bool SetTransition(Idx x1, Idx ev, Idx x2);

	/** 
	 * Add a transition to generator by names. Statename and eventname
	 * must already exist!
	 *
	 * @param rX1
	 *   Predecessor state name
	 * @param rEv
	 *   Event name
	 * @param rX2
	 *   Successor state name
	 *
	 * @return
	 *   True, if the transition was new the generator
	 *
	 * @exception Exception
	 *   - state or event not in generator (id 95)
	 *   - state name not known (id 90)
	 *   - event name not known (id 66)
	 */
	bool SetTransition(const std::string& rX1, const std::string& rEv,
					   const std::string& rX2);

	/** 
	 * Add a transition with attribute to generator. States and event
	 * must already exist!
	 *
	 * Define FAUDES_CHECKED for consistency checks.
	 *
	 * @param rTransition
	 *   transition
	 * @param rAttr
	 *   attribute
	 *
	 * @return
	 *   True, if the transition was new the generator
	 *
	 */
	bool SetTransition(const Transition& rTransition, const TransAttr& rAttr);

	/**
	 * Inserts new PushdownTransition constructed from parameters.
	 * Performs consistency checks for x1, x2, ev and all stack symbols in rPop and rPush.
	 *
	 * @param rTrans
	 *    new transition 
	 * @param rPop
	 *    Stack symbol vector to be popped when transitioning
	 * @param rPush
	 *    Stack symbol vector to be pushed when transitioning
	 * @return
	 *   True, if the transition was new to the generator
	 */
	bool SetTransition(const Transition& rTrans, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush);

	/** XXX:NEW
	 * Inserts new PushdownTransition constructed from parameters.
	 * Performs consistency checks for x1, x2, ev and all stack symbols in rPop and rPush.
	 *
	 * @param rTrans
	 *    new transition
	 * @param rPop
	 *    Stack symbol vector to be popped when transitioning
	 * @param rPush
	 *    Stack symbol vector to be pushed when transitioning
	 * @return
	 *   True, if the transition was new to the generator
	 */
	bool SetTransition(const Transition& trans, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);

    /**
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and all stack symbols in rPop and rPush.
     *
     * @param rTrans
     *    new transition
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
     */
    bool SetTransition(const Transition& rTrans, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);


    /**
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in rPop and rPush.
     *
     * @param x1
     *    Start state of new PushdownTransition.
     * @param ev
     *    Event of new PushdownTransition.
     * @param x2
     *    End state of new PushdownTransition.
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
     */
    bool SetTransition(Idx x1, Idx ev, Idx x2,
              const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush);

    /**
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in rPop and rPush.
     *
     * @param x1
     *    Start state of new PushdownTransition.
     * @param ev
     *    Event of new PushdownTransition.
     * @param x2
     *    End state of new PushdownTransition.
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
     */
    bool SetTransition(Idx x1, Idx ev, Idx x2,
              const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);

    /**
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in rPop and rPush.
     *
     * @param rX1
     *    Start state of new PushdownTransition.
     * @param rEv
     *    Event of new PushdownTransition.
     * @param rX2
     *    End state of new PushdownTransition.
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
    */
    bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush);

    /**
     * XXX NEW !
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in rPop and rPush.
     * (Note: necessary for parsing .dot files)
     *
     * @param rX1
     *    Start state of new PushdownTransition.
     * @param rEv
     *    Event of new PushdownTransition.
     * @param rX2
     *    End state of new PushdownTransition.
     * @param rPop
     *    Vector of Stack symbol names to be popped when transitioning
     * @param rPush
     *    Vector of Stack symbol names to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
    */
    bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);

    /**
     * XXX NEW !
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in rPop and rPush.
     * (Note: necessary for parsing .dot files)
     *
     * @param rX1
     *    Start state of new PushdownTransition.
     * @param rEv
     *    Event of new PushdownTransition.
     * @param rX2
     *    End state of new PushdownTransition.
     * @param rPop
     *    Vector of Stack symbol names to be popped when transitioning
     * @param rPush
     *    Vector of Stack symbol names to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
     */
    bool SetTransition(Idx x1, Idx ev, Idx x2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);

    /**
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in rPop and rPush.
     *
     * @param rX1
     *    Start state of new PushdownTransition.
     * @param rEv
     *    Event of new PushdownTransition.
     * @param rX2
     *    End state of new PushdownTransition.
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was new to the generator
    */
    bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);


    /**
     * XXX: NEW
     * Inserts new PushdownTransition constructed from parameters.
     * Performs consistency checks for x1, x2, ev and stack symbols in pop and push.
     * Pop and Push vector are given as strings where stack symbol names
     * written in squared brackets separated by comma
     *
     * (Function specially to access from LuaFAUDES)
     *
     * @param rX1
     *    Start state of new PushdownTransition.
     * @param rEv
     *    Event of new PushdownTransition.
     * @param rPop
     * 	  String of to be popped stack symbols when transitioning
     * @param rPush
     * 	  String of to be pushed stack symbols when transitioning
     * @param rX2
     *    End state of new PushdownTransition.
     *
     * @return
     *   True, if the transition was new to the generator
    */
    bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rPop,
    					  const std::string& rPush, const std::string& rX2);

    /**
     * XXX: NEW
     * Delete an existing PushdownTransition constructed with the provided parameters.
     * Pop and Push vector are given as strings where stack symbol names
     * written in squared brackets separated by comma
     *
     * (Function specially to access from LuaFAUDES)
     *
     * @param rX1
     *    Start state of new PushdownTransition.
     * @param rEv
     *    Event of new PushdownTransition.
     * @param rPop
     * 	  String of to be popped stack symbols when transitioning
     * @param rPush
     * 	  String of to be pushed stack symbols when transitioning
     * @param rX2
     *    End state of new PushdownTransition.
     *
     * @return
     *   True, if the transition was deleted from the generator
    */
    bool ClrTransition(const std::string& rX1, const std::string& rEv, const std::string& rPop,
      					  const std::string& rPush, const std::string& rX2);

    /**
     * Delete an exisiting PushdownTransition with the provided parameters.
     *
     * @param x1
     *    Start state of the PushdownTransition.
     * @param ev
     *    Event of the PushdownTransition.
     * @param x2
     *    End state of the PushdownTransition.
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was deleted from the generator
     */
    bool ClrTransition(Idx x1, Idx ev, Idx x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush);

    /**
     * Delete an exisiting PushdownTransition with the provided parameters.
     *
     * @param x1
     *    Start state of the PushdownTransition.
     * @param ev
     *    Event of the PushdownTransition.
     * @param x2
     *    End state of the PushdownTransition.
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was deleted from the generator
     */
    bool ClrTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);

    /**XXX:NEW
      * Delete an exisiting PushdownTransition with the provided parameters.
      *
      * @param x1
      *    Start state of the PushdownTransition.
      * @param ev
      *    Event of the PushdownTransition.
      * @param x2
      *    End state of the PushdownTransition.
      * @param rPop
      *    Stack symbol vector to be popped when transitioning
      * @param rPush
      *    Stack symbol vector to be pushed when transitioning
      * @return
      *   True, if the transition was deleted from the generator
      */
    bool ClrTransition(Idx x1, Idx ev, Idx x2,const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);

    /**XXX:NEW
      * Delete an exisiting PushdownTransition with the provided parameters.
      *
      * @param x1
      *    Start state of the PushdownTransition.
      * @param ev
      *    Event of the PushdownTransition.
      * @param x2
      *    End state of the PushdownTransition.
      * @param rPop
      *    Stack symbol vector to be popped when transitioning
      * @param rPush
      *    Stack symbol vector to be pushed when transitioning
      * @return
      *   True, if the transition was deleted from the generator
      */
    bool ClrTransition(std::string x1, std::string ev, std::string x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);

    /**XXX:NEW
      * Delete an exisiting PushdownTransition with the provided parameters.
      *
      * @param x1
      *    Start state of the PushdownTransition.
      * @param ev
      *    Event of the PushdownTransition.
      * @param x2
      *    End state of the PushdownTransition.
      * @param rPop
      *    Stack symbol vector to be popped when transitioning
      * @param rPush
      *    Stack symbol vector to be pushed when transitioning
      * @return
      *   True, if the transition was deleted from the generator
      */
    bool ClrTransition(std::string x1, std::string ev, std::string x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush);

    /**XXX:NEW
      * Delete an exisiting PushdownTransition with the provided parameters.
      *
      * @param x1
      *    Start state of the PushdownTransition.
      * @param ev
      *    Event of the PushdownTransition.
      * @param x2
      *    End state of the PushdownTransition.
      * @param rPop
      *    Stack symbol vector to be popped when transitioning
      * @param rPush
      *    Stack symbol vector to be pushed when transitioning
      * @return
      *   True, if the transition was deleted from the generator
      */
    bool ClrTransition(std::string x1, std::string ev, std::string x2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);

    /**
     * Delete an exisiting PushdownTransition with the provided parameters.
     *
     * @param rTrans
     *    the transition
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was deleted from the generator
     */
    bool ClrTransition(const Transition& rTrans, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush);

    /** XXX:NEW
      * Delete an exisiting PushdownTransition with the provided parameters.
      *
      * @param rTrans
      *    the transition
      * @param rPop
      *    Stack symbol name vector to be popped when transitioning
      * @param rPush
      *    Stack symbol name vector to be pushed when transitioning
      * @return
      *   True, if the transition was deleted from the generator
      */
    bool ClrTransition(const Transition& rTrans, const std::vector<std::string>& rPop,const std::vector<std::string>& rPush);

    /**
     * Delete an exisiting PushdownTransition with the provided parameters.
     *
     * @param rTrans
     *    the transition
     * @param rPop
     *    Stack symbol vector to be popped when transitioning
     * @param rPush
     *    Stack symbol vector to be pushed when transitioning
     * @return
     *   True, if the transition was deleted from the generator
     */
    bool ClrTransition(const Transition& rTrans, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);


    /**
     * Test for transition given by x1, ev, x2
     *
     *
     * @param rX1
     *   name of Predecessor state
     * @param rEv
     *   name of Event
     * @param rX2
     *   name of Successor state
     *
     * @return
     *   true / false
     */
    bool ExistsTransition(
      const std::string& rX1, const std::string& rEv, const std::string& rX2) const;

    /**
     * Test for transition given by x1, ev, x2
     *
     * @param x1
     *   Predecessor state
     * @param ev
     *   Event
     * @param x2
     *   Successor state
     *
     * @return
     *   true / false
     */
    bool ExistsTransition(Idx x1, Idx ev, Idx x2) const;

    /**
     * Test for transition
     *
     *
     * @param rTrans
     *   transition
     *
     * @return
     *   true / false
     */
    bool ExistsTransition(const Transition& rTrans) const;

    /**
     * Test for transition given by x1, ev
     *
     * @param x1
     *   Predecessor state
     * @param ev
     *   Event
     *
     * @return
     *   true / false
     */
    bool ExistsTransition(Idx x1, Idx ev) const;

    /**
     * Test for transition given by x1
     *
     * @param x1
     *   Predecessor state
     *
     * @return
     *   true / false
     */
    bool ExistsTransition(Idx x1) const;

      /**
       * XXX NEW !
       * Test for transition given by x1, ev, x2, pop, push
       *
       * @param x1
       *   Predecessor state
       * @param ev
       *   Event
       * @param x2
       *   Successor state
       * @param rPop
       *   Vector of stacksym indices to be popped
       * @param rPush
       *   Vector of stacksym indices to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& pop, const std::vector<Idx>& push) const;

      /**
       * XXX NEW !
       * Test for transition given by x1, ev, x2, pop, push
       *
       * @param x1
       *   Predecessor state
       * @param ev
       *   Event
       * @param x2
       *   Successor state
       * @param rPop
       *   Vector of stacksym names to be popped
       * @param rPush
       *   Vector of stacksym names to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<std::string>& pop, const std::vector<std::string>& push) const;

      /**
        * XXX NEW !
        * Test for transition given by x1, ev, x2, pop, push
        *
        * @param x1
        *   Predecessor state
        * @param ev
        *   Event
        * @param x2
        *   Successor state
        * @param rPop
        *   Stack symbol vector to be popped
        * @param rPush
        *   Stack symbol vector to be pushed
        *
        * @return
        *   true / false
        */
      bool ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush)const;

      /**
        * XXX NEW !
        * Test for transition given by x1, ev, x2, pop, push
        *
        * @param x1
        *   Name of predecessor state
        * @param ev
        *   Name of event
        * @param x2
        *   Name of successor state
        * @param rPop
        *   Stack symbol vector to be popped
        * @param rPush
        *   Stack symbol vector to be pushed
        *
        * @return
        *   true / false
        */
      bool ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& x2, const std::vector<StackSymbol>& pop, const std::vector<StackSymbol>& push) const;

      /**
       * XXX NEW !
       * Test for transition given by x1, ev, x2, pop, push
       *
       * @param x1
       *   Name of predecessor state
       * @param ev
       *   Name of event
       * @param x2
       *   Name of successor state
       * @param rPop
       *   Vector of stacksym names to be popped
       * @param rPush
       *   Vector of stacksym names to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& x2, const std::vector<std::string>& pop, const std::vector<std::string>& push) const;

      /**
       * XXX NEW !
       * Test for transition given by x1, ev, x2, pop, push
       *
       * @param x1
       *   Name of predecessor state
       * @param ev
       *   Name of event
       * @param x2
       *   Name of successor state
       * @param rPop
       *   Vector of stacksym indices to be popped
       * @param rPush
       *   Vector of stacksym indices to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& x2, const std::vector<Idx>& pop, const std::vector<Idx>& push) const;

      /**
       * XXX NEW !
       * Test for transition given by transition, pop, push
       *
       * @param rTrans
       * 	Transition
       * @param rPop
       *   Vector of stacksymbols to be popped
       * @param rPush
       *   Vector of stacksymbols to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(const Transition& rTrans, const std::vector<StackSymbol>& rPop,const std::vector<StackSymbol>& rPush)const;

      /**
       * XXX NEW !
       * Test for transition given by transition, pop, push
       *
       * @param rTrans
       * 	Transition
       * @param rPop
       *   Vector of stacksym names to be popped
       * @param rPush
       *   Vector of stacksym names to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(const Transition& rTrans, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush) const;

      /**
       * XXX: NEW !
       * Test for transition given trans, pop, push
       *
       * @param rTrans
       *   The transition
      * @param rPop
       *   Stack symbol vector to be popped
       * @param rPush
       *   Stack symbol vector to be pushed
       *
       * @return
       *   true / false
       */
      bool ExistsTransition(const Transition& rTrans, const std::vector<Idx>& pop, const std::vector<Idx>& push) const;

      /**
       * XXX: NEW
       * Test for transition given with the provided parameters.
       * Pop and Push vector are given as strings where stack symbol names
       * written in squared brackets separated by comma
       *
       * (Function specially to access from LuaFAUDES)
       *
       * @param rX1
       *    Start state of new PushdownTransition.
       * @param rEv
       *    Event of new PushdownTransition.
       * @param rPop
       * 	  String of to be popped stack symbols when transitioning
       * @param rPush
       * 	  String of to be pushed stack symbols when transitioning
       * @param rX2
       *    End state of new PushdownTransition.
       *
       * @return
       *   True, if the transition was deleted from the generator
      */
      bool ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& pop,const std::string& push, const std::string& x2) const;


    /**
     * Get the pop/push set attached to this transition
     *
     * @param rTrans
     *      the transition
     * @return
     *      the pop/push set
     */
    const PopPushSet& PopPush(const Transition& rTrans) const;

    /**
     * Get an iterator to the beginning of the pop/push set attached to this
     * transition
     *
     * @param rTrans
     *      the transition
     * @return
     *      iterator to the beginning of the pop/push set
     */
    PopPushSet::const_iterator PopPushBegin(const Transition& rTrans) const;

    /**
     * Get an iterator to the end of the pop/push set attached to this
     * transition
     *
     * @param rTrans
     *      the transition
     * @return
     *      iterator to the end of the pop/push set
     */
    PopPushSet::const_iterator PopPushEnd(const Transition& rTrans) const;

    /**XXX: NEW
      * Add lambda stack symbol to generator or return existing one
      *
      * @return
      *      index of lambda stack symbol
      */
    Idx InsLambdaStackSymbol();

    /**
     * determine if the stack symbol associated with the given index is lambda
     *
     * @param index
     *      index of a stack symbol
     * @return
     *      true if the associated stack symbol is lambda, else false
     */
    bool IsStackSymbolLambda(Idx index) const;

    /**
     * determine if the event associated with the given index is lambda
     *
     * @param index
     *      index of an event
     * @return
     *      true if the associated event is lambda, else false
     */
    bool IsEventLambda(Idx index) const;


   /**
    * Throw exception if stack symbol refers to stack symbol not
    * in stack symbol set
    *
    * @exception Exception
    *   - invalid stack symbol (id 1001)
    */
    void ConsistentStackSymbol(const StackSymbol& rStackSymbol) const;

    /**
    * Throw exception if stack symbol refers to stack symbol not
    * in stack symbol set
    *
    * @exception Exception
    *   - invalid stack symbol (id 1001)
    */
    void ConsistentStackSymbol(Idx idx) const;

    /**
    * Throw exception if vector of stack symbols contains stack symbols not
    * in generators stack symbol set
    *
    * @exception Exception
    *   - invalid stack symbol (id 1001)
    */
    void ConsistentVectorStackSymbol(const std::vector<StackSymbol>& rVector) const;

   /**
    * Throw exception if vector of stack symbols contains stack symbols not
    * in generators stack symbol set
    *
    * @exception Exception
    *   - invalid stack symbol (id 1001)
    */
    void ConsistentVectorStackSymbol(const std::vector<Idx>& rVector) const;

    /**
    * Throw exception if vector of stack symbols is empty
    *
    * @exception Exception
    *   - invalid stack symbol (id 1001)
    */
    void EmptyVectorPopPush(const std::vector<StackSymbol>& rVector) const;

    /**
    * Throw exception if vector of stack symbols is empty
    *
    * @exception Exception
    *   - invalid stack symbol (id 1001)
    */
    void EmptyVectorPopPush(const std::vector<Idx>& rVector) const;

    /**
     * Marks a state as being merged from other data type by setting mpMerge.
     *
     * @param stateName
     *      the name of the state to mark
     * @param rMerge
     *      the merged state
     */
     void SetMerge(const std::string& stateName, MergeAbstract& rMerge);

     /**
      * Marks a state as being merged from other data type by setting mpMerge.
      *
      * @param index
      *      the index of the state to mark
      * @param rMerge
      *      the merged state
      */
     void SetMerge(Idx index, MergeAbstract& rMerge);

     /**
      * Return the merge attribute of a state.
      *
      * @param index
      *      the index of the state
      */
     const MergeAbstract* Merge(Idx index) const;

     /**
      * Marks a state as being derived from the intersection with a DFA.
      *
      * @param stateName
      *      the name of the state to mark
      * @param dfaIndex
      *      the index of the DFA state
      */
     void SetDfaState(const std::string& stateName, Idx dfaIndex);

     /**
      * Marks a state as being derived from the intersection with a DPA.
      *
      * @param index
      *      the index of the state to mark
      * @param dfaIndex
      *      the index of the DFA state
      */
     void SetDfaState(Idx index, Idx dfaIndex);

     /**
      * Return the dfaState attribute of a state.
      *
      * @param index
      *      the index of the state
      */
     Idx DfaState(Idx index) const;

    /**
     * Check if generator is valid
     *
     * @return
     *   Success
     */
    virtual bool Valid(void) const;

    /**
     * XXX: NEW !
     * Writes generator to dot input format.
     * The dot file format is specified by the graphviz package; see http://www.graphviz.org.
     * The package includes the dot command line tool to generate a graphical
     * representation of the generators graph. See also vGenerator::GraphWrite().
     *
     *
     * @param rFileName
     *   File to write
     *
     * @exception Exception
     *   - IO errors (id 2)
     */
    virtual void DotWrite(const std::string& rFileName) const;

    /**
      * XXX: NEW !
      * Writes generator to dot input format.
      * The dot file format is specified by the graphviz package; see http://www.graphviz.org.
      * The package includes the dot command line tool to generate a graphical
      * representation of the generators graph.
      *
      *
      * @param rFileName
      *   File to write
      *
      * @param printInfo
      *   Printing contained informations
      *   (eg. Alphabet, Stack symbols etc.)
      *
      * @param lr
      *   Create graph from left to right
      *
      * @exception Exception
      *   - IO errors (id 2)
      */
    virtual void DotWrite(const std::string& rFileName, bool printInfo, bool lr) const;

    /**
      * XXX: NEW !
      * Create generator from dot input format.
      * The dot file format is specified by the graphviz package (see http://www.graphviz.org).
      * The package includes the dot command line tool to generate a graphical
      * representation of the generators graph.
      * (For more details see pd_dotparser.h)
      *
      * Note: Function will clear current generator.
      *
      *
      * @param rFileName
      *   File to parse
      *
      * @exception Exception
      *   - IO errors (id 2)
      */
    virtual void DotRead(const std::string& rFileName);



private:

    /**
     * Get stack symbols from given string.
     * stack symbol names should be written in squared brackets separated by comma
     */
    std::vector<std::string> ParseStackSymbolNames(const std::string& rStr) const;

}; //end class TpdGenerator

/** Convenience typedef for PushdownGenerator */
typedef TpdGenerator<AttributePushdownGlobal, AttributePushdownState,
						AttributeCFlags,AttributePushdownTransition>    PushdownGenerator;

typedef TaTransSet<AttributePushdownTransition> PdTransSet;


// convenient scope macros  
#define THIS TpdGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define BASE TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define TEMP template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>

// TpdGenerator(void)
TEMP THIS::TpdGenerator(void) : BASE() {
  // set basic members (cosmetic)
  BASE::pGlobalAttribute->mStackSymbols.Name("StackSymbols");
  BASE::pGlobalAttribute->mpStackSymbolTable=StackSymbolSet::StaticSymbolTablep();
  FD_DG("PushdownGenerator(" << this << ")::PushdownGenerator() with csymtab "
    << (BASE::pGlobalAttribute->mpStackSymbolTable ));
}

// TpdGenerator(rOtherGen)
TEMP THIS::TpdGenerator(const TpdGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("PushdownGenerator(" << this << ")::PushdownGenerator(rOtherGen) with csymtab"
    << "(BASE::pGlobalAttribute->mpStackSymbolTable)" );
}

// TpdGenerator(rOtherGen)
TEMP THIS::TpdGenerator(const vGenerator& rOtherGen) : BASE(rOtherGen) {
  // set basic members (cosmetic)
  BASE::pGlobalAttribute->mStackSymbols.Name("StackSymbols");
  BASE::pGlobalAttribute->mpStackSymbolTable=StackSymbolSet::StaticSymbolTablep();
  FD_DG("PushdownGenerator(" << this << ")::PushdownGenerator(rOtherGen) with csymtab"
    << "(BASE::pGlobalAttribute->mpStackSymbolTable)" );
}

// TpdGenerator(rFilename)
TEMP THIS::TpdGenerator(const std::string& rFileName) : BASE()  {
  FD_DG("PushdownGenerator(" << this << ")::PushdownGenerator(" << rFileName << ") with csymtab"
    << "(BASE::pGlobalAttribute->mpStackSymbolTable)" );
  // my  members
  BASE::pGlobalAttribute->mStackSymbols.Name("StackSymbols");
  BASE::pGlobalAttribute->mpStackSymbolTable=StackSymbolSet::StaticSymbolTablep();
  if(rFileName.substr(rFileName.find_last_of(".") + 1) == "dot") {
	  THIS::DotRead(rFileName);
  }
  else {
    BASE::Read(rFileName); // TM 2017
  }
}

// copy from other faudes type
TEMP THIS& THIS::Assign(const Type& rSrc) {
  FD_DG("TpdGenerator(" << this << ")::Assign([type] " << &rSrc << ")");
  // bail out on match
  if(&rSrc==static_cast<const Type*>(this)) return *this;

  // cast tests (clang 8.0.0 issues, 2017)
  /*
  const THIS* pgen=dynamic_cast<const THIS*>(&rSrc);
  FD_WARN("TpdGenerator(" << this << ")::Assign(..): pgen " << pgen);
  const BASE* cgen=dynamic_cast<const BASE*>(&rSrc);
  FD_WARN("TpdGenerator(" << this << ")::Assign(..): cgen " << cgen);
  const vGenerator* vgen=dynamic_cast<const vGenerator*>(&rSrc);
  FD_WARN("TpdGenerator(" << this << ")::Assign(..): vgen " << vgen);
  */

  // pass on to base
  FD_DG("TpdGenerator(" << this << ")::Assign([type] " << &rSrc << "): call base");
  BASE::Assign(rSrc);  
  return *this;
}

//StateStr(idx)
TEMP std::string THIS::StateStr(Idx idx) const{
	FD_DG("TpdGenerator(" << this << ")::StateStr("<< idx <<"\")");
	#ifdef FAUDES_CHECKED
	  if (! BASE::ExistsState(idx)) {
	    std::stringstream errstr;
	    errstr << "state \"" << idx << "\" not found in generator \""
		   << BASE::Name() << "\"";
	    throw Exception("TpdGenerator::StateStr(idx)", errstr.str(), 89);
	  }
	#endif

	std::string str= BASE::StateName(idx);
	if(str =="") str="Idx:" + ToStringInteger(idx);
	return str;
}

//EventStr(idx)
TEMP std::string THIS::EventStr(Idx idx) const{
	FD_DG("TpdGenerator(" << this << ")::EventStr("<< idx <<"\")");
	#ifdef FAUDES_CHECKED
	  if (! BASE::ExistsEvent(idx)) {
	    std::stringstream errstr;
	    errstr << "event \"" << idx << "\" not found in generator \""
		   << BASE::Name() << "\"";
	    throw Exception("TpdGenerator::EventStr(idx)", errstr.str(), 89);
	  }
	#endif

   	std::string str= BASE::EventName(idx);
   	if(str =="") str="Idx:" + ToStringInteger(idx);
   	return str;
}

//GlobalStackSymbolTablep
TEMP SymbolTable* THIS::GlobalStackSymbolTablep(void) {
  return StackSymbolSet::StaticSymbolTablep();
}

// StackSymbolTablep()
TEMP SymbolTable* THIS::StackSymbolTablep(void) const {
  return BASE::pGlobalAttribute->mpStackSymbolTable;
}

// StackSymbolTablep(pSymTab)
TEMP void THIS::StackSymbolTablep(SymbolTable* pSymTab) {
   BASE::Clear();
   BASE::pGlobalAttribute->mpStackSymbolTable=pSymTab;
}

// New
TEMP THIS* THIS::New(void) const {
  // allocate
  THIS* res = new THIS;
  // fix base data
  res->EventSymbolTablep(BASE::mpEventSymbolTable);
  res->mStateNamesEnabled=BASE::mStateNamesEnabled;
  res->mReindexOnWrite=BASE::mReindexOnWrite;
  // fix my data
  res->StackSymbolTablep(StackSymbolTablep());
  return res;
}

// Copy
TEMP THIS* THIS::Copy(void) const {
  // allocate
  THIS* res = new THIS(*this);
  // done
  return res;
}

// NewTGen
TEMP THIS THIS::NewPdGen(void) const {
  // call base (fixes by assignment constructor)
  THIS res= BASE::NewCGen();
  // fix my data
  res.StackSymbolTablep(StackSymbolTablep());
  return res;
}

// StackSymbolsSize() const
TEMP Idx THIS::StackSymbolsSize(void) const {
  return BASE::pGlobalAttribute->mStackSymbols.Size();
}

// StackSymbols()
TEMP const StackSymbolSet& THIS::StackSymbols(void) const {
  return BASE::pGlobalAttribute->mStackSymbols;
}

// StackSymbolssp()
TEMP StackSymbolSet* THIS::StackSymbolsp(void) {
  return &BASE::pGlobalAttribute->mStackSymbols;
}

// InjectStackSymbols(set)
TEMP void THIS::InjectStackSymbols(const StackSymbolSet& newstacksymbols) {
  BASE::pGlobalAttribute->mStackSymbols=newstacksymbols;
  BASE::pGlobalAttribute->mStackSymbols.Name("StackSymbols");
}

// StackSymbolName(index)
TEMP std::string THIS::StackSymbolName(Idx index) const {
  return BASE::pGlobalAttribute->mStackSymbols.SymbolicName(index);
}

// StackSymbolName(index,name)
TEMP void THIS::StackSymbolName(Idx index, const std::string& rName) const {
	  FD_DG("TpdGenerator(" << this << ")::StackSymbolName("
	      << index << ",\"" << rName << "\")");
	#ifdef FAUDES_CHECKED
	  if (! ExistsStackSymbol(index)) {
	    std::stringstream errstr;
	    errstr << "stack symbol idex \"" << index << "\" not found in generator \""
		   << BASE::Name() << "\"";
	    throw Exception("TpdGenerator::StateName(index, name)", errstr.str(), 90);
	  }
	#endif
	  BASE::pGlobalAttribute->mStackSymbols.SymbolicName(index, rName);
}

//StackSymbolObj(index)
TEMP StackSymbol THIS::StackSymbolObj(Idx index) const {
  return StackSymbol(BASE::pGlobalAttribute->mStackSymbols.SymbolicName(index));
}

// StackSymbolIndex(name)
TEMP Idx THIS::StackSymbolIndex(const std::string& rName) const {
  return BASE::pGlobalAttribute->mStackSymbols.Index(rName);
}

// InsStackSymbol(name)
TEMP Idx THIS::InsStackSymbol(const std::string& rName) {
  Idx idx =   BASE::pGlobalAttribute->mStackSymbols.Insert(rName);
  //FD_WARN("PDGenerator::InsStackSymbol \""<< rName << "\" idx " << idx); 
  return idx;
}

//InsStackSymbol(symbol)
TEMP Idx THIS::InsStackSymbol(const StackSymbol& rSymbol){
  return InsStackSymbol(rSymbol.Symbol());
}

// InsStackSymbols(set)
TEMP void THIS::InsStackSymbols(const StackSymbolSet& rStackSymbolSet) {
  BASE::pGlobalAttribute->mStackSymbols.InsertSet(rStackSymbolSet);
}

// SetStackBottom(name)
TEMP Idx THIS::SetStackBottom(const std::string& rName) {
  Idx i = BASE::pGlobalAttribute->mStackSymbols.Insert(rName);
  BASE::pGlobalAttribute->mStackBottom = i;
  return i;
}

//SetStackBottom(symbol)
TEMP Idx THIS::SetStackBottom(const StackSymbol& rSymbol){
  Idx i = BASE::pGlobalAttribute->mStackSymbols.Insert(rSymbol.Symbol());
  BASE::pGlobalAttribute->mStackBottom = i;
  return i;
}

//SetStackBottom(index)
TEMP Idx THIS::SetStackBottom(const Idx idx){
  if(!ExistsStackSymbol(idx)) {
      std::stringstream errstr;
      errstr << "stack symbol with index " << idx << " not found in generator. " << std::endl;
      throw Exception("PushdownGenerator::SetStackBottom(idx)", errstr.str(), 200);
  }
  BASE::pGlobalAttribute->mStackBottom = idx; // TM 2017
  return idx;
}

//StackBottom()
TEMP Idx THIS::StackBottom() const{
  return BASE::pGlobalAttribute->mStackBottom;
}

// DelStackSymbol(index)
TEMP bool THIS::DelStackSymbol(Idx index) {
  FD_DG("PushdownGenerator(" << this << ")::DelStackSymbol(" << index << ")");
  return BASE::pGlobalAttribute->mStackSymbols.Erase(index);
}

// DelStackSymbol(name)
TEMP bool THIS::DelStackSymbol(const std::string& rName) {
  Idx index=BASE::pGlobalAttribute->mStackSymbols.Index(rName);
  return DelStackSymbol(index);
}

// DelStackSymbols(set)
TEMP void THIS::DelStackSymbols(const StackSymbolSet& rStackSymbols) {
  StackSymbolSet::Iterator it=StackSymbolsBegin();
  while(it!=StackSymbolsEnd()){
    DelStackSymbol(*(it++)); // fixed: 2013-12-17
  }
}

// ExistsStackSymbol(index)
TEMP bool THIS::ExistsStackSymbol(Idx index) const {
  return BASE::pGlobalAttribute->mStackSymbols.Exists(index);
}

// ExistsStackSymbol(name)
TEMP bool THIS::ExistsStackSymbol(
  const std::string& rName) const {
  return BASE::pGlobalAttribute->mStackSymbols.Exists(rName);
}

// FindStackSymbol(index)
TEMP StackSymbolSet::Iterator THIS::FindStackSymbol(Idx index) const {
  return BASE::pGlobalAttribute->mStackSymbols.Find(index);
}

// FindStackSymbol(name)
TEMP StackSymbolSet::Iterator THIS::FindStackSymbol(const std::string& rName) const {
  return BASE::pGlobalAttribute->mStackSymbols.Find(rName);
}

//StackSymbolStr(idx)
TEMP std::string THIS::StackSymbolStr(Idx idx) const{
	FD_DG("TpdGenerator(" << this << ")::StackSymbolStr("<< idx <<"\")");
	#ifdef FAUDES_CHECKED
	  if (! ExistsStackSymbol(idx)) {
	    std::stringstream errstr;
	    errstr << "stack symbol \"" << idx << "\" not found in generator \""
		   << BASE::Name() << "\"";
	    throw Exception("TpdGenerator::StackSymbolStr(idx)", errstr.str(), 89);
	  }
	#endif
	std::string str= THIS::StackSymbolName(idx);
	if(str =="") str="Idx:" + ToStringInteger(idx);
	return str;
}

//UniqueStackSymbolName(rName)
TEMP std::string THIS::UniqueStackSymbolName(const std::string& rName) const {
  FD_DG("PushdownGenerator(" << this << ")::UniqueStackSymbolName(" << rName << ")");
  std::string name=rName;
  if(name=="") name="s";
  return BASE::pGlobalAttribute->mpStackSymbolTable->UniqueSymbol(name) ;
}

//Iterator StackSymbolsBegin() const
TEMP StackSymbolSet::Iterator THIS::StackSymbolsBegin(void) const {
  return BASE::pGlobalAttribute->mStackSymbols.Begin();
}

//Iterator StackSymbolsEnd() const
TEMP StackSymbolSet::Iterator THIS::StackSymbolsEnd(void) const {
  return BASE::pGlobalAttribute->mStackSymbols.End();
}
//StackSymbolsToIndices
TEMP std::vector<Idx> THIS::StackSymbolsToIndices(const std::vector<StackSymbol> symbols) const{
  std::vector<StackSymbol>::const_iterator ssit;
  std::vector<Idx> rV;
  for(ssit = symbols.begin(); ssit != symbols.end(); ssit++){
    rV.push_back(StackSymbolIndex(ssit->Symbol()));
  }
  return rV;
}

//StackSymbolNamesToIndices
TEMP std::vector<Idx> THIS::StackSymbolNamesToIndices(const std::vector<std::string> symbolnames) const{
  std::vector<std::string>::const_iterator ssit;
  std::vector<Idx> rV;
  for(ssit = symbolnames.begin(); ssit != symbolnames.end(); ssit++){
    rV.push_back(StackSymbolIndex(*ssit));
  }
  return rV;
}

//IsDeterministic() const
TEMP bool THIS::IsDeterministic() const{

	StateSet::Iterator stateit;
	TransSet::Iterator transit1, transit2;
	PopPushSet::iterator ppit1, ppit2;

	//for each state
	for(stateit = BASE::StatesBegin(); stateit != BASE::StatesEnd(); ++stateit){
		//check each transition pair
		for(transit1 = BASE::TransRelBegin(*stateit); transit1 != BASE::TransRelEnd(*stateit); ++transit1){
			for(transit2 = transit1; transit2 != BASE::TransRelEnd(*stateit); ++transit2){
				//if same event or one event is lambda
				if((transit1->Ev == transit2->Ev) ||
					THIS::IsEventLambda(transit2->Ev) ||
					THIS::IsEventLambda(transit1->Ev)){

					uint matches = 0;

					//check popped stack symbols
					for(ppit1 = THIS::PopPushBegin(*transit1); ppit1 != THIS::PopPushEnd(*transit1); ++ppit1){
						for(ppit2 = THIS::PopPushBegin(*transit2); ppit2 != THIS::PopPushEnd(*transit2); ++ppit2){
							if(ppit1->first == ppit2->first)
								matches++;
						}
						if(transit1 == transit2){
							// decrease matches counter because for sure there will be one match (with the transition itself)
							matches--;
						}
						if(matches > 0)
							return false;
					}
				}
			}
		}
	}

	return true;
}

TEMP Idx THIS::TransRelSize() const{

  TransSet::Iterator transit;
  Idx s = 0;
  for(transit = BASE::TransRelBegin(); transit != BASE::TransRelEnd(); transit++){
    s += PopPush(*transit).size();
  }
  return s;
}

// SetTransition(rX1, rEv, rX2)
TEMP bool THIS::SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2) {
  return BASE::SetTransition(rX1,rEv,rX2);
}


// SetTransition(x1, ev, x2)
TEMP bool THIS::SetTransition(Idx x1, Idx ev, Idx x2) {
  return BASE::SetTransition(Transition(x1,ev,x2));
}

// SetTransition(rTransition, rAttr)
TEMP bool THIS::SetTransition(const Transition& rTransition, const TransAttr& rAttr) {
  return BASE::SetTransition(rTransition,rAttr);
}

// SetTransition(trans,....)
TEMP bool THIS::SetTransition(const Transition& rTrans, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush) {
  return SetTransition(rTrans, StackSymbolsToIndices(rPop), StackSymbolsToIndices(rPush));
}

TEMP bool THIS::SetTransition(const Transition& rTrans,const std::vector<Idx>& rPop, const std::vector<Idx>& rPush) {
  FD_DG("PushdownGenerator(" << this << ")::SetTransition(" << (BASE::TStr(rTrans)) <<", " <<
    ", PopVector" << ", " << "PushVector" << ") const");
#ifdef FAUDES_CHECKED
  EmptyVectorPopPush(rPop);
  EmptyVectorPopPush(rPush);
#endif
  BASE::SetTransition(rTrans);
  //get the transition attribute or take new one if it does not exist
  TransAttr attr;
  if(BASE::TransAttributep(rTrans) != 0){
    attr = *BASE::TransAttributep(rTrans);
  }
  std::pair<std::vector<Idx>, std::vector<Idx> > popPushPair;
  popPushPair.first = rPop;
  popPushPair.second = rPush;
  //add new PopPushPair
  attr.mPopPush.insert(popPushPair);
#ifdef FAUDES_CHECKED
  ConsistentVectorStackSymbol(rPop);
  ConsistentVectorStackSymbol(rPush);
#endif
  return BASE::SetTransition(rTrans,attr);
}

// SetTransition(x1,ev,x2, pop, push)
TEMP bool THIS::SetTransition(Idx x1, Idx ev, Idx x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush) {
  return SetTransition(Transition(x1,ev,x2),rPop,rPush);
}

// SetTransition(x1,ev,x2, pop, push)
TEMP bool THIS::SetTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush) {
  return SetTransition(Transition(x1,ev,x2),rPop,rPush);
}

TEMP bool THIS::SetTransition(const Transition& rTrans, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush) {
	return SetTransition(rTrans,StackSymbolNamesToIndices(rPop),StackSymbolNamesToIndices(rPush));
}

// SetTransition(X1,Ev,X2, ...)
TEMP bool THIS::SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush) {
    FD_DG("PushdownGenerator(" << this << ")::SetTransition(" << rX1 << " " << rEv <<" " << rX2 <<
      ", PopVector" << ", " << "PushVector" << ") const");
  //try to add transition, will do nothing if transition exists
  bool res=BASE::SetTransition(rX1,rEv,rX2);
  //get transition via iterator
  Transition rTrans = *(BASE::FindTransition(rX1,rEv,rX2));
#ifdef FAUDES_CHECKED
  EmptyVectorPopPush(rPop);
  EmptyVectorPopPush(rPush);
#endif
  //get the transition attribute or take new one if it does not exist
  TransAttr attr;
  if(BASE::TransAttributep(rTrans) != 0){
    attr = *BASE::TransAttributep(rTrans);
  }
  std::pair<std::vector<Idx>, std::vector<Idx> > popPushPair;
  popPushPair.first = StackSymbolsToIndices(rPop);
  popPushPair.second = StackSymbolsToIndices(rPush);
  //add new PopPushPair
  attr.mPopPush.insert(popPushPair);
#ifdef FAUDES_CHECKED
  ConsistentVectorStackSymbol(rPop);
  ConsistentVectorStackSymbol(rPush);
#endif
  BASE::TransAttribute(Transition(BASE::StateIndex(rX1),BASE::EventIndex(rEv),BASE::StateIndex(rX2)),attr);
  return res;
}

// SetTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::SetTransition(Idx x1, Idx ev, Idx x2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush) {
    return SetTransition(Transition(x1,ev,x2),rPop,rPush);
}

// SetTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush) {
	return SetTransition(Transition(BASE::StateIndex(rX1),BASE::EventIndex(rEv),BASE::StateIndex(rX2)),rPop,rPush);
}

// SetTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush) {
  return SetTransition(Transition(BASE::StateIndex(rX1),BASE::EventIndex(rEv),BASE::StateIndex(rX2)),rPop,rPush);
}

// ParseStackSymbolNames("[X,Y]")
TEMP std::vector<std::string> THIS::ParseStackSymbolNames(const std::string& rStr) const{
		std::string::size_type end_position,pos= 0;
		std::vector<std::string> syms;
		std::string s = rStr;

		//remove spaces from string
		s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

		//get pop/push symbols
		pos = s.find("[");
		if (pos != std::string::npos)
		{
			//get stacksymbols
			end_position = s.find("]",++pos);
			if (end_position != std::string::npos)
			{
				//get string between squared brackets
				std::string found_text = s.substr(pos, end_position-pos);

				if (!found_text.empty()) {
					std::string::size_type curr = 0, end = 0;

					//substring
					std::string sub;

					//while comma separator exists from current position
					while ((end = found_text.find(",", curr)) != std::string::npos) {
						sub = found_text.substr(curr, end - curr);
						syms.push_back(sub);
						curr = end + 1;
					}

					//add last symbol
					if (curr < found_text.size())
						syms.push_back(found_text.substr(curr));
				}
			}
			#ifdef FAUDES_CHECKED
			else{
				std::stringstream errstr;
				errstr << " Missing ']' for pop or push ." << std::endl;
				throw Exception("TpdGenerator::parsePPString", errstr.str(), 200);
			}
			#endif
		}
		#ifdef FAUDES_CHECKED
		else{
			std::stringstream errstr;
			errstr << " Missing '[' for pop or push ." << std::endl;
			throw Exception("TpdGenerator::parsePPString", errstr.str(), 200);
		}
		#endif

		//if no stacksym find, add lambda
		if(syms.empty())
			syms.push_back(FAUDES_PD_LAMBDA);

		return syms;
}

// SetTransition(rX1,rEv,rPop,rPush, rX2) (note: luabinding)
TEMP bool THIS::SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rPop,
								  const std::string& rPush, const std::string& rX2){
	return SetTransition(rX1,rEv,rX2,ParseStackSymbolNames(rPop),ParseStackSymbolNames(rPush));
}

//clearTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ClrTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush){
  return ClrTransition(Transition(x1,ev,x2),rPop,rPush);
}

//clearTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ClrTransition(Idx x1, Idx ev, Idx x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush){
  return ClrTransition(Transition(x1,ev,x2),rPop,rPush);
}

//clearTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ClrTransition(Idx x1, Idx ev, Idx x2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush){
  return ClrTransition(Transition(x1,ev,x2),rPop,rPush);
}

//clearTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ClrTransition(std::string x1, std::string ev, std::string x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush){
  return ClrTransition(Transition(BASE::StateIndex(x1),BASE::EventIndex(ev),BASE::StateIndex(x2)),rPop,rPush);
}

//clearTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ClrTransition(std::string x1, std::string ev, std::string x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush){
	  return ClrTransition(Transition(BASE::StateIndex(x1),BASE::EventIndex(ev),BASE::StateIndex(x2)),rPop,rPush);
}

//clearTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ClrTransition(std::string x1, std::string ev, std::string x2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush){
	  return ClrTransition(Transition(BASE::StateIndex(x1),BASE::EventIndex(ev),BASE::StateIndex(x2)),rPop,rPush);
}

//clearTransition(rTrans,rPop,rPush)
TEMP bool THIS::ClrTransition(const Transition& rTrans, const std::vector<StackSymbol>& rPop,const std::vector<StackSymbol>& rPush){
  return ClrTransition(rTrans,StackSymbolsToIndices(rPop),StackSymbolsToIndices(rPush));
}

//clearTransition(rTrans,rPop,rPush)
TEMP bool THIS::ClrTransition(const Transition& rTrans, const std::vector<std::string>& rPop,const std::vector<std::string>& rPush){
	return ClrTransition(rTrans,StackSymbolNamesToIndices(rPop),StackSymbolNamesToIndices(rPush));
}

//clearTransition(rTrans,rPop,rPush)
TEMP bool THIS::ClrTransition(const Transition& rTrans, const std::vector<Idx>& rPop,const std::vector<Idx>& rPush){
  //check for existence of base transition
  if(!BASE::ExistsTransition(rTrans)){
    return false;
  }

  //delete pop/push pair in said transition
  if(!BASE::pTransRel->Attributep(rTrans)->ClrPopPush(rPop, rPush)){
    return false;
  }

  //delete transition if popPush is empty
  if(PopPush(rTrans).empty()){
    BASE::ClrTransition(rTrans);
  }

  return true;
}

// ClrTransition(rX1,rEv,rPop,rPush, rX2) (note: luabinding)
TEMP bool THIS::ClrTransition(const std::string& rX1, const std::string& rEv, const std::string& rPop,const std::string& rPush, const std::string& rX2){
	return ClrTransition(rX1,rEv,rX2,ParseStackSymbolNames(rPop),ParseStackSymbolNames(rPush));
}

// ExistsTransition(rX1,rEv,rX2)
TEMP bool THIS::ExistsTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2) const{
	return BASE::ExistsTransition(rX1,rEv,rX2);
}

// ExistsTransition(x1, ev, x2)
TEMP bool THIS::ExistsTransition(Idx x1, Idx ev, Idx x2) const{
	return BASE::ExistsTransition(x1,ev,x2);
}

// ExistsTransition(rTrans)
TEMP bool THIS::ExistsTransition(const Transition& rTrans) const{
	return BASE::ExistsTransition(rTrans);
}

// ExistsTransition(x1, ev)
TEMP bool THIS::ExistsTransition(Idx x1, Idx ev) const{
	return BASE::ExistsTransition(x1, ev);
}

// ExistsTransition(x1)
TEMP bool THIS::ExistsTransition(Idx x1) const{
	return BASE::ExistsTransition(x1);
}

//ExistsTransition(x1,ev,x2,pop,push)
TEMP bool THIS::ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& pop, const std::vector<Idx>& push) const{
	return THIS::ExistsTransition(Transition(x1, ev, x2),pop,push);
}

//ExistsTransition(x1,ev,x2,pop,push)
TEMP bool THIS::ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<std::string>& pop, const std::vector<std::string>& push) const{
	return THIS::ExistsTransition(Transition(x1, ev, x2),pop,push);
}

//ExistsTransition(x1,ev,x2,rPop,rPush)
TEMP bool THIS::ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<StackSymbol>& rPop, const std::vector<StackSymbol>& rPush)const{
  return ExistsTransition(Transition(x1,ev,x2),rPop,rPush);
}

//ExistsTransition(x1,ev,x2,pop,push)
TEMP bool THIS::ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& x2, const std::vector<StackSymbol>& pop, const std::vector<StackSymbol>& push) const{
	return THIS::ExistsTransition(Transition(BASE::StateIndex(x1), BASE::EventIndex(ev), BASE::StateIndex(x2)),pop,push);
}

//ExistsTransition(x1,ev,x2,pop,push)
TEMP bool THIS::ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& x2, const std::vector<std::string>& pop, const std::vector<std::string>& push) const{
	return THIS::ExistsTransition(Transition(BASE::StateIndex(x1), BASE::EventIndex(ev), BASE::StateIndex(x2)),pop,push);
}

//ExistsTransition(x1,ev,x2,pop,push)
TEMP bool THIS::ExistsTransition(const std::string&  x1, const std::string& ev, const std::string& x2, const std::vector<Idx>& pop, const std::vector<Idx>& push) const{
	return THIS::ExistsTransition(Transition(BASE::StateIndex(x1), BASE::EventIndex(ev), BASE::StateIndex(x2)),pop,push);
}

//ExistsTransition(rTrans,rPop,rPush)
TEMP bool THIS::ExistsTransition(const Transition& rTrans, const std::vector<StackSymbol>& rPop,const std::vector<StackSymbol>& rPush)const{
  return ExistsTransition(rTrans,StackSymbolsToIndices(rPop),StackSymbolsToIndices(rPush));
}

//ExistsTransition(rTrans,rPop,rPush)
TEMP bool THIS::ExistsTransition(const Transition& rTrans, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush) const{
	return ExistsTransition(rTrans,StackSymbolNamesToIndices(rPop),StackSymbolNamesToIndices(rPush));
}

//ExistsTransition(rTrans,rPop,rPush)
TEMP bool THIS::ExistsTransition(const Transition& rTrans, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush) const{
	if(BASE::ExistsTransition(rTrans)){
		PopPushSet popPushSet = PopPush(rTrans);

		std::pair<std::vector<Idx>, std::vector<Idx> > popPushPair;
		  popPushPair.first = rPop;
		  popPushPair.second = rPush;

		  return popPushSet.find(popPushPair) != popPushSet.end();
	}

	return false;
}

// ExistsTransition(rX1,rEv,rPop,rPush, rX2) (note: luabinding)
TEMP bool THIS::ExistsTransition(const std::string& rX1, const std::string& rEv, const std::string& rPop,
		const std::string& rPush, const std::string& rX2)const{

	return ExistsTransition(rX1,rEv,rX2,ParseStackSymbolNames(rPop),ParseStackSymbolNames(rPush));
}

//PopPush(trans)
TEMP const PopPushSet& THIS::PopPush(const Transition& rTrans) const{
#ifdef FAUDES_CHECKED
  if(!BASE::ExistsTransition(rTrans)) {
      std::stringstream errstr;
      errstr << "transition " << BASE::TStr(rTrans) << " not found ";
      errstr << std::endl;
      throw Exception("PushdownGenerator::PopPush(trans)", errstr.str(), 200);
  }
#endif
  return BASE::pTransRel->Attribute(rTrans).PopPush();
}

//PopPushBegin(trans)
TEMP PopPushSet::const_iterator THIS::PopPushBegin(const Transition& rTrans) const {
  return BASE::pTransRel->Attribute(rTrans).PopPush().begin();
}

//PopPushEnd(trans)
TEMP PopPushSet::const_iterator THIS::PopPushEnd(const Transition& rTrans) const {
  return BASE::pTransRel->Attribute(rTrans).PopPush().end();
}

//InsLambdaStackSymbol
TEMP Idx THIS::InsLambdaStackSymbol(){
	if(ExistsStackSymbol(FAUDES_PD_LAMBDA))
		return StackSymbolIndex(FAUDES_PD_LAMBDA);
	else
		return InsStackSymbol(FAUDES_PD_LAMBDA);
}

//IsStackSymbolLambda(index)
TEMP bool THIS::IsStackSymbolLambda(Idx index) const{
  if(StackSymbolName(index).compare(FAUDES_PD_LAMBDA) == 0)
    return true;
  else
    return false;
}

//IsEventLambda(index)
TEMP bool THIS::IsEventLambda(Idx index) const{
  if(BASE::EventName(index).compare(FAUDES_PD_LAMBDA) == 0)
    return true;
  else
    return false;
}


// ConsistentStackSymbol(rStackSymbol)
TEMP void THIS::ConsistentStackSymbol(const StackSymbol& rStackSymbol) const {
  FD_DG("PushdownGenerator(" << this << ")::ConsistentStackSymbol(rStackSymbol)");
  if(!StackSymbols().Exists(rStackSymbol.Symbol())) {
    std::stringstream errstr;
    errstr << "stack symbol table mismatch (symbol " << rStackSymbol.mSymbol << " does not exist)" << std::endl;
    throw Exception("PushdownGenerator::ConsistentStackSymbol", errstr.str(), 1001);
  }
  FD_DG("PushdownGenerator(" << this << ")::ConsistentStackSymbol(rStackSymbol): ok");
}

// ConsistentStackSymbol(Idx)
TEMP void THIS::ConsistentStackSymbol(Idx idx) const {
  FD_DG("PushdownGenerator(" << this << ")::ConsistentStackSymbol(idx)");
  if(!StackSymbols().Exists(idx)) {
    std::stringstream errstr;
    errstr << "stack symbol table mismatch (symbol with idx " << idx << " does not exist)" << std::endl;
    throw Exception("PushdownGenerator::ConsistentStackSymbol", errstr.str(), 1001);
  }
  FD_DG("PushdownGenerator(" << this << ")::ConsistentStackSymbol(idx): ok");
}

// ConsistentVectorStackSymbol(vector)
TEMP void THIS::ConsistentVectorStackSymbol(const std::vector<StackSymbol>& rVector) const {
  FD_DG("PushdownGenerator(" << this << ")::ConsistentVectorStackSymbol(rVector)");
  std::vector<StackSymbol>::const_iterator it;
  it = rVector.begin();
  for ( ; it < rVector.end(); it++){
    if(!StackSymbols().Exists(it->Symbol())) {
      std::stringstream errstr;
      errstr << "stack symbol table mismatch (symbol " << it->mSymbol << " does not exist)" <<std::endl;
      throw Exception("PushdownGenerator::ConsistentVectorStackSymbol", errstr.str(), 1001);
    }
  }
  FD_DG("PushdownGenerator(" << this << ")::ConsistentVectorStackSymbol(rVector): ok");
}

// ConsistentVectorStackSymbol(vector)
TEMP void THIS::ConsistentVectorStackSymbol(const std::vector<Idx>& rVector) const {
  FD_DG("PushdownGenerator(" << this << ")::ConsistentVectorStackSymbol(rVector)");
  std::vector<Idx>::const_iterator it;
  it = rVector.begin();
  for ( ; it < rVector.end(); it++){
    if(!StackSymbols().Exists(*it)) {
      std::stringstream errstr;
      errstr << "stack symbol table mismatch (symbol with index " << *it << " does not exist)" <<std::endl;
      throw Exception("PushdownGenerator::ConsistentVectorStackSymbol", errstr.str(), 1001);
    }
  }
  FD_DG("PushdownGenerator(" << this << ")::ConsistentVectorStackSymbol(rVector): ok");
}

// EmptyVector(vector)
TEMP void THIS::EmptyVectorPopPush(const std::vector<StackSymbol>& rVector) const {
  FD_DG("PushdownGenerator(" << this << ")::EmptyVector(rVector)");
  if(rVector.empty()) {
    std::stringstream errstr;
    errstr << "empty vector not allowed in pop or push" <<std::endl;
    throw Exception("PushdownGenerator::EmptyVector", errstr.str(), 1001);
  }
  FD_DG("PushdownGenerator(" << this << ")::EmptyVectorPopPush(rVector): ok");
}

// EmptyVector(vector)
TEMP void THIS::EmptyVectorPopPush(const std::vector<Idx>& rVector) const {
  FD_DG("PushdownGenerator(" << this << ")::EmptyVector(rVector)");
  if(rVector.empty()) {
    std::stringstream errstr;
    errstr << "empty vector not allowed in pop or push" <<std::endl;
    throw Exception("PushdownGenerator::EmptyVector", errstr.str(), 1001);
  }
  FD_DG("PushdownGenerator(" << this << ")::EmptyVectorPopPush(rVector): ok");
}


//SetMerge(stateName,rMerge)
TEMP void THIS::SetMerge(const std::string& stateName, MergeAbstract& rMerge){
  Idx index=BASE::StateIndex(stateName);
  BASE::pStates->Attributep(index)->SetMerge(&rMerge);
}

//SetMerge(index,rMerge)
TEMP void THIS::SetMerge(Idx index, MergeAbstract& rMerge){
  BASE::pStates->Attributep(index)->SetMerge(&rMerge);
}

//Merge(index)
TEMP const MergeAbstract* THIS::Merge(Idx index) const{
  return BASE::pStates->Attributep(index)->Merge();
}

//SetDfaState(stateName, dpaIndex)
TEMP void THIS::SetDfaState(const std::string& stateName, Idx dfaIndex){
  Idx index=BASE::StateIndex(stateName);
  BASE::pStates->Attributep(index)->DfaState(dfaIndex);
}

//SetDfaState(index, dpaIndex)
TEMP void THIS::SetDfaState(Idx index, Idx dfaIndex){
  BASE::pStates->Attributep(index)->DfaState(dfaIndex);
}

//DfaState(index)
TEMP Idx THIS::DfaState(Idx index) const{
  return BASE::pStates->Attributep(index)->DfaState();
}

// Valid() TODO checks?
TEMP bool THIS::Valid(void) const {
    FD_DV("PushdownGenerator(" << this << ")::Valid()");
    //call base
    if(!BASE::Valid()) return false;
    // check my names

    // check my clockset

    // check all clocksymboltables

    return true;
}


//DotWrite(rFileName, printInfo, lr)
TEMP void THIS::DotWrite(const std::string& rFileName, bool printInfo, bool lr) const {
	  FD_DG("TpdGenerator(" << this << ")::DotWrite(" << rFileName << ","<< printInfo << "," << lr << ")");

	  BASE::SetMinStateIndexMap();
	  StateSet::Iterator sit;
	  EventSet::Iterator eit;
	  StackSymbolSet stacksymbols = StackSymbols();
	  StackSymbolSet::Iterator ssit;
	  TransSet::Iterator tit;
  	  PopPushSet::iterator ppit;
  	  std::vector<Idx>::iterator pit;

	  try {
	    std::ofstream stream;
	    stream.exceptions(std::ios::badbit|std::ios::failbit);
	    stream.open(rFileName.c_str());
	    stream << "// dot output generated by libFAUDES TpdGenerator" << std::endl;
	    stream << "digraph \"" << BASE::Name() << "\" {";
	    stream << std::endl;

	    if(lr) stream << "  rankdir=LR" << std::endl;


		std::string initState = "_", stackbottom = "_";
		if(BASE::ExistsState(BASE::InitState()))
			initState = StateStr(BASE::InitState());
		if(ExistsStackSymbol(StackBottom()))
			stackbottom = StackSymbolStr(StackBottom());

	    if(printInfo){
			//"M = (Q, Sigma, Gamma, initState, stackBottom, Qm, delta)
			//Sigma = Sigma_c U Sigma_uc
	    	//Sigma_c = {...}
	    	//Sigma_uc = {...}
			//Gamma = {...}
			stream << "  \"\" [shape = none " << std::endl;
			stream << "        label = < " << std::endl;

			//"M = (Q, Sigma, Gamma, initState, stackBottom, Qm, delta)
			stream << "  M=(Q, &Sigma; , &Gamma; , "<< initState <<" , "<< stackbottom <<", &delta;, Qm) <br align=\"LEFT\"/>" << std::endl;
			//Sigma = Sigma_c U Sigma_uc
			stream << "  &Sigma; = &Sigma;c &cup; &Sigma;uc <br align=\"LEFT\"/>" << std::endl;
				std::stringstream ConEv,UnConEv;
				for(eit = BASE::AlphabetBegin(); eit != BASE::AlphabetEnd(); ++eit)
					if(BASE::Controllable(*eit)){
						if(!IsEventLambda(*eit))
							ConEv << " "<< EventStr(*eit) << ",";
						//else
						//	ConEv << " "<< "&lambda;" << ",";
					}else{
						if(!IsEventLambda(*eit))
							UnConEv << " "<< EventStr(*eit) << ",";
					}
			//Sigma_c
			std::string evstr = ConEv.str();
			if(!evstr.empty()) evstr.resize(evstr.size()-1);
			stream << "  &Sigma;c = {"<< evstr <<" }  <br align=\"LEFT\"/>" << std::endl;
			//Sigma_uc
			evstr = UnConEv.str();
			if(!evstr.empty()) evstr.resize(evstr.size()-1);
			stream << "  &Sigma;uc = {"<< evstr <<" } <br align=\"LEFT\"/>" << std::endl;
			//Gamma = {...}
			stream << "  &Gamma; = { ";
					int countSymbols = 0;
					for(ssit = StackSymbolsBegin(); ssit != StackSymbolsEnd(); ++ssit){
						if(!IsStackSymbolLambda(*ssit)){
							if(countSymbols % 10 == 9){
								stream << "<br align=\"LEFT\"/>" << std::endl;
								stream << "              ";
							}
							if(countSymbols != 0)
								stream << ", ";
							stream << StackSymbolStr(*ssit);
							countSymbols++;
						}
					}
					stream << " } <br align=\"LEFT\"/>" << std::endl;
			stream << "                >];" << std::endl;
			stream << std::endl;
	    }
	    stream << "  node [shape=circle];" << std::endl;
	    stream << std::endl;

	    //initial states
	    stream << "  // initial states" << std::endl;
	    int i = 1;
	    for (sit = BASE::InitStatesBegin(); sit != BASE::InitStatesEnd(); ++sit) {
	      stream << "  dot_dummyinit_" << i << " [shape=none, label=\"\", width=\"0.0\", height=\"0.0\" ];" << std::endl;
	      stream << "  dot_dummyinit_" << i << " -> \"" << StateStr(*sit) << "\";" << std::endl;
	      i++;
	    }
	    stream << std::endl;

	    //stackbottom
		stream << "  // stackbottom" << std::endl;
	    if(stackbottom != "")
			stream << "  \"" << stackbottom << "\" [style=\"invis\", attr=\"stackbottom\"];" <<std::endl;

	    stream << std::endl;

	    //marked states
	    stream << "  // mstates" << std::endl;
	    for (sit = BASE::MarkedStatesBegin(); sit != BASE::MarkedStatesEnd(); ++sit)
	      stream << "  \"" << StateStr(*sit) << "\" [shape=doublecircle];" << std::endl;

	    stream << std::endl;

	    //rest of stateset
	    stream << "  // rest of stateset" << std::endl;
	    for (sit = BASE::StatesBegin(); sit != BASE::StatesEnd(); ++sit) {
	      if (! (BASE::ExistsInitState(*sit) || BASE::ExistsMarkedState(*sit)) )
			stream << "  \"" << StateStr(*sit) << "\";" << std::endl;
	    }
	    stream << std::endl;

	    //events
	    stream << "  // events" << std::endl;
	    for(eit = BASE::Alphabet().Begin(); eit != BASE::Alphabet().End(); ++eit) {
	    	if(!THIS::IsEventLambda(*eit)){
				//"e" [style="invis", attr="cOfA"];
				stream << "  \"" <<EventStr(*eit) <<"\" [style=\"invis\", attr=\"";
				stream << (BASE::Controllable(*eit)? "C" : "c");
				stream << (BASE::Observable(*eit)? 	 "O" : "o");
				stream << (BASE::Forcible(*eit)? 	 "F" : "f");
				stream << (BASE::Highlevel(*eit)? 	 "A" : "a");
				stream << "\"];" << std::endl;
	    	}
	    }
	    stream << std::endl;

	    //transition
	    stream << "  // transition relation" << std::endl;
	    for(tit = BASE::TransRelBegin(); tit != BASE::TransRelEnd(); ++tit) {
	      std::string eventname;
	      if(BASE::EventName(tit->Ev) == FAUDES_PD_LAMBDA)
	    	  eventname = "&lambda;";
	      else
	    	  eventname = EventStr(tit->Ev);

	      if(PopPush(*tit).empty()){
	    	  stream << "  \"" << StateStr(tit->X1)  << "\" -> \"" << StateStr(tit->X2)
											 << "\" [label=\"" << eventname << " \"];" << std::endl;
	      }else
			  for (ppit = PopPushBegin(*tit); ppit != PopPushEnd(*tit); ++ppit) {
						  std::vector<Idx> vPop = (*ppit).first;
						  std::vector<Idx> vPush = (*ppit).second;

						  // "x1" -> "x2" [label="ev,[pop],[push]"]
						  stream << "  \"" << StateStr(tit->X1)  << "\" -> \"" << StateStr(tit->X2)
									 << "\" [label=\"" << eventname << ",";

						  //[pop]
						  stream << "[";
						  for (pit = vPop.begin(); pit != vPop.end(); ++pit) {
							  if(!IsStackSymbolLambda(*pit))
								  stream << StackSymbolName(*pit); //stackSymbol;
							  else
								  stream << "&lambda;";

							  if(pit+1 != vPop.end()) stream << ",";
						  }
						  stream << "]";

						  //[push]
						  stream << "[";
						  for (pit = vPush.begin(); pit != vPush.end(); ++pit) {
							  if(!IsStackSymbolLambda(*pit))
							  	  stream << StackSymbolName(*pit); //stackSymbol;
							  else
							  	  stream << "&lambda;";

							  if(pit+1 != vPush.end()) stream << ",";
						  }
						  stream << "]\"";

						  // if uncontrollable event transition
						  // "x1" -> "x2" [label="ev,[pop],[push]" style ="dashed"]
						  if(!BASE::Controllable(tit->Ev))
							  stream << " style=\"dashed\"";

						  stream << " ];" << std::endl;
			  }
			}
	    stream << "}" << std::endl;
	    stream.close();
	  }
	  catch (std::ios::failure&) {
	    throw Exception("vGenerator::DotWrite",
			    "Exception opening/writing dotfile \""+rFileName+"\"", 2);
	  }
}

//DotWrite(rFileName)
TEMP void THIS::DotWrite(const std::string& rFileName) const {
	  FD_DG("TpdGenerator(" << this << ")::DotWrite(" << rFileName << ")");
	  DotWrite(rFileName,true,true);
}

//CreatPdFromDot(rFileName) in pd_parseDot_pd.h
TEMP void CreatPdFromDot(const std::string& rFileName,THIS& pd);

//DotRead(rFileName)
TEMP void THIS::DotRead(const std::string& rFileName) {
    FD_DG("TpdGenerator(" << this << ")::DotRead(" << rFileName << ")");
    this->Clear();
    try{
    	CreatPdFromDot(rFileName,*this);
    }catch(...){
    	std::stringstream errstr;
    	errstr << "Unable to parse "<< rFileName << std::endl;
    	throw Exception("TpdGenerator::DotRead", errstr.str(), 200);
    }
}


// clean up defs
#undef THIS 
#undef BASE 
#undef TEMP


} // namespace faudes


#endif


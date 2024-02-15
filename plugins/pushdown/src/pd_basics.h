/** @file pd_basics.h  Attributes for pushdown automata */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_BASICS_H
#define FAUDES_PD_BASICS_H

#include "corefaudes.h"
#include <list>


#if __cplusplus < 201103L
#define FAUDES_PD_TR1
#endif


#define FAUDES_PD_LAMBDA "lambda"
#define FAUDES_PD_STACK_BOTTOM "bottom"
namespace faudes {



/* 
define non-lsb compliant "uint" from gcc" (tmoor)
note: "pd_include.h" is meant for the external API,
not for internal declarations; this should be reorganized
to have e.g. the below typedef local to the pushdown plug-in
*/
typedef unsigned int uint;



/**
 * Container class to model a set of stack symbols. 
 * Technically, this is a NameSet with a static SymbolTable to map
 * symbolic stack symbol names to stack symbol indices. Thus, stack symbol names are
 * global similar to event names. 
 *
 * @ingroup PushdownPlugin 
 *
 */

class FAUDES_API StackSymbolSet : public NameSet {

  FAUDES_TYPE_DECLARATION(StackSymbolSet,StackSymbolSet,NameSet)

  public:

  /**
   * Constructor 
   */
  StackSymbolSet(void);

  /**
   * Copy-constructor.
   *
   * @param rOtherSet
   *   Set to copy
   */
  StackSymbolSet(const StackSymbolSet& rOtherSet);

  /**
    * Construct from file.
    * Uses the NameSet's Read() function to scan a file for a 
    * specified clockset.
    *
    * @param rFilename
    *   File to read
    * @param rLabel
    *   Section label for the clocks in the file; default value "Clocks"
    */
  StackSymbolSet(const std::string& rFilename, const std::string& rLabel = "StackSymbols");

  /**
    * Get pointer to static stack SymbolTable
    *
    * @return
    *   Pointer to static stack SymbolTable
    */
  static SymbolTable* StaticSymbolTablep(void);

  protected:

  /** Static global SymbolTable for stack symbol names */
  static SymbolTable msSymbolTable;

  /**
   * Assign from other stack symbol set.
   *
   * @param rSourceSet
   *   Destination to copy from
   * @return
   *   ref to this set
   */
  void DoAssign(const StackSymbolSet& rSourceSet);

  /**
   * Test equality with other clock set.
   *
   * @param rOtherSet
   *   Set to compare with
   * @return
   *   True/false
   */
  bool DoEqual(const StackSymbolSet& rOtherSet) const;

}; // end class StackSymbolSet

/**
* 
* Deprecated, do not use!
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

class FAUDES_API StackSymbol{
  public:
  
  
  std::string mSymbol;
  
  /**
   * Constructor
   */
  StackSymbol(void) {};
  
  /**
   * Constructor
   * @param
   *    symbol name to set
   */
  StackSymbol(std::string symbol) : mSymbol(symbol) {};
  
  /**
   * Getter for mSymbol
   * 
   * @return 
   *    mSymbol
   */
  const std::string& Symbol() const {return mSymbol;}
  
  
  /**
   * Write to TokenWriter. Currently not used!
   *
   * @param tw
   *   Reference to TokenWriter
   *
   * @exception std::ios::failure     
   *   Thrown on i/o error.
   */
  void Write(TokenWriter& tw) const;
    
  /**
   * Write to TokenWriter with a given label
   *
   * @param tw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label for set in file
   *
   * @exception std::ios::failure
   *   Thrown on i/o error.
   */
  void Write(TokenWriter& tw, const std::string& rLabel) const;
  
  /**
   * Test inequality with other stack symbol.
   * 
   * @param other
   *    stack symbol to compare with
   * @return
   *    True if mSymbol is not the same, else false
   */
  bool operator!=(const StackSymbol& other) const;
  
  /**
   * Test equality with other stack symbol.
   * 
   * @param other
   *    stack symbol to compare with
   * @return
   *    True if mSymbol is the same, else false
   */
  bool operator==(const StackSymbol& other) const;
  
  /**
   * Test equality with other stack symbol.
   * 
   * @param other
   *    stack symbol to compare with
   * @return
   *    True if mSymbol is the same, else false
   */
  bool operator<(const StackSymbol& other) const;
  
  /**
   * Test if stack symbol is lambda.
   * 
   * @return
   *    True if mSymbol is lambda, else false
   */
  bool IsLambda() const;
  
  /**
   * Write mSymbol to string
   * @return
   *    mSymbol as String
   */
  std::string ToString() const;
  
  /**
   * Tests if mSymbol is an empty String ("")
   * @return
   *    true if mSymbol is an emptry String, else false
   */
  bool Empty() const;
  
}; // end class Stack Symbol


} // namespace faudes

#endif

/** @file pd_debug.h Debug functions */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14  Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef PD_DEBUG_H
#define PD_DEBUG_H

#include "libfaudes.h"
#include "pd_include.h"


namespace faudes{

/**
 * Set level of debugging
 * 0: do nothing
 * 1: print to console
 * 2: print generator to file (.dot and .png)
 */
#define PD_DEBUG_LEVEL 1

//Maximal number of printable transitions
#define PD_DEBUG_MAX_TRANS 1000


//***************************************************************
//   Print Ops
// **************************************************************

/**
 * Print generator depending on debug level (PD_DEBUG_LEVEL)
 *
 * @param msg
 * 		debugging message and name of printed file
 * @param pd
 * 		reference to generator to be printed
 *
 * @return
 */
void debug_printf(const std::string& msg, const PushdownGenerator& pd);

/**
 * Print given text depending on debug level (PD_DEBUG_LEVEL)
 *
 * @param msg
 * 		debugging message and name of printed file
 * @param tex
 * 		text to be printed
 *
 * @return
 */
void debug_printf(const std::string& msg, const std::string& tex = "");


#if true
 #define DEBUG_PRINTF(b, x, y) if(b)debug_printf(x,y)
#else
 #define DEBUG_PRINTF(b, x, y)
#endif



/**
 * Print pushdown generator to file,
 * if the number of transitions does not exceed PD_DEBUG_MAX_TRANS
 *
 * The file will be annoutated with pd.XXXXX.name where XXXX stands for an unique
 * number for each file.
 *
 * @param pd
 *    reference to pushdown generator to be printed
 * @param name
 *    name of file
 * @param printDOT
 *    true, if DOT-file should be print
 * @param printPNG
 *    true, if PNG Image should be print
 *
 * @return
 *
 */
void ToFile(const PushdownGenerator& pd,std::string name, bool printDOT =true, bool printPNG=true);

/**
 * Print given text to file.
 *
 * The file will be annoutated with pd.XXXXX.name where XXXX stands for an unique
 * number for each file.
 *
 * @param pd
 *    reference to pushdown generator to be print
 * @param name
 *    name of file
 *
 */
void ToFile(const std::string& tex,std::string name);


}

#endif

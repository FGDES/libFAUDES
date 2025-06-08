/** @file omg_hoa.h Serialisation in HOA format */

/*

   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef FAUDES_OMG_HOA_H
#define FAUDES_OMG_HOA_H

#include "corefaudes.h"
#include "omg_rabinaut.h"

namespace faudes {


/**
 * Export Automaton as HOA formated stream
 *
 * We iterate over all relevant entities of the specified automaton
 * and somewhat adboc writes HOA formated entities to the specified
 * stream.
 *
 * Our current implementation is restricted to (nondetermistic) Buechi
 * automata.
 *
 * @param rOutStream
 *   Stream to write to
 * @param rAut
 *   Automaton to write
 * @param
 *   Optional symbol table to record event mapping
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void ExportHoa(
  std::ostream& rOutStream,
  const Generator& rAut,
  SymbolTable* pSymTab=nullptr);

/**
 * Export Automaton as HOA formated stream
 *
 * Convenience wrapper, for detail see ExportHoa(std::ostream&, const Generator&)
 *
 * @param rFilename
 *   File to write to
 * @param rAut
 *   Automaton to write
 * @param
 *   Optional symbol table to record event mapping
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void ExportHoa(
  const std::string& rFilename,
  const Generator& rAut,
  SymbolTable* pSymTab=nullptr);


/**
 * Import Automaton from HOA formated stream
 *
 * To reading input in HAO format we use the cpphoafparser library,
 * authored/copyrighted by
 *
 * Joachim Klein <klein@tcs.inf.tu-dresden.de>
 * DDavid Mueller <david.mueller@tcs.inf.tu-dresden.de>
 *
 * We have found the original sources at
 *
 * http://automata.tools/hoa/cpphoafparser
 *
 * They are distributed under LGPL v2.1 conditions and we include
 * them with libFAUDES under the same license terms.
 *
 * Our current implementation can read deterministic Rabin automata
 * with implicit edges. This is not a restriction of cpphoafparser
 * and we extend for further use cases in future.
 *
 * @param rInStream
 *   Stream to read from
 * @param rAut
 *   Resultimg automaton
 * @param
 *   Optional symbol table to provide event mapping
 * @param resolve
 *   Ask cpphoafparser to resolve aliase
 * @param trace
 *   Ask cpphoafparser for a parse trace on std::cerr
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void ImportHoa(
  std::istream& rInStream,
  RabinAutomaton& rAut,
  const SymbolTable* pSymTab=nullptr,
  bool resolve=false,
  bool trace=false);


/**
 * Import Automaton from HOA formated file
 *
 * Convenience wrapper, see ImportHoa(std::istream&,RabinAutomaton&) for details.
 *
 * @param rFilename
 *   File to read from
 * @param rAut
 *   Resultimg automaton
 * @param
 *   Optional symbol table to provide event mapping
 * @param resolve
 *   Ask cpphoafparser to resolve aliase
 * @param trace
 *   Ask cpphoafparser for a parse trace on std::cerr
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void ImportHoa(
  const std::string& rFilename,
  RabinAutomaton& rAut,
  const SymbolTable* pSymTab=nullptr,
  bool resolve=false,
  bool trace=false);

} // namespace faudes

#endif 


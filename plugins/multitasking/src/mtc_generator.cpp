/** @file mtc_generator.cpp

Methods for handling multitasking generators

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Matthias Singer
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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


#include "mtc_generator.h"

namespace faudes {

// rti wrapper
bool IsStronglyCoaccessible(const MtcSystem& rGen) {
  return rGen.IsStronglyCoaccessible();
}

// rti wrapper
bool IsStronglyTrim(const MtcSystem& rGen) {
  return rGen.IsStronglyTrim();
}

// rti wrapper
void StronglyCoaccessible(MtcSystem& rGen) {
  rGen.StronglyCoaccessible();
}

// rti wrapper
void StronglyCoaccessible(const MtcSystem& rGen, MtcSystem& rRes) {
  rRes=rGen;
  rRes.StronglyCoaccessible();
}  

// rti wrapper
void StronglyTrim(MtcSystem& rGen) {
  rGen.StronglyTrim();
}

// rti wrapper
void StronglyTrim(const MtcSystem& rGen, MtcSystem& rRes) {
  rRes=rGen;
  rRes.StronglyTrim();
}  


} // namespace

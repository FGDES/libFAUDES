/** @file omg_interface.i @brief bindings for omegaaut plug-in */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025  Thomas Moor
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

/*
The omageaut plug-in does not provide bindings for data structures yet, so this
interface file is a dummy
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module omegaaut
#ifndef SwigModule
#define SwigModule "SwigOmegaAut"
#endif


// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy synthesis to faudes name space
for k,v in pairs(omegaaut) do faudes[k]=v end
}
#endif				 




// extra c code for swig
%{

%}

/*
**************************************************
**************************************************
**************************************************

actual interface

**************************************************
**************************************************
**************************************************
*/

/* put interface of data structures here */


/*
**************************************************
**************************************************
**************************************************

Interface: auto generated

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "priorities"
SwigHelpTopic("OmegaAut","Omega Automata PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigOmegaAut"
%include "rtiautoload.i"
#endif
 

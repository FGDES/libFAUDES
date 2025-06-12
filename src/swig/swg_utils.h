/** @file swg_utils.h  utility functions for bindings*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008-2025 Thomas Moor
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

#ifndef FAUDES_SWGUTILS_H
#define FAUDES_SWGUTILS_H

#include "cfl_definitions.h"
#include "cfl_utils.h"
#include "cfl_generator.h"

namespace faudes {

// programatically throw exceptions
extern FAUDES_API void faudes_throw_exception(const std::string& msg);


// behavioural features
extern FAUDES_API void faudes_statenames_on(void);
extern FAUDES_API void faudes_statenames_off(void);
extern FAUDES_API void faudes_dotexecpath(const std::string& filename);
extern FAUDES_API std::string faudes_dotexecpath();
extern FAUDES_API std::string  faudes_version(void);
extern FAUDES_API std::string  faudes_build(void);
extern FAUDES_API bool faudes_dotready(void);


// help: insert topic to dictionary
extern FAUDES_API void faudes_dict_insert_topic(const std::string& topic, const std::string& text);

// help: insert entry to dictionary
extern FAUDES_API void faudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry);

// show main help text
extern FAUDES_API void faudes_help(void);

// help: show section text
extern FAUDES_API void faudes_help(const std::string& topic);
  

}//namespace  

#endif // .h

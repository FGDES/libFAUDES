/** @file cfl_exception.cpp Class Exception */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
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


#include "cfl_exception.h"

namespace faudes {

Exception::Exception(const std::string& rFunctionName, 
		     const std::string& rDescription, unsigned int errorId, bool mute) 
		: exception(), mFunctionName(rFunctionName), 
		mDescription(rDescription), mErrorId(errorId) { 
	mMessage="libFAUDES Exception: " + rFunctionName + ": " +rDescription;
#ifdef FAUDES_DEBUG_EXCEPTIONS
        if(!mute) {
	     FAUDES_WRITE_CONSOLE("FAUDES_EXCEPTION: " + rFunctionName + ": " +rDescription);
	}
#endif
}

Exception::~Exception() throw() { }

const char* Exception::Where() const throw() {
	return mFunctionName.c_str();
}

const char* Exception::What() const throw() {
	return mDescription.c_str();
}

unsigned int Exception::Id() const throw() {
	return mErrorId;
}

const char* Exception::Message() const throw() {
	return mMessage.c_str();
}

} // namespace faudes

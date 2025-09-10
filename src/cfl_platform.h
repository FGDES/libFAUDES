/** @file cfl_platform.h  Platform dependant wrappers */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013, 2024  Thomas Moor

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNES FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 
*/


#ifndef FAUDES_PLATFORM_H
#define FAUDES_PLATFORM_H

/* Sense POSIX versus Windows */
#ifndef FAUDES_POSIX
#ifndef FAUDES_WINDOWS
#ifndef FAUDES_GENERIC

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define FAUDES_WINDOWS
#endif
#if defined (__unix__) || (__linus__) || (__linux__) || (defined (__APPLE__) && defined (__MACH__))
#define FAUDES_POSIX
#endif

#ifndef FAUDES_POSIX
#ifndef FAUDES_WINDOWS
#define FAUDES_GENERIC
#endif
#endif

#endif
#endif
#endif


/* Interface export/import symbols: windows */
#ifdef FAUDES_WINDOWS
  #ifdef __GNUC__
    #ifdef FAUDES_BUILD_DSO
      #define FAUDES_API __attribute__ ((dllexport))
    #endif
    #ifdef FAUDES_BUILD_APP
      #define FAUDES_API __attribute__ ((dllimport))
    #endif
  #else
    #ifdef FAUDES_BUILD_DSO
      #define FAUDES_API __declspec(dllexport) 
    #endif
    #ifdef FAUDES_BUILD_APP
      #define FAUDES_API __declspec(dllimport) 
    #endif
  #endif
#endif

/* Interface export/import symbols: posix/gcc */
#ifdef FAUDES_POSIX
#ifdef FAUDES_BUILD_DSO
  #if __GNUC__ >= 4
    #define FAUDES_API __attribute__ ((visibility ("default")))
    #define FAUDES_TAPI  __attribute__ ((visibility ("default")))
  #endif
#endif
#endif

/* Interface export/import symbols: default */
#ifndef FAUDES_API
   #define FAUDES_API
#endif
#ifndef FAUDES_TAPI
   #define FAUDES_TAPI
#endif
 


/* Std headers */
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <limits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <iterator>


// Extra POSIX headers 
#ifdef FAUDES_POSIX

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#ifdef FAUDES_SYSTIME
#include <time.h>
#include <sys/time.h>
#endif

#ifdef FAUDES_NETWORK
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#ifdef FAUDES_THREADS
#include <pthread.h>
#endif

#endif // include POSIX headers


// Extra Windows headers
#ifdef FAUDES_WINDOWS

#ifdef FAUDES_NETWORK
#include <winsock2.h>
#include <ws2tcpip.h>  // MS VC 2017
#include <fcntl.h>
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define FAUDES_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef FAUDES_LEAN_AND_MEAN
#undef FAUDES_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

#ifdef FAUDES_SYSTIME
#include <time.h>  // ok for cl
//#include <sys/time.h> //mingw only
#include <mmsystem.h>
#endif


#ifdef FAUDES_THREADS
#include <process.h>
#endif

#include <io.h>

// dislike min/max msvc macros
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#endif



// Path-seperators (see cfl_utils.cpp)
//  * the first separator is the one used to prepend directories etc
//  * all other separators are used to extract filenames, i.e., to strip the path
//  * as of libFAUDES 2.32 we internally treat "/" is our separator; we set
//    path seperator to "/" for POSIX and "/\\:" for Windows.
//  * up to libFAUDES 2.31 we used "/" for POSIX and "\\:/" for Windows; this was
//    asking for trouble.
extern FAUDES_API const std::string& faudes_pathseps(void); 
// Path-seperator (first char of above, see cfl_utils.cpp)
extern FAUDES_API const std::string& faudes_pathsep(void);


// Extanal vs internal paths conversion -- we are so bored
//
// Internal is as of v2.32  posix style, i.e., '/' is the only separtor, no
// drive letters whatsowever. Is need be, they are converted along the
// pattern "C:\ ==> /c/". libFAUDES should not operate on absolute oaths anyway.
// Occasionally (when a libFAUDES tool needs to invoke a shell), we need to
// convery back to external representaion. Likewise, libFAUDES tools may be
// invoked with posix style argumetns and may need to convert. Henve the following
// two conversion functons (which should be identity on posix systems)
extern FAUDES_API std::string faudes_normpath(const std::string& rExtPath);
extern FAUDES_API std::string faudes_extpath(const std::string& rNormIntPath);




// uniform get/set dir (use posix style interface)
extern FAUDES_API std::string faudes_getwd(void);
extern FAUDES_API int faudes_chdir(const std::string& nwd);

// Uniform exit-signalhandler for POSIX/Windows (see e.g. simfaudes.cpp)
extern FAUDES_API void faudes_termsignal(void (*sighandler)(int));

// Uniform signal names for POSIX/Windows (see e.g. simfaudes.cpp)
extern FAUDES_API const char* faudes_strsignal(int sig);

// Uniform sleep for POSIX/Windows (see e.g. iodevice plug-in)
extern FAUDES_API void faudes_sleep(long int sec);
extern FAUDES_API void faudes_usleep(long int usec); 


// have time
#ifdef FAUDES_SYSTIME

// Uniform system time using POSIX pthreads semantics
#ifdef FAUDES_POSIX
typedef timespec faudes_systime_t;
typedef long int faudes_mstime_t;
#endif
#ifdef FAUDES_WINDOWS
typedef struct {
  long int tv_sec;
  long int tv_nsec;
} faudes_systime_t;
typedef long int faudes_mstime_t;
#endif
#ifdef FAUDES_GENERIC
#error option systime not available on generic platform
#endif

// Uniform system time definitions
extern FAUDES_API void faudes_gettimeofday(faudes_systime_t* now);
extern FAUDES_API void faudes_diffsystime(const faudes_systime_t& end, const faudes_systime_t& begin, faudes_systime_t* res);
extern FAUDES_API void faudes_diffsystime(const faudes_systime_t& end, const faudes_systime_t& begin, faudes_mstime_t* res);
extern FAUDES_API void faudes_sumsystime(const faudes_systime_t& begin, const faudes_systime_t& duration, faudes_systime_t* res);
extern FAUDES_API void faudes_msdelay(faudes_mstime_t msecs,faudes_systime_t* end);
extern FAUDES_API void faudes_usdelay(faudes_mstime_t usecs,faudes_systime_t* end);

// global performance times
extern FAUDES_API faudes_systime_t gPerfTimer1;

#endif // systime


// have IP network
#ifdef FAUDES_NETWORK

// Uniform POSIX sockets (see iop_modbus.cpp and iop_simplenet.cpp)
extern FAUDES_API int faudes_closesocket(int fd);
extern FAUDES_API int faudes_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
extern FAUDES_API int faudes_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen);
extern FAUDES_API int faudes_setsocket_nonblocking(int fd, bool noblo);
extern FAUDES_API int faudes_getsocket_error(int fd);

// POSIX sockets to have BSD style REUSEPORT option (see iop_modbus.cpp and iop_simplenet.cpp)
#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

#endif


#ifdef FAUDES_THREADS

/* 
The remaining section of this file provides elementary support for threads, 
using a minimalistic subset of the POSIX threads interface. It is tailored for 
the use of edge-detection and networking as required by the iodevice plug-in. 
In general, libFAUDES is not threadsafe due to global variables, e.g. 
symoltables. This may change in a future revision.
*/


// Common return codes 
#define FAUDES_THREAD_SUCCESS  0 
#define FAUDES_THREAD_ERROR    1 
#define FAUDES_THREAD_TIMEOUT  2 

// Thread data type (use plain POSIX thread)
#ifdef FAUDES_POSIX
typedef pthread_t faudes_thread_t;
#endif

// Thread data type 
// We wrap the client function to provide pointer-typed  
// return values (as opposed to Windows int-typed return values)
#ifdef FAUDES_WINDOWS
typedef struct {
  HANDLE mHandle;          // Windows thread handle             
  void *(*mFnct)(void *);  // client function
  void *mArg;              // argument
  void *mRes;              // result
} faudes_thread_record_t;
typedef faudes_thread_record_t* faudes_thread_t;
#endif

// not supported on generic platform
#ifdef FAUDES_GENERIC
#error option threads not available on generic platform
#endif


// Thread functions
extern FAUDES_API int faudes_thread_create(faudes_thread_t *thr, void *(*fnct)(void *), void *arg);
extern FAUDES_API faudes_thread_t faudes_thread_current(void);
extern FAUDES_API int faudes_thread_detach(faudes_thread_t thr);
extern FAUDES_API int faudes_thread_equal(faudes_thread_t thr0, faudes_thread_t thr1);
extern FAUDES_API void faudes_thread_exit(void* res);
extern int faudes_thread_join(faudes_thread_t thr, void **res);

// Mutex data type (use plain POSIX mutex)
#ifdef FAUDES_POSIX
typedef pthread_mutex_t faudes_mutex_t;
#endif

// Mutex data type (use Windows "critical section")
#ifdef FAUDES_WINDOWS
typedef CRITICAL_SECTION faudes_mutex_t;
#endif

// Mutex functions 
extern FAUDES_API int faudes_mutex_init(faudes_mutex_t* mtx);
extern FAUDES_API void faudes_mutex_destroy(faudes_mutex_t* mtx);
extern FAUDES_API int faudes_mutex_lock(faudes_mutex_t *mtx);
extern FAUDES_API int faudes_mutex_trylock(faudes_mutex_t *mtx);
extern FAUDES_API int faudes_mutex_unlock(faudes_mutex_t *mtx);

// Condition variables (use plain POSIX cond vars)
#ifdef FAUDES_POSIX
typedef pthread_cond_t faudes_cond_t;
#endif

// Condition variables for Windows 
// The approach is taken from "Strategies for Implementing POSIX Condition Variables 
// on Win32" by Douglas C. Schmidt and Irfan Pyarali, Department of Computer 
// Science, Washington University. 
#ifdef FAUDES_WINDOWS
typedef struct {
  HANDLE mEvents[2];                   // signal and broadcast event handles
  unsigned int mWaitersCount;          // count the number of waiters
  CRITICAL_SECTION mWaitersCountMutex; // mutex access to waiterscount
} faudes_cond_t;
#endif

// Condition functions
extern FAUDES_API int faudes_cond_init(faudes_cond_t* cond);
extern FAUDES_API void faudes_cond_destroy(faudes_cond_t* cond);
extern FAUDES_API int faudes_cond_signal(faudes_cond_t *cond);
extern FAUDES_API int faudes_cond_broadcast(faudes_cond_t *cond);
extern FAUDES_API int faudes_cond_wait(faudes_cond_t *cond, faudes_mutex_t *mtx);
extern FAUDES_API int faudes_cond_timedwait(faudes_cond_t *cond, faudes_mutex_t *mtx, const faudes_systime_t *abstime);
extern FAUDES_API int faudes_cond_reltimedwait(faudes_cond_t *cond, faudes_mutex_t *mtx, faudes_mstime_t duration);



#endif // threads

#endif // header


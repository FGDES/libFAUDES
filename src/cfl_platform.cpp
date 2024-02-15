/** @file cfl_platform.cpp  Platform dependant wrappers */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Thomas Moor

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



// My header
#include "cfl_definitions.h"
#include "cfl_platform.h"

// Extra header
#include <signal.h>
#include <exception>
#include <ostream>

#ifdef FAUDES_GENERIC
void faudes_invalid(const std::string& msg) {
  std::cerr << msg << std:endl;
  throw 0;
}
#endif



// Path separator (statics, see faudes_pathsep)
#ifdef FAUDES_POSIX
const std::string  faudes_pathseps_str = "/"; 
const std::string  faudes_pathsep_str = "/"; 
#endif
#ifdef FAUDES_WINDOWS
const std::string  faudes_pathseps_str = "\\:/"; 
const std::string  faudes_pathsep_str = "\\"; 
#endif
#ifdef FAUDES_GENERIC
const std::string  faudes_pathseps_str = "/"; 
const std::string  faudes_pathsep_str = "/"; 
#endif


// Uniform signalhandler on termination
void faudes_termsignal(void (*sighandler)(int)) {
#ifdef SIGTERM
  signal(SIGTERM, sighandler);
#endif
#ifdef SIGINT
  signal(SIGINT,  sighandler);
#endif
#ifdef SIGQUIT
  signal(SIGQUIT, sighandler);
#endif
#ifdef SIGHUP
  signal(SIGHUP,  sighandler);
#endif
#ifdef SIGABRT
  signal(SIGABRT, sighandler);
#endif
}

// Uniform signal names for POSIX / Windows/MinGW (see e.g. simfaudes.cpp)
const char* faudes_strsignal(int sig) {
#ifdef FAUDES_POSIX
  return strsignal(sig);
#endif
#ifdef FAUDES_WINDOWS
  return "unknown";
#endif
}


#ifdef FAUDES_SYSTIME

// static debugging timer
faudes_systime_t gPerfTimer1;

// Uniform system time (using pthread timespec semantics, trust MinGW to provide)
#if defined(FAUDES_POSIX) 
void faudes_gettimeofday(faudes_systime_t* now) {
  timeval nowval;
  gettimeofday(&nowval,0);
  now->tv_sec=nowval.tv_sec;
  now->tv_nsec=nowval.tv_usec * 1000;
}
#endif

#if defined(FAUDES_WINDOWS)
void faudes_gettimeofday(faudes_systime_t* now) {
  DWORD nowval;
  nowval= timeGetTime();
  now->tv_sec=nowval / 1000;
  now->tv_nsec=(nowval % 1000) *1000000;
}
#endif



// Uniform system time (supporting functions)
void faudes_diffsystime(const faudes_systime_t& end, const faudes_systime_t& begin, faudes_systime_t* res) {
  res->tv_sec = end.tv_sec-begin.tv_sec;
  res->tv_nsec = end.tv_nsec-begin.tv_nsec;
  if(res->tv_nsec <= 0){
    res->tv_sec--;
    res->tv_nsec += 1000000000;
  }
}

// Uniform system time (supporting functions)
void faudes_diffsystime(const faudes_systime_t& end, const faudes_systime_t& begin, faudes_mstime_t* res) {
  *res = 1000*( end.tv_sec-begin.tv_sec);
  *res += (end.tv_nsec-begin.tv_nsec) / 1000000;
}

// Uniform system time (supporting functions)
void faudes_sumsystime(const faudes_systime_t& begin, const faudes_systime_t& duration, faudes_systime_t* res) {
  res->tv_sec = begin.tv_sec + duration.tv_sec;
  res->tv_nsec = begin.tv_nsec + duration.tv_nsec;
  if(res->tv_nsec >= 1000000000) {
    res->tv_sec++;
    res->tv_nsec-=1000000000;
  }
}

// Uniform system time (supporting functions)
void faudes_msdelay(faudes_mstime_t msecs,faudes_systime_t* end) {
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  faudes_systime_t delta;
  delta.tv_sec = msecs/1000;
  delta.tv_nsec = (msecs % 1000) *1000000;
  faudes_sumsystime(now,delta,end);
}

// Uniform system time (supporting functions)
void faudes_usdelay(faudes_mstime_t usecs,faudes_systime_t* end) {
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  faudes_systime_t delta;
  delta.tv_sec = usecs/1000000;
  delta.tv_nsec = (usecs % 1000000) * 1000;
  faudes_sumsystime(now,delta,end);
}


#endif // systime


#ifdef FAUDES_NETWORK


// Uniform socketoption: test for error
int faudes_getsocket_error(int fd) {
#ifdef FAUDES_POSIX 
  int opt=0;
  socklen_t len = sizeof(opt);
  int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &len) < 0 ? -1 : 0;
  if(opt!=0) res=-1;
  return res;
#endif
#ifdef FAUDES_WINDOWS 
  int opt=0;
  socklen_t len = sizeof(opt);
  int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*) &opt, &len) < 0 ? -1 : 0;
  if(opt!=0) res=-1;
  return res;
#endif
}

// Uniform socketoption: set nonblocking
int faudes_setsocket_nonblocking(int fd, bool noblo) {
#ifdef FAUDES_POSIX 
  int sopt=fcntl(fd, F_GETFL, NULL);
  if(sopt<0) return -1; // error 
  if(noblo) {
    int rc=fcntl(fd, F_SETFL, sopt|O_NONBLOCK);
    return rc < 0 ? -1 : 0;
  } else {
    int rc=fcntl(fd, F_SETFL, sopt& (~O_NONBLOCK));
    return rc < 0 ? -1 : 0;
  }
#endif
#ifdef FAUDES_WINDOWS 
  unsigned long onoff=0;
  if(noblo) onoff=1;
  return ioctlsocket(fd, FIONBIO, &onoff) == SOCKET_ERROR ? -1 : 0;
#endif
}






#endif // network


#ifdef FAUDES_THREADS


// Use Windows thread local storage to implement pointer-typed return values
#ifdef FAUDES_WINDOWS
DWORD faudes_thread_tlsidx=TLS_OUT_OF_INDEXES;
#endif

// Windows thread function wrapper with faudes_thread_t argument;
#ifdef FAUDES_WINDOWS
static unsigned WINAPI faudes_thread_wrapper(void *argfth) {
  // access my thread struct
  faudes_thread_t fthread = (faudes_thread_t) argfth;
  // record my thread struct as tls
  TlsSetValue(faudes_thread_tlsidx,fthread);
  // extract actual function an arguments
  void *(*fnct)(void*) =  fthread->mFnct;
  void *arg = fthread->mArg;
  // call function
  void* res = fnct(arg);
  // uniform exit to retrieve return value
  faudes_thread_exit(res);
  // return dummy
  return 0;
}
#endif

// Thread functions (Windows to mimic pthreads)
#ifdef FAUDES_WINDOWS
int faudes_thread_create(faudes_thread_t *thr, void *(*fnct)(void *), void *arg){
  // initialize thread local storage block
  if(faudes_thread_tlsidx==TLS_OUT_OF_INDEXES) 
    faudes_thread_tlsidx=TlsAlloc();
  if(faudes_thread_tlsidx==TLS_OUT_OF_INDEXES) 
    return FAUDES_THREAD_ERROR;
  // malloc my thread structure
  *thr = (faudes_thread_t) malloc(sizeof(faudes_thread_record_t));
  if(!*thr) return FAUDES_THREAD_ERROR;
  // initialze my thread structure with function and argument
  (*thr)->mFnct = fnct;
  (*thr)->mArg = arg;
  (*thr)->mRes = NULL;
  // start the thread
  (*thr)->mHandle = (HANDLE) _beginthreadex(NULL, 0, faudes_thread_wrapper, (void*) (*thr), 0, NULL);
  // done
  return  (*thr)->mHandle ?  FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
#endif

#ifdef FAUDES_WINDOWS
// Thread functions (Windows to mimic pthreads)
faudes_thread_t faudes_thread_current(void) {
  // retrieve my thread struct from tls
  faudes_thread_t fthread = (faudes_thread_t) TlsGetValue(faudes_thread_tlsidx);
  // note: returns NULL for parent thread
  return fthread;
}
#endif

#ifdef FAUDES_WINDOWS
// Thread functions (Windows to mimic pthreads)
int faudes_thread_detach(faudes_thread_t thr) {
  CloseHandle(thr->mHandle);
  // free my mem
  free(thr);
  return FAUDES_THREAD_SUCCESS;
}
#endif

#ifdef FAUDES_WINDOWS
// Thread functions (Windows to mimic pthreads)
int faudes_thread_equal(faudes_thread_t thr0, faudes_thread_t thr1) {
  // the parent thread has no tls record and thus reports NULL
  if( (thr0==NULL) && (thr1=NULL) ) return true;
  if( (thr0==NULL) || (thr1=NULL) ) return false;
  // std case, compare by handle
  return thr0->mHandle == thr1->mHandle;
}
#endif

#ifdef FAUDES_WINDOWS
// Thread functions (Windows to mimic pthreads)
void faudes_thread_exit(void *res) {
  // retrieve thread structure from tls to pass on result
  faudes_thread_t fthread = (faudes_thread_t) TlsGetValue(faudes_thread_tlsidx);
  if(fthread) fthread->mRes=res;
  // call winapi
  ExitThread(0);
}
#endif

#ifdef FAUDES_WINDOWS
// Thread functions (Windows to mimic pthreads)
int faudes_thread_join(faudes_thread_t thr, void **res) {
  // default result
  if(res) *res = 0;
  // do the join
  DWORD rc = WaitForSingleObject(thr->mHandle, INFINITE);
  // retrieve result from thread structure
  if( (rc!=WAIT_FAILED) && (res))  *res = thr->mRes;  
  // free mem
  free(thr);
  // done
  if(rc == WAIT_FAILED) return FAUDES_THREAD_ERROR;
  return FAUDES_THREAD_SUCCESS;
}
#endif

// The approach to condition variables on Windows is taken from 
// "Strategies for Implementing POSIX Condition Variables on Win32" 
// by Douglas C. Schmidt and Irfan Pyarali, Department of Computer 
// Science, Washington University. 

// Convenience access my two condition events
#ifdef FAUDES_WINDOWS
#define EVENT_SIGNAL 0
#define EVENT_BROADCAST 1
#endif

// Condition functions (Windows to mimic plain pthread)
#ifdef FAUDES_WINDOWS
int faudes_cond_init(faudes_cond_t* cond) {
  // #waiters=0, with mutexed access
  cond->mWaitersCount = 0;
  InitializeCriticalSection(&cond->mWaitersCountMutex);
  // auto-reset event to signal the condition
  cond->mEvents[EVENT_SIGNAL]=CreateEvent(NULL, FALSE, FALSE, NULL);
  // manual-reset event to broadcast the condition
  cond->mEvents[EVENT_BROADCAST] = CreateEvent(NULL, TRUE, FALSE, NULL);
  // initialisation succeeded
  if(cond->mEvents[EVENT_SIGNAL] != NULL) 
  if(cond->mEvents[EVENT_BROADCAST] != NULL) 
    return FAUDES_THREAD_SUCCESS;
  // allow for destroy even on failed initialisation
  if(cond->mEvents[EVENT_SIGNAL] != NULL) 
    CloseHandle(cond->mEvents[EVENT_SIGNAL]);
  if(cond->mEvents[EVENT_BROADCAST] != NULL) 
    CloseHandle(cond->mEvents[EVENT_BROADCAST]);
  cond->mEvents[EVENT_BROADCAST] = NULL;
  cond->mEvents[EVENT_SIGNAL]=NULL;  
  return FAUDES_THREAD_ERROR;
}
#endif

#ifdef FAUDES_WINDOWS
// Condition functions (Windows to mimic plain pthread)
void faudes_cond_destroy(faudes_cond_t* cond) {
  if(cond->mEvents[EVENT_SIGNAL] != NULL) 
    CloseHandle(cond->mEvents[EVENT_SIGNAL]);
  if(cond->mEvents[EVENT_BROADCAST] != NULL) 
    CloseHandle(cond->mEvents[EVENT_BROADCAST]);
  DeleteCriticalSection(&cond->mWaitersCountMutex);
}
#endif

#ifdef FAUDES_WINDOWS
// Condition functions (Windows to mimic plain pthread)
int faudes_cond_signal(faudes_cond_t *cond){
  int waiters;
  // any waiters ?
  EnterCriticalSection(&cond->mWaitersCountMutex);
  waiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountMutex);
  // set event (one waiter will see the auto-reset event)
  if(waiters){
    if(SetEvent(cond->mEvents[EVENT_SIGNAL]) == 0) 
      return FAUDES_THREAD_ERROR;
  }
  return FAUDES_THREAD_SUCCESS;
}
#endif

#ifdef FAUDES_WINDOWS
// Condition functions (Windows to mimic plain pthread)
int faudes_cond_broadcast(faudes_cond_t *cond) {
  int waiters;
  // any waiters ?
  EnterCriticalSection(&cond->mWaitersCountMutex);
  waiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountMutex);
  // set event (all waiters will see, last waiter does the manual-reset)
  if(waiters) {
    if(SetEvent(cond->mEvents[EVENT_BROADCAST]) == 0)
      return FAUDES_THREAD_ERROR;
  }
  return FAUDES_THREAD_SUCCESS;
}
#endif

#ifdef FAUDES_WINDOWS
// Condition functions (Windows to mimic plain pthread)
int faudes_cond_reltimedwait(faudes_cond_t *cond, faudes_mutex_t *mtx, faudes_mstime_t duration) {
  // increment #waiters
  EnterCriticalSection(&cond->mWaitersCountMutex);
  ++ cond->mWaitersCount;
  LeaveCriticalSection(&cond->mWaitersCountMutex);
  // release mutex while waiting 
  LeaveCriticalSection(mtx);
  // wait for either event to be set 
  int res = WaitForMultipleObjects(2, cond->mEvents, FALSE, (DWORD) duration);
  // maintane my data ...
  EnterCriticalSection(&cond->mWaitersCountMutex);
  // ... a) decrement #waiters 
  -- cond->mWaitersCount;
  // ... b) test if this was a broadcast and we were the last waiter ...
  int last = 
    (res == (WAIT_OBJECT_0 + EVENT_BROADCAST)) &&
    (cond->mWaitersCount == 0);
  LeaveCriticalSection(&cond->mWaitersCountMutex);
  // ... c) if so, do a manual-reset of the event
  if(last) ResetEvent(cond->mEvents[EVENT_BROADCAST]);
  // reaquire mutex 
  EnterCriticalSection(mtx);
  // uniform return value
  if(res == (int) WAIT_TIMEOUT) 
    return FAUDES_THREAD_TIMEOUT;
  if(res == (int) WAIT_FAILED) 
    return FAUDES_THREAD_ERROR;
  return FAUDES_THREAD_SUCCESS;
}
#endif

#ifdef FAUDES_WINDOWS
// Condition functions (Windows to mimic plain pthread)
int faudes_cond_wait(faudes_cond_t *cond, faudes_mutex_t *mtx) {
  return faudes_cond_reltimedwait(cond, mtx, INFINITE);
}
#endif

#ifdef FAUDES_WINDOWS
// Condition functions (Windows to mimic plain pthread)
int faudes_cond_timedwait(faudes_cond_t *cond, faudes_mutex_t *mtx, const faudes_systime_t *abstime) {
  // get absolute time
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  // convert to rel. time in secs
  DWORD rels = abstime->tv_sec - now.tv_sec;
  // saturation on overflow 
  if(rels > (4294967295u/1000)-1) rels = 4294967295u/1000-1;
  // convert to rel. time in msecs
  DWORD relms = rels*1000 + (abstime->tv_nsec - now.tv_nsec + 500000) / 1000000;
  // pass on to reltimedwait
  return faudes_cond_reltimedwait(cond, mtx, relms);
}
#endif 


#endif // threads





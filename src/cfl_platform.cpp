/** @file cfl_platform.cpp  Platform dependant wrappers */

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



// My header
#include "cfl_definitions.h"
#include "cfl_platform.h"

// Extra header
#include <csignal>
#include <exception>
#include <ostream>

#ifdef FAUDES_GENERIC
void faudes_invalid(const std::string& msg) {
  std::cerr << "faudes_invalid(): " << msg << std:endl;
  throw 0;
}
#endif


// Path separator (statics, see faudes_pathsep)
#ifdef FAUDES_POSIX
const std::string& faudes_pathseps(void) {
  static std::string  faudes_pathsep_str = "/"; 
  return faudes_pathsep_str;
}
const std::string& faudes_pathsep(void) {
  return faudes_pathseps(); 
}
#endif
#ifdef FAUDES_WINDOWS
const std::string& faudes_pathseps(void) {
  static std::string  faudes_pathseps_str = "/\\:";
  return faudes_pathseps_str;
}
const std::string& faudes_pathsep(void) {
  static std::string  faudes_pathsep_str = "";
  if(faudes_pathsep_str=="") 
    faudes_pathsep_str=faudes_pathseps().substr(0,1);
  return faudes_pathsep_str;
}
#endif
#ifdef FAUDES_GENERIC
const std::string& faudes_pathseps(void) {
  static std::string  faudes_pathsep_str = "/"; 
  return faudes_pathsep_str;
}
const std::string& faudes_pathsep(void) {
  return faudes_pathseps(); 
}
#endif


// Noramliese to internal path representation (as of 2.32 posix)
std::string faudes_normpath(const std::string& rPath){
  std::string res;
  // if the path begins with "drive-letter+':'" it become '/'+drive-letter+'/'
  res=rPath;
  if(rPath.size()>=2) {
    if(rPath.at(1)==':')
      res=faudes_pathsep() + rPath.at(0) + faudes_pathsep() + rPath.substr(2);
  }
  // all seps become my sep (as of 2.32 '/')
  size_t pos;
  for(pos=0; pos<res.size(); ++pos) {
    char c=res.at(pos);
    if(faudes_pathseps().find(c)!=std::string::npos) 
      res.at(pos)=faudes_pathseps().at(0);
  }
  // could do more here: "//"? "NUL:"?
  return res;
}    
  
// Externalise internal path representation to shell compatile format
// note: this addresse windows cmd.exe only
std::string faudes_extpath(const std::string& rPath){
  std::string res=rPath;
#ifdef FAUDES_WINDOWS  
  // all seps become cmd.exe-sep (aka \)
  size_t pos;
  for(pos=0; pos<res.size(); ++pos) {
    char c=res.at(pos);
    if(c==faudes_pathseps().at(0))  
      res.at(pos)='\\';
  }
  // if the path begins with "\drive-letter+'\' it becomes 'drive-letter+':'
  // TODO
  //std::cerr << "faudes_extpath(): ---> " << res << std::endl;
#endif
  return res;
}    


// uniform get/set dir (use posix style interface)
#ifdef FAUDES_POSIX
std::string faudes_getwd(void) {
  std::string res;
  char buf[(PATH_MAX)+1];
  char* wd =getcwd(buf,PATH_MAX);
  if(wd==nullptr) return res;
  res=wd;
  return res;
}
int faudes_chdir(const std::string& nwd) {
  return (chdir(nwd.c_str())==0 ? 0 : -1);
}
#endif
#ifdef FAUDES_WINDOWS
#include <tchar.h>
std::string faudes_getwd(void) {
  std::string res;
  TCHAR buf[MAX_PATH];
  DWORD ret = GetCurrentDirectory(MAX_PATH, buf);
  if((ret == 0) || (ret>MAX_PATH)) return res;
  res=buf;
  return res;
}
int faudes_chdir(const std::string& nwd) {
  TCHAR buf[MAX_PATH];
  if(nwd.size()+1>MAX_PATH)
    return -1;
  strncpy(buf,nwd.c_str(),MAX_PATH-1);
  if(!SetCurrentDirectory(buf))
    return -1;
  return 0;
}
#endif
#ifdef FAUDES_GENERIC
std::string faudes_getwd(void) {
  faudes_invalid("faudes_getwd()");
  std::string res;
  return res;
}
int faudes_chdir(const std::string& nwd) {
  faudes_invalid("faudes_chdir()");
  return -1;
}
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

// Uniform sleep for POSIX/Windows (see e.g. iodevice plug-in)
#ifdef FAUDES_POSIX
void faudes_sleep(long int sec) {sleep(sec);}
void faudes_usleep(long int usec) {usleep(usec);}
#endif
#ifdef FAUDES_WINDOWS
void faudes_sleep(long int sec) {Sleep((sec) * 1000);}
void faudes_usleep(long int usec) {Sleep((usec) / 1000);}
#endif
#ifdef FAUDES_GENERIC
void faudes_sleep(long int sec) { faudes_invalid("faudes_sleep()"); }
void faudes_usleep(long int usec) { faudes_invalid("faudes_usleep()"); }
#endif



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

#ifdef FAUDES_POSIX
int faudes_closesocket(int fd) {
  return close(fd);
}
int faudes_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen) {
  return setsockopt(fd,level,optname,optval,optlen);
}
int faudes_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen) {
  return getsockopt(fd,level,optname,optval,optlen);
}
int faudes_getsocket_error(int fd) {
  int opt=0;
  socklen_t len = sizeof(opt);
  int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &len) < 0 ? -1 : 0;
  if(opt!=0) res=-1;
  return res;
}
int faudes_setsocket_nonblocking(int fd, bool noblo) {
  int sopt=fcntl(fd, F_GETFL, NULL);
  if(sopt<0) return -1; // error 
  if(noblo) {
    int rc=fcntl(fd, F_SETFL, sopt|O_NONBLOCK);
    return rc < 0 ? -1 : 0;
  } else {
    int rc=fcntl(fd, F_SETFL, sopt& (~O_NONBLOCK));
    return rc < 0 ? -1 : 0;
  }
}
#endif
#ifdef FAUDES_WINDOWS
typedef int socklen_t;
int faudes_closesocket(int fd) {
  return closesocket(fd);
}
int faudes_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen) {
  return setsockopt(fd,level,optname,(char*) optval,optlen);
}
int faudes_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen) {
  return getsockopt(fd,level,optname,(char*) optval,optlen);
}
int faudes_getsocket_error(int fd) {
  int opt=0;
  socklen_t len = sizeof(opt);
  int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*) &opt, &len) < 0 ? -1 : 0;
  if(opt!=0) res=-1;
  return res;
}
int faudes_setsocket_nonblocking(int fd, bool noblo) {
  unsigned long onoff=0;
  if(noblo) onoff=1;
  return ioctlsocket(fd, FIONBIO, &onoff) == SOCKET_ERROR ? -1 : 0;
}
#endif
#ifdef FAUDES_GENERIC
#error option network not available on generic platform
#endif

#endif // network



// straight PPOSIX threads (aka plain wrappers)
#ifdef FAUDES_THREADS
#ifdef FAUDES_POSIX
// Thread data type (use plain POSIX thread)
typedef pthread_t faudes_thread_t;
// Thread functions (plain pthread wrapper)
int faudes_thread_create(faudes_thread_t *thr, void *(*fnct)(void *), void *arg){
  // prepare attribute for plain joinable thread
  pthread_attr_t attr;	
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  // start execute
  int ret  = pthread_create(thr, &attr, fnct, arg);
  // done
  pthread_attr_destroy(&attr);
  return ret == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
faudes_thread_t faudes_thread_current(void) {
  return pthread_self();
}
int faudes_thread_detach(faudes_thread_t thr) {
  return pthread_detach(thr)==0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_thread_equal(faudes_thread_t thr0, faudes_thread_t thr1) {
  return pthread_equal(thr0, thr1);
}
void faudes_thread_exit(void* res) {
  pthread_exit(res);
}
int faudes_thread_join(faudes_thread_t thr, void **res) {
  return pthread_join(thr, res) == 0 ? FAUDES_THREAD_ERROR : FAUDES_THREAD_SUCCESS;
}
#endif
#endif // POSIX threads

// Windows to mimique PPOSIX threads 
#ifdef FAUDES_THREADS
#ifdef FAUDES_WINDOWS

// Use Windows thread local storage to implement pointer-typed return values
DWORD faudes_thread_tlsidx=TLS_OUT_OF_INDEXES;

// Windows thread function wrapper with faudes_thread_t argument;
static unsigned WINAPI faudes_thread_wrapper(void *argfth) {
  // access my thread struct
  faudes_thread_t fthread = (faudes_thread_t) argfth;
  // record my thread struct as tls
  TlsSetValue(faudes_thread_tlsidx,fthread);
  // extract actual function and arguments
  void *(*fnct)(void*) =  fthread->mFnct;
  void *arg = fthread->mArg;
  // call function
  void* res = fnct(arg);
  // uniform exit to retrieve return value
  faudes_thread_exit(res);
  // return dummy
  return 0;
}

// Thread functions (Windows to mimic pthreads)
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

// Thread functions (Windows to mimic pthreads)
faudes_thread_t faudes_thread_current(void) {
  // retrieve my thread struct from tls
  faudes_thread_t fthread = (faudes_thread_t) TlsGetValue(faudes_thread_tlsidx);
  // note: returns NULL for parent thread
  return fthread;
}

// Thread functions (Windows to mimic pthreads)
int faudes_thread_detach(faudes_thread_t thr) {
  CloseHandle(thr->mHandle);
  // free my mem
  free(thr);
  return FAUDES_THREAD_SUCCESS;
}

// Thread functions (Windows to mimic pthreads)
int faudes_thread_equal(faudes_thread_t thr0, faudes_thread_t thr1) {
  // the parent thread has no tls record and thus reports NULL
  if( (thr0==NULL) && (thr1=NULL) ) return true;
  if( (thr0==NULL) || (thr1=NULL) ) return false;
  // std case, compare by handle
  return thr0->mHandle == thr1->mHandle;
}

// Thread functions (Windows to mimic pthreads)
void faudes_thread_exit(void *res) {
  // retrieve thread structure from tls to pass on result
  faudes_thread_t fthread = (faudes_thread_t) TlsGetValue(faudes_thread_tlsidx);
  if(fthread) fthread->mRes=res;
  // call winapi
  ExitThread(0);
}

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
#endif // Windows threads



// straight PPOSIX mutexes (aka plain wrappers)
#ifdef FAUDES_THREADS
#ifdef FAUDES_POSIX
int faudes_mutex_init(faudes_mutex_t* mtx){
  return pthread_mutex_init(mtx, NULL)==0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
void faudes_mutex_destroy(faudes_mutex_t* mtx){
  pthread_mutex_destroy(mtx);
}
int faudes_mutex_lock(faudes_mutex_t *mtx) {
  return pthread_mutex_lock(mtx) == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_mutex_trylock(faudes_mutex_t *mtx){
  return (pthread_mutex_trylock(mtx) == 0) ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_mutex_unlock(faudes_mutex_t *mtx){
  return pthread_mutex_unlock(mtx) == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
#endif
#endif // POSIX mutexes

// Windows to mimique PPOSIX mutexes
#ifdef FAUDES_THREADS
#ifdef FAUDES_WINDOWS
int faudes_mutex_init(faudes_mutex_t *mtx){
  InitializeCriticalSection(mtx);
  return FAUDES_THREAD_SUCCESS;
}
void faudes_mutex_destroy(faudes_mutex_t *mtx){
  DeleteCriticalSection(mtx);
}
int faudes_mutex_lock(faudes_mutex_t *mtx) {
  EnterCriticalSection(mtx);
  return FAUDES_THREAD_SUCCESS;
}
int faudes_mutex_trylock(faudes_mutex_t *mtx){
  return TryEnterCriticalSection(mtx) ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_mutex_unlock(faudes_mutex_t *mtx){
  LeaveCriticalSection(mtx);
  return FAUDES_THREAD_SUCCESS;
}
#endif
#endif // Windows mutexes


// straight PPOSIX conditions (aka plain wrappers)
#ifdef FAUDES_THREADS
#ifdef FAUDES_POSIX
int faudes_cond_init(faudes_cond_t* cond) {
  return pthread_cond_init(cond, NULL) == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
void faudes_cond_destroy(faudes_cond_t* cond) {
  pthread_cond_destroy(cond);
}
int faudes_cond_signal(faudes_cond_t *cond){
  return pthread_cond_signal(cond) == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_cond_broadcast(faudes_cond_t *cond) {
  return pthread_cond_signal(cond) == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_cond_wait(faudes_cond_t *cond, faudes_mutex_t *mtx) {
  return pthread_cond_wait(cond, mtx) == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
int faudes_cond_timedwait(faudes_cond_t *cond, faudes_mutex_t *mtx, const faudes_systime_t *abstime) {
  int ret = pthread_cond_timedwait(cond, mtx, abstime);
  if(ret == ETIMEDOUT) return FAUDES_THREAD_TIMEOUT;
  return ret == 0 ? FAUDES_THREAD_SUCCESS : FAUDES_THREAD_ERROR;
}
// Extension: timed wait with duration as opposed to absolute time
int faudes_cond_reltimedwait(faudes_cond_t *cond, faudes_mutex_t *mtx, faudes_mstime_t duration) {
  faudes_systime_t abstime;
  faudes_msdelay(duration,&abstime);
  return faudes_cond_timedwait(cond,mtx,&abstime);
}
#endif
#endif // POSIX condition


// Windows to mimique POSIX conditions
// The approach to condition variables on Windows is taken from 
// "Strategies for Implementing POSIX Condition Variables on Win32" 
// by Douglas C. Schmidt and Irfan Pyarali, Department of Computer 
// Science, Washington University. 
#ifdef FAUDES_THREADS
#ifdef FAUDES_WINDOWS

// Convenience access my two condition events
#define EVENT_SIGNAL 0
#define EVENT_BROADCAST 1

// Condition functions (Windows to mimic plain pthread)
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

// Condition functions (Windows to mimic plain pthread)
void faudes_cond_destroy(faudes_cond_t* cond) {
  if(cond->mEvents[EVENT_SIGNAL] != NULL) 
    CloseHandle(cond->mEvents[EVENT_SIGNAL]);
  if(cond->mEvents[EVENT_BROADCAST] != NULL) 
    CloseHandle(cond->mEvents[EVENT_BROADCAST]);
  DeleteCriticalSection(&cond->mWaitersCountMutex);
}

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

// Condition functions (Windows to mimic plain pthread)
int faudes_cond_wait(faudes_cond_t *cond, faudes_mutex_t *mtx) {
  return faudes_cond_reltimedwait(cond, mtx, INFINITE);
}

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
#endif // Windows conditions








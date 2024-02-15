/** @file iobridge.cpp  Test utility for IO devices


This tutorial demonstrates how to use elementary access to 
external signals for a bridging device, ie inputs of one device
are mapped to outputs of the other device and vice versa.
This addresses the situation where digital io events are to be
transmitted to or receviced from a network device without running
a simulation.

@ingroup Tutorials

@include iobridge.cpp

*/

#include "libfaudes.h"
#include <signal.h>


using namespace faudes;

// iobridge clean-up on exit
void iobridge_exit(void);

// signal handler recursion flag
volatile sig_atomic_t signal_in_progress = 0;

// signal handler to stop devices
void catch_signal(int sig) {
  // detect recursion, pass on
  if(signal_in_progress) raise(sig);
  signal_in_progress = 1;
  // report
  std::cerr << "iobridge: signal: " << faudes_strsignal(sig) << std::endl;
  // call my exit function
  iobridge_exit();
  // re-install default handler
  signal(sig, SIG_DFL);
  // pass on signal
  raise(sig);
}

// iobridge clean-up on exit
void iobridge_exit(void) {
  // stop all devices
  vDevice::StopAll();
}


// iobridge
int main(int argc, char* argv[]) {

  // install my signal handler
  faudes_termsignal(catch_signal);

  // install my exit fnct
  atexit(iobridge_exit);


  // first two arguments must be the device files
  if(argc!=3) {
    std::cerr << "iobridge: " << VersionString()  << std::endl;
    std::cerr << "usage: iobridge <one device-file> <other device-file>" << std::endl;
    return(-1);
  }

#ifdef FAUDES_NETWORK
#ifdef FAUDES_WINDOWS
  // initialise winsocks
  WSADATA wsaData;
  if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0) {
    std::cerr << "iobridge: failed to initialize winsocks" << std::endl;
    return -1;
  }
#endif
#endif

  //initialize vDevice A
  FD_DH("Initialize vDevice A");
  vDevice* adev;
  adev=vDevice::FromFile(std::string(argv[1]));

  //initialize vDevice A
  FD_DH("Initialize vDevice B");
  vDevice* bdev;
  bdev=vDevice::FromFile(std::string(argv[2]));

  // have mutex/condition for common wait
  faudes_mutex_t wmutex;
  faudes_cond_t wcond;
  faudes_mutex_init(&wmutex);
  faudes_cond_init(&wcond);
  adev->UseCondition(&wmutex,&wcond);
  bdev->UseCondition(&wmutex,&wcond);

  // set up devices
  adev->Compile();
  bdev->Compile();

  // start devices
  adev->Start();
  bdev->Start();

  // loop forever
  while(1) {
   
    std::cout << "% ################ iobridge: waiting for input events" << std::endl;

    // lock waiting
    faudes_mutex_lock(&wmutex);

    // wait for inputs
    faudes_cond_wait(&wcond,&wmutex);

    // test inputs
    while(Idx ev =adev->ReadInput()) {
      std::cout << "% ################ iobridge: sensed " << adev->Name() << "-input " << adev->EStr(ev) << std::endl;
      bdev->WriteOutput(ev);
    } 
    while(Idx ev =bdev->ReadInput()) {
      std::cout << "% ################ iobridge: sensed " << bdev->Name() <<  "-input " << bdev->EStr(ev) << std::endl;
      adev->WriteOutput(ev);
    } 

    // unlock waiting
    faudes_mutex_unlock(&wmutex);

  }

  // never happens
  iobridge_exit();

  // never happens	
  return 0;
}




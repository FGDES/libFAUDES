/** @file iomonitor.cpp  Test utility for IO devices


This tutorial demonstrates elementary access to external signals
via the class faudes::vDevice. It can be used as a test uitility
for device configuration. 


@ingroup Tutorials

@include iomonitor.cpp

*/


#include "libfaudes.h"

#include <signal.h>

using namespace faudes;

///////////////////////////////////////////////////////
// clean exit on signals

// iomonitor clean-up on exit
void iomonitor_exit(void);

// signal handler recursion flag
volatile sig_atomic_t signal_in_progress = 0;

// signal handler to stop devices
void catch_signal(int sig) {
  // detect recursion, pass on
  if(signal_in_progress) raise(sig);
  signal_in_progress = 1;
  // report
  std::cerr << "iomonitor: signal: " << faudes_strsignal(sig) << std::endl;
  // call my exit function
  iomonitor_exit();
  // re-install default handler
  signal(sig, SIG_DFL);
  // pass on signal
  raise(sig);
}

// iomonitor clean-up on exit
void iomonitor_exit(void) {
  // stop all devices
  vDevice::StopAll();
}


///////////////////////////////////////////////////////
//Basic handling

// list all known events
void ListEvents(const vDevice* dev) {
  std::cout<< "% ###############################################################" << std::endl;
  std::cout<< "% # InputEvents" << std::endl;
  dev->Inputs().Write();
  std::cout<< "% ###############################################################" << std::endl;
  std::cout<< "% # OutputEvents " << std::endl;
  dev->Outputs().Write();
  std::cout<< "% ###############################################################" << std::endl;
}


///////////////////////////////////////////////////////
// Query Time

// read time
void ReadTime(vDevice* dev) {

  // read and report
  std::cout << "% ###############################################################" << std::endl;
  std::cout << "% # ReadTime: current time in ftu: " << dev->CurrentTime() << std::endl;
  std::cout << "% # ReadTime: using scale: " << dev->TimeScale() << std::endl;

}


///////////////////////////////////////////////////////
//Signal-I/O

// read signal value
void ReadSignalValue(sDevice* dev) {
  // declare static buffer
  static bool* samplePrevious=0;
  static bool* sampleCurrent=0;
  static int sampleSize=-1;
  // allocate memory for buffer
  if(sampleSize != dev->MaxBitAddress()+1) {
    sampleSize = dev->MaxBitAddress()+1;
    if(samplePrevious!=0) delete samplePrevious;
    if(sampleCurrent!=0) delete sampleCurrent;
    samplePrevious= new bool[sampleSize];
    sampleCurrent= new bool[sampleSize];
  }
  // read and report
  std::cout << "% ###############################################################" << std::endl;
  for(int bit=0; bit<sampleSize; bit++) {
    samplePrevious[bit]=sampleCurrent[bit];
  }
  for(int bit=0; bit<sampleSize; bit++) {
    sampleCurrent[bit]=dev->ReadSignal(bit);
  }
  std::cout << "% # ReadValue: current input reading: " << std::endl;
  for(int bit=0; bit<sampleSize; bit++) {
    std::cout<< "@" << (bit < 10 ? "0" : "") << bit << ":" << sampleCurrent[bit] << "   ";
    if((bit%8)==7 || bit+1==sampleSize) std::cout << std::endl;
  }
  std::cout <<"% # ReadValue: edges wrt previous reading: " << std::endl;
  int cnt =0;
  for(int bit=0; bit<sampleSize; bit++) {
    if(samplePrevious[bit]!=sampleCurrent[bit]) {
      std::cout<< "@"<< (bit < 10 ? "0" : "") << bit << ":" << sampleCurrent[bit] << "   ";
      if((cnt%8)==7 || bit+1==sampleSize) std::cout << std::endl;
      cnt+=1;
    }
  }		
  std::cout << "% ###############################################################" << std::endl;
}


// poll input signals
void PollInputSignals(sDevice* sdev){
  std::cout<<"ReadInputs: time (secs) to monitor input signals: ";    
  faudes_mstime_t time_all;
  std::cin>>time_all;
  time_all*=1000; // convert to msecs
  std::cout<<"ReadEvents: time (msecs) to sleep between two polls: ";    
  faudes_mstime_t  time_delta;
  std::cin>>time_delta;
  // reset all input data so far
  sdev->Reset();
  // report performace, part 1
  faudes_systime_t time_start, time_stop;
  faudes_gettimeofday(&time_start);

  // declare static buffer
  static bool* samplePrevious=0;
  static bool* sampleCurrent=0;
  static int sampleSize=-1;
  // allocate memory for buffer
  if(sampleSize != sdev->MaxBitAddress()+1) {
    sampleSize = sdev->MaxBitAddress()+1;
    if(samplePrevious!=0) delete samplePrevious;
    if(sampleCurrent!=0) delete sampleCurrent;
    samplePrevious= new bool[sampleSize];
    sampleCurrent= new bool[sampleSize];
  }
  // reset buffer
  for(int bit=0; bit<sampleSize; bit++) {
    samplePrevious[bit]=false;
    sampleCurrent[bit]=false;
  }

  // loop until time is up
  bool edges=true;
  for(int time_togo=time_all; time_togo>0; time_togo-=time_delta) {
    // read
    for(int bit=0; bit<sampleSize; bit++) {
      sampleCurrent[bit]=sdev->ReadSignal(bit);
      if(sampleCurrent[bit] != samplePrevious[bit]) edges=true;
    }
    // report
    if(edges) {
      std::cout << "% ###############################################################" << std::endl;
      for(int bit=0; bit<sampleSize; bit++) {
	std::cout<< "@"<< (bit < 10 ? "0" : "") << bit << ":" << sampleCurrent[bit];
        if(sampleCurrent[bit] != samplePrevious[bit]) 
	  std::cout << "!  ";
        else
          std::cout << "   ";
        if((bit%8)==7 || bit+1==sampleSize) std::cout << std::endl;
      }
    }
    // copy
    for(int bit=0; bit<sampleSize; bit++) {
      samplePrevious[bit]=sampleCurrent[bit];
    }
    // clear edge detection 
    edges=0;
    // sleep
    faudes_usleep(1000*time_delta);
  };

  // report performance, part2
  faudes_gettimeofday(&time_stop);
  faudes_mstime_t diffms;
  faudes_diffsystime(time_stop,time_start,&diffms);
  std::cout << "# performance: overall time: " << diffms << "ms" << std::endl;
  std::cout << "# performance: sleep time:   " << time_all << "ms" << std::endl;
  std::cout << "# performance: process time per loop:   " << 
    ((double) (diffms - time_all)) / (time_all/time_delta)<< "ms" << std::endl;
}

// write signal value
void WriteSignalValue(sDevice* sdev) {
  int bit, val;
  std::cout<<"WriteValue: enter bit address (or -1 to exit): ";
  std::cin>>bit;
  if(bit<0) return;
  std::cout<<"WriteValue: enter value (or -1 to exit): ";
  std::cin>>val;
  if(val<0) return;
  std::cout<<"WriteValue: setting output " << bit << " to value " << val << std::endl;
  sdev->WriteSignal(bit,val!=0);
}

//////////////////////////////////////////////////////////////
//Event-handling

// execute output event
void WriteOutputEvent(vDevice* dev) {
  std::cout<<"WriteOutput: enter event by name: ";	
  std::string testEvent;
  std::cin>>testEvent;
  if(!dev->Outputs().Exists(testEvent)) {
    std::cout<<"Unknown output event " << std::endl;	
    return;
  }
  faudes::Idx fev= dev->Outputs().Index(testEvent);
  dev->WriteOutput(fev);
  dev->FlushOutputs();
}


// poll input events
void PollInputEvents(vDevice* dev){
  std::cout<<"ReadInputs: time (secs) to monitor input events: ";    
  faudes_mstime_t time_all;
  std::cin>>time_all;
  time_all*=1000; // convert to msecs
  std::cout<<"ReadEvents: time (msecs) to sleep between two polls: ";    
  faudes_mstime_t  time_delta;
  std::cin>>time_delta;
  // reset all input data so far
  dev->Reset();
  // report performace, part 1
  faudes_systime_t time_start, time_stop;
  faudes_gettimeofday(&time_start);

  // loop until time is up
  for(int time_togo=time_all; time_togo>0; time_togo-=time_delta) {
    Idx sev=dev->ReadInput();
    if(sev!=0) 
      std::cout<<"ReadInputs: event " << dev->Inputs().SymbolicName(sev) << std::endl;
    faudes_usleep(1000*time_delta);
  };

  // report performance, part2
  faudes_gettimeofday(&time_stop);
  faudes_mstime_t diffms;
  faudes_diffsystime(time_stop,time_start,&diffms);
  std::cout << "# performance: overall time: " << diffms << "ms" << std::endl;
  std::cout << "# performance: sleep time:   " << time_all << "ms" << std::endl;
  std::cout << "# performance: process time per loop:   " << 
    ((double) (diffms - time_all)) / (time_all/time_delta)<< "ms" << std::endl;

}

// WaitInputEvent(vDevice* dev)
void WaitInputEvent(vDevice* dev){
  std::cout<<"Enter max. duration (in faudes-time units) to wait for a input-event to occur"<<std::endl;
  std::cout<<"Note: 1 faudes-time unit is configured to " << dev->TimeScale() << " msecs" <<std::endl;
  Time::Type duration;
  std::cin>>duration;
  EventSet occuredEvents;
  //wait for input-event to occur
  dev->WaitInputs(duration); 
  //identify occured events
  while(Idx sev=dev->ReadInput()) occuredEvents.Insert(sev); 
  //report occured events
  if(!occuredEvents.Empty()) std::cout << occuredEvents.ToString();
  else std::cout<<"No event recognized";
}

// FlushInputEvent(vDevice* dev)
void FlushInputEvents(vDevice* dev){
  //identify occured events
  EventSet occuredEvents;
  while(Idx sev=dev->ReadInput()) occuredEvents.Insert(sev); 
  //report occured events
  if(!occuredEvents.Empty()) std::cout << occuredEvents.ToString();
  else std::cout<<"No event recognized";
}


//////////////////////////////////////////////////////////////
//User-Interface loop

int main(int argc, char* argv[]) {

  // debugging autoregistration :-( 
  /*
  std::cerr << " ====== auto registration " << std::endl;
  TypeRegistry::G()->Write(); 
  std::cerr << " ====== registration " << std::endl;
  AutoRegisterType<cDevice> gRtiLocalIOReg("ComediDevice");
  TypeRegistry::G()->Write(); 
  */

  // install my signal handler
  faudes_termsignal(catch_signal);

  // first argument has to be filename
  if(argc!=2) {
    std::cerr << "iomonitor: " << VersionString()  << std::endl;
    std::cerr << "usage: iomonitor <device-file>" << std::endl;
    return -1;
  }

#ifdef FAUDES_NETWORK
#ifdef FAUDES_WINDOWS
  // initialise winsocks
  WSADATA wsaData;
  if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0) {
    std::cerr << "iomonitor: failed to initialize winsocks" << std::endl;
    return -1;
  }
#endif
#endif

  //initialize vDevice
  FD_DH("Initialize vDevice");
  std::cout << "iomonitor: instantiate device from file" << std::endl;
  vDevice* dev;
  dev=vDevice::FromFile(std::string(argv[1]));
  sDevice* sdev=dynamic_cast<sDevice*>(dev);

  //start vDevice
  std::cout << "iomonitor: starting device " << std::endl;
  dev->Start();

  //loop until device is up
  while(dev->Status()!=vDevice::Up){;}

  // loop until user terminates
  while(true) {

  // set up cheap console userinterface
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "# iomonitor commands are:" << std::endl;
  std::cout << "#" << std::endl;
  std::cout << "#   read faudes event via wait           (re)" << std::endl;
  std::cout << "#   read faudes events via polling       (pe)" << std::endl;
  std::cout << "#   flush faudes input events            (fe)" << std::endl;
  if(sdev) std::cout << "#   read signal value by bitaddress      (rs)" << std::endl;
  if(sdev) std::cout << "#   read signal values via polling       (ps)" << std::endl;
  std::cout << "#" << std::endl;
  std::cout << "#   write faudes event        	  	 (we)" << std::endl;
  if(sdev) std::cout << "#   write signal value by bitaddress     (ws)" << std::endl;
  std::cout << "#" << std::endl;
  std::cout << "#   device time                 	 (time)" << std::endl;
  std::cout << "#   reset device                 	 (reset)" << std::endl;
  std::cout << "#   list all device events          	 (list) " << std::endl;
  std::cout << "#   exit                            	 (exit) " << std::endl;
  std::cout << "#" << std::endl;
  std::cout << ">";
  // get user-choice
  std::string choice;
  std::cin >> choice;
  //execute user-choice
  if(choice=="exit") break;
  if(choice=="reset") {dev->Reset();};
  if(choice=="time") ReadTime(dev);
  if(choice=="list") ListEvents(dev); 
  if(choice=="pe") PollInputEvents(dev);
  if(choice=="re") WaitInputEvent(dev);
  if(choice=="fe") FlushInputEvents(dev);
  if(sdev && choice=="rs") ReadSignalValue(sdev);   
  if(sdev && choice=="ps") PollInputSignals(sdev);   
  if(choice=="we") WriteOutputEvent(dev);	
  if(sdev && choice=="ws") WriteSignalValue(sdev);  

  }
	
  std::cout << "# iomonitor: done " << std::endl;
  std::cout << "##########################################" << std::endl;

 
  FD_DH("Stopping vDevice");

  //stop background thread  
  dev->Stop();
	
  return 0;
}




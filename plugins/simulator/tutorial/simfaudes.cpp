/** @file simfaudes.cpp  Simple simulator application for faudes generators  

This tutorial demonstrates how to build a simulator application from
the class faudes::ProposingExecutor. When compiled with the 
IO device plugin, the faudes::DeviceExecutor is used to run a 
hardware-in-the-loop simulation.

@code
~/libfaudes/plugins/simulator/tutorial> ./simfaudes -?

simfaudes: usage: 

  simfaudes [-q][-v][-i][-bc] [-bt <nnn>][-bs <nnn>] [-l <logfile>] [-ls] [-le] [-lt] <simfile> 

where 
  <simfile>: simulation configuration file or generator file

  -q:  less console output 
  -qq: absolutely no console output 
  -v:  more console output
  -vv: even more console output
  -i: interactive mode 

  -bc: break on condition
  -bt <nnn>: break on time <nnn> 
  -bs <nnn>: break on step <nnn> 

  -l <logfile>: log to <logfile> 
  -ls: log states
  -le: log events
  -lt: log time
  -la: log all
  -t <nnn>: fifo trace buffer length <nnn> 

  -d <devfile>: use io device configured from file
  -dt <nnn>: tolerance in time synchronisation
  -dr: executer reset on device request
@endcode


You may test the simulator with the examples provided in the data
directory:

@code

~/libfaudes/plugins/simulator/tutorial>./simfaudes -bs 5 data/gausstest.sim 
% simfaudes: ========================================= current state:
<DiscreteState> "idle"         </DiscreteState>
% simfaudes: ========================================= current time:
<Time> 0 </Time>
% simfaudes: ========================================= proposed action:
<ProposedTime> 205 </ProposedTime>
<ProposedEvent> "alpha" </ProposedEvent>
% simfaudes: ========================================= execute event:
<ExecutedEvent> "alpha" </ExecutedEvent>
% simfaudes: ========================================= current state:
<DiscreteState> "busy"         </DiscreteState>
% simfaudes: ========================================= current time:
<Time> 205 </Time>
% simfaudes: ========================================= found conditions satisfied:
<SatisfiedConditions> "BusyCond"     </SatisfiedConditions>
% simfaudes: ========================================= proposed action:
<ProposedTime> 51 </ProposedTime>
% simfaudes: ========================================= current state:
<DiscreteState> "busy"         </DiscreteState>
% simfaudes: ========================================= current time:
<Time> 256 </Time>
% simfaudes: ========================================= found conditions satisfied:
<SatisfiedConditions> "BusyCond"     </SatisfiedConditions>
% simfaudes: ========================================= proposed action:
<ProposedTime> 39 </ProposedTime>
<ProposedEvent> "beta" </ProposedEvent>
% simfaudes: ========================================= execute event:
<ExecutedEvent> "beta" </ExecutedEvent>
% simfaudes: ========================================= current state:
<DiscreteState> "idle"         </DiscreteState>
% simfaudes: ========================================= current time:
<Time> 295 </Time>
% simfaudes: ========================================= found conditions satisfied:
<SatisfiedConditions> "IdleCond"     </SatisfiedConditions>
% simfaudes: ========================================= proposed action:
<ProposedTime> 191 </ProposedTime>
<ProposedEvent> "alpha" </ProposedEvent>
% simfaudes: ========================================= execute event:
<ExecutedEvent> "alpha" </ExecutedEvent>
% simfaudes: ========================================= current state:
<DiscreteState> "busy"         </DiscreteState>
% simfaudes: ========================================= current time:
<Time> 486 </Time>
% simfaudes: ========================================= found conditions satisfied:
<SatisfiedConditions> "BusyCond"     </SatisfiedConditions>
% simfaudes: ========================================= proposed action:
<ProposedTime> 51 </ProposedTime>
% simfaudes: =========================================  end simulation 

@endcode

The code is straight forward: after some command line parsing
for behavioural configuration, it reads a proposing executor from file
and loops to execute the proposed transitions. 


@ingroup Tutorials

@include simfaudes.cpp

*/


#include "libfaudes.h"
#include <signal.h>


using namespace faudes;


// global vars used in exit handler          
DeviceExecutor mExecutor;
int mConsoleOut=0;
std::string mMark="% simfaudes: ========================================= ";

// simulator clean-up on exit
void simfaudes_exit(void);

// signal handler to stop devices
void catch_termsignal(int sig) {
  // report
  std::cout << std::endl;
  std::cout << mMark << "simfaudes: signal " << faudes_strsignal(sig) << std::endl;
  // clean up
  simfaudes_exit();
  // re-install default handler
  signal(sig, SIG_DFL);
  // pass on
  raise(sig);
}

// clean-up on exit
void simfaudes_exit(void) {
  // clean up only once
  static sig_atomic_t exit_in_progress = 0;
  if(exit_in_progress>0) return;
  exit_in_progress++;
  std::cout << mMark << "simfaudes: exit handler" << std::endl;
#ifdef FAUDES_PLUGIN_IODEVICE
  // report device performance
  if(vDevice* dev=mExecutor.Devicep()) {
    dev->WritePerformance();
  }
  // stop all devices
  vDevice::StopAll();
#endif
  // report statistics
  if(mConsoleOut>=-1) {
    std::cout << mMark << " end simulation #" << mExecutor.Size() << std::endl;
    for(ProposingExecutor::ConditionIterator cit=mExecutor.ConditionsBegin();  
        cit!=mExecutor.ConditionsEnd(); cit++) {
      if(!mExecutor.Condition(*cit).Enabled()) continue;
      std::cout << mMark << "statistics for simulation condition \"" << 
         mExecutor.Conditions().SymbolicName(*cit) << "\"" << std::endl;
      mExecutor.Condition(*cit).mSamplesPeriod.Compile();
      std::cout << mExecutor.Condition(*cit).mSamplesPeriod.Str();
      mExecutor.Condition(*cit).mSamplesDuration.Compile();
      std::cout << mExecutor.Condition(*cit).mSamplesDuration.Str() << std::endl;
    }
  }
  // close log file
  mExecutor.LogClose();
  // reset incl device if such  
  mExecutor.Reset();
}

// print usage info and exit
void usage_exit(const std::string& message="") {
  if(message!="") {
    std::cout << "simfaudes: " << message << std::endl;
    std::cout << "" << std::endl;
    exit(-1);
  }
  std::cout << "simfaudes: version " << VersionString() << std::endl;
  std::cout << "" << std::endl;
  std::cout << "simfaudes: usage: " << std::endl;
  std::cout << "  simfaudes [-q][-v][-i][-bc] [-bt <nnn>][-bs <nnn>] [-l <logfile>] [-ls] [-le] [-lt] <simfile> " << std::endl;
  std::cout << "where " << std::endl;
  std::cout << "  <simfile>: simulation configuration file" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  -q:  less console output " << std::endl;
  std::cout << "  -qq: absolutely no console output " << std::endl;
  std::cout << "  -v:  more console output" << std::endl;
  std::cout << "  -vv: even more console output" << std::endl;
  std::cout << "  -i: interactive mode " << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  -bc: break on condition" << std::endl;
  std::cout << "  -bt <nnn>: break on time <nnn> " << std::endl;
  std::cout << "  -bs <nnn>: break on step <nnn> " << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  -l <logfile>: log to <logfile> " << std::endl;
  std::cout << "  -ls: log states" << std::endl;
  std::cout << "  -le: log events" << std::endl;
  std::cout << "  -lt: log time" << std::endl;
  std::cout << "  -la: log all" << std::endl;
  std::cout << "  -t <nnn>: fifo trace buffer length <nnn> " << std::endl;
#ifdef FAUDES_PLUGIN_IODEVICE
  std::cout << "" << std::endl;
  std::cout << "  -d <devfile>: use io device configured from file" << std::endl;
  std::cout << "  -dt <nnn>: tolerance in time synchronisation" << std::endl;
  std::cout << "  -dr: executer reset on device request" << std::endl;
#endif
  std::cout << "" << std::endl;
  std::cout << "" << std::endl;
  exit(-1);
}

// parse commandline, read executor and run executor
int main(int argc, char* argv[])
{

  // install my signal handler
  faudes_termsignal(catch_termsignal);

  // install my exit fnct
  atexit(simfaudes_exit);

  // default behaviour
  mConsoleOut=0;
  bool mInteractive=false;
  std::string mSimFile="";
  std::string mDevFile="";
  Time::Type mTolerance=-1;
  bool mBreakCondition=false;
  Time::Type mBreakTime=Time::Max();
  int mBreakStep=-1;
  std::string mLogFile="";
  int mLogMode=0;
  int mTraceLength=5;
  bool mResetRequest=false;

  // primitive commad line parsing
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: quiet
    if((option=="-q") || (option=="--quiet")) {
      mConsoleOut=-1;
      continue;
    }
    // option: more quiet
    if((option=="-qq") || (option=="--quietquiet")) {
      mConsoleOut=-2;
      continue;
    }
    // option: verbose
    if((option=="-v") || (option=="--verbose")) {
      mConsoleOut=1;
      continue;
    }
    // option: more verbose
    if((option=="-vv") || (option=="--verboseverbose")) {
      mConsoleOut=2;
      continue;
    }
    // option: interactive
    if((option=="-i") || (option=="--interactive")) {
      mInteractive=true;
      continue;
    }
    // option: io device
    if((option=="-d") || (option=="--device")) {
      i++; if(i>=argc) usage_exit();
      mDevFile=argv[i];
      continue;
    }
    // option: io device tolerance
    if((option=="-dt") || (option=="--tolerance")) {
      i++; if(i>=argc) usage_exit();
      mTolerance=(Time::Type) ToIdx(argv[i]);
      continue;
    }
    // option: io device reset request
    if((option=="-dr") || (option=="--resetrequest")) {
      mResetRequest=true;
      continue;
    }
    // option: break condition
    if((option=="-bc") || (option=="--breakcondition")) {
      mBreakCondition=true;
      continue;
    }
    // option: break time
    if((option=="-bt") || (option=="--breaktime")) {
      i++; if(i>=argc) usage_exit();
      mBreakTime=(Time::Type) ToIdx(argv[i]);
      continue;
    }
    // option: break step
    if((option=="-bs") || (option=="--breakstep")) {
      i++; if(i>=argc) usage_exit();
      mBreakStep=(int) ToIdx(argv[i]);
      continue;
    }
    // option: log file
    if((option=="-l") || (option=="--logfile")) {
      i++; if(i>=argc) usage_exit();
      mLogFile=argv[i];
      continue;
    }
    // option: log states
    if((option=="-ls") || (option=="--logstates")) {
      mLogMode |= LoggingExecutor::LogStates;
      continue;
    }
    // option: log events
    if((option=="-le") || (option=="--logevents")) {
      mLogMode |= LoggingExecutor::LogEvents;
      continue;
    }
    // option: log time
    if((option=="-lt") || (option=="--logtime")) {
      mLogMode |= LoggingExecutor::LogTime;
      continue;
    }
    // option: log all
    if((option=="-la") || (option=="--logall")) {
      mLogMode |= 0xff;
      continue;
    }
    // option: trace
    if((option=="-t") || (option=="--trace")) {
      i++; if(i>=argc) usage_exit();
      mTraceLength=(int) ToIdx(argv[i]);
      continue;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage_exit();
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage_exit("unknown option "+ option);
      continue;
    }
    // filename
    if(mSimFile!="")
      usage_exit("more than one filname specified" );
    mSimFile=option;
  }

  // insist in filename
  if(mSimFile=="")
      usage_exit("you must specify a filename" );

  // dont have both, interactive and sync physics
  if(mDevFile!="" && mInteractive) 
      usage_exit("you must not specify both interactive and synchrone mode");
  
  // mute libFAUDES console out
  if(mConsoleOut<0) 
    ConsoleOut::G()->Verb(0);

  try{

    // relaxed read configuration: test for generator file  
    bool gfile=false;        
    TokenReader* tr = new TokenReader(mSimFile);
    Token token;
    tr->Peek(token);
    if(token.Type()==Token::Begin)
    if(token.StringValue()=="Generator") 
      gfile=true;  
    // read configuration
    if(gfile) {
      mExecutor.Insert(mSimFile);
      mExecutor.Reset();
    } else {        
      mExecutor.Read(mSimFile);
      mExecutor.Reset();
    }

  } catch(const Exception& fe) {
    std::cout << std::flush;
    std::cerr << "simfaudes: caught [[" << fe.Message() << "]]" << std::endl;
    std::cerr << "simfaudes: presumably missing/missformed configuration file" << std::endl;
    return 1;
  }

  // report configuration
  if(mConsoleOut>=1) {
    std::cout << mMark << "dumping configuration"  << std::endl;
    // generators
    for(Idx i=0; i< mExecutor.Size(); i++) {
      std::cout << mMark << "found generator #" << i+1 << 
	": " <<  mExecutor.At(i).Generator().Name() << std::endl;
    }
    // event attributes
    for(EventSet::Iterator eit=mExecutor.Alphabet().Begin();  
        eit!=mExecutor.Alphabet().End(); eit++) {
      std::cout << mMark << "found event attributes for \"" << 
         mExecutor.EventName(*eit) << "\"" << std::endl;
      std::cout << mExecutor.Alphabet().Attribute(*eit).ToString() << std::endl;
    }
    // conditions
    for(ProposingExecutor::ConditionIterator cit=mExecutor.ConditionsBegin();  
        cit!=mExecutor.ConditionsEnd(); cit++) {
      std::cout << mMark << "found simulation condition \"" << 
         mExecutor.Conditions().SymbolicName(*cit) << "\"" << std::endl;
      std::cout << mExecutor.Condition(*cit).ToString() << std::endl;
    }
  }

  // report generators (disabled, max output level is 2)
  if(mConsoleOut>=3) {
    // generators
    for(Idx i=0; i< mExecutor.Size(); i++) {
      std::cout << mMark << "generator #" << i+1 << std::endl;
      mExecutor.At(i).Generator().DWrite();
    }
  }

  // initialze log file
  if(mLogFile!="") {
    mExecutor.LogOpen(mLogFile,mLogMode | LoggingExecutor::LogStatistics);
  }
  if(mLogFile=="" && mLogMode!=0) {
    TokenWriter* ptw= new TokenWriter(TokenWriter::Stdout);
    mExecutor.LogOpen(*ptw, mLogMode | LoggingExecutor::LogStatistics);
  }

  // set trace buffer
  mExecutor.TraceClear(mTraceLength);

  // ************************************************  synchronous prep
  if(mDevFile!="") {
#ifdef FAUDES_PLUGIN_IODEVICE

    // create device from file
    vDevice* dev;
    try {
      dev=vDevice::FromFile(mDevFile);
    } catch(const Exception& fe) {
      std::cout << std::flush;
      std::cerr << "simfaudes: [[" << fe.Message() << "]]" << std::endl;
      std::cerr << "simfaudes: presumably missing/missformed configuration file" << std::endl;
      return 1;
    }

    
#ifdef FAUDES_NETWORK
#ifdef FAUDES_WINDOWS
    // initialise winsocks
    if(mConsoleOut>=0) 
      std::cout << mMark << "Initialze network" << std::endl;
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0) {
      usage_exit("cannot start winsock (network error)");
    }
#endif
#endif

    // report
    if(mConsoleOut>=0) 
      std::cout << mMark << "Execute via IO device: \""<< dev->Name() << "\"" << std::endl;

    // set tolerance
    if(mTolerance!=-1) mExecutor.ToleranceTime(mTolerance);

    // assign device to executor and wait for startup to complete
    mExecutor.Devicep(dev);
    mExecutor.Reset(); 
    if(mBreakTime==Time::UnDef())  mBreakTime=Time::Max();
    mExecutor.DeviceStart();
    while(dev->Status()!=vDevice::Up) {
      std::cout << mMark << "Starting IO device \""<< dev->Name() << "\" Status: " << dev->StatusString() << std::endl;
      faudes_sleep(1);
    }
    dev->CurrentTime(0); // sync time; dont use reset, since we would loose events
    std::cout << mMark << "IO device \""<< dev->Name() << "\" is Up" << std::endl;
#else
    // cannot run device without plugin
    usage_exit("cannot load device \""+mDevFile+"\": device plugin not present");
#endif

  }
  

  // ************************************************* interactive loop
  std::cout << mMark << " begin simulation #" << mExecutor.Size() << std::endl;
  bool mRunning=true;
  bool mInterTemp=mInteractive;
  SimConditionSet mSatisfied;
  mSatisfied.Name("SatisfiedConditions");
  while(mRunning) {
    // report current state
    if(mConsoleOut>=2) {
      std::cout << mMark << "current state:" << std::endl;
      std::cout << mExecutor.CurrentParallelTimedState().ToString("TimedState",&mExecutor) << std::endl;
      std::cout << mMark << "marking reached:" << std::endl;
      for(Idx i=0; i<mExecutor.Size(); i++) {
	if(mExecutor.At(i).Generator().ExistsMarkedState( mExecutor.At(i).CurrentState() ))
          std::cout <<  mExecutor.At(i).Name() << ": marked" << std::endl;
      }
    }
    // report current state
    if(mConsoleOut>=0 && mConsoleOut<2) {
      std::cout << mMark << "current state:" << std::endl;
      std::cout << mExecutor.CurrentParallelTimedState().ToString("DiscreteState",&mExecutor) << std::endl;
    }  
    // report current time
    if(mConsoleOut>=1) {
      std::cout << mMark << "current time:" << std::endl;
      std::cout << "<Time> " << mExecutor.CurrentTime() << " </Time>" << std::endl;
      std::cout << "<Step> " << mExecutor.CurrentStep() << " </Step>" << std::endl;
    }  
    // report current time
    if(mConsoleOut==0) {
      std::cout << mMark << "current time:" << std::endl;
      std::cout << "<Time> " << mExecutor.CurrentTime() << " </Time>" << std::endl;
    }  
    // report satisfied conditions
    if(mConsoleOut>=0) {
      mSatisfied.Clear();
      for(ProposingExecutor::ConditionIterator cit=mExecutor.ConditionsBegin();  
          cit!=mExecutor.ConditionsEnd(); cit++) {
        if(mExecutor.Condition(*cit).Satisfied()) mSatisfied.Insert(*cit);
      }
      if(mSatisfied.Size()>0) {
        std::cout << mMark << "found conditions satisfied:" << std::endl;
        std::cout << mSatisfied.ToString() << std::endl;
      }
    }
    // report internal state
    if(mConsoleOut>=2) {
      std::cout << mMark << "simulation event states:" << std::endl;
      std::cout << mExecutor.EventStatesToString() << std::endl;
    }
    // report enables per component
    if(mConsoleOut>=2 && mExecutor.Size()>1) {
      std::cout << mMark << "disabled events (per component):" << std::endl;
      for(Idx i=0; i<mExecutor.Size(); i++) {
	std::string enevs = mExecutor.At(i).DisabledEvents().ToString();
        std::cout <<  mExecutor.At(i).Name() << ": " << enevs << std::endl;
      }
    }
    // report enabled transitions
    if(mConsoleOut>=1) {
      std::cout << mMark << "enabled events:" << std::endl;
      std::cout << mExecutor.EnabledEvents().ToString() << std::endl;
      std::cout << mMark << "enabled interval:" << std::endl;
      std::cout << mExecutor.EnabledInterval().Str() << std::endl;
      std::cout << mMark << "enabled time:" << std::endl;
      std::cout << mExecutor.EnabledTime().Str() << std::endl;
    }  
    // test break: time up
    if(mExecutor.CurrentTime() >= Time::Max()) {
      if(mConsoleOut>=-1) std::cout << mMark << "time is up" << std::endl;
      mInterTemp=false;
      mRunning=false;
      break;
    }
    // test break: condition
    if(mExecutor.BreakCondition() && (mBreakCondition || mInteractive)) {
     if(mConsoleOut>=-1) std::cout << mMark << "break condition triggered" << std::endl;
      mInterTemp=mInteractive;
      mRunning=mInteractive;
    }
    // test break: time
    if(mBreakTime!=Time::UnDef())
    if(mExecutor.CurrentTime() >= mBreakTime) {
      if(mConsoleOut>=-1) std::cout << mMark << "break time reached" << std::endl;
      mInterTemp=mInteractive;
      mRunning=mInteractive;
    }
    // test break: step
    if(mBreakStep>=0)
    if(mExecutor.CurrentStep() >= mBreakStep) {
      if(mConsoleOut>=-1) std::cout << mMark << "break step reached" << std::endl;
      mInterTemp=mInteractive;
      mRunning=mInteractive;
    }
    // test break: synchronous device
    if(!mExecutor.IsSynchronous()) {
      if(mConsoleOut>=-1) std::cout << mMark << "device out of sync" << std::endl;
      mInterTemp=false;
      mRunning=false;
      break;
    }
    // proposed action
    TimedEvent mPropTrans=mExecutor.ProposeNextTransition();
    if(mConsoleOut>=0) {
      std::cout << mMark << "proposed action:" << std::endl;
      if(mPropTrans.mTime>0)
        std::cout << "<ProposedTime> " << ToStringInteger(mPropTrans.mTime) << " </ProposedTime>" << std::endl;
      if(mPropTrans.mEvent!=0)
        std::cout << "<ProposedEvent> \"" << mExecutor.EventName(mPropTrans.mEvent)  << "\" </ProposedEvent>" << std::endl;
      if((mPropTrans.mTime<=0) && (mPropTrans.mEvent==0) )
        std::cout << "+DeadLock+" << std::endl;
    }
    // record transition
    Idx mEvent=0;
    // ask choice
    while(mInterTemp) {
       // get user input
       std::cout << mMark << "enter command:" << std::endl;
       std::string line;
       std::getline(std::cin,line);
       // separate cmd from arg
       std::string choice;
       std::string param;
       std::istringstream sline(line);
       sline >> choice;
       sline >> param;
       // convert to int
       int ichoice =-1;
       std::istringstream schoice(choice);
       schoice >> ichoice;
       if(!schoice) ichoice=-1;
       int iparam =-1;
       std::istringstream sparam(param);
       sparam >> iparam;
       if(!sparam) iparam=-1;
       // convert to symbol
       std::string nchoice=choice;
       if(choice.length()>2)
       if(choice.at(0)=='"' && choice.at(choice.length()-1)== '"')
         nchoice=choice.substr(1,choice.length()-2);
       // switch cases
       bool err=false;
       if(choice=="x" || choice == "exit") {
         mRunning=false;
       } else
       if(choice=="p" || choice=="proposal" || choice=="") {
         mExecutor.ExecuteTime(mPropTrans.mTime);
         if(mExecutor.ExecuteEvent(mPropTrans.mEvent))
           mEvent=mPropTrans.mEvent;
       } else
       if(choice=="r" || choice=="run") {
         mInterTemp=false;
       } else
       if(choice=="v" || choice=="revert") {
         int step = mExecutor.CurrentStep()-1;
         if(iparam!=-1) step=iparam;
         std::cout << mMark << "revert to step " << step << std::endl;
         mExecutor.RevertToStep(step);          
       } else
       if(choice=="t" || choice=="trace") {
         std::cout << mMark << "system trace" << std::endl;
         mExecutor.TraceWrite();
         continue;
       } else
       if(ichoice>0) {
         mExecutor.ExecuteTime(ichoice);
       } else
       if(mExecutor.Alphabet().Exists(nchoice)) {
         if(mExecutor.ExecuteEvent(mExecutor.EventIndex(nchoice)))
           mEvent=mExecutor.EventIndex(nchoice);
       } else {
         std::cout << mMark << "simfaudes interactive mode" << std::endl;
         std::cout << "%"  << std::endl;
         std::cout << "%  execute time and/or transitions"  << std::endl;
         std::cout << "%  * <nn> to pass a specified duration <nn> (excl brackets)" << std::endl;
         std::cout << "%  * \"event\" to execute an event (incl quotes)" << std::endl;
         std::cout << "%  * [P] or [Ret] to execute the recent proPosal " << std::endl;
         std::cout << "%"  << std::endl;
         std::cout << "%  show trace and revert"  << std::endl;
         std::cout << "%  * [T] to show a Trace of recent events and states" << std::endl;
         std::cout << "%  * [V] <nn> to reVert to step <nn> (obmit <nn> for one step backward) "<< std::endl;
         std::cout << "%"  << std::endl;
         std::cout << "%  other"  << std::endl;
         std::cout << "%  * [X] to eXit" << std::endl<< std::endl;
         err=true;
       }
       if(!err) break;
    }  
    // execute proposal
    if(!mInterTemp && mDevFile=="") {
       mExecutor.ExecuteTime(mPropTrans.mTime);
       if(mExecutor.ExecuteEvent(mPropTrans.mEvent))
         mEvent=mPropTrans.mEvent;
    }
#ifdef FAUDES_PLUGIN_IODEVICE
    // sync step
    if(mDevFile!="") {
      // reset request ?
      bool rr= mExecutor.DeviceResetRequest();
      if(rr && mConsoleOut>=0 && !mResetRequest) {
        std::cout << mMark << "ignoring reset request" << std::endl;
        rr=false;
      }
      if(rr && mConsoleOut>=0) 
        std::cout << mMark << "reset on request" << std::endl;
      if(rr) 
        mExecutor.Reset();
      // sync proposal
      if(!rr) {
        if(mConsoleOut>=0 && mPropTrans.mTime>0)
           std::cout << mMark << "sync wait" << std::endl;
        mEvent=mExecutor.SyncStep();
        mExecutor.SyncTime();
      }
    }
#endif
    // report event
    if(mConsoleOut>=0 && mEvent!=0) {
      std::cout << mMark << "execute event:" << std::endl;
      std::cout << "<ExecutedEvent> \"" << mExecutor.EventName(mEvent)  << "\" </ExecutedEvent>"
        << std::endl;
    }  

  } // loop: while mRunning

  // done
  return 0;
}

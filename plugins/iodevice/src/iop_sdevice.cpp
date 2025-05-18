/** @file iop_sdevice.cpp Virtual device for interface definition  */

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008, Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/



#include "iop_sdevice.h"
#include "sp_densityfnct.h"

namespace faudes {


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeSignalOutput

 **********************************************
 **********************************************
 **********************************************
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSignalOutput,AttributeVoid)

//DoAssign(Src)
void AttributeSignalOutput::DoAssign(const AttributeSignalOutput& rSrcAttr) {
  FD_DHV("AttributeSignalOutput(" << this << "):DoAssign(): assignment from " <<  &rSrcAttr);
  mActions=rSrcAttr.mActions;
}

//DoWrite(rTw);
void AttributeSignalOutput::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  
  FD_DHV("AttributeSignalOutput::DoWrite(): #" << mActions.size() );
  rTw.WriteBegin("Actions");
  std::vector<Action>::const_iterator ait;
  for(ait=mActions.begin(); ait!=mActions.end(); ait++) {
    Token atoken;
    if(ait->mValue==Set)
      atoken.SetEmpty("Set");
    if(ait->mValue==Clr)
      atoken.SetEmpty("Clr");
    if(ait->mValue==Inv)
      atoken.SetEmpty("Inv");
    atoken.InsAttributeInteger("address",ait->mBit);
    rTw << atoken;
  }
  rTw.WriteEnd("Actions");
}


//DoRead(rTr)
void AttributeSignalOutput::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  Token token;
  FD_DHV("AttributeSignalOutput::DoRead()");

  // sense and digest pre 2.16 format
  rTr.Peek(token);
  if(token.IsBegin()) 
  if((token.StringValue()=="Output") || (token.StringValue()=="Actuator")) {
    rTr.ReadBegin("Actuator");
    bool err=false;
    rTr.ReadBegin("Actions");
    while(!rTr.Eos("Actions")) {
      Action action;
      action.mBit=rTr.ReadInteger();
      std::string value=rTr.ReadOption();
      if(value== "Set") {
        action.mValue=Set;
      } else if(value == "Clr") {
        action.mValue=Clr;
      } else if(value == "Inv") {
        action.mValue=Inv;
      } else {
        err=true;
        break;
      }
      mActions.push_back(action);
    }
    rTr.ReadEnd("Actions");
    rTr.ReadEnd("Actuator");
    if(err) {
      std::stringstream errstr;
      errstr << "Invalid output event property" << rTr.FileLine();
      throw Exception("AttributeSignalOutput::Read", errstr.str(), 52);  //52 oder 352
    }
  } // end pre 2.16


  // test for actions section
  rTr.Peek(token);
  if(!token.IsBegin()) return;
  if(token.StringValue()!="Actions") return;
  // read begin
  bool err=false;
  rTr.ReadBegin("Actions");
  while(!rTr.Eos("Actions")) {
    // prepare
    Token atag;
    Action action;
    rTr.Peek(atag);
    // set
    if(atag.IsBegin())
    if(atag.StringValue()=="Set") {
      rTr.ReadBegin("Set");
      action.mValue=Set;
      action.mBit=atag.AttributeIntegerValue("address");
      mActions.push_back(action);
      rTr.ReadEnd("Set");
      continue;
    }
    // clr
    if(atag.IsBegin())
    if(atag.StringValue()=="Clr") {
      rTr.ReadBegin("Clr");
      action.mValue=Clr;
      action.mBit=atag.AttributeIntegerValue("address");
      mActions.push_back(action);
      rTr.ReadEnd("Clr");
      continue;
    }
    // inv
    if(atag.StringValue()=="Inv") {
      rTr.ReadBegin("Inv");
      action.mValue=Inv;
      action.mBit=atag.AttributeIntegerValue("address");
      mActions.push_back(action);
      rTr.ReadEnd("Inv");
      continue;
    }
    // error on unknown tokens
    err=true;
    break;
  }
  // done
  rTr.ReadEnd("Actions");
  // report error
  if(err) {
    std::stringstream errstr;
    errstr << "Invalid output event property" << rTr.FileLine();
    throw Exception("AttributeSignalOutput::Read", errstr.str(), 52);  //52 oder 352
  }

}//DoRead


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeSignalInput

 **********************************************
 **********************************************
 **********************************************
 */


// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSignalInput,AttributeVoid)


//DoAssign(Src)
void AttributeSignalInput::DoAssign(const AttributeSignalInput& rSrcAttr) {
  FD_DHV("AttributeSignalInput(" << this << "):DoAssign(): assignment from " <<  &rSrcAttr);
  mTriggers=rSrcAttr.mTriggers;
}


//DoWrite(rTw,rLabel,pContext)
void AttributeSignalInput::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  FD_DHV("AttributeSignalInput()::DoWrite()");
  rTw.WriteBegin("Triggers");
  std::vector<Trigger>::const_iterator ait;
  for(ait=mTriggers.begin(); ait!=mTriggers.end(); ait++) {
    Token atoken;
    if(ait->mPos &&  (!ait->mNeg)) {
      atoken.SetEmpty("PositiveEdge");
    }
    if((!ait->mPos) &&  ait->mNeg) {
      atoken.SetEmpty("NegativeEdge");
    }
    if((ait->mPos) &&  ait->mNeg) {
      atoken.SetEmpty("AnyyEdge");
      atoken.InsAttributeString("edge","any");
    }
    atoken.InsAttributeInteger("address",ait->mBit);
    rTw << atoken;

  }
  rTw.WriteEnd("Triggers");
}//DoWrite(rTw,rLabel,pContext)


//DoRead(rTr,rLabel,pContext)
void AttributeSignalInput::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  Token token;
  FD_DHV("AttributeSignalInput()::DoRead(tr)");


  // sense and digest pre 2.16 format
  rTr.Peek(token);
  if(token.IsBegin())
  if((token.StringValue()=="Input") || (token.StringValue()=="Sensor")) {
    rTr.ReadBegin("Sensor");
    bool err=false;
    rTr.ReadBegin("Triggers");
    while(!rTr.Eos("Triggers")) {
      Trigger trigger;
      trigger.mBit=rTr.ReadInteger();
      std::string value=rTr.ReadOption();
      if (value == "PosEdge") {
        trigger.mPos=true;
        trigger.mNeg=false;
      } else if (value == "NegEdge") {
        trigger.mPos=false;
        trigger.mNeg=true;
      } else if (value == "AnyEdge") {
        trigger.mPos=true;
        trigger.mNeg=true;
      } else {
        err=true;
        break;
      }
      mTriggers.push_back(trigger);
    }
    rTr.ReadEnd("Triggers");
    rTr.ReadEnd("Sensor");
    if(err) {
      std::stringstream errstr;
      errstr << "invalid input event property" << rTr.FileLine();
      throw Exception("AttributeSignalInput::Read", errstr.str(), 52);
    }
  } // end pre2.16
 
  // test for triggers section
  rTr.Peek(token);
  if(!token.IsBegin()) return;
  if(token.StringValue()!="Triggers") return;
  // read begin
  bool err=false;
  rTr.ReadBegin("Triggers");
  while(!rTr.Eos("Triggers")) {
    // prepare
    Token atag;
    Trigger trigger;
    rTr.Peek(atag);
    // positive edge
    if(atag.IsBegin())
    if(atag.StringValue()=="PositiveEdge") {
      rTr.ReadBegin("PositiveEdge");
      trigger.mPos=true;
      trigger.mNeg=false;
      trigger.mBit=atag.AttributeIntegerValue("address");
      mTriggers.push_back(trigger);
      rTr.ReadEnd("PositiveEdge");
      continue;
    }
    // negative edge
    if(atag.IsBegin())
    if(atag.StringValue()=="NegativeEdge") {
      rTr.ReadBegin("NegativeEdge");
      trigger.mPos=false;
      trigger.mNeg=true;
      trigger.mBit=atag.AttributeIntegerValue("address");
      mTriggers.push_back(trigger);
      rTr.ReadEnd("NegativeEdge");
      continue;
    }
    // any edge
    if(atag.IsBegin())
    if(atag.StringValue()=="AnyEdge") {
      rTr.ReadBegin("AnyEdge");
      trigger.mPos=true;
      trigger.mNeg=true;
      trigger.mBit=atag.AttributeIntegerValue("address");
      mTriggers.push_back(trigger);
      rTr.ReadEnd("AnyEdge");
      continue;
    }
    // error on unknown tokens
    err=true;
    break;
  }
  // done
  rTr.ReadEnd("Triggers");
  // report error
  if(err) {
    std::stringstream errstr;
    errstr << "Invalid input event property" << rTr.FileLine();
    throw Exception("AttributeSignalInput::Read", errstr.str(), 52);  //52 oder 352
  }
}



/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeSignalEvent

 **********************************************
 **********************************************
 **********************************************
 */



// std faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSignalEvent,AttributeDeviceEvent)

// Default constructor, set my prototypes
AttributeSignalEvent::AttributeSignalEvent(void) : AttributeDeviceEvent() {
  FD_DHV("AttributeSignalEvent::AttributeSignalEvent(" << this << ")");
  pOutputPrototype=OutputPrototypep();
  pInputPrototype=InputPrototypep();
}

// Copy constructor 
AttributeSignalEvent::AttributeSignalEvent(const AttributeSignalEvent& rOtherAttr) : 
  AttributeDeviceEvent() 
{
  FD_DHV("AttributeSimplenetEvent(" << this << "): form other attr " <<  &rOtherAttr);
  pOutputPrototype=OutputPrototypep();
  pInputPrototype=InputPrototypep();
  DoAssign(rOtherAttr);
}



// autoregister event confiration (required for XML token format)
AutoRegisterType< TaNameSet<AttributeSignalEvent> > 
  gRti1RegisterSignalDeviceEventSet("SignalDeviceEventSet");
AutoRegisterElementTag< TaNameSet<AttributeSignalEvent> > 
  gRti1ElementTagSignalDeviceEventSet("SignalDeviceEventSet", "Event");

// pseudo statics
const AttributeSignalOutput* AttributeSignalEvent::OutputPrototypep(void){
  AttributeSignalOutput* attrp= new AttributeSignalOutput();
  return attrp;
}

// pseudo statics
const AttributeSignalInput* AttributeSignalEvent::InputPrototypep(void) {
  AttributeSignalInput* attrp= new AttributeSignalInput();
  return attrp;
}

/*
 **********************************************
 **********************************************
 **********************************************

 implementation: sDevice

 **********************************************
 **********************************************
 **********************************************
 */


////////////////////////////////////////////////
// construction and destruction

//sDevice()
sDevice::sDevice(void) : vDevice() {
  // constructor

  FD_DHV("sDevice(" << mName << ")::sDevice()");
  // have event set with appropriate attributes
  mpConfiguration = new TaNameSet<AttributeSignalEvent>;
  pConfiguration = dynamic_cast< TaNameSet<AttributeSignalEvent>* >(mpConfiguration);
  // have appropriate default label for token io
  mDefaultLabel ="SignalDevice";
  // my signal data
  mMaxBitAddress=-1;
  mpInputEdges=0;
  mpRecentInputEdges=0;
  mpOutputLevels=0;
  // background thread:
  // install mutex
  faudes_mutex_init(&mMutex);
  // configuration defaults
  mName="SignalDevice";
  mCycleTime=1000; // 1ms
  mTimeScale=500;  // 500ms is one faudes time unit
  mSyncWrite=false;
}


//~sDevice()
sDevice::~sDevice(void) {
  // destructor
  FD_DHV("sDevice(" << mName << ")::~sDevice()");
  // cancel background thread: stop
  Stop();
  // free mem
  faudes_mutex_destroy(&mMutex);
  // free mem
  if(mpInputEdges) delete mpInputEdges;
  if(mpRecentInputEdges) delete mpRecentInputEdges;
  if(mpOutputLevels) delete mpOutputLevels;
}


//DoWritePreface(rTw,rLabel,pContext)
void sDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel,  const Type* pContext) const {
  
  FD_DHV("sDevice("<<mName<<")::DoWritePreface()");
  //call base
  vDevice::DoWritePreface(rTw,rLabel,pContext);
  //write cycle-time of background task
  Token ttag;
  ttag.SetEmpty("SampleInterval");
  ttag.InsAttributeInteger("value",mCycleTime);
  rTw<<ttag;
  // write sync option
  if(mSyncWrite) {
    Token stag;
    stag.SetEmpty("SynchronousWrite");
    stag.InsAttributeString("value","true");
    rTw<<stag;
  }
}

//DoReadPreface(rTr,rLabel,pContext)
void sDevice::DoReadPreface(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  
  FD_DHV("sDevice("<<mName<<")::DoReadPreface()");
  // call base
  vDevice::DoReadPreface(rTr,rLabel,pContext);

  // sense and digest pre 2.16 format
  Token token;
  rTr.Peek(token);
  if(token.IsInteger()) {
    mMaxBitAddress = rTr.ReadInteger();
    mCycleTime = rTr.ReadInteger();
    Token token;
    rTr.Peek(token);
    if(token.IsOption())
    if(token.OptionValue()=="SyncWrite") {
      rTr.Get(token);
      mSyncWrite=true;
    }
    return;
  }
  
  // loop my members
  while(true) {
    Token token;
    rTr.Peek(token);
    // cycle time
    if(token.IsBegin()) 
    if(token.StringValue()=="SampleInterval") {
      rTr.ReadBegin("SampleInterval", token);
      mCycleTime=token.AttributeIntegerValue("value");
      rTr.ReadEnd("SampleInterval");
      continue;
    }
    // sync write
    if(token.IsBegin()) 
    if(token.StringValue()=="SynchronousWrite") {
      rTr.ReadBegin("SynchronousWrite");
      rTr.ReadEnd("SynchronousWrite");
      mSyncWrite=true;
      if(token.ExistsAttributeString("value"))
        if(token.AttributeStringValue("value")!="true")
          mSyncWrite=false;
      continue;
    }
    // break on unknown
    break;
  }

  // done
}
 

// lock - unlock shortcuts;
#define LOCK_E  {int rc = faudes_mutex_lock(&mMutex); \
  if(rc) {FD_ERR("sDevice("<<mName<<")::LOCK_E: lock mutex error\n"); exit(1); }}
#define UNLOCK_E  {int rc = faudes_mutex_unlock(&mMutex); \
  if(rc) {FD_ERR("sDevice("<<mName<<")::LOCK_E: unlock mutex error\n"); exit(1); }}


#define TLOCK_E  {int rc = faudes_mutex_lock(&sdevice->mMutex); \
  if(rc) {FD_ERR("sDevice::TLOCK_E: lock mutex error\n"); exit(1); }}

#define TUNLOCK_E  {int rc = faudes_mutex_unlock(&sdevice->mMutex); \
  if(rc) {FD_ERR("sDevice::TLOCK_E: unlock mutex error\n"); exit(1); }}





// Start(void)
void sDevice::Start(void) {
  // bail out if device was allready started
  if( (mState == Up) || (mState == StartUp) )  return;
  FD_DHV("sDevice("<<mName<<")::Start()");
  // call base incl virtual reset, state=up
  vDevice::Start();
  // create background for edge detection and sync write
  if(!Inputs().Empty() || mSyncWrite){
    mState=StartUp;
    mCancelRequest = false;
    // create and run thread
    int rc  = faudes_thread_create(&mThreadSynchro, SDeviceSynchro, this);
    if(rc) {
      FD_ERR("sDevice("<<mName<<")::Start(): cannot creat thread\n");
      exit(1);
    }
  }
}

// Stop()
void sDevice::Stop(void) {
  // stop implies reset
  Reset();
  // bail out if device allready got the command to shut down
  if(mState != Up && mState !=StartUp)  return;
  FD_DH("sDevice("<<mName<<")::Stop()");
  // dont kill thread if no thread was created
  if(!Inputs().Empty() || mSyncWrite) {
    // cancle thread
    LOCK_E;
    FD_DHV("sDevice("<<mName<<")::Stop(): Lock passed, waiting to join thread");
    mCancelRequest=true;
    UNLOCK_E;
    int pc = faudes_thread_join(mThreadSynchro,NULL);
    if(pc) {
      FD_ERR("sDevice("<<mName<<")::Stop(): cannot join  thread??\n");
    }
  }
  // indicate success
  FD_DHV("sDevice("<<mName<<")::Stop(): done");
  mState=Down;
}


//Reset()
void sDevice::Reset(){
  // delete dynamic data
  FD_DHV("sDevice("<<mName<<")::Reset()");
  // call base (resets time)
  vDevice::Reset();
  //delete dynamic data
  FD_DHV("sDevice("<<mName<<")::Reset(): reset signals");
  ClrInputSignals();
  ClrOutputSignals();
  // done
  FD_DHV("sDevice("<<mName<<")::Reset(): done");
}

// Clear()
void sDevice::Clear(void) {
  //delete static data
  FD_DHV("sDevice(" << mName << ")::Clear()");
  // call base; note: clear implies stop
  vDevice::Clear();
  // clear compiled data
  mInputPosEdgeIndexMap.clear();
  mInputNegEdgeIndexMap.clear();
  mMaxBitAddress=-1;
  mSyncWrite=false;
  mCycleTime=1000; // usec
}

//Compile(void)
void sDevice::Compile(void){
  //setup up internal data structure
  FD_DHV("sDevice(" << mName << ")::Compile()");
  // call base
  vDevice::Compile();
  // clear my internal data
  mInputPosEdgeIndexMap.clear();
  mInputNegEdgeIndexMap.clear();
  mOutputLevelIndexMap.clear();
  mMaxBitAddress=-1;
  // setup my internal data
  for(EventSet::Iterator eit=pConfiguration->Begin(); eit!=pConfiguration->End(); eit++) {
    AttributeSignalEvent attr = pConfiguration->Attribute(*eit);
    if(attr.IsInput()) {
      // fill in reverse maps
      std::vector<AttributeSignalInput::Trigger>::const_iterator ait;
      const AttributeSignalInput* sattr = attr.Inputp();
      for(ait=sattr->mTriggers.begin(); ait!=sattr->mTriggers.end(); ait++) {
        if(ait->mPos) mInputPosEdgeIndexMap[ait->mBit].Insert(*eit);
        if(ait->mNeg) mInputNegEdgeIndexMap[ait->mBit].Insert(*eit);
        if(ait->mBit > mMaxBitAddress) mMaxBitAddress=ait->mBit;
      }
    }
    // check max address, set inverse map
    if(attr.IsOutput()) {
      std::vector<AttributeSignalOutput::Action>::const_iterator ait;
      const AttributeSignalOutput* aattr = attr.Outputp();
      for(ait=aattr->mActions.begin(); ait!=aattr->mActions.end(); ait++) {
        if(ait->mBit > mMaxBitAddress) mMaxBitAddress=ait->mBit;
        mOutputLevelIndexMap[ait->mBit].Insert(*eit);
        if(ait->mValue == AttributeSignalOutput::Inv && !mSyncWrite) {
          throw Exception("sDevice::Compile(): ", 
           "Inv-Action is only supported with SyncWrite", 52);
        }
      }
    }
  }
  // prep state data
  if(mpInputEdges) delete mpInputEdges;
  if(mpRecentInputEdges) delete mpRecentInputEdges;
  if(mpOutputLevels) delete mpOutputLevels;
  mpInputEdges = new Edges[mMaxBitAddress+1];
  mpRecentInputEdges = new Edges[mMaxBitAddress+1];
  mpOutputLevels = new Levels[mMaxBitAddress+1];
  // report
#ifdef FAUDES_DEBUG_IODEVICE
  for(int i=0; i<=mMaxBitAddress; i++) {
    if(mOutputLevelIndexMap[i].Empty()) continue;
    FD_DH("sDevice(" << mName << ")::Compile(): output signal at address " << i);
  }
#endif
}


// programmatic config: insert input event
void sDevice::InsInputEvent(const std::string& event) {
  if(mState!=Down) return;
  AttributeSignalEvent inp;
  inp.DefaultInput();
  Idx ev=pConfiguration->Insert(event);
  pConfiguration->Attribute(ev, inp);
}


// programmatic config: add trigger
void sDevice::AppendTrigger(const std::string& event, const Trigger& trigger) {
  if(mState!=Down) return;
  if(!pConfiguration->Exists(event)) return;
  Idx ev=pConfiguration->Index(event);
  AttributeSignalEvent* attr = pConfiguration->Attributep(ev);
  if(!attr->IsInput()) return;
  AttributeSignalInput* iattr = attr->Inputp();
  iattr->mTriggers.push_back(trigger);
}

// programmatic config: insert output event
void sDevice::InsOutputEvent(const std::string& event) {
  if(mState!=Down) return;
  AttributeSignalEvent outp;
  outp.DefaultOutput();
  Idx ev=pConfiguration->Insert(event);
  pConfiguration->Attribute(ev, outp);
}


// programmatic config: add action
void sDevice::AppendAction(const std::string& event, const Action& action) {
  if(mState!=Down) return;
  if(!pConfiguration->Exists(event)) return;
  if(!pConfiguration->Exists(event)) return;
  Idx ev=pConfiguration->Index(event);
  AttributeSignalEvent* attr = pConfiguration->Attributep(ev);
  if(!attr->IsOutput()) return;
  AttributeSignalOutput* oattr = attr->Outputp();
  oattr->mActions.push_back(action);
}


//CycleTime(void)
int sDevice::CycleTime(void) const {
  //Report current cycle time
  FD_DHV("sDevice(" << mName << ")::CycleTime()");
  return mCycleTime;
}

//CycleTime(int)
void sDevice::CycleTime(int cycleTime) {
  //Set cycle time
  FD_DHV("sDevice(" << mName << ")::CycleTime(int)");
  if( mState==Up || mState==StartUp){
    std::stringstream errstr;
    errstr << "Changing cycle-time not possible while background thread is still running ";
    throw Exception("sDevice::CycleTime: ", errstr.str(), 100);
  }
  mCycleTime = cycleTime;
}


//ReadSignal(int)
bool sDevice::ReadSignal(int bit){
  FD_DHV("sDevice(" << mName << ")::ReadSignal(int)");
  // make sure device is able to read signals
  if(Inputs().Empty()){
    std::stringstream errstr;
    errstr << "(" << mName << ") is not able to read signals";
    throw Exception("sDevice::ReadSignal():", errstr.str(), 552);  
  }
  //return if device is running
  if(mState!=Up && mState!=StartUp) {
    std::stringstream errstr;
    errstr << "Signal access not possible while background thread is not running ";
    throw Exception("sDevice::ReadSignal: ", errstr.str(), 552);
  }
  // lock global image, prevent background thread from reading/writing
  LOCK_E;
  // run with hooks
  bool res=false;
  if(DoReadSignalsPre()) {
    res=DoReadSignal(bit);
    DoReadSignalsPost();
  }
  // free global image
  UNLOCK_E;
  return res;
}

//WriteSignal(int,int)
void sDevice::WriteSignal(int bit, bool value){
  // write one actor value, adressed by bit number (0 to 63);
  FD_DHV("sDevice(" << mName << ")::WriteSignal(int)");
  // make sure device supports outputs 
  if(Outputs().Empty()){
    std::stringstream errstr;
    errstr << "(" << mName << ") is not configured for outputs";
    throw Exception("sDevice::WriteSignal():", errstr.str(), 552);  
  }
  // bail out if device is running
  if(mState!=Up && mState!=StartUp) {
    std::stringstream errstr;
    errstr << "Signal access not possible while background thread is not running ";
    throw Exception("sDevice::ReadSignal: ", errstr.str(), 552);
  }
  // lock global image, prevent background thread from reading/writing
  LOCK_E;
  // case a) asynchronous write
  if(!mSyncWrite) {
    if(DoWriteSignalsPre()) {
      DoWriteSignal(bit,value);
      DoWriteSignalsPost();
    }
  }
  // case b) synchronous write
  if(mSyncWrite) {
    // record next value
    Levels* level= &mpOutputLevels[bit];
    level->next=value;
    if(level->edge) level->lost=true;
    level->edge=true;
  }
  // free global image
  UNLOCK_E;
}


//SDeviceSynchro(void*)
void* SDeviceSynchro(void* arg){

  //background-thread main-routine

  // cast this object
  sDevice* sdevice = static_cast<sDevice*>(arg);

  FD_DH("sDevice(" << sdevice->mName << ")::Synchro(" << sdevice << "): with ct " << sdevice->mCycleTime);

#ifdef FAUDES_DEBUG_IOTIMING_X
	/////////////////////////////////////////////////////
	//  cyletime analysis
	//  timeA: time consumed for an entire cycle
	//  timeB: time consumed for edge detection and i/o
	//  timeC: time left for usleep
        //
	//  provide arrays for samples
	faudes_systime_t* timeA = new faudes_systime_t[MAX_SAMPLES];
	faudes_systime_t* timeB = new faudes_systime_t[MAX_SAMPLES];
	faudes_systime_t* timeC = new faudes_systime_t[MAX_SAMPLES];
        //  provide pointer to current sample
	int itime=0;
	//  take a sample
	faudes_gettimeofday(timeA+itime);
#endif

	bool running=true;

	while(running){

          // loop timer
  	  faudes_systime_t timeL1;
	  faudes_gettimeofday(&timeL1);

	  // lock global variables
	  TLOCK_E;

          // call hook
          sdevice->DoLoopCallback();

          // start up: initialize dynamic state
          if(sdevice->mState==sDevice::StartUp) {
            // reset outputs (clear image)
            bool aready= sdevice->DoWriteSignalsPre();
   	    if(aready) {
              // output image: set to zero
	      FD_DHV("sDevice("<<sdevice->Name()<<")::synchro: reset: clear lines");
              for(int bit=0; bit<=sdevice->mMaxBitAddress; bit++) {
                sDevice::Levels* level= &sdevice->mpOutputLevels[bit];
                level->current=false;
                level->next=false;
                level->edge=false;
                level->lost=false;
                level->rel = (!sdevice->mOutputLevelIndexMap[bit].Empty());
		if(level->rel) sdevice->DoWriteSignal(bit,false);
	      }
              sdevice->DoWriteSignalsPost();
            }
            // reset inputs (edge detecteion)
            bool sready= sdevice->DoReadSignalsPre();
   	    if(sready) {
              // edge detection: use actual signal levels
	      FD_DHV("sDevice("<<sdevice->Name()<<")::synchro: reset: edge detection");
              for(int bit = 0;bit<=sdevice->mMaxBitAddress;bit++) {
                sDevice::Edges* edge= &sdevice->mpInputEdges[bit];
                edge->current=sdevice->DoReadSignal(bit);
                edge->past = edge->current;
                edge->pos =    false;
                edge->neg =    false;
                edge->posrel = (!sdevice->mInputPosEdgeIndexMap[bit].Empty());
                edge->negrel = (!sdevice->mInputNegEdgeIndexMap[bit].Empty());
                edge->lost =    false;
              }
              sdevice->DoReadSignalsPost();
              for(int bit = 0;bit<=sdevice->mMaxBitAddress;bit++) {
                sDevice::Edges* redge= &sdevice->mpRecentInputEdges[bit];
                redge->pos =    false;
                redge->neg =    false;
                redge->posrel = false;
                redge->negrel = false;
                redge->lost =   false;
              }
	    }
            // new state
            if(sready && aready) 
              sdevice->mState=sDevice::Up;   	    
	  }

          // normal operation, inputs
          if(sdevice->mState==sDevice::Up) {
            bool ready=sdevice->DoReadSignalsPre();
            if(!ready) 
              sdevice->mState=sDevice::StartUp;   
            if(ready) {
  	      // edge detection, accumulative
	      for(int bit = 0; bit<=sdevice->mMaxBitAddress; bit++) {
	        // define pointer to Inputedges
	        sDevice::Edges* edge= &sdevice->mpInputEdges[bit];
	        // pass edge-info about edge in last cycle
	        edge->past = edge->current;
	        // and get corresponding input-value
	        edge->current = sdevice->DoReadSignal(bit);
	        if(edge->posrel)
		if(edge->current && (!edge->past) ) {
		  FD_DHV("sDevice::synchro: sensed positive edge at bit address "  << bit);
		  edge->pos = true;
		  // queue events to buffer
		  faudes_mutex_lock(sdevice->pBufferMutex);
		  EventSet::Iterator eit=sdevice->mInputPosEdgeIndexMap[bit].Begin();
		  EventSet::Iterator eit_end=sdevice->mInputPosEdgeIndexMap[bit].End();
		  for(; eit!=eit_end; eit++) {
		    sdevice->pInputBuffer->push_back(*eit);
		  }
		  faudes_mutex_unlock(sdevice->pBufferMutex);
		  //send signal to function "WaitSenosrs()"
		  FD_DHV("sDevice::synchro: send signal " );
		  faudes_cond_broadcast(sdevice->pWaitCondition);
		}
	        if(edge->negrel)
		if( (!edge->current) && edge->past ) {
		  FD_DHV("sDevice::synchro: sensed negative edge at bit address "  << bit);
		  edge->neg = true;
		  sdevice->mInputReady = true;
		  // queue events to buffer
		  faudes_mutex_lock(sdevice->pBufferMutex);
		  EventSet::Iterator eit=sdevice->mInputNegEdgeIndexMap[bit].Begin();
		  EventSet::Iterator eit_end=sdevice->mInputNegEdgeIndexMap[bit].End();
		  for(; eit!=eit_end; eit++) {
		    sdevice->pInputBuffer->push_back(*eit);
		  }
		  faudes_mutex_unlock(sdevice->pBufferMutex);
		  //send signal to function "WaitSenosrs"
		  FD_DHV("sDevice::synchro: send signal " );
		  faudes_cond_broadcast(sdevice->pWaitCondition);
		}
	      } // loop bits
              sdevice->DoReadSignalsPost();
	    } // end-if device ok
	  } // end-if inputs
 
 
          // normal operation, outputs
          if(sdevice->mState==sDevice::Up && sdevice->mSyncWrite) {
            bool ready=sdevice->DoWriteSignalsPre();
            if(!ready) 
              sdevice->mState=sDevice::StartUp;   
            if(ready) {
  	      // write values from image
	      for(int bit = 0; bit<=sdevice->mMaxBitAddress; bit++) {
	        // define pointer to levels
	        sDevice::Levels* level= &sdevice->mpOutputLevels[bit];
	        // not written by libfaudes
	        if(!level->edge) continue;
	        // write and record
                sdevice->DoWriteSignal(bit,level->next);
	        level->current = level->next;
	        level->edge = false;
	        level->lost = false;
	      }
              sdevice->DoWriteSignalsPost();
	    }
	  }
 
	  // check for cancel request
	  if(sdevice->mCancelRequest) { 
            running=false;
            sdevice->mState=vDevice::ShutDown; 
	  }
	  // unlock global variables
	  TUNLOCK_E;

#ifdef FAUDES_DEBUG_IOTIMING_X
	  //////////////////////////////////////
	  //cycletime analysis: take a sample
	  if(itime < MAX_SAMPLES) faudes_gettimeofday(timeB+itime);
#endif

	  // let time pass  
	  faudes_systime_t timeL2;
	  faudes_gettimeofday(&timeL2);
          long int delta = (timeL2.tv_nsec - timeL1.tv_nsec)/1000; 
          delta+= 1000000 * (timeL2.tv_sec - timeL1.tv_sec);
          if(delta < sdevice->mCycleTime) faudes_usleep(sdevice->mCycleTime-delta);
          else {
	    if(delta > sdevice->mCycleTime) 
            if(sdevice->mState==sDevice::Up)
	      FD_DH("sDevice::synchro: missed cycle time by "  <<  delta - sdevice->mCycleTime << " usec");
	  }


#ifdef FAUDES_DEBUG_IOTIMING_X
	  //////////////////////////////////////
	  // cycletime analysis

	  // take a sample
	  if(itime < MAX_SAMPLES) faudes_gettimeofday(timeC+itime);
          // increment pointer
	  itime++;
	  if(itime < MAX_SAMPLES)  faudes_gettimeofday(timeA+itime);

#endif
	  // count cycles
	  sdevice->mCycleCount++;
	} // loop while running


#ifdef FAUDES_DEBUG_IOTIMING_X

	/////////////////////////////////////////////
	// cycletime analysis
	// set up statsitics
      	FD_DHT("sDevice::synchro: performance analysis");

	SampledDensityFunction SamplesA;
	SamplesA.Clear();
        SamplesA.Dim(100);
        SamplesA.Name("time stamp AA: overall period");

	SampledDensityFunction SamplesB;
        SamplesB.Clear();
	SamplesB.Dim(100);
        SamplesB.Name("time stamp AB: process time");

	SampledDensityFunction SamplesC;
	SamplesC.Clear();
        SamplesC.Dim(100);
        SamplesC.Name("time stamp CB: sleep time");

	SampledDensityFunction SamplesWS;
	SamplesWS.Clear();
	SamplesWS.Dim(100);
	SamplesWS.Name("time passed till the next call of WaitInputs");

	// timeval-structures needed for further computations
	faudes_systime_t dAA,dBA,dCB,dER;

	// perform analysis
	// time for a hole cycle
	for(int ind = 0; ind < itime-2; ind++){
	  // compute time needed
	  // for a hole cycle
	  faudes_diffsystime(timeA[ind+1],timeA[ind],&dAA);
	  // for computing input-state
	  faudes_diffsystime(timeB[ind],timeA[ind],&dBA);
	  // by usleep(mCyleTime)
	  faudes_diffsystime(timeC[ind],timeB[ind],&dCB);
	  // time passed by until the next call of WaitInputs
	  faudes_diffsystime(sdevice->mptimeBegin[ind+1],sdevice->mptimeEnd[ind],&dER);
	  // insert samples to density functions
	  SamplesA.Sample(dAA.tv_sec*1000000 + dAA.tv_nsec/1000);
	  SamplesB.Sample(dBA.tv_sec*1000000 + dBA.tv_nsec/1000);
	  SamplesC.Sample(dCB.tv_sec*1000000 + dCB.tv_nsec/1000);
	  SamplesWS.Sample(1000000*dER.tv_sec + dER.tv_nsec/1000);
	}

	// perform statistic computation
        SamplesA.Compile();
        SamplesB.Compile();
	SamplesC.Compile();
	SamplesWS.Compile();

	// token output
	// SamplesA.Write();
	// SamplesB.Write();
	// SamplesC.Write();
	// SamplesWS.Write();

        // pretty print output
        std::cout << SamplesA.Str() << std::endl;
        std::cout << SamplesB.Str() << std::endl;
        std::cout << SamplesC.Str() << std::endl;
	std::cout << SamplesWS.Str() << std::endl;
#endif

  FD_DH("sDevice(" << sdevice->mName << ")::synchro: terminate background thread");
  faudes_thread_exit(NULL);
  // never happens, prevent gcc warning
  return NULL; 
}

// Write Output(Idx)
void sDevice::WriteOutput(Idx output) {

  FD_DHV("sDevice("<<mName<<")::WriteOutput(" << output <<")");
  //return if device is running
  if(mState!=Up && mState!=StartUp) return;
  //make sure event is part of devices eventset
  if(!mOutputs.Exists(output)) {
    std::stringstream errstr;
    errstr << "Unknown output event " << output;
    throw Exception("sDevice::WriteOutput", errstr.str(), 65);
  }
  // find properties
  const AttributeSignalOutput* aattr = pConfiguration->Attribute(output).Outputp();
  if(!aattr) {
    std::stringstream errstr;
    errstr << "Invalid output attribute " << output;
    throw Exception("sDevice::WriteOutput", errstr.str(), 65);
  }
  //debug-flag
  FD_DHV("sDevice("<<mName<<")::WriteOutput: properties found");
  // execute command ...
  std::vector<AttributeSignalOutput::Action>::const_iterator eit;
  // ... case a) asynchronous write (does not supper invert)
  if(!mSyncWrite) {
    // prepare
    if(DoWriteSignalsPre()) {
      // loop actions
      for(eit=aattr->mActions.begin(); eit!=aattr->mActions.end(); eit++){
        FD_DHV("sDevice("<<mName<<")::WriteOutput: mBit "<<eit->mBit<< " mValue "<<eit->mValue);
        DoWriteSignal(eit->mBit,eit->mValue == AttributeSignalOutput::Set);
      }
      DoWriteSignalsPost();
    }
  }
  // ... case b) synchronous write (does support invert)
  if(mSyncWrite) {
    // lock global image
    LOCK_E;
    // loop actions and record
    for(eit=aattr->mActions.begin(); eit!=aattr->mActions.end(); eit++){
      FD_DHV("sDevice("<<mName<<")::WriteOutput: mBit "<<eit->mBit<< " mValue "<<eit->mValue);
      Levels* level= &mpOutputLevels[eit->mBit];
      switch(eit->mValue) {
      case AttributeSignalOutput::Set:
        level->next=1; break;
      case AttributeSignalOutput::Clr:
        level->next=0;  break;
      case AttributeSignalOutput::Inv:
        level->next= !level->next; break;
      }
      if(level->edge) level->lost=true;
      level->edge=true;
    }
    // free global image
    UNLOCK_E;
  }
  // done
  FD_DHV("sDevice("<<mName<<")::WriteOutput: done");
}

//ClrInputSignals()
void sDevice::ClrInputSignals(void) {
  FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): MaxBitAddress: " << mMaxBitAddress);
  //return if device is not running or device is not able to read inputs
  if( mState!=Up || Inputs().Empty()) return;
  // lock global variables
  FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): lock mutex ");
  LOCK_E;
  FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): edge detection ");
  // initialise edge detection: assume low signals
  for(int bit = 0;bit<=mMaxBitAddress;bit++) {
    Edges* edge= &mpInputEdges[bit];
    FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): edge detetion ReadSignal bit "  << bit);
    edge->current=0;
    edge->past = 0;
    edge->pos =    false;
    edge->neg =    false;
    edge->posrel = (!mInputPosEdgeIndexMap[bit].Empty());
    edge->negrel = (!mInputNegEdgeIndexMap[bit].Empty());
    edge->lost =    false;
  }
  // initialise edge detection: actual signal levels
  if(DoReadSignalsPre()) {
    for(int bit = 0;bit<=mMaxBitAddress;bit++) {
      Edges* edge= &mpInputEdges[bit];
      FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): edge detetion ReadSignal bit "  << bit);
      edge->current=DoReadSignal(bit);
      edge->past = edge->current;
      edge->pos =    false;
      edge->neg =    false;
      edge->posrel = (!mInputPosEdgeIndexMap[bit].Empty());
      edge->negrel = (!mInputNegEdgeIndexMap[bit].Empty());
      edge->lost =    false;
    }
    DoReadSignalsPost();
  }
  // initialize edge detection: recent edges
  FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): initialize recent edges ");
  for(int bit = 0;bit<=mMaxBitAddress;bit++) {
    Edges* redge= &mpRecentInputEdges[bit];
    redge->pos =    false;
    redge->neg =    false;
    redge->posrel = false;
    redge->negrel = false;
    redge->lost =   false;
  }
  // reset cyclecount
  mCycleCount=0;
  mRecentCycleCount=0;
  mInputReady=false;
  // unlock global variables
  UNLOCK_E;
  FD_DHV("sDevice("<<mName<<")::ClrInputSignals(): done");
}

//ClrOutputSignals()
void sDevice::ClrOutputSignals(void) {
  //clear all output line levels
  FD_DHV("sDevice("<<mName<<")::ClrOutputSignals");
  // return if device is not running or not abel to set output signals
  if(mState!=Up || Outputs().Empty()) return;
  // lock variables, prevent background from writing
  LOCK_E;
  FD_DHV("sDevice("<<mName<<")::ClrOutputSignals: passed lock");
  // clear all (allways asynchronous)
  if(DoWriteSignalsPre()) {
    for(int bit=0; bit<=mMaxBitAddress; bit++)  
      if(mpOutputLevels[bit].rel) DoWriteSignal(bit, false);
    DoWriteSignalsPost();
  }
  // clear image
  for(int bit=0; bit<=mMaxBitAddress; bit++) {
    Levels* level= &mpOutputLevels[bit];
    level->current=false;
    level->next=false;
    level->edge=false;
    level->lost=false;
  }
  // done
  UNLOCK_E;
  FD_DHV("sDevice("<<mName<<")::ClrOutputSignals: done");
}

}//end namespace faudes



/** @file iop_vdevice.cpp Virtual device for interface definition  */

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008, Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/



#include "iop_vdevice.h"

#include "iop_simplenet.h"
#include "iop_comedi.h"
#include "iop_wago.h"
#include "iop_xdevice.h"
#include "sp_densityfnct.h"


namespace faudes {



/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeDeviceEvent

 **********************************************
 **********************************************
 **********************************************
 */


// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeDeviceEvent,AttributeVoid)

// Default constructor (no attributes, aka undefined) */
AttributeDeviceEvent::AttributeDeviceEvent(void) :
  AttributeVoid(), 
  mpOutputAttribute(0), 
  mpInputAttribute(0),
  pOutputPrototype(0), 
  pInputPrototype(0)
{
  FD_DHV("AttributeDeviceEvent::AttributeDeviceEvent(" << this << ")");
  pOutputPrototype= FallbackAttributep(); 
  pInputPrototype= FallbackAttributep();
}

// Copy constructor (never called)
AttributeDeviceEvent::AttributeDeviceEvent(const AttributeDeviceEvent& rOtherAttr) :
  AttributeVoid(), 
  mpOutputAttribute(0), 
  mpInputAttribute(0), 
  pOutputPrototype(0), 
  pInputPrototype(0)
{
  FD_DHV("AttributeDeviceEvent(" << this << "): form other attr " <<  &rOtherAttr);
  pOutputPrototype= FallbackAttributep(); 
  pInputPrototype= FallbackAttributep();
  DoAssign(rOtherAttr);
}

// Destructor    
AttributeDeviceEvent::~AttributeDeviceEvent(void) {
  if(mpOutputAttribute) delete mpOutputAttribute;
  if(mpInputAttribute)   delete mpInputAttribute;
}

// pseudo statics
const AttributeVoid* AttributeDeviceEvent::FallbackAttributep(void) {
  static AttributeVoid* avoid=new AttributeVoid();
  return avoid;
}

// Clear
void AttributeDeviceEvent::Clear(void) {
  if(mpOutputAttribute) delete mpOutputAttribute;
  if(mpInputAttribute)   delete mpInputAttribute;
  mpOutputAttribute=0;
  mpInputAttribute=0;
}

// Assignement
void AttributeDeviceEvent::DoAssign(const AttributeDeviceEvent& rSrcAttr) {
  FD_DHV("AttributeDeviceEvent(" << this << "):DoAssign(): " 
    << typeid(*this).name() << " from " << typeid(rSrcAttr).name());
  Clear();
  if(rSrcAttr.IsInput())   Input(*rSrcAttr.Inputp());
  if(rSrcAttr.IsOutput()) Output(*rSrcAttr.Outputp());
}



// DoWrite(rTw,rLabel,pContext)
void AttributeDeviceEvent::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  FD_DHV("AttributeDeviceEvent(" << this << ")::DoWrite(tw): " << IsOutput() << " " << IsInput()  );
  if(IsOutput()) mpOutputAttribute->Write(rTw,rLabel,pContext);
  if(IsInput())   mpInputAttribute->Write(rTw,rLabel,pContext);
}

//DoRead(rTr,rLabel,pContext)
void AttributeDeviceEvent::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  FD_DHV("AttributeDeviceEvent(" << this << ")::DoRead(tr)");
  // clear
  if(mpInputAttribute) delete mpInputAttribute;
  if(mpOutputAttribute) delete mpOutputAttribute;
  mpInputAttribute=0;
  mpOutputAttribute=0;
  // test for output/input section
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin()) return;
  // read output/input
  if(token.StringValue()=="Output" || token.StringValue()=="Actuator") {
    DefaultOutput();
    mpOutputAttribute->Read(rTr,rLabel,pContext);
    FD_DHV("Found Output : " << mpOutputAttribute->ToString());
  }
  // read output/input
  if(token.StringValue()=="Input" || token.StringValue()=="Sensor") {
    DefaultInput();
    mpInputAttribute->Read(rTr,rLabel,pContext);
    FD_DHV("Found Input: " << mpInputAttribute->ToString() << " " << rLabel);
  }
  // ignore others
}


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: vDevice

 **********************************************
 **********************************************
 **********************************************
 */


//vDevice()
vDevice::vDevice(void) {
  // constructor

  FD_DHV("vDevice(" << mName << ")::vDevice()");
  //set defaults
  mName = "IoDevice";				//device-name
  mpConfiguration = NULL;			//derived class to set: pointer to config-set
  mState = Down;				//initial state
  mDefaultLabel ="Device";			//label used with token-io
  mTimeScale = 1000;				//faudes-time scaling-factor 1[ftu] =^ 1[sec]
  mMaxSyncGap = Time::Step();			//max time-delay used with simulator
  pWaitMutex=&mWaitMutex;			//use object - mutex
  pWaitCondition=&mWaitCondition; 		//use object - condition
  pBufferMutex=&mBufferMutex;			//use object - buffer-mutex
  pInputBuffer=&mInputBuffer; 		//use object - input-buffer

  //initial mutex- and condition variables
  faudes_mutex_init(&mWaitMutex);
  faudes_cond_init(&mWaitCondition);
  faudes_mutex_init(&mBufferMutex);

#ifdef FAUDES_DEBUG_IOPERF
  // initialize arrays to store time stamps
  mpPerformanceWaitEnter = new faudes_systime_t[FAUDES_DEBUG_IOPERF_SAMPLES];
  mpPerformanceWaitExit = new faudes_systime_t[FAUDES_DEBUG_IOPERF_SAMPLES];
  mPerformanceEndIterator = 0;
  mPerformanceBeginIterator = 0;
#endif

  // track devices
  AllDevices().insert(this);

}

// pseudo static members
std::set<vDevice*>& vDevice::AllDevices(void) {
  static std::set<vDevice*> *dlist = new std::set< vDevice* >();
  return *dlist;
}


//~vDevice()
vDevice::~vDevice(void) {
  // destructor
  FD_DHV("vDevice(" << mName << ")::~vDevice()");
  //delete configuration
  if(mpConfiguration) delete mpConfiguration;
  //destroy mutex
  faudes_mutex_destroy(&mWaitMutex);
  faudes_cond_destroy(&mWaitCondition);
  faudes_mutex_destroy(&mBufferMutex);

#ifdef FAUDES_DEBUG_IOPERF
  // delete time stamps
  delete mpPerformanceWaitEnter;
  delete mpPerformanceWaitExit;
#endif

  // track devices
  AllDevices().erase(this);
}

//FromTokenReader
vDevice* vDevice::FromTokenReader(TokenReader& rTr) {
  // construct from token reader
  FD_DHV("vDevice::FromTokenReader()");
  // peek first token // s&e tr->rTr
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin()) {
    std::stringstream errstr;
    errstr << "Expected Begin Section in  \"" << rTr.FileName() << "\"";
    throw Exception("vDevice::FromFile", errstr.str(), 52);
  }
  // instantiate typed faudes object
  std::string ftype=token.StringValue();
  FD_DHV("vDevice::FromFile(" << rTr.FileName() <<"): device type " << ftype);
  Type* fobject= NewFaudesObject(ftype);
  if(!fobject) {
    std::stringstream errstr;
    errstr << "Unknown type \"" << ftype << "\" at  \"" << rTr.FileLine()  << "\"";
    throw Exception("vDevice()::FromFile", errstr.str(), 52);
    return 0;
  }
  // cast to vdevice
  vDevice* fdevice= dynamic_cast<vDevice*>(fobject);
  if(!fdevice) {
    std::stringstream errstr;
    errstr << "Could not cast to device object from type \"" << ftype << "\" at  \"" << rTr.FileLine()  << "\")";
    throw Exception("vDevice()::FromFile", errstr.str(), 52);
    return 0;
  }
  // do read stream
  fdevice->Read(rTr);
  return fdevice;
}


//FromFile(rFileName)
vDevice* vDevice::FromFile(const std::string& rFileName) {
  FD_DHV("vDevice::FromFile(" << rFileName <<")");
  // peek first token
  TokenReader tr(rFileName);
  return FromTokenReader(tr);
}


// static member function
void vDevice::StopAll(void) {
  // cosmetic bail out
  if(AllDevices().size()==0) return;
  // report
  FD_DH("vDevice::StopAll()");
  // reset
  std::set<vDevice*>::const_iterator dit;
  for(dit=AllDevices().begin(); dit != AllDevices().end(); dit++) {
    if( dynamic_cast<xDevice*>(*dit) ) continue;
    FD_DH("vDevice::StopAll(): Reset: " << (*dit)->Name());
    (*dit)->Reset();
  }
  // stop
  for(dit=AllDevices().begin(); dit != AllDevices().end(); dit++) {
    if( dynamic_cast<xDevice*>(*dit) ) continue;
    FD_DH("vDevice::StopAll(): Stop " << (*dit)->Name());
    (*dit)->Stop();
  }
  // report
  FD_DH("vDevice::StopAll(): done");
}


// Start(void)
void vDevice::Start(void) {
  // only start if currently down
  if(mState!=Down) return;

  FD_DHV("vDevice(" << mName << ")::Start()");
  if(!mpConfiguration) {
    std::stringstream errstr;
    errstr << "cannot start device when not configured";
    throw Exception("vDevice()::Start()", errstr.str(), 550);
  }
  // optimistic default in base class
  mState=Up;
  // Start implies Reset
  Reset();
}


// Stop()
void vDevice::Stop(void) {
  // stop implies Reset
  Reset();
  // only stop if up or starting up
  if(mState!=Up && mState!=StartUp) return;
  FD_DHV("vDevice(" << mName << ")::Stop()");
  // indicate success
  mState=Down;
}


// Reset()
void vDevice::Reset(void){
  //delete dynamic data
  FD_DHV("vDevice(" << mName <<")::Reset()");
  //reset time and delete dynamic data
  CurrentTime(0);
  // reset input buffer
  faudes_mutex_lock(pBufferMutex);
  pInputBuffer->clear();
  mResetRequest=false;
  faudes_mutex_unlock(pBufferMutex);
}


// Clear()
void vDevice::Clear(void) {
  //delete static data
  FD_DHV("vDevice("<< mName <<")::Clear()");
  //implies stop
  Stop();
  //clear exisiting data
  if(mpConfiguration) mpConfiguration->Clear();
  mOutputs.Clear();
  mInputs.Clear();
  // sensible defaults
  mTimeScale=1000;
}


// Name(rName)
void vDevice::Name(const std::string& rName) {
  FD_DHV("vDevice(" << mName << ")::Name(\"" << rName << "\")");
  mName = rName;
}

// Name()
const std::string& vDevice::Name(void) const {
  FD_DHV("vDevice(" << mName << ")::Name()");
  return mName;
}


//StatusString()
std::string vDevice::StatusString(void) {
  //status string, informal (should prepare mutex)

  FD_DHV("vDevice(" << mName << ")::Name()");
  //prepare result
  std::string res;
  //get curretn device state
  DeviceState currentState=Status();
  switch(currentState) {
  case Down: res="down"; break;
  case StartUp: res="starting up"; break;
  case Up: res="up and running"; break;
  case ShutDown: res="shutting down"; break;
  }

  return res;
}


//DoWrite(rTr,rLabel,pContext)
void vDevice::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext; (void) rTw;
  FD_DHV("vDevice("<<mName<<")::DoWrite()");
  // Set up defaults
  std::string label=rLabel;
  std::string ftype=TypeName();
  // Write begin
  if(label=="") label=TypeName(); 
  if(label=="") label = mDefaultLabel;
  Token btag;
  btag.SetBegin(label);
  if(Name()!=label) btag.InsAttributeString("name",Name());
  if(ftype!=label && ftype!="") btag.InsAttributeString("ftype",ftype);
  rTw << btag;
  // dispatch
  DoWritePreface(rTw,"",pContext);
  DoWriteConfiguration(rTw,"",pContext);
  // end of my section
  rTw.WriteEnd(label);
}

//DoWritePreface(rTw,rLabel,pContext)
void vDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext; (void) rTw;
  // virtual interface for token-output: device specific parameters
  FD_DHV("vDevice("<<mName<<")::DoWritePreface()");
  // write faudes-time scaling-factor
  Token atoken;
  atoken.SetEmpty("TimeScale");
  atoken.InsAttributeInteger("value",mTimeScale);
  rTw<<atoken;
}


//DoWriteConfiguration(rTw,rLabel,pContext)
void vDevice::DoWriteConfiguration(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext; (void) rTw;
  // virtual interface for for token-output: event configuration
  FD_DHV("vDevice("<<mName<<")::DoWriteConfiguration()");
  // figure section, write begin
  std::string label=rLabel;
  if(label == "") label = "EventConfiguration";
  rTw.WriteBegin(label);
  // loop events
  for (EventSet::Iterator it = mpConfiguration->Begin(); it != mpConfiguration->End(); ++it) {
    Token etoken;
    const AttributeVoid& attr=mpConfiguration->Attribute(*it);
    const AttributeDeviceEvent* attrp = dynamic_cast<const AttributeDeviceEvent*>(&attr);
#ifdef FAUDES_DEBUG_CODE
    if(!attrp) {
      std::stringstream errstr;
      errstr << "Invalid attribute configuration";
      throw Exception("vDevice::DoWriteConfiguration", errstr.str(), 550);
    }
#endif
    if(!attrp) continue; 
    // figure whether we have an effetively default attribute 
    // note: we should have a smart token writer to figure empty sections automatically
    bool def=false;
    if(attrp->IsInput())
      if(attrp->Inputp()->IsDefault()) def=true;
    if(attrp->IsOutput())
      if(attrp->Outputp()->IsDefault()) def=true;
    // case a: no attribute value
    if(def) {
      etoken.SetEmpty("Event");
      etoken.InsAttributeString("name",mpConfiguration->SymbolicName(*it));
      if(attrp->IsInput())  etoken.InsAttributeString("iotype","input");
      if(attrp->IsOutput()) etoken.InsAttributeString("iotype","output");
      rTw << etoken;
    } 
    // case b: incl attribute value
    if(!def) {
      etoken.SetBegin("Event");
      etoken.InsAttributeString("name",mpConfiguration->SymbolicName(*it));
      if(attrp->IsInput()) etoken.InsAttributeString("iotype",std::string("input"));
      if(attrp->IsOutput()) etoken.InsAttributeString("iotype","output");
      rTw << etoken;
      attrp->Write(rTw,"",pContext);
      etoken.SetEnd("Event");
      rTw << etoken;
    }
  }
  // loop done
  rTw.WriteEnd(label);


  // write event-configuration
  //if(mpConfiguration) mpConfiguration->XWrite(rTw,label);
}

//DoRead(rTr,rLabel,pContext)
void vDevice::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) pContext; (void) rLabel; (void) rTr;
  // virtual interface for token-input: dispatcher
  FD_DHV("vDevice("<<mName<<")::DoRead()");
  Clear();
  // set default label if no label specified (configure in type constructor)
  std::string label = rLabel;
  if(label=="") label = mDefaultLabel;
  // read my section
  Token btag;
  rTr.ReadBegin(label,btag);
  // extract name
  Name(label);
  if(btag.ExistsAttributeString("name"))
    Name(btag.AttributeStringValue("name"));
  // call DoReadConfiguration and DoReadPreface
  DoReadPreface(rTr,"",pContext);
  DoReadConfiguration(rTr,"",pContext);
  // end of my section
  rTr.ReadEnd(label);
  // organize internal data structures
  Compile();
}

//DoReadConfiguration(rTr,rLabel,pContext)
void vDevice::DoReadConfiguration(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  // bail out on bad configuration
  if(!mpConfiguration) return;
  // set default label if no label specified
  std::string label=rLabel;
  if(label=="") label="EventConfiguration";
  // set name
  mpConfiguration->Name(label);
  // test for  begin
  /*
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin()) return;
  if(token.StringValue()!=label) return;
  */
  // read begin
  rTr.ReadBegin(label); 
  FD_DHV("vDevice("<<mName<<")::DoReadConfiguration()");
  // prepare attribute
  AttributeVoid* vattrp = mpConfiguration->AttributeType()->New();
  AttributeDeviceEvent* attrp = dynamic_cast<AttributeDeviceEvent*>(vattrp);
#ifdef FAUDES_DEBUG_CODE
  if(!attrp) {
    std::stringstream errstr;
    errstr << "Invalid attribute configuration" << rTr.FileLine();
    throw Exception("vDevice::DoReadConfiguration", errstr.str(), 550);
  }
#endif
  if(!attrp) return; 
  FD_DHV("vDevice::DoReadConfiguration(" << this << "): attribute type " << typeid(*attrp).name());
  // loop tokens
  std::string name;
  std::string iotype;
  while(!rTr.Eos(label)) {
    Token token;
    rTr.Peek(token);

    // snse and digest pre 2.16 format
    if(token.IsString()) {
      rTr.Get(token);
      name=token.StringValue();
      // read faudes attribute
      attrp->Read(rTr,"",pContext);
      // skip unknown faudes attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      Idx index=mpConfiguration->Insert(name); 
      mpConfiguration->Attribute(index,*attrp);
      continue;
    }

    // read element section 
    if(token.IsBegin()) 
    if(token.StringValue()=="Event") {
      FD_DHV("vDevice(" << this << ")::DoReadConfiguration(..): " << token.Str() << "\"");
      rTr.ReadBegin("Event",token);
      name=token.AttributeStringValue("name");
      iotype=token.AttributeStringValue("iotype");
      // input event
      if(iotype=="input") 
        attrp->ReadInput(rTr);
      // output event
      if(iotype=="output") 
        attrp->ReadOutput(rTr);
      // valid
      if((iotype=="input") || (iotype=="output")) {
        // insert element with attribute
        Idx index=mpConfiguration->Insert(name); 
        mpConfiguration->Attribute(index,*attrp);
        rTr.ReadEnd("Event");
        continue;
      }
    }
    // cannot process token
    delete attrp;
    std::stringstream errstr;
    errstr << "Invalid token of type " << token.Type() << " at " << rTr.FileLine();
    throw Exception("vDevice::DoReadConfiguration", errstr.str(), 50);
  }
  rTr.ReadEnd(label);
  delete attrp;
}

//DoReadPreface(rTr,rLabel,pContext)
void vDevice::DoReadPreface(TokenReader& rTr, const std::string& rLabel, const Type* pContext){
  (void) pContext; (void) rLabel; (void) rTr;
  // virtual interface for token-input: device specific parameters
  FD_DHV("vDevice("<<mName<<")::DoReadPreface()");

  // sense and digest pre 2.16 format
  Token token;
  rTr.Peek(token);
  if(token.IsString()) {
    mName = rTr.ReadString();
    mTimeScale = rTr.ReadInteger();
    return;
  }

  // loop my members
  while(true) {
    Token token;
    rTr.Peek(token);
    // time scale
    if(token.IsBegin()) 
    if(token.StringValue()=="TimeScale") {
      rTr.ReadBegin("TimeScale", token);
      mTimeScale=token.AttributeIntegerValue("value");
      rTr.ReadEnd("TimeScale");
      continue;
    }
    // break on unknown
    break;
  }
}

// Configure(Idx,AttributeDeviceEvent&)
void vDevice::Configure(Idx event, const AttributeDeviceEvent& attr){
  //inserts an attributed event and compiles internal data structure  
  FD_DHV("vDevice(" << mName << ")::Configure(event)");
  Stop();
  // test type
  const AttributeVoid* cap = mpConfiguration->AttributeType();
  if(typeid(attr)!=typeid(*cap)) {
    std::stringstream errstr;
    errstr << "Cannot cast attribute to device event attribute";
    throw Exception("vDevice::Configure", errstr.str(), 550);
  }
  // record
  mpConfiguration->Insert(event);
  mpConfiguration->Attribute(event,attr);
  // setup internal data structure
  Compile();
}

// Configure(EventSet)
void vDevice::Configure(const EventSet& rPhysicalEvents){
  FD_DHV("vDevice(" << mName << ")::Configure(alphabet)");
  Stop();
  // test type
  const AttributeVoid* cap = mpConfiguration->AttributeType();
  const AttributeVoid* oap = rPhysicalEvents.AttributeType();
  if(typeid(*oap)!=typeid(*cap)) {
    std::stringstream errstr;
    errstr << "Cannot cast attribute to device event attribute";
    throw Exception("vDevice::Configure", errstr.str(), 550);
  }
  // record events
  for(EventSet::Iterator eit=rPhysicalEvents.Begin(); eit!=rPhysicalEvents.End(); eit++) {
    const AttributeVoid attr = rPhysicalEvents.Attribute(*eit);
    mpConfiguration->Insert(*eit);
    mpConfiguration->Attribute(*eit,attr); // why does this do the job???
  }
  // setup internal data structure
  Compile();
}

//Compile()
void vDevice::Compile(void){
  FD_DHV("vDevice(" << mName << ")::Compile()");
  // Compile() implies Stop()
  Stop();
  // clear input- and output-set
  mInputs.Clear();
  mOutputs.Clear();
  // loop over events and set outputs and inputs
  mInputs.Clear();
  mOutputs.Clear();
  for(EventSet::Iterator eit=mpConfiguration->Begin(); eit!=mpConfiguration->End(); eit++) {
    AttributeVoid* attr = mpConfiguration->Attributep(*eit);
    AttributeDeviceEvent* dattr = dynamic_cast< AttributeDeviceEvent* >(attr);
    if(!dattr) {
      std::stringstream errstr;
      errstr << "Cannot cast attribute to device event attribute";
      throw Exception("vDevice()::Configure", errstr.str(), 550);
    }
    if(dattr->IsInput()) mInputs.Insert(*eit);
    if(dattr->IsOutput()) mOutputs.Insert(*eit);
  }
}

//UseCondition(faudes_mutex_t*)
void vDevice::UseCondition(faudes_mutex_t* wmutex, faudes_cond_t* wcond) {
  // tell device which condition variable to use
  FD_DHV("vDevice(" << mName << ")::UseCondition()");
  // set cond/mutex for wait inputs
  if(wmutex) pWaitMutex=wmutex;
  if(wcond) pWaitCondition=wcond;
}

//UseBuffer(mutex, buffer)
void vDevice::UseBuffer(faudes_mutex_t* bmutex, std::deque<Idx>* bbuffer) {
  // Tell the device which buffer to use
  FD_DHV("vDevice(" << mName << ")::UseBuffer()");
  //set input buffer and buffer-mutex and
  if(bmutex) pBufferMutex=bmutex;
  if(bbuffer) pInputBuffer=bbuffer;
}

// Outputs()
const EventSet& vDevice::Outputs(void) const {
  return mOutputs;
}

// Inputs()
const EventSet& vDevice::Inputs(void) const {
  return mInputs;
}

//FtuToSystemTime(int)
faudes_systime_t vDevice::FtuToSystemTime(Time::Type duration){
  // convert faudes-time to sys.time)
  FD_DHV("vDevice("<<mName<<")::FtuToSystemTime(" << duration << "): using scale " << mTimeScale);
  //prepare result
  faudes_systime_t res;
  //get current system-time
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  //convert duration to real time msec
  long int durationMsec = mTimeScale*duration;
  //convert duration from msec (long int) to nsec (sys.time)
  faudes_systime_t delta;
  delta.tv_sec = durationMsec/1000;
  delta.tv_nsec = ( durationMsec - (delta.tv_sec*1000)  ) *1000000;
  //specify the absolut point of time
  faudes_sumsystime(now,delta,&res);
  // done
  return res;
}

// FtuToMs(Time::Type)
long int vDevice::FtuToMs(Time::Type faudes_time){
  //convert faudes-time to realtime in ms
  FD_DHV("vDevice("<<mName<<")::FtuToMs() ");
  // avoid buffer-overflow while trying to convert an
  // "infinite" periode of time
  if(faudes_time>= Time::Max()/ (mTimeScale+1)) return Time::Max();
    // return real time in ms
  return mTimeScale*faudes_time;
}

// MsToFtu(long int)
Time::Type vDevice::MsToFtu(long int real_time){
  //Convert real time in ms to faudes-time

  FD_DHV("vDevice(" << mName << ")::MsToFtu()");
  return (long int)((1.0/mTimeScale)*real_time/1000*1000);
}

//CurrentTime(void)
Time::Type vDevice::CurrentTime(void) {
  //return current faudes-time
  FD_DHV("vDevice("<<mName<<")::CurrentTime() ");
  //get current system-time
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  // compute the time elapsed since backgroundthread-startup
  faudes_systime_t diff;
  faudes_diffsystime(now,mTimeZero,&diff);
  // convert physical time to faudes-time
  return (long int) ((1000.0/mTimeScale) * (diff.tv_sec + diff.tv_nsec*1e-9));
}

//CurrentTime(now)
void vDevice::CurrentTime(Time::Type now){
  // set current faudes-time
  FD_DHV("vDevice(" << mName <<")::CurrentTime("<<Time::Str(now) <<")");
  CurrentTimeMs(mTimeScale*now);
}

//CurrentTimeMs()
long int vDevice::CurrentTimeMs(void) {
  //return current real-time in ms
  FD_DHV("vDevice("<<mName<<")::CurrentTimeMs() ");
  //get current system-time
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  // compute the time elapsed since backgroundthread-startup
  faudes_systime_t diffTime;
  faudes_diffsystime(now,mTimeZero,&diffTime);
  // convert current physical time to msecs
  return 1000*diffTime.tv_sec + diffTime.tv_nsec/1000000;
}


//CurrentTimeMs(long int)
void vDevice::CurrentTimeMs(long int nowms){
  // set current real-time in ms
  FD_DHV("vDevice("<<mName<<")::CurrentTimeMs(nowms) ");
  // set for now =0
  faudes_systime_t now;
  faudes_gettimeofday(&now);
  // adjust to now
  faudes_systime_t delta;
  delta.tv_sec = nowms/1000;
  delta.tv_nsec = 1000000*(nowms%1000);
  // do adjust
  faudes_diffsystime(now,delta,&mTimeZero);
  // report
  /*
  time_t systime;
  time(&systime);
  struct tm* timeinfo;
  timeinfo= localtime(&systime);
  char strbuf[80];
  strftime(strbuf,80,"%X",timeinfo);
  */
}


// WaitInputs(Time::Type)
bool vDevice::WaitInputs(Time::Type duration) {
  //wait a specified time for input events to occur

  FD_DHV("vDevice("<<mName<<")::WaitInputs("<< duration << ")");

  // adjust arguments TODO
  if(duration> 3600*24) { duration= 3600*24;};

  // if this is a nontrivial wait, flush outputs
  if(duration>0) FlushOutputs();

  // wait mutex gets released while waiting
  faudes_mutex_lock(pWaitMutex);

  // get preliminary result
  bool sr=InputReady();

  // actually wait only if there are no input yet
  if(!sr && duration>0) {

#ifdef FAUDES_DEBUG_IOPERF
    // performance time stamp
    faudes_systime_t now;
    faudes_gettimeofday(&now);
    mpPerformanceWaitEnter[mPerformanceEndIterator]=now;
#endif

    //convert duration in systemtime
    faudes_systime_t condtime = vDevice::FtuToSystemTime(duration);
    //wait for report from background thread about on available events
    FD_DHV("vDevice("<<mName<<")::WaitInputs("<< duration << "): waiting for condition");
    faudes_cond_timedwait(pWaitCondition, pWaitMutex, &condtime);
    FD_DHV("vDevice("<<mName<<")::WaitInputs("<< duration << "): release at "<< CurrentTime());
    // update result
    sr=InputReady();

#ifdef FAUDES_DEBUG_IOPERF
    // performance time stamp
    faudes_gettimeofday(&now);
    mpPerformanceWaitExit[mPerformanceEndIterator]=now;
    mPerformanceEndIterator++;
    if(mPerformanceEndIterator >= FAUDES_DEBUG_IOPERF_SAMPLES) 
      mPerformanceEndIterator=0;
    if(mPerformanceEndIterator == mPerformanceBeginIterator)
      mPerformanceBeginIterator++;
    if(mPerformanceBeginIterator >= FAUDES_DEBUG_IOPERF_SAMPLES) 
      mPerformanceBeginIterator=0;
#endif

  }

  // release wait mutex
  faudes_mutex_unlock(pWaitMutex);


  return sr;
}


// WaitInputsMs(long int)
bool vDevice::WaitInputsMs(long int duration) {
  //wait a specified time for input events to occur

  FD_DHV("vDevice("<<mName<<")::WaitInputsMs("<< duration << ")");

  // adjust arguments TODO
  if(duration> 30*3600*24) { duration= 30*3600*24;};

  // if this is a nontrivial wait, flush outputs
  if(duration>0) FlushOutputs();

  // wait mutex gets released while waiting
  faudes_mutex_lock(pWaitMutex);

  // get preliminary result
  bool sr=InputReady();
  // actually wait onnly if there are no input yet
  if(!sr && duration>0) {

#ifdef FAUDES_DEBUG_IOPERF
    // performance time stamp
    faudes_systime_t now;
    faudes_gettimeofday(&now);
    mpPerformanceWaitEnter[mPerformanceEndIterator]=now;
#endif

    //convert duration in nsec spec
    faudes_systime_t condtime;
    faudes_msdelay(duration,&condtime);
    //wait for report from background thread on available events
    faudes_cond_timedwait(pWaitCondition, pWaitMutex, &condtime);
    // update result
    sr=InputReady();

#ifdef FAUDES_DEBUG_IOPERF
    // performance time stamp
    faudes_gettimeofday(&now);
    mpPerformanceWaitExit[mPerformanceEndIterator]=now;
    mPerformanceEndIterator++;
    if(mPerformanceEndIterator >= FAUDES_DEBUG_IOPERF_SAMPLES) 
      mPerformanceEndIterator=0;
    if(mPerformanceEndIterator == mPerformanceBeginIterator)
      mPerformanceBeginIterator++;
    if(mPerformanceBeginIterator >= FAUDES_DEBUG_IOPERF_SAMPLES) 
      mPerformanceBeginIterator=0;
#endif

  }

  // released wait mutex
  faudes_mutex_unlock(pWaitMutex);

  return sr;
}

// FlushOutputs(void)
void vDevice::FlushOutputs(void) {
}

// ResetRequest()
bool vDevice::ResetRequest(void) {
  bool res=false;
  faudes_mutex_lock(pBufferMutex);
  res=mResetRequest;
  mResetRequest=false;
  faudes_mutex_unlock(pBufferMutex);
  return res;
}

//ReadInputs()
Idx vDevice::ReadInput(void) {
  //reads input-buffer and returns first occured events by index

  FD_DHV("vDevice("<<mName<<")::ReadInput()");
  // prepare result, invalid
  Idx res=0;

  // take oldest entry
  faudes_mutex_lock(pBufferMutex);
  if(!pInputBuffer->empty())  {
    res=pInputBuffer->front();
    pInputBuffer->pop_front();
  }
  faudes_mutex_unlock(pBufferMutex);
  FD_DHV("vDevice("<<mName<<")::ReadInput(): found: " << res);
  return res;
}


//PeekInputs()
Idx vDevice::PeekInput(void) {
  //reads input-buffer and returns first occured events by index
  // does not remove the event from the buffer

  FD_DHV("vDevice("<<mName<<")::PeekInput()");
  // prepare result, invalid
  Idx res=0;

  // take oldest entry
  faudes_mutex_lock(pBufferMutex);
  if(!pInputBuffer->empty())  {
    res=pInputBuffer->front();
  }
  faudes_mutex_unlock(pBufferMutex);

  FD_DHV("vDevice("<<mName<<")::PeekInput(): found: " << res);
  return res;
}


//InputReady()
bool vDevice::InputReady(void) {
  // returns true if a input-event occured

  FD_DHV("vDevice("<<mName<<")::ReadInput()");
  //prepare result
  bool res;
  //read buffer
  faudes_mutex_lock(pBufferMutex);
  res= !mInputBuffer.empty();
  faudes_mutex_unlock(pBufferMutex);

  return res;
}


// timing analysis
SampledDensityFunction vDevice::Performance(void) {
#ifdef FAUDES_DEBUG_IOPERF

  FD_DHV("vDevice::Performance(" << Name() << "): " << mPerformanceBeginIterator << " " << mPerformanceEndIterator);

  // have samples
  SampledDensityFunction busyduration;
  busyduration.Dim(50);
  busyduration.Clear();
  busyduration.Name("performance " + Name() + " (busy duration in usecs)");

  // inspect time stamps
  for(int i = mPerformanceBeginIterator; ;i++){
    if(i>= FAUDES_DEBUG_IOPERF_SAMPLES) i=0;
    int ip = i+1;
    if(ip>= FAUDES_DEBUG_IOPERF_SAMPLES) ip =0;
    if(ip==mPerformanceEndIterator) break;
    faudes_systime_t busy;
    faudes_diffsystime(mpPerformanceWaitEnter[ip],mpPerformanceWaitExit[i],&busy);
    busyduration.Sample(busy.tv_sec*1000000 + busy.tv_nsec/1000);
  }   

  // compile
  busyduration.Compile();

  // done
  return busyduration;

#else

  FD_DHV("vDevice::Performance(" << Name() << "): not enabled");
  SampledDensityFunction busyduration;
  busyduration.Name("performance monitoring not enabled");
  return busyduration;

#endif
}

// WritePerformance
void vDevice::WritePerformance(void) {
#ifdef FAUDES_DEBUG_IOPERF
  SampledDensityFunction busyduration=Performance();
  // token output
  busyduration.Write();
  // pretty print
  std::cout << busyduration.Str() << std::endl;
#endif
}

// ClearPerformance
void vDevice::ResetPerformance(void) {

#ifdef FAUDES_DEBUG_IOPERF
  mPerformanceEndIterator = 0;
  mPerformanceBeginIterator = 0;
#endif

}

} // name space


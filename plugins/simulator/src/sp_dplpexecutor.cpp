/** @file sp_dplpexecutor.cpp Executor with IO device  */

/* 
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Thomas Moor

*/


// load configuration
#include "corefaudes.h"


#include "sp_dplpexecutor.h"
#include <iostream>

namespace faudes {

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(DeviceExecutor,DeviceExecutor,LoggingExecutor)

// constructor
DeviceExecutor::DeviceExecutor(void) : ProposingExecutor() , pDevice(0) { 
  mMaxSyncGap=10*Time::Step();
  mSyncMode=0x0;
}

// constructor
DeviceExecutor::DeviceExecutor(const DeviceExecutor& rOther) : ProposingExecutor() , pDevice(0) { 
  mMaxSyncGap=10*Time::Step();
  mSyncMode=0x0;
  DoAssign(rOther);
}

// destructor
DeviceExecutor::~DeviceExecutor(void) { 
}



// DoAssign(other)
void DeviceExecutor::DoAssign(const DeviceExecutor& rOther) {
  FD_DX("LoggingExecutor(" << this << ")::DoAssign(other)");
  // my members
  Devicep(NULL);
  // base
  ProposingExecutor::DoAssign(rOther);
}

// set io device
void DeviceExecutor::Devicep(vDevice* dev) {
  pDevice=dev;
  mSyncError=false;
#ifdef FAUDES_PLUGIN_IODEVICE
  if(!dev) return;
  pDevice->Reset();
#endif
}

// reset the device
void DeviceExecutor::DeviceReset(void) {
#ifdef FAUDES_PLUGIN_IODEVICE
  if(!pDevice) return;
  pDevice->Reset();
#endif
}

// start the device
void DeviceExecutor::DeviceStart(void) {
#ifdef FAUDES_PLUGIN_IODEVICE
  if(!pDevice) return;
  pDevice->Start();
#endif
}

// stop the device
void DeviceExecutor::DeviceStop(void) {
#ifdef FAUDES_PLUGIN_IODEVICE
  if(!pDevice) return;
  pDevice->Stop();
#endif
}

// clear static data
void DeviceExecutor::Clear(void){
  Reset();
  ProposingExecutor::Clear();
}

// clear dynamic data
void DeviceExecutor::Reset(long int seed){
  FD_DS("DeviceExecutor::Reset()");
#ifdef FAUDES_PLUGIN_IODEVICE
  // reset the device
  if(pDevice) 
     pDevice->Reset();
  // overwrite input attributes 
  if(pDevice) {
    SimPriorityEventAttribute neverpa;
    neverpa.mPriority=-9999; // todo: have a never-priority                
    SimEventAttribute never;
    never.Priority(neverpa);
    EventSet::Iterator eit=pDevice->Inputs().Begin();
    for(;eit!=pDevice->Inputs().End(); eit++) {
      if(Alphabet().Exists(*eit)) EventAttribute(*eit,never);
    }
  }
#endif
  // call base
  ProposingExecutor::Reset(seed);
  mSyncError=false;
  FD_DS("DeviceExecutor::Reset(): done");
}

// Convenience
bool DeviceExecutor::DeviceResetRequest(void) {
#ifdef FAUDES_PLUGIN_IODEVICE
  // bail out
  if(!pDevice) return false;
  // pass on to device
  return pDevice->ResetRequest();
#else
  return false;
#endif
}

//ExecuteEvent
bool DeviceExecutor::ExecuteEvent(Idx event){
  FD_DS("DeviceExecutor::ExecuteEvent(): "<< EventName(event));
  /*
#ifdef FAUDES_PLUGIN_IODEVICE
  // sync time
  if(pDevice) SyncStep(0);
#endif
  */
  // execute event
  bool res=ProposingExecutor::ExecuteEvent(event);
  /*
#ifdef FAUDES_PLUGIN_IODEVICE
  // pass on to callbacks
  if(res && pDevice) pDevice->WriteOutput(event);
#endif
  */
  // report
  return res;
}


//SyncTime()
bool DeviceExecutor::SyncTime(void){

  // no plugin, no run
#ifndef FAUDES_PLUGIN_IODEVICE
  return false;
#else

  // no device, no run
  if(!pDevice) return false;

  // figure situation
  Time::Type devNow=pDevice->CurrentTime();
  Time::Type genNow=CurrentTime();
  Time::Type syncTime=devNow-genNow;
  Time::Type propTime=ProposeNextTransition().mTime;


  // time sync: no chance since generator is ahead
  if(mSyncMode & SyncStrictTime)
  if(syncTime < -mMaxSyncGap) {
    FD_DS_SYNC("DeviceExecutor::SyncTime(): device time " << devNow << " gen time " << genNow);
    FD_DS_SYNC("DeviceExecutor::SyncTime(): sync error: gen time ahead");
    mSyncError=true;
    pDevice->Reset();
    return false;
  }

  // time sync: check whether proposal allows for sync
  if(mSyncMode & SyncStrictTime)
  if(syncTime - mMaxSyncGap > propTime || propTime==Time::UnDef()) {
    FD_DS_SYNC("DeviceExecutor::SyncTime(): device time " << devNow << " gen time " << genNow);
    FD_DS_SYNC("DeviceExecutor::SyncTime(): sync error: cannot sync enough time");
    mSyncError=true;
    pDevice->Reset();
    return false;
  }

  // fix undef time (aka deadlock)
  // note: this is pointless, since the executor will not recover
  if(propTime==Time::UnDef()) propTime=Time::Max();
 
  // adjust by proposal
  if(syncTime > propTime) {
    FD_DS_SYNC("DeviceExecutor::SyncTime(): device time " << devNow << " gen time " << genNow);
    FD_DS_SYNC("DeviceExecutor::SyncTime(): adjust by proposal");
    syncTime=propTime;
  }

  // time sync: pass generator time 
  if(syncTime>0) {
    FD_DS_SYNC("DeviceExecutor::SyncTime(): generator to pass " << syncTime); 
    if(!ProposingExecutor::ExecuteTime(syncTime)) {
      Time::Type maxsync=EnabledTime().UB();
      if(maxsync >= syncTime-mMaxSyncGap) {
        syncTime=maxsync;
      }
      FD_DS_SYNC("DeviceExecutor::SyncTime(): retry: generator to pass " << syncTime);
      if(!ProposingExecutor::ExecuteTime(syncTime)) 
      if(mSyncMode & SyncStrictTime) {
        FD_DS_SYNC("DeviceExecutor::SyncTime(): exec error");
        mSyncError=true;
        pDevice->Reset();
        return false;
      }
    };
  }

  // done
  /*
  FD_DS_SYNC("DeviceExecutor::SyncTime(): ok: device time " << 
    pDevice->CurrentTime() << " gen time " << CurrentTime());
  */
  return true;

#endif
}


//SyncEvents()
Idx DeviceExecutor::SyncEvents(void){

  // no plugin, no run
#ifndef FAUDES_PLUGIN_IODEVICE
  return 0;
#else

  // no device, no run
  if(!pDevice) return 0;

  // ignore alien hardware event
  if(Idx sev=pDevice->PeekInput()) {
    if(!ParallelExecutor::Alphabet().Exists(sev)) {
      FD_DS_SYNC("DeviceExecutor::SyncEvents(): input event " << EStr(sev) << " ignored");
      pDevice->ReadInput();
    }
  }

  // get and execute hardware events, if accepted
  if(Idx sev=pDevice->PeekInput()) {
    if(EnabledEvents().Exists(sev)) {
      pDevice->ReadInput();
      ProposingExecutor::ExecuteEvent(sev);
      FD_DS_SYNC("DeviceExecutor::SyncEvents(): input event " << EStr(sev) << " accepted");
      return sev;
    }
    FD_DS_SYNC("DeviceExecutor::SyncEvents(): input event " << EStr(sev) << " postponed");
  }

  // check and execute scheduled event
  const TimedEvent& proposedTrans=ProposeNextTransition();
  if(proposedTrans.mTime==0 && proposedTrans.mEvent!=0) {
    FD_DS_SYNC("DeviceExecutor::SyncEvents(): scheduled event: " << EventName(proposedTrans.mEvent));
    ProposingExecutor::ExecuteEvent(proposedTrans.mEvent);
    if(pDevice->Outputs().Exists(proposedTrans.mEvent))
      pDevice->WriteOutput(proposedTrans.mEvent);
    FD_DS_SYNC("DeviceExecutor::SyncEvents(): scheduled event: execueted");
    return proposedTrans.mEvent;
  }

  // get and execute hardware events
  if(Idx sev=pDevice->ReadInput()) {
    bool exec = ProposingExecutor::ExecuteEvent(sev);
    if(!exec && (mSyncMode & SyncStrictEvents)) {
      FD_DS_SYNC("DeviceExecutor::SyncEvents(): sync error: invalid input event");
      mSyncError=true;
      pDevice->Reset();
    }
    return (exec ? sev : 0);
  }


  /*
  FD_DS_SYNC("DeviceExecutor::SyncEvents(): no events ready");
  */
  return 0;

#endif
}

//SyncWait()
bool DeviceExecutor::SyncWait(Time::Type duration){

  // no plugin, no run
#ifndef FAUDES_PLUGIN_IODEVICE
  return false;
#else

  // no device, no run
  if(!pDevice) return false;

  // report
  FD_DS_SYNC("DeviceExecutor::SyncWait(): sleep " << Time::Str(duration) );

  // adjust
  const TimedEvent& proposedTrans=ProposeNextTransition();
  if(proposedTrans.mTime<duration) duration=proposedTrans.mTime;
  if(duration <0) duration=0;

  // report
  FD_DS_SYNC("DeviceExecutor::SyncWait(): adjusted by proposal: " << Time::Str(duration) );

  // doit
  bool res=pDevice->WaitInputs(duration);

  // done
  return res;

#endif // io plugin
}


//SyncWaitMs()
bool DeviceExecutor::SyncWaitMs(int durationms) {

  // no plugin, no run
#ifndef FAUDES_PLUGIN_IODEVICE
  return false;
#else

  // no device, no run
  if(!pDevice) return false;

  // report
  /*
  FD_DS_SYNC("DeviceExecutor::SyncWaitMs(): sleep " << durationms );
  */

  // adjust
  const TimedEvent& proposedTrans=ProposeNextTransition();
  if(proposedTrans.mTime*pDevice->TimeScale() <durationms) durationms=proposedTrans.mTime *pDevice->TimeScale();
  if(durationms <0) durationms=0;

  // report
  /*
  FD_DS_SYNC("DeviceExecutor::SyncWaitMs(): adjusted by proposal: " << durationms );
  */

  // doit
  bool res=pDevice->WaitInputsMs(durationms);

  // done
  return res;

#endif // io plugin
}


//SyncStep(duartion)
Idx DeviceExecutor::SyncStep(Time::Type duration){
  FD_DS_SYNC("DeviceExecutor::SyncStep("<< Time::Str(duration) << ")");

  // no plugin, no run
#ifndef FAUDES_PLUGIN_IODEVICE
  return 0;
#else

  // no device, no run
  if(!pDevice) return 0;

  // result
  Idx res=0;

  // doit
  if(!SyncTime()) return res;
  if((res=SyncEvents())!=0) return res;
  if(!SyncWait(duration)) return res;
  if(!SyncTime()) return res;
  res=SyncEvents();

  // done  
  return res;

#endif // io plugin
}

//SyncRun(duartion)
bool DeviceExecutor::SyncRun(Time::Type duration){
  FD_DS_SYNC("DeviceExecutor::SyncRun("<< Time::Str(duration) << ")");

  // prepare
  Time::Type genNow=CurrentTime();
  Time::Type genStop=genNow+duration;
  Time::Type genLeft=genStop-genNow;
  if(duration==Time::Max()) {
    genStop=Time::Max();
    genLeft=Time::Max();
  }

  // loop time
  while(genLeft>0) {
    // step
    if(SyncStep(genLeft)==0) return false;
    // update
    genNow=CurrentTime();
    genLeft=genStop-genNow;
    if(duration==Time::Max()) genLeft=Time::Max();
  }
  // done
  return true;
}

} // namespace


/** @file iop_d3ripURT.cpp d3ripURT protocol and friends */

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2011, Ece Schmidt, Klaus Schmidt, Ulas Turan

*/


#include "iop_d3ripURT.h"


//only compile with D3RIP-URT protocol operation
#ifdef FAUDES_IODEVICE_D3RIP_URT


#include <sys/mman.h> 
#include <errno.h>
#include <signal.h>
namespace faudes {


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeD3ripURTOutput

 **********************************************
 **********************************************
 **********************************************
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeD3ripURTOutput,AttributeVoid)

//DoAssign(Src)
void AttributeD3ripURTOutput::DoAssign(const AttributeD3ripURTOutput& rSrcAttr) {
  FD_DHV("AttributeD3ripURTOutput(" << this << "):DoAssign(): assignment from " <<  &rSrcAttr);
  mChannelToTransmit=rSrcAttr.mChannelToTransmit;
  mParameterRecords=rSrcAttr.mParameterRecords;
  mEventId=rSrcAttr.mEventId;
}

//DoWrite(rTw);
void AttributeD3ripURTOutput::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  
  FD_DHV("AttributeD3ripURTOutput::DoWrite()" );
  Token ptoken;
  ptoken.SetEmpty("ChannelToTransmit");
  ptoken.InsAttributeInteger("value",mChannelToTransmit);
  rTw << ptoken;
}


//DoRead(rTr)
void AttributeD3ripURTOutput::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  mParameterRecords.reserve(10);
  (void) rLabel; (void) pContext;
  Token token;
  FD_DHV("AttributeD3ripURTOutput::DoRead()");
  // loop all D3ripURT parameters
  while(1) {
    rTr.Peek(token);
    if(!token.IsBegin()) return;
    //ChannelToTransmit
    if(token.IsBegin("ChannelToTransmit")) {
      rTr.Get(token);      
      if(token.ExistsAttributeInteger("value")) 
        mChannelToTransmit=token.AttributeIntegerValue("value");
      rTr.ReadEnd("ChannelToTransmit");
      continue;
    }
    if(token.IsBegin("EventId")) {
      rTr.Get(token);      
      if(token.ExistsAttributeInteger("value")) 
        mEventId=token.AttributeIntegerValue("value");
      rTr.ReadEnd("EventId");
      continue;
    }

    //ParameterRecord
    if(token.IsBegin("ParameterRecord")) {
      // mParameterRecords is a std::vector<ParameterRecord>
      mParameterRecords.resize(mParameterRecords.size()+1); //grow by one, get reference to last
      ParameterRecord& record = mParameterRecords.back();
      rTr.ReadBegin("ParameterRecord",token);    
      //if(token.ExistsAttributeInteger("name")) 
      while(!rTr.Eos("ParameterRecord")) {
        rTr.Peek(token);  
        //DestinationNode
        if(token.IsBegin("DestinationNode")) {          
          rTr.Get(token);      
          if(token.ExistsAttributeInteger("value")) 
            record.destinationNode=token.AttributeIntegerValue("value");
          rTr.ReadEnd("DestinationNode");
          continue;
        }
        //DestinationChannel
        if(token.IsBegin("DestinationChannel")) {
          rTr.Get(token);      
          if(token.ExistsAttributeInteger("value")) 
            record.destinationChannel=token.AttributeIntegerValue("value");
          rTr.ReadEnd("DestinationChannel");
          continue;
        }
        //EligibilityTime
        if(token.IsBegin("EligibilityTime")) {
          rTr.Get(token);      
          if(token.ExistsAttributeInteger("value")) 
            record.eligibilityTime=token.AttributeIntegerValue("value");
          rTr.ReadEnd("EligibilityTime");
          continue;
        }
        //DeadlineTime
        if(token.IsBegin("DeadlineTime")) {
          rTr.Get(token);      
          if(token.ExistsAttributeInteger("value")) 
            record.deadlineTime=token.AttributeIntegerValue("value");
          rTr.ReadEnd("DeadlineTime");
          continue;
        }
        rTr.Get(token);
        rTr.ReadEnd(token.StringValue());
      }
      rTr.ReadEnd("ParameterRecord");
      continue;
    }
    // unknown
    break;
  }
}




/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeD3ripURTInput

 **********************************************
 **********************************************
 **********************************************
 */


// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeD3ripURTInput,AttributeVoid)


//DoAssign(Src)
void AttributeD3ripURTInput::DoAssign(const AttributeD3ripURTInput& rSrcAttr) {
  FD_DHV("AttributeD3ripURTInput(" << this << "):DoAssign(): assignment from " <<  &rSrcAttr);
  mEventId=rSrcAttr.mEventId;
}


//DoWrite(rTw,rLabel,pContext)
void AttributeD3ripURTInput::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
}//DoWrite(rTw,rLabel,pContext)


//DoRead(rTr,rLabel,pContext)
void AttributeD3ripURTInput::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  Token token;
  FD_DHV("AttributeD3ripURTOutput::DoRead()");
  // loop all D3ripURT parameters
  while(1) {
    rTr.Peek(token);
    if(!token.IsBegin()) return;
    if(token.IsBegin("EventId")) {
      rTr.Get(token);      
      if(token.ExistsAttributeInteger("value")) 
        mEventId=token.AttributeIntegerValue("value");
      rTr.ReadEnd("EventId");
      continue;
    }
 
    // unknown
    break;
  }
}



/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeD3ripURTEvent

 **********************************************
 **********************************************
 **********************************************
 */



// std faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeD3ripURTEvent,AttributeDeviceEvent)

// Default constructor, set my prototypes
AttributeD3ripURTEvent::AttributeD3ripURTEvent(void) : AttributeDeviceEvent() {
  FD_DHV("AttributeD3ripURTEvent::AttributeD3ripURTEvent(" << this << ")");
  pOutputPrototype=OutputPrototypep();
  pInputPrototype=InputPrototypep();
}

// Copy constructor 
AttributeD3ripURTEvent::AttributeD3ripURTEvent(const AttributeD3ripURTEvent& rOtherAttr) : 
  AttributeDeviceEvent() 
{
  FD_DHV("AttributeD3ripURTEvent(" << this << "): form other attr " <<  &rOtherAttr);
  pOutputPrototype=OutputPrototypep();
  pInputPrototype=InputPrototypep();
  DoAssign(rOtherAttr);
}


// autoregister event configuration (required for XML token format)
AutoRegisterType< TaNameSet<AttributeD3ripURTEvent> > 
  gRti1RegisterD3ripURTDeviceEventSet("D3ripURTDeviceEventSet");
AutoRegisterElementTag< TaNameSet<AttributeD3ripURTEvent> > 
  gRti1ElementTagD3ripURTDeviceEventSet("D3ripURTDeviceEventSet", "Event");

// pseudo statics
const AttributeD3ripURTOutput* AttributeD3ripURTEvent::OutputPrototypep(void){
  AttributeD3ripURTOutput* attrp= new AttributeD3ripURTOutput();
  return attrp;
}

// pseudo statics
const AttributeD3ripURTInput* AttributeD3ripURTEvent::InputPrototypep(void) {
  AttributeD3ripURTInput* attrp= new AttributeD3ripURTInput();
  return attrp;
}

/*
 **********************************************
 **********************************************
 **********************************************

 implementation: d3ripURTDevice

 **********************************************
 **********************************************
 **********************************************
 */


// autoregister
AutoRegisterType<d3ripURTDevice> gRtiRegisterD3ripURTDevice("D3ripURTDevice");

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(D3ripURTDevice,d3ripURTDevice,vDevice)
void* DoListenModule(void* threadArg);
  
////////////////////////////////////////////////
// construction and destruction

// constructor
d3ripURTDevice::d3ripURTDevice(void) : vDevice() {
  FD_DHV("d3ripURTDevice(" << mName << ")::d3ripURTDevice()");
  // have event set with appropriate attributes
  mpConfiguration = new TaNameSet<AttributeD3ripURTEvent>;
  pConfiguration = dynamic_cast< TaNameSet<AttributeD3ripURTEvent>* >(mpConfiguration);
  // have appropriate default label for token io
  mDefaultLabel ="D3ripURTDevice";
  // configuration defaults
  mName="D3ripURTDevice";
}


//~d3ripURTDevice()
d3ripURTDevice::~d3ripURTDevice(void) {
  // destructor
  FD_DHV("d3ripURTDevice(" << mName << ")::~d3ripURTDevice()");
}


// Clear()
void d3ripURTDevice::Clear(void) {
  //delete static data
  FD_DHV("d3ripURTDevice(" << mName << ")::Clear()");
  // call base; note: clear implies stop (incl. all eventg attributes)
  vDevice::Clear();
  // clear compiled data
  // xxxx
}


// Start(void)
void d3ripURTDevice::Start(void) {
  //set device state and create thread

  //return if device was allready started
  if( (mState == Up)||(mState == StartUp) )  return;
  FD_DHV("d3ripURTDevice("<<mName<<")::Start()");
  //call base incl virtual reset
  vDevice::Start();
  mState=StartUp;
  //create background for listening module
	if (mlockall(MCL_CURRENT | MCL_FUTURE)) {/*Lock all process memory pages in RAM(disable paging)*/
	  		perror("mlockall failed:");
	}
	struct sched_param scheduling_parameters;
	scheduling_parameters.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(0, SCHED_FIFO, &scheduling_parameters);

  NameSet::Iterator eit;
  for(eit=pConfiguration->Begin(); eit != pConfiguration->End(); eit++) {

    AttributeD3ripURTEvent attr;
    attr = pConfiguration->Attribute(*eit);    
    const AttributeD3ripURTOutput* oattr = attr.Outputp();
    if(!oattr) continue;

    FD_DHV("d3ripURTDevice("<<mName<<") demo: output idx  " << *eit);
    FD_DHV("d3ripURTDevice("<<mName<<") demo: output name " <<  pConfiguration->SymbolicName(*eit));
    FD_DHV("d3ripURTDevice("<<mName<<") demo: Channel To Transmit: " << oattr->mChannelToTransmit);
  }

  pthread_create(&mThreadListenModule, NULL, &DoListenCLModule, this);
  mState=Up;

}

// Stop()
void d3ripURTDevice::Stop(void) {
  //return if device allready got the command to shut down
  if(mState != Up && mState !=StartUp)  return;
  FD_DHV("d3ripURTDevice("<<mName<<")::Stop()");
  //set device state, kill thread and close message queues
  mContinueListening=0;
  mq_close(mMQueueToSend);
  mq_close(mMQueueToReceive);
  pthread_kill(mThreadListenModule, SIGTERM);
  // call base
  vDevice::Stop();
}//end Stop()


//Reset()
void d3ripURTDevice::Reset(void){
  //delete dynamic data
  FD_DHV("d3ripURTDevice("<<mName<<")::Reset()");
  // call base (resets time)
  vDevice::Reset();
  // xxxxx
}

//Compile(void)
void d3ripURTDevice::Compile(void){
  //setup up internal data structure
  mContinueListening=1;
  struct mq_attr wanted_attrs;
  wanted_attrs.mq_flags=0;
  wanted_attrs.mq_maxmsg=10;
  wanted_attrs.mq_msgsize=1500;
  wanted_attrs.mq_curmsgs=0;  

  mMQueueToSend = mq_open("/MQUEUE_AP2CL_ID", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG , &wanted_attrs);

  if(mMQueueToSend==-1 || mMQueueToReceive==-1) {
    FD_DHV("d3ripURTDevice("<<mName<<"): Cannot Open Message Queue");
  }

  int i=0;
  int j=0;
  NameSet::Iterator eit;
  for(eit=pConfiguration->Begin(); eit != pConfiguration->End(); eit++) {

    AttributeD3ripURTEvent attr;
    attr = pConfiguration->Attribute(*eit);    
    const AttributeD3ripURTOutput* oattr = attr.Outputp();
    const AttributeD3ripURTInput* iattr = attr.Inputp();

    if(oattr) {
      //maps a global EventId for all output event names for the consistency between controllers
      mEventIdMap[mOutputs.SymbolicName(*eit)] = oattr->mEventId;
      //header part
      mEventParameters[i][D3RIP_URT_EVENT_ID]=*eit;
      mEventParameters[i][D3RIP_URT_CHANNEL_TO_TRANSMIT]=oattr->mChannelToTransmit;
   
      //and the parameters
      for(j=0;j<oattr->mParameterRecords.size();j++) {
        mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_DESTIONATION_NODE]=oattr->mParameterRecords[j].destinationNode;
        mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_DESTIONATION_CHANNEL]=oattr->mParameterRecords[j].destinationChannel;
        mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_ELIGIBILITY_TIME]=oattr->mParameterRecords[j].eligibilityTime;
        mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_DEADLINE_TIME]=oattr->mParameterRecords[j].deadlineTime;
      }
      //set the parameter count field in the end
      mEventParameters[i][D3RIP_URT_PARAMETER_COUNT]=j;
      i++;
    }
    else if(iattr) {
      mEventIdMap[mInputs.SymbolicName(*eit)] = iattr->mEventId;
    }
    else 
      continue;
  }
  mEventCount=i;
  FD_DHV("d3ripURTDevice(" << mName << ")::Compile()");
  // call base
  vDevice::Compile();
}

// send/execute output event
void d3ripURTDevice::WriteOutput(Idx output) {
  int i;
  int j;
  int messageLength=0;
  mEventsToSendCount=0;
  for(i=0;i<mEventCount;i++) {
    if(mEventParameters[i][D3RIP_URT_EVENT_ID]==output){
      //populate the mCommunicationRequests array with Communication requests of the event(s)
      for(j=0;j<mEventParameters[i][D3RIP_URT_PARAMETER_COUNT];j++){
        
        mCommunicationRequests[mCommunicationRequestCount].destinationNode=
                                    mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_DESTIONATION_NODE];

        mCommunicationRequests[mCommunicationRequestCount].destinationChannel=
                                    mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_DESTIONATION_CHANNEL];

        mCommunicationRequests[mCommunicationRequestCount].eligibilityTime=
                                    mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_ELIGIBILITY_TIME];

        mCommunicationRequests[mCommunicationRequestCount].deadlineTime=  
                                    mEventParameters[i][D3RIP_URT_HEADER_SIZE+j*D3RIP_URT_PARAMETER_SIZE+D3RIP_URT_DEADLINE_TIME];


        mCommunicationRequestCount++;
      }
      //store Ids for event(s) in unsigned char format
      mEventIdsToSend[mEventsToSendCount]=output;
      mEventsToSendCount++;
      break;
    }
  }
  FlushOutput(1);
}

void d3ripURTDevice::FlushOutput(unsigned char channel) {
    int i;   
    int msgLength;
    char msgToSend[D3RIP_RT_MESSAGE_MAX_LENGTH];
    msgToSend[0]=1;
    msgToSend[1]=mCommunicationRequestCount;
    for(i=0;i<mCommunicationRequestCount;i++){
      msgToSend[2+D3RIP_URT_DESTIONATION_NODE+i*D3RIP_URT_PARAMETER_SIZE]=mCommunicationRequests[i].destinationNode;
      msgToSend[2+D3RIP_URT_DESTIONATION_CHANNEL+i*D3RIP_URT_PARAMETER_SIZE]=mCommunicationRequests[i].destinationChannel;
      msgToSend[2+D3RIP_URT_ELIGIBILITY_TIME+i*D3RIP_URT_PARAMETER_SIZE]=mCommunicationRequests[i].eligibilityTime;
      msgToSend[2+D3RIP_URT_DEADLINE_TIME+i*D3RIP_URT_PARAMETER_SIZE]=mCommunicationRequests[i].deadlineTime;
    }

    msgToSend[2+mCommunicationRequestCount*D3RIP_URT_PARAMETER_SIZE]=mEventsToSendCount;
    for(i=0;i<mEventsToSendCount;i++) {
      msgToSend[i+3+mCommunicationRequestCount*D3RIP_URT_PARAMETER_SIZE]=mEventIdMap[mOutputs.SymbolicName(mEventIdsToSend[i])];
      //printf("Event Send! Idx: %d",mEventIdMap[mOutputs.SymbolicName(mEventIdsToSend[i])]);
    }

    msgLength=i+3+mCommunicationRequestCount*D3RIP_URT_PARAMETER_SIZE;
    if(msgLength>D3RIP_RT_MESSAGE_MAX_LENGTH){
      FD_DHV("RT Message Length exceeds MAX limit!");
      return;
    }      
    mq_send(mMQueueToSend,(char*)&msgToSend[0],msgLength,0);

    //reset counters for events and CRs
    mCommunicationRequestCount=0;    
    mEventsToSendCount=0;
}


//DoWritePreface(rTw,rLabel,pContext)
void d3ripURTDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel,  const Type* pContext) const{
  FD_DHV("d3ripURTDevice("<<mName<<")::DoWritePreface()");
  //call base
  vDevice::DoWritePreface(rTw,rLabel,pContext);
}

//DoReadPreface(rTr,rLabel,pContext)
void d3ripURTDevice::DoReadPreface(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  FD_DHV("d3ripURTDevice("<<mName<<")::DoReadPreface()");
  // call base
  vDevice::DoReadPreface(rTr,rLabel,pContext);
}

//DoListenCLModule(d3ripURTDevice)
void* DoListenCLModule(void* threadArg){
  d3ripURTDevice* pD3ripURTDevice = (d3ripURTDevice*)threadArg;
  int i;  
  int length;
  int communicationRequestCount;
  int eventIdCount;
  unsigned char receivedEventId;
  char msg[D3RIP_RT_MESSAGE_MAX_LENGTH];
  struct timespec timeout;
  mq_attr attr;
  mq_getattr(pD3ripURTDevice->mMQueueToReceive,&attr);
  std::map<std::string,int>::iterator ii;
  int evIndexToProcess;
  struct timespec ts,tr;
  ts.tv_sec = 1;
  ts.tv_nsec = 0; 
  nanosleep(&ts, &tr); 
  ts.tv_sec = 0;
  ts.tv_nsec = 5000; 

  fflush(stdout);
  mqd_t mMQueueToReceive = mq_open("/MQUEUE_CL2AP_ID", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG , 0);
  fflush(stdout);
  for(;;) {	

    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 1; 
    length = mq_receive(mMQueueToReceive, msg, 1500, 0); 
    nanosleep(&ts, &tr);
    if(length > 0)
    {
      FD_DHV("CL2AP received");
      communicationRequestCount=msg[0];
      if(communicationRequestCount>0) {
	 	    fflush(stdout);
      }
	    
      eventIdCount=msg[1+communicationRequestCount*D3RIP_URT_PARAMETER_SIZE];
      for(i=0;i<eventIdCount;i++){        
        receivedEventId=msg[i+2+communicationRequestCount*D3RIP_URT_PARAMETER_SIZE];
        fflush(stdout);
        for( ii=pD3ripURTDevice->mEventIdMap.begin(); ii!=pD3ripURTDevice->mEventIdMap.end(); ++ii)
        {
          if((*ii).second==receivedEventId) {
            evIndexToProcess=pD3ripURTDevice->mInputs.Index((*ii).first.c_str());
		        if(pD3ripURTDevice->mInputs.Exists(evIndexToProcess)) {
              pthread_mutex_lock(pD3ripURTDevice->pBufferMutex);
       	     pD3ripURTDevice->pInputBuffer->push_back(evIndexToProcess);
          	  pthread_mutex_unlock(pD3ripURTDevice->pBufferMutex); 
              //printf("event received! %d->%d\n",receivedEventId,evIndexToProcess);
              pthread_mutex_lock(pD3ripURTDevice->pWaitMutex);
              pthread_cond_broadcast(pD3ripURTDevice->pWaitCondition);
              pthread_mutex_unlock(pD3ripURTDevice->pWaitMutex);
              //printf("event notification sent!\n");
              fflush(stdout);
            }
          }
        }     
      }
    }
    if( !pD3ripURTDevice->mContinueListening ) {
      FD_DHV("Stopping the CL Listener");
      break;
    };
  }
  return NULL;
}

}//end namespace faudes

#endif // configure


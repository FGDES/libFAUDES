/** @file iop_simplenet.cpp Simple networked events via tcp/ip */

/* 
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008, Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/



#include "iop_simplenet.h"


namespace faudes {


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: SimplenetAddress

 **********************************************
 **********************************************
 **********************************************
 */

// std faudes
FAUDES_TYPE_IMPLEMENTATION(SimplenetDevice,nDevice,vDevice)

// construct
SimplenetAddress::SimplenetAddress(void) {
  mIp="";
  mPort=-1;
}

// copy construct
SimplenetAddress::SimplenetAddress(const SimplenetAddress& rOther) {
  mIp=rOther.mIp;
  mPort=rOther.mPort;
}

// construct from string
SimplenetAddress::SimplenetAddress(const std::string& rString) {
  IpColonPort(rString);
}

// validity
bool SimplenetAddress::Valid(void) const {
  if(mPort<=0) return false;
  if(mIp=="") return false;
  if(mIp.find(':',0)!=std::string::npos) return false;
  return true;
}  

// get colon seperated string 
std::string SimplenetAddress::IpColonPort(void) const {
  std::string res;
  if(!Valid()) return res;
  res=mIp+":"+ToStringInteger(mPort);
  return res;
}

// Set from colon seperated string 
void SimplenetAddress::IpColonPort(std::string ipcolonport) {
  FD_DHV("SimplenetAddress::IpColonPort(): " << ipcolonport << " --> ?");
  // invalid
  mIp="";
  mPort=-1;
  // find colon
  std::size_t cpos = ipcolonport.find(':',0);
  if(cpos==std::string::npos) return;
  if(cpos==0) return;
  if(cpos+1>= ipcolonport.length()) return;
  // extract ip
  mIp=ipcolonport.substr(0,cpos);
  // extract port
  mPort=ToIdx(ipcolonport.substr(cpos+1));
  // report
  FD_DHV("SimplenetAddress::IpColonPort(): " << ipcolonport << " --> " << IpColonPort());
  // test for errors (might be too strict)
  if(IpColonPort()!=ipcolonport) {
    mIp="";
    mPort=-1;
  }
}
  
// sorting
bool SimplenetAddress::operator < (const SimplenetAddress& rOther) const {
  if(this->mIp < rOther.mIp) return true;
  if(this->mIp > rOther.mIp) return false;
  if(this->mPort < rOther.mPort) return true;
  return false;
}


// only compile for simplenet support
#ifdef FAUDES_IODEVICE_SIMPLENET

/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeSimplenetOutput

 **********************************************
 **********************************************
 **********************************************
 */

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSimplenetOutput,AttributeVoid)


//DoWrite(rTw);
void AttributeSimplenetOutput::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
}


//DoRead(rTr)
void AttributeSimplenetOutput::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  // report
  FD_DHV("AttributeSimplenetOutput(" << this << ")::DoRead(tr)");

  // sense and digest pre 2.16 format
  Token token;
  rTr.Peek(token);
  if(token.Type()==Token::Begin) 
  if(token.StringValue()=="Output") {
    rTr.ReadBegin("Output");
    rTr.ReadEnd("Output");
  }
}


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeSimplenetInput

 **********************************************
 **********************************************
 **********************************************
 */


// std faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSimplenetInput,AttributeVoid)


//DoWrite(rTw);
void AttributeSimplenetInput::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
}


//DoRead(rTr)
void AttributeSimplenetInput::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  // report
  FD_DHV("AttributeSimplenetInput(" << this << ")::DoRead(tr)");

  // sense and digest pre 2.16 format
  Token token;
  rTr.Peek(token);
  if(token.Type()==Token::Begin) 
  if(token.StringValue()=="Input") {
    rTr.ReadBegin("Input");
    rTr.ReadEnd("Input");
  }
}




/*
 **********************************************
 **********************************************
 **********************************************

 implementation: AttributeSimplenetEvent

 **********************************************
 **********************************************
 **********************************************
 */



// std faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSimplenetEvent,AttributeDeviceEvent)

// Default constructor, set my prototypes
AttributeSimplenetEvent::AttributeSimplenetEvent(void) : AttributeDeviceEvent() {
  FD_DHV("AttributeSimplenetEvent::AttributeSimplenetEvent(" << this << ")");;
  pOutputPrototype=OutputPrototypep();
  pInputPrototype=InputPrototypep();
}

// Copy constructor 
AttributeSimplenetEvent::AttributeSimplenetEvent(const AttributeSimplenetEvent& rOtherAttr) : 
  AttributeDeviceEvent() 
{
  FD_DHV("AttributeSimplenetEvent(" << this << "): form other attr " <<  &rOtherAttr);
  pOutputPrototype=OutputPrototypep();
  pInputPrototype=InputPrototypep();
  DoAssign(rOtherAttr);
}


// pseudo statics
const AttributeSimplenetOutput* AttributeSimplenetEvent::OutputPrototypep(void){
  AttributeSimplenetOutput* attrp= new AttributeSimplenetOutput();
  return attrp;
}

// pseudo statics
const AttributeSimplenetInput* AttributeSimplenetEvent::InputPrototypep(void) {
  AttributeSimplenetInput* attrp= new AttributeSimplenetInput();
  return attrp;
}


/*
 **********************************************
 **********************************************
 **********************************************

 implementation: nDevice

 **********************************************
 **********************************************
 **********************************************
 */

// autoregister
AutoRegisterType<nDevice> gRtiRegisterSimplenetDevice("SimplenetDevice");

// helper: send entire buffer
int syncSend(int dest, const char* data, int len, int flag) {
  int from=0;
  int left=len;
  while(left>0) {
     int rc=send(dest, data+from, left, 0);
     if(rc<0) {
       std::stringstream errstr;
       errstr << "Simplenet fatal network error (cannot send message)";
       throw Exception("nDevice::syncSend", errstr.str(), 553, true); // mute console out
     }
    left-=rc;
  }
  return len;
}


// constructor
nDevice::nDevice(void) : vDevice() {
  FD_DHV("nDevice(" << this << ")::nDevice()");
  mpConfiguration = new TaNameSet<AttributeSimplenetEvent>;
  pConfiguration = dynamic_cast< TaNameSet<AttributeSimplenetEvent>* >(mpConfiguration);
  // default token section 
  mDefaultLabel="SimplenetDevice";
  // default network data
  mName="SimplenetNode";
  mNetwork="Simplenet";
  mListenAddress.IpColonPort("localhost:40000");
  mBroadcastAddress.IpColonPort("255.255.255.255:40000");
  // clear/prepare state
  faudes_mutex_init(&mMutex); 	
  mListenSocket=-1;
  mBroadcastSocket=-1;
}

// destructor
nDevice::~nDevice(void) {
  FD_DHV("nDevice(" << this << ")::~nDevice()");
  // stop
  Stop();
  // clean up my data
  faudes_mutex_destroy(&mMutex);
}

// clear all configuration
void nDevice::Clear(void) {
  FD_DHV("nDevice(" << this << ")::Clear()");
  // call base, incl stop
  vDevice::Clear();
  // clear compiled data
  mInputSubscriptions.clear();
}


// programmatic config: server address
void nDevice::ServerAddress(const std::string& rAddr) {
  if(mState!=Down) return;
  mListenAddress.IpColonPort(rAddr);
}

// programmatic config: broadcast address
void nDevice::BroadcastAddress(const std::string& rAddr) {
  if(mState!=Down) return;
  mBroadcastAddress.IpColonPort(rAddr);
}


// programmatic config: network name
void nDevice::NetworkName(const std::string& rNetwork) {
  if(mState!=Down) return;
  mNetwork=rNetwork;
}

// programmatic config: insert node name
void nDevice::InsNode(const std::string& rNodeName) {
  if(mState!=Down) return;
  mNetworkNodes[rNodeName]="unknown:0";
}

// programmatic config: insert node address
void nDevice::InsNodeAddress(const std::string& rNodeName, const std::string& rNodeAddress) {
  if(mState!=Down) return;
  mNetworkNodes[rNodeName]=rNodeAddress;
}

// programmatic config: clear known nodes
void nDevice::ClearNodes(void) {
  if(mState!=Down) return;
  mNetworkNodes.clear();
}

// programmatic config: insert input event
void nDevice::InsInputEvent(const std::string& event) {
  if(mState!=Down) return;
  AttributeSimplenetEvent inp;
  inp.DefaultInput();
  Idx ev=pConfiguration->Insert(event);
  pConfiguration->Attribute(ev, inp);
}

// programmatic config: insert output event
void nDevice::InsOutputEvent(const std::string& event) {
  if(mState!=Down) return;
  AttributeSimplenetEvent outp;
  outp.DefaultOutput();
  Idx ev=pConfiguration->Insert(event);
  pConfiguration->Attribute(ev, outp);
}


//Compile(void)
void nDevice::Compile(void){
  //setup up internal data structure
  FD_DHV("nDevice(" << this << ")::Compile()");
  // call base
  vDevice::Compile();
}


//DoWritePreface(rTr,rLabel)
void nDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  FD_DHV("nDevice::DoWrite()");
  //call base
  vDevice::DoWritePreface(rTw,rLabel,pContext);
  // write my data: my server role ip address
  Token vtag;
  vtag.SetEmpty("ServerAddress");
  vtag.InsAttributeString("value",mListenAddress.IpColonPort());
  rTw<<vtag;
  // write my data: my server broadcast address
  Token btag;
  btag.SetEmpty("BroadcastAddress");
  btag.InsAttributeString("value",mBroadcastAddress.IpColonPort());
  rTw<<btag;
  // write my data network topology
  Token ntag;
  ntag.SetBegin("Network");
  ntag.InsAttributeString("name",mNetwork);
  rTw<<ntag;
  std::map<std::string,std::string>::const_iterator nit;
  for(nit=mNetworkNodes.begin();nit!=mNetworkNodes.end();nit++) {
    vtag.SetEmpty("Node");
    vtag.InsAttributeString("name",nit->first);
    SimplenetAddress defaddress(nit->second);
    if(defaddress.Valid()) 
      vtag.InsAttributeString("address",nit->second);
    rTw<<vtag;
  }
  rTw.WriteEnd("Network");
}

//DoRead(rTr,rLabel)
void nDevice::DoReadPreface(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  FD_DHV("nDevice::DoReadPreface()");
  // call base (reads name and timescale)
  vDevice::DoReadPreface(rTr,rLabel,pContext);

  // sense and digest pre 2.16 format
  Token token;
  rTr.Peek(token);
  if(token.IsString()) {
    mListenAddress.IpColonPort(rTr.ReadString());
    if(!mListenAddress.Valid()) {
      std::stringstream errstr;
      errstr << "Simplenet address expected at " << rTr.FileLine();
      throw Exception("nDevice::DoRead", errstr.str(), 50);
    }
    mNetwork=rTr.ReadString();
    mNetworkNodes.clear();
    rTr.ReadBegin("Network");
    while(!rTr.Eos("Network")) {
      mNetworkNodes[rTr.ReadString()]="unknown:0";
    }
    rTr.ReadEnd("Network");
    return;
  }

  // read my data: server address
  Token atag;
  rTr.ReadBegin("ServerAddress",atag);
  mListenAddress.IpColonPort(atag.AttributeStringValue("value"));
  if(!mListenAddress.Valid()) {
    std::stringstream errstr;
    errstr << "Simplenet address expected at " << rTr.FileLine();
    throw Exception("nDevice::DoRead", errstr.str(), 50);
  }
  rTr.ReadEnd("ServerAddress");
  // read my data: broadcast address (optional)
  mBroadcastAddress.IpColonPort("255.255.255.255:40000");
  rTr.Peek(token);
  if(token.IsBegin("BroadcastAddress")) {
    rTr.ReadBegin("BroadcastAddress",atag);
    mBroadcastAddress.IpColonPort(atag.AttributeStringValue("value"));
    if(!mBroadcastAddress.Valid()) {
      std::stringstream errstr;
      errstr << "Simplenet address expected at " << rTr.FileLine();
      throw Exception("nDevice::DoRead", errstr.str(), 50);
    }
    rTr.ReadEnd("BroadcastAddress");
  }
  // read my data: network
  Token ntag;
  rTr.ReadBegin("Network",ntag);
  mNetwork=ntag.AttributeStringValue("name");
  // loop network nodes
  while(!rTr.Eos("Network")) {
    rTr.ReadBegin("Node",ntag);
    std::string node=ntag.AttributeStringValue("name");
    InsNode(node);
    // undocumented feature: explicit server addresses in dev file; tmoor 20121113
    if(ntag.ExistsAttributeString("address")) {
      SimplenetAddress defaddress;
      defaddress.IpColonPort(ntag.AttributeStringValue("address"));
      if(!defaddress.Valid()) {
        std::stringstream errstr;
        errstr << "Simplenet address expected at " << rTr.FileLine();
        throw Exception("nDevice::DoRead", errstr.str(), 50);
      }
      mNetworkNodes[node]=defaddress.IpColonPort();
    }
    rTr.ReadEnd("Node");
  }
  rTr.ReadEnd("Network");
}


// lock - unlock shortcuts;
#define LOCK_E  {int rc = faudes_mutex_lock(&mMutex); \
  if(rc) {FD_ERR("nDevice::LOCK_E: lock mutex error\n"); exit(1); }}		
#define UNLOCK_E  {int rc = faudes_mutex_unlock(&mMutex); \
  if(rc) {FD_ERR("nDevice::LOCK_E: unlock mutex error\n"); exit(1); }}		
#define TLOCK_E  {int rc = faudes_mutex_lock(&ndevice->mMutex); \
  if(rc) {FD_ERR("nDevice::TLOCK_E: lock mutex error\n"); exit(1); }}		
#define TUNLOCK_E  {int rc = faudes_mutex_unlock(&ndevice->mMutex); \
  if(rc) {FD_ERR("nDevice::TLOCK_E: unlock mutex error\n"); exit(1); }}		


// Write Output 
void nDevice::WriteOutput(Idx output) {

  FD_DHV("nDevice::WriteOutput(" << mOutputs.SymbolicName(output) << ")");

  // bail out (do notify clients even when servers incomplete)
  if(mState!=Up && mState!=StartUp) return;

  // test event
  if(!mOutputs.Exists(output)) {
    std::stringstream errstr;
    errstr << "Unknown output event " << output;
    throw Exception("nDevice::WriteOutput", errstr.str(), 65);
  }

  // find properties
  const AttributeSimplenetOutput* aattr = pConfiguration->Attribute(output).Outputp();
  if(!aattr) {
    std::stringstream errstr;
    errstr << "Invalid output attribute " << output;
    throw Exception("nDevice::WriteOutput", errstr.str(), 65);
  }

  // report
  std::string message= "<Notify> " + mOutputs.SymbolicName(output) + " </Notify>\n";
  FD_DHV("nDevice::WriteOutput(): message: " << message.substr(0,message.length()-1));
  
  // send event to those clients that did subscribe 
  LOCK_E;
  int clientsock=-1;
  try {
    std::map<int,ClientState>::iterator sit=mOutputClientStates.begin();
    for(;sit!=mOutputClientStates.end();sit++) {
      if(!sit->second.mEvents.Empty())
        if(!sit->second.mEvents.Exists(output))
          continue;
      clientsock=sit->second.mClientSocket; 
      if(clientsock>0) {
        FD_DHV("nDevice::WriteOutput(): to socket " << clientsock);
        syncSend(clientsock, message.c_str(), message.length(), 0);
      }
    }
  } catch (faudes::Exception&) {
    FD_DH("nDevice::WriteOutput(): failed to notify client on socket " << clientsock);
  }
  UNLOCK_E;
  FD_DHV("nDevice::WriteOutput(): done");
}



// Start(void)
void nDevice::Start(void) {
  if(mState!=Down) return;
  FD_DH("nDevice(" << mName <<")::Start()");
  // call base
  vDevice::Start();
  mState=StartUp;
  // clear event server states
  mInputServerStates.clear();
  std::map<std::string,std::string>::iterator nit;
  for(nit=mNetworkNodes.begin(); nit!=mNetworkNodes.end();nit++) {
    if(nit->first == mName) continue;
    mInputServerStates[nit->first].mAddress= SimplenetAddress(nit->second);
    mInputServerStates[nit->first].mEvents= EventSet();
    mInputServerStates[nit->first].mServerSocket=-1;
    mInputServerStates[nit->first].mLineBuffer="";
  }
  // clear client states
  mOutputClientStates.clear();
  // set my effective address
  char hostname[1024];
  int hostname_len =1023;
  if(gethostname(hostname,hostname_len)!=0) {
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot get hostname)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }
  hostname[hostname_len]=0;
  mEffectiveListenAddress=mListenAddress;
  mEffectiveListenAddress.Ip(hostname);
  FD_DH("nDevice::Start(): server adress " << mEffectiveListenAddress.IpColonPort());
  // open a tcp port to listen: create socket
  mListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(mListenSocket<=0) {
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot open server socket)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }
  int reuse=1;
  faudes_setsockopt(mListenSocket,SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));  
  // open a tcp port to listen: set up address
  struct sockaddr_in serveraddr;
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
  serveraddr.sin_port = htons(mListenAddress.Port());     
  // open a tcp port to listen: bind socket to address
  if(bind(mListenSocket, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) <0) {
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot bind socket)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }
  // open a tcp port to listen: start to listen
  if(listen(mListenSocket, 77) < 0) {  // todo: max pending connections
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot listen from socket)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }
  // open a udp port to listen: create socket
  mBroadcastSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(mBroadcastSocket<=0) {
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot open broadcast socket)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }
  //int reuse=1;
  faudes_setsockopt(mBroadcastSocket,SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));  
  faudes_setsockopt(mBroadcastSocket,SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));  
  // open a udp port: enable broadcast 
  // int reuse
  if(faudes_setsockopt(mBroadcastSocket, SOL_SOCKET, SO_BROADCAST, &reuse, sizeof(reuse)) ) {
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot setopt broadcast socket)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }  
  // open a udp port to listen: set up address
  struct sockaddr_in broadcastaddr;
  memset(&broadcastaddr, 0, sizeof(broadcastaddr));
  broadcastaddr.sin_family = AF_INET;
  broadcastaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
  broadcastaddr.sin_port = htons(mBroadcastAddress.Port());     
  // open a udp port to listen: bind socket to address
  if(bind(mBroadcastSocket, (struct sockaddr *) &broadcastaddr,sizeof(broadcastaddr)) <0) {
    std::stringstream errstr;
    errstr << "Simplenet fatal network error (cannot bind broadcast socket)";
    throw Exception("nDevice::Start", errstr.str(), 553);
  }
  // start background thread to listen: create & run thread
  mStopListen=false;
  int rc  = faudes_thread_create(&mThreadListen, NDeviceListen, this);
  // thread error 
  if(rc) {
    std::stringstream errstr;
    errstr << "Simplenet fatal thread error (cannot create thread)";
    throw Exception("nDevice::Start", errstr.str(), 554);
  }
}

// Stop(void)
void nDevice::Stop(void) {
  // bail out
  if(mState!=Up && mState!=StartUp) return;
  FD_DH("nDevice::Stop()");
  LOCK_E;
  // stop background threads
  mStopListen=true;
  UNLOCK_E;
  // signal update to my listen thread: via udp message ... release select
  std::string message= "<Stop> " + mNetwork + " " + mName + " </Stop>\n";
  struct sockaddr_in broadcastaddr;
  memset(&broadcastaddr, '\0', sizeof(broadcastaddr));
  broadcastaddr.sin_family=AF_INET;
  broadcastaddr.sin_port=htons(mBroadcastAddress.Port());
  broadcastaddr.sin_addr.s_addr=inet_addr(mBroadcastAddress.Ip().c_str());
  LOCK_E;
  sendto(mBroadcastSocket,message.c_str(),message.length(),0,
     (struct sockaddr *) & broadcastaddr, sizeof(broadcastaddr));
  UNLOCK_E;
  // wait until listen thread finished 
  FD_DH("nDevice::Stop(): waiting for listen thread");
  faudes_thread_join(mThreadListen, NULL);
  FD_DH("nDevice::Stop(): listen thread finished");
  // close broadcast socket
  shutdown(mBroadcastSocket,2);
  faudes_closesocket(mBroadcastSocket);
  mBroadcastSocket=-1;
  // close server socket 
  shutdown(mListenSocket,2);
  faudes_closesocket(mListenSocket);
  mListenSocket=-1;
  // call base (implies reset)
  vDevice::Stop();
}



// background thread, 
// - receiving requests on broadcast port 
// - sending requests broadcats
// - accept connections on listen port for output clients 
// - notify connected output clients about events
// - connecting to input servers to receiving notifications
void* NDeviceListen(void* arg){
  bool term;
  std::map<std::string,nDevice::ServerState>::iterator sit;
  std::map<int,nDevice::ClientState>::iterator cit;
  // cast this object
  nDevice* ndevice= static_cast<nDevice*>(arg);
  // say hello
  FD_DH("nDevice::Listen(" << ndevice << ")");
  // clear broadcast time stamp
  faudes_systime_t lastbroadcast; 	
  lastbroadcast.tv_sec=0;
  lastbroadcast.tv_nsec=0;
#ifdef FAUDES_DEBUG_IODEVICE
  // clear debugging time stamp
  int debuglisten=0;
#endif

  // infinite loop 
  while(true){

    // detect missing servers
    int servermis=0;
    int serverunknown=0;
    for(sit=ndevice->mInputServerStates.begin(); sit!=ndevice->mInputServerStates.end(); sit++) {
      // have no address?
      if(!sit->second.mAddress.Valid()) {
        FD_DH("nDevice::Listen(): missing server address for node: " << sit->first);
        serverunknown++;
        servermis++;
        continue;
      }
      // have no connection
      if(sit->second.mServerSocket<=0) {
        FD_DH("nDevice::Listen(): missing server connection for node: " << sit->first);
        servermis++;
        continue;
      }
    }

    // detect missing clients (extension 2.22i, trust by number, should ask nodename on subscription)
    int clientmis= ndevice->mNetworkNodes.size()-1;
    for(cit=ndevice->mOutputClientStates.begin(); cit!=ndevice->mOutputClientStates.end(); cit++) {
      if(cit->second.mClientSocket<0) continue;
      if(cit->second.mConnected) clientmis--;
    }
#ifdef FAUDES_DEBUG_IODEVICE
    if(clientmis!=servermis)
      FD_DH("nDevice::Listen(): missing clients to subscribe: #"<< clientmis);
#endif

    // update state
    if((servermis>0 || clientmis>0) && ndevice->mState==vDevice::Up) {
      TLOCK_E;
      ndevice->mState=vDevice::StartUp;
      TUNLOCK_E;
    }
    if(servermis==0 && clientmis==0 && ndevice->mState==vDevice::StartUp) {
      TLOCK_E;
      ndevice->mState=vDevice::Up;
      TUNLOCK_E;
    }


    // try to find input servers
    if(serverunknown>0 && ndevice->mState==vDevice::StartUp) {
      // is a broadcast due? (period 5sec)
      faudes_systime_t now; 	
      faudes_gettimeofday(&now);
      faudes_mstime_t diffms;
      faudes_diffsystime(now,lastbroadcast,&diffms);
      if(diffms>5000) {
        // udp message
        std::string message= "<Request> " 
          + ndevice->mNetwork + " " + ndevice->mName + " </Request>\n";
        // udp broadcast
        struct sockaddr_in broadcastaddr;
        memset(&broadcastaddr, '\0', sizeof(broadcastaddr));
        broadcastaddr.sin_family=AF_INET;
        broadcastaddr.sin_port=htons(ndevice->mBroadcastAddress.Port());
        broadcastaddr.sin_addr.s_addr=inet_addr(ndevice->mBroadcastAddress.Ip().c_str());
        TLOCK_E;
        int rc=sendto(ndevice->mBroadcastSocket,message.c_str(),message.length(),
          0,(struct sockaddr *) & broadcastaddr, sizeof(broadcastaddr));
        (void) rc;
        FD_DH("nDevice::Listen(): broadcast request: " <<  message.substr(0,message.length()-1) << " #" << rc);
        TUNLOCK_E;
        faudes_gettimeofday(&lastbroadcast);
      }
    }

    // subscribe to missing servers
    for(sit=ndevice->mInputServerStates.begin(); sit!=ndevice->mInputServerStates.end(); sit++) {
      // have active connection?
      if(sit->second.mServerSocket>0) continue;
      // have no address?
      if(!sit->second.mAddress.Valid()) continue;
      // try to connect
      FD_DH("nDevice::Listen(): subscribing to " << sit->first << 
          " at " << sit->second.mAddress.IpColonPort());
      // open a tcp port: create socket
      int serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
      if(serversock<=0) {
        FD_DH("nDevice::Listen(): subscription failed: no socket");
        continue;
      }
      // open a tcp port: set up internet address
      unsigned long int serverinaddr = INADDR_NONE;
      if(serverinaddr==INADDR_NONE) {
        FD_DH("nDevice::Listen(): using address as advertised");
        serverinaddr = inet_addr(sit->second.mAddress.Ip().c_str());
      }
      if(serverinaddr==INADDR_NONE) {
        struct hostent *host;
        host = gethostbyname(sit->second.mAddress.Ip().c_str());
        if(host!=0) {
          FD_DH("nDevice::Listen(): using address by name lookup");
          serverinaddr = *(unsigned long int*) host->h_addr;
        }
      }
      if(serverinaddr==INADDR_NONE) {
        FD_DH("nDevice::Listen(): subscription failed: invalid address " << sit->second.mAddress.Ip());
        faudes_closesocket(serversock);
        continue;
      }
      // open a tcp port: set up socket address 
      struct sockaddr_in serveraddress;
      memset(&serveraddress, 0, sizeof(serveraddress));
      serveraddress.sin_family = AF_INET;
      serveraddress.sin_addr.s_addr=serverinaddr; 
      serveraddress.sin_port = htons(sit->second.mAddress.Port());     
      // open a tcp port: connect
      if(connect(serversock, (struct sockaddr*) &serveraddress, sizeof(serveraddress))<0) {
        FD_DH("nDevice::Listen(): subscription failed: connect");
        faudes_closesocket(serversock);
        continue;
      }    
      // say hello to remote input server
      try {
        std::string hello;
        hello="% Simplenet universal event subscription: "+ndevice->mName+" subscribing from "+sit->first+"\n"; 
        syncSend(serversock, hello.c_str(), hello.length(), 0);
        hello="% Expecting notifications in format '<Notify> event_name </Notify>'\n";
        syncSend(serversock, hello.c_str(), hello.length(), 0);
        hello="% Trying to subscribe to all required events\n";
        syncSend(serversock, hello.c_str(), hello.length(), 0);
      } catch (faudes::Exception&) {
        faudes_closesocket(serversock);
	serversock=-1;
      }
      if(serversock<0) {
        FD_DH("nDevice::Listen(): subscription failed: cannot write");
        faudes_closesocket(serversock);
        continue;
      }    
      // record success 
      FD_DH("nDevice::Listen(): subscribing to " << sit->first << " via socket " << serversock);
      sit->second.mServerSocket=serversock;
      // subscribe to all input events
      EventSet sevents=ndevice->Inputs();
      sevents.Name("Subscribe");
      std::string message=sevents.ToString() + "\n";
      syncSend(serversock,message.c_str(), message.length(),0); 
      // used to get info in pre 2.22h 
      /*
      hello="% Going to Sending info command, explicit subscription may follow\n";
      hello="<Cmd> Info </Cmd>\n";
      syncSend(serversock, hello.c_str(), hello.length(), 0);
      */
      FD_DH("nDevice::Listen(): subscribing to " << sit->first << " via socket " << serversock << ": ok");
    }


    // prepare relevant wait on sources ... 
    fd_set mysocks;
    int mysocks_max=0;
    FD_ZERO(&mysocks);
    // ... my server listen socket, expecting other nodes to connect and subscribe
    if(mysocks_max<ndevice->mListenSocket) mysocks_max=ndevice->mListenSocket;
    if(mysocks_max>= FD_SETSIZE) FD_ERR("NDeviceListen: fail to select socket " << mysocks_max);
    FD_SET(ndevice->mListenSocket, &mysocks);
    // ... udp port, expecting requests and adverts 
    if(mysocks_max< ndevice->mBroadcastSocket) mysocks_max=ndevice->mBroadcastSocket;
    if(mysocks_max>= FD_SETSIZE) FD_ERR("NDeviceListen: fail to select socket " << mysocks_max);
    FD_SET(ndevice->mBroadcastSocket, &mysocks);
    // ... input server connections, expecting notifications
    for(sit=ndevice->mInputServerStates.begin(); sit!=ndevice->mInputServerStates.end(); sit++) {
      int serversock=sit->second.mServerSocket;
      if(serversock<0) continue;
      if(mysocks_max< serversock) mysocks_max=serversock;
      if(mysocks_max>= FD_SETSIZE) FD_ERR("NDeviceListen: fail to select socket " << mysocks_max);
      FD_SET(serversock, &mysocks);
    }
    // ... output client connections, expecting commands
    for(cit=ndevice->mOutputClientStates.begin(); cit!=ndevice->mOutputClientStates.end(); cit++) {
      int clientsock=cit->second.mClientSocket;
      if(clientsock<0) continue;
      if(mysocks_max< clientsock) mysocks_max=clientsock;
      if(mysocks_max>= FD_SETSIZE) FD_ERR("NDeviceListen: fail to select socket " << mysocks_max);
      FD_SET(clientsock, &mysocks);
    }

    // wait for traffic with moderate timeout 
    struct timeval tv;
    tv.tv_sec =  1;
    tv.tv_usec = 0;
    int avail=select(mysocks_max+1, &mysocks, NULL, NULL, &tv);

    // terminate thread on request (before accepting incomming connections)
    TLOCK_E;
    term= ndevice->mStopListen;
    TUNLOCK_E; 
    if(term) break;

    // reduce debugging output
    #ifdef FAUDES_DEBUG_IODEVICE
    debuglisten++;
    if((debuglisten>10) || (avail>0)) {    
      FD_DH("nDevice::Listen(): listen as node \"" << ndevice->mName << "\" on network \"" << ndevice->mNetwork << "\"" << " #" << avail);
      debuglisten=0;
    }
    #endif

    // handle incomming connection requests
    if(avail>0)  
      if(FD_ISSET(ndevice->mListenSocket,&mysocks)) {
      avail--;
      int clientsock=-1;
      struct sockaddr_in clientaddr;
      socklen_t clientaddr_len = sizeof(clientaddr);
      clientsock=accept(ndevice->mListenSocket, (struct sockaddr *) &clientaddr, &clientaddr_len );
      if(clientsock<0) {
        FD_DH("nDevice::Listen(): failed to accept incomming connection");
        break;
      }
      FD_DH("nDevice::Listen(): accepted connection from client " << inet_ntoa(clientaddr.sin_addr) << 
        " on socket " << clientsock);
      // say hello
      try {
        std::string hello;
        hello="% Simplenet Event Server: "+ndevice->mName+" providing events\n";
        syncSend(clientsock, hello.c_str(), hello.length(), 0);
        hello="% Notifications will have format '<Notify> event_name </Notify>'\n";
        syncSend(clientsock, hello.c_str(), hello.length(), 0);
        hello="% Commands are accepted in format '<Cmd> cmd_name </Cmd>'\n";
        syncSend(clientsock, hello.c_str(), hello.length(), 0);
        hello="% Supported commands are Subscribe, Info, Status, and ResetRequest\n";
        syncSend(clientsock, hello.c_str(), hello.length(), 0);
      } catch (faudes::Exception&) {
        faudes_closesocket(clientsock);
	clientsock=-1;
      }
      if(clientsock<0) {
        FD_DH("nDevice::Listen(): connection test failed: cannot write");
        faudes_closesocket(clientsock);
        break;
      }    
      // record client by socket
      TLOCK_E;
      nDevice::ClientState* cstate = &ndevice->mOutputClientStates[clientsock];
      cstate->mClientSocket=clientsock;
      cstate->mEvents.Clear();
      cstate->mConnected=false;
      cstate->mLineBuffer="";
      TUNLOCK_E;
    }

    // handle incomming broadcast
    if(avail>0)  
    if(FD_ISSET(ndevice->mBroadcastSocket,&mysocks)) {
      avail--;
      // get message
      char data[1024]; 
      int data_len=1023;
      struct sockaddr_in fromaddr;
      socklen_t fromaddr_len = sizeof(fromaddr);
      data_len=recvfrom(ndevice->mBroadcastSocket,data,data_len,0, (struct sockaddr*) &fromaddr,&fromaddr_len);
      if(data_len<0) data_len=0; // todo:  eof
      data[data_len]=0;
      if(data_len>=1) if(data[data_len-1]=='\n') data[data_len-1]=0;
      FD_DH("nDevice::Listen(): received udp datagram " << data << 
         " from " << inet_ntoa(fromaddr.sin_addr));
      // interpret message
      TokenReader tr(TokenReader::String,std::string(data));
      try {
        Token token;
        tr.Peek(token);
        // interpret udp request ...
        if(token.IsBegin("Request")) {
	  tr.ReadBegin("Request");
          if(tr.ReadString()==ndevice->mNetwork) {
            // extension 2.22i: identify sender (optional for compatibility)
	    std::string snode;
            Token stoken;
            tr.Peek(stoken);
            if(stoken.IsString()) snode=stoken.StringValue();
            // extension 2.22i: if this is a missing server, reset my request timer
            sit=ndevice->mInputServerStates.find(snode);
            if(sit!=ndevice->mInputServerStates.end()) {
	      if(sit->second.mServerSocket==-1) {
                lastbroadcast.tv_sec=0;
                lastbroadcast.tv_nsec=0;
	      }
	    }
            // extension 2.22i: ignore my own requests
            if(snode!=ndevice->mName) {
              // set up advert
              std::string message= "<Advert> " 
                  + ndevice->mNetwork + " " 
                  + ndevice->mName    + " " +
 	          ndevice->mEffectiveListenAddress.IpColonPort()+ "  </Advert>\n"; 
              // udp reply
              struct sockaddr_in replyaddr;
              memset(&replyaddr, '\0', sizeof(replyaddr));
              replyaddr.sin_family=AF_INET;
              replyaddr.sin_port=htons(ndevice->mBroadcastAddress.Port());
              //replyaddr.sin_addr.s_addr=fromaddr.sin_addr.s_addr;
              replyaddr.sin_addr.s_addr=inet_addr(ndevice->mBroadcastAddress.Ip().c_str());
              //replyaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
              TLOCK_E;
              int rc = sendto(ndevice->mBroadcastSocket,message.c_str(),message.length(),0,(struct sockaddr *) & replyaddr, sizeof(replyaddr));
	      TUNLOCK_E
	        FD_DH("nDevice::Listen(): reply advert: " <<  message.substr(0,message.length()-1) << " #" << rc);
	    } else {
              FD_DH("nDevice::Listen(): ingoring request from myself");
	    }
	  } else {
            FD_DH("nDevice::Listen(): ingoring request from other network");
	  }
	}
        // interpret udp advert ...
        if(token.IsBegin("Advert")) {
	  tr.ReadBegin("Advert");
          if(tr.ReadString()==ndevice->mNetwork) {
            std::string node = tr.ReadString();
            std::string saddr = tr.ReadString();
            SimplenetAddress addr(saddr);
            addr.Ip(inet_ntoa(fromaddr.sin_addr)); // overwrite with actual ip
            FD_DHV("nDevice::Listen(): figure actual ip address " << addr.Ip());
            if(!addr.Valid()) {
              addr=saddr;
              FD_DH("nDevice::Listen(): fallback to explicit ip address " << addr.Ip());
	    }
            std::map<std::string,nDevice::ServerState>::iterator sit;
            sit=ndevice->mInputServerStates.find(node);
            if(sit==ndevice->mInputServerStates.end()) { 
              FD_DH("nDevice::Listen(): ignoring irrelevant advert from " << node);
            } else if(sit->second.mAddress.Valid()) {
              FD_DH("nDevice::Listen(): ignoring address overwrite (hardwired?) " << node);
	    }
            if(sit!=ndevice->mInputServerStates.end())  
	    if(!sit->second.mAddress.Valid()) {
              FD_DH("nDevice::Listen(): accept advert " << node);
              sit->second.mAddress=addr;
              if(sit->second.mServerSocket>=0) faudes_closesocket(sit->second.mServerSocket);
              sit->second.mServerSocket=-1;
	    }
          } else {
            FD_DH("nDevice::Listen(): ingoring advert from other network");
	  }
        }
      } catch (faudes::Exception&) {
        FD_DH("nDevice::Listen(): ignore invalid udp message");
      }
    }
       

    // handle input servers: receive event notification 
    int revcount=0;
    if(avail>0)  
    for(sit=ndevice->mInputServerStates.begin(); sit!=ndevice->mInputServerStates.end(); sit++) {
      int serversock=sit->second.mServerSocket;
      if(serversock<0) continue;
      if(FD_ISSET(serversock, &mysocks)) {
        avail--;
        FD_DH("nDevice::Listen(): reading sock " <<  serversock);
        // buffer data in line buffer
        char buffer[1025];
        int count = recv(serversock, buffer, 1024, 0);
        if(count<=0) { // todo: test eof
          FD_DH("nDevice::Listen(): reading server sock " <<  serversock << " : eof");
          faudes_closesocket(serversock); 
          sit->second.mServerSocket=-1;
          continue;          
        } 
        FD_DH("nDevice::Listen(): reading server sock " <<  serversock  << ": #" << count);
        buffer[count]=0;
        sit->second.mLineBuffer +=std::string(buffer);
        // interpret line(s)
        if(count>0) 
        if(buffer[count-1]=='\n') 
        if(sit->second.mLineBuffer.length()>0) 
        {
          const std::string& linebuffer = sit->second.mLineBuffer;
#ifdef FAUDES_DEBUG_IODEVICE
          if(linebuffer.length()>0)
	  if(linebuffer[0]!='%')
           FD_DH("nDevice::Listen(): reading server sock " <<  serversock  << ": line: " << linebuffer);
#endif
          // tokenise notification
          TokenReader tr(TokenReader::String,linebuffer);
          try {
            Token token;
            while(tr.Peek(token)) {
              // its an event notify
              if(token.Type()==Token::Begin && token.StringValue()=="Notify") {
                tr.ReadBegin("Notify");
  	        std::string event = tr.ReadString();
                tr.ReadEnd("Notify");
                faudes_mutex_lock(ndevice->pBufferMutex);
                FD_DH("nDevice::Listen(): found event " << event);
                Idx sev=ndevice->mInputs.Index(event);
                if(ndevice->mInputs.Exists(sev)) ndevice->pInputBuffer->push_back(sev);
                faudes_mutex_unlock(ndevice->pBufferMutex);
                revcount++;
	        continue;
              }
              // its an info reply (ignored as of 2.22i)
              if(token.Type()==Token::Begin && token.StringValue()=="SimplenetDevice") {
                FD_DH("nDevice::Listen(): found device info");
	        nDevice remote;
                remote.Read(tr);
                FD_DH("nDevice::Listen(): found device with outputs " << remote.Outputs().ToString());      
                // used to subscribe on relevant events in pre 2.22h
                /*   
                EventSet sevents=ndevice->Inputs();
                sevents.SetIntersection(remote.Outputs());
	        sevents.Name("Subscribe");
	        std::string message=sevents.ToString();
                FD_DH("nDevice::Listen(): subscribing events " << message);
                message += "\n";
                syncSend(serversock,message.c_str(), message.length(),0); 
                */
	        continue;
              }
              // skip other sections
              if(token.Type()==Token::Begin) {
                FD_DH("nDevice::Listen(): ignore section " << token.StringValue());
		std::string section=token.StringValue();
                tr.ReadBegin(section);
	        while(!tr.Eos(section)) tr.Get(token);
                tr.ReadEnd(section);
	        continue;
	      }
              // ignore token
              FD_DH("nDevice::Listen(): error: ignore token");
              tr.Get(token);
            }
          } catch (faudes::Exception&) {
            FD_DH("nDevice::Listen(): " <<  serversock  << ": invalid notification");
          }
          sit->second.mLineBuffer.clear();
	}
      }
    }

    // handle output clients: reply to commands 
    if(avail>0)  
    for(cit=ndevice->mOutputClientStates.begin(); cit!=ndevice->mOutputClientStates.end(); cit++) {
      int clientsock=cit->second.mClientSocket;
      if(clientsock<0) continue;
      if(FD_ISSET(clientsock, &mysocks)) {
        avail--;
        FD_DH("nDevice::Listen(): reading client sock " <<  clientsock);
        // buffer data in line buffer
        char buffer[1025];
        int count = recv(clientsock, buffer, 1024, 0);
        if(count<=0) { // todo: test eof
          FD_DH("nDevice::Listen(): reading client sock " <<  clientsock << " : eof");
          TLOCK_E;
          faudes_closesocket(clientsock); 
          cit->second.mClientSocket=-1;
          cit->second.mConnected=false;
          TUNLOCK_E;
	  continue;          
        } 
        FD_DH("nDevice::Listen(): reading client sock " <<  clientsock  << ": #" << count);
        buffer[count]=0;
        cit->second.mLineBuffer +=std::string(buffer);
        // interpret line(s)
        if(count>0) 
        if(buffer[count-1]=='\n') 
        if(cit->second.mLineBuffer.length()>0) 
        {
          const std::string& linebuffer = cit->second.mLineBuffer;
#ifdef FAUDES_DEBUG_IODEVICE
          if(linebuffer.length()>0)
	  if(linebuffer[0]!='%')
           FD_DH("nDevice::Listen(): reading client sock " <<  clientsock  << ": line: " << linebuffer);
#endif
          // tokenise command
          TokenReader tr(TokenReader::String,linebuffer);
          try {
            Token token;
            while(tr.Peek(token)) {
              // its a command
              if(token.IsBegin("Cmd")) { 
  	        tr.ReadBegin("Cmd");
	        std::string cmd = tr.ReadString();
  	        tr.ReadEnd("Cmd");
                std::string response="<NAck> </NAck>\n";
                FD_DH("nDevice::Reply(" <<  clientsock  << "): received cmd " << cmd);
                // command: info
                if(cmd=="Info") {
                  TLOCK_E;
                  response=ndevice->ToString() + "\n";
                  TUNLOCK_E;
                }
                // command: status
                if(cmd=="Status") {
                  TLOCK_E;
                  if(ndevice->mState==vDevice::Up) response="<Ack> Up </Ack>\n";
                  if(ndevice->mState==vDevice::StartUp) response="<Ack> StartUp </Ack>\n";
                  if(ndevice->mState==vDevice::ShutDown) response="<Ack> ShutDown </Ack>\n";
                  TUNLOCK_E;
                }
                // its a reset request
                if(cmd=="ResetRequest") {
                  FD_DH("nDevice::Reply(" <<  clientsock  << "): reset request");
                  faudes_mutex_lock(ndevice->pBufferMutex);
	          if(!ndevice->mResetRequest) revcount++;   
	          ndevice->mResetRequest=true;
                  faudes_mutex_unlock(ndevice->pBufferMutex);
                  response="";
   	        }
                // send reply
                syncSend(clientsock, response.c_str(), response.length(), 0);
	      }
              // its a event subscription
              if(token.IsBegin("Subscribe")) {  
   	        EventSet sevents;
                sevents.Read(tr,"Subscribe");
                sevents.RestrictSet(ndevice->Outputs());
                sevents.Name("Subscribed");
                FD_DH("nDevice::Reply(" <<  clientsock  << "): providing events " << sevents.ToString());
                TLOCK_E;
                cit->second.mEvents.Clear();              
                cit->second.mEvents.InsertSet(sevents);
                cit->second.mConnected=true;
	        std::string response=sevents.ToString()+"\n";
                TUNLOCK_E;
                // send reply
                syncSend(clientsock, response.c_str(), response.length(), 0);
	      }
	    }
          } catch (faudes::Exception&) {
            FD_DH("nDevice::Reply(" <<  clientsock  << "): invalid cmd");
          }
          cit->second.mLineBuffer.clear();
  	}
      }
    }

    // signal condition for received events / reset requests
    if(revcount>0) {
      FD_DH("nDevice::Listen(): broadcast condition");
      faudes_mutex_lock(ndevice->pWaitMutex);
      faudes_cond_broadcast(ndevice->pWaitCondition);
      faudes_mutex_unlock(ndevice->pWaitMutex);
      revcount=0;
    }

    // should remove unconnected clients ?


    // some error
    if(avail<0) {
      FD_DH("nDevice::Listen(): select error");
    }

  }

  // close clientsockets
  FD_DH("nDevice::Listen(): close client sockets");
  TLOCK_E;
  for(cit=ndevice->mOutputClientStates.begin(); cit!=ndevice->mOutputClientStates.end(); cit++) {
    int clientsock= cit->second.mClientSocket;
    if(clientsock>0) faudes_closesocket(clientsock);
    cit->second.mClientSocket=-1;
    cit->second.mConnected=false;	
  }
  ndevice->mOutputClientStates.clear();
  TUNLOCK_E; 
  // close serversockets
  FD_DH("nDevice::Listen(): close server sockets");
  for(sit=ndevice->mInputServerStates.begin(); sit!=ndevice->mInputServerStates.end(); sit++) {
    int serversock=sit->second.mServerSocket;
    if(serversock>0) faudes_closesocket(serversock);
    sit->second.mServerSocket=-1;
  }
  FD_DH("nDevice::Listen(): terminating listen thread");
  faudes_thread_exit(NULL);
  return NULL;
}




// reset dynamic faudes state (buffered events, current time)
void nDevice::Reset(void) {
  // call base for time and input buffer
  vDevice::Reset();
  // bail out (do notify clients even when servers incomplete)
  if(mState!=Up && mState!=StartUp) return;
  // have message
  std::string message= "<Cmd> ResetRequest </Cmd>\n";
  // send cmd to all my servers
  LOCK_E;
  std::map<std::string,ServerState>::iterator sit=mInputServerStates.begin();
  for(; sit!=mInputServerStates.end(); sit++) {
    int serversock=sit->second.mServerSocket;
    if(serversock<0) continue;
    FD_DH("nDevice::Reset(): sending reset request to socket " << serversock);
    syncSend(serversock, message.c_str(), message.length(), 0);
  }
  UNLOCK_E;
}


#endif // configured simplenet


} // name space




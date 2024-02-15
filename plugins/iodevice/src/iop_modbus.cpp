/** @file iop_modbus.cpp Process image via modbus/tcp */

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2011, Thomas Moor.

*/


// include header
#include "iop_modbus.h"

// only compile for use with spi configured
#ifdef FAUDES_IODEVICE_MODBUS



namespace faudes {



/*
 **********************************************
 **********************************************
 **********************************************

 implementation: mbDevice

 **********************************************
 **********************************************
 **********************************************
 */


// std faudes, incl dummy
FAUDES_TYPE_IMPLEMENTATION(ModbusDevice,mbDevice,sDevice)

// autoregister (not functional, see xdevice constructor)
AutoRegisterType<mbDevice> gRtiRegisterSpiDevice("ModbusDevice");

//constructor
mbDevice::mbDevice(void) : sDevice() {
  FD_DHV("mbDevice(" << mName << ")::mbDevice()");
  // have appropriate default label for token io
  mDefaultLabel = "ModbusDevice";
  // pointer to internal I/O-image 
  mpImage=0;
  pOutputImage=0;
  pInputImage=0;
  mpOutputMask=0;
  // modbus/tcp io buffers
  mMessage= new char[260];
  // behavioural defaults
  mMasterRole=true;
  mSlaveAddress.IpColonPort("localhost:502");
  mSyncWrite=true;
}

//deconstructor
mbDevice::~mbDevice(void) {
  FD_DHV("mbDevice(" << mName << ")::~mbDevice()");
  Stop();
  // must wait for thread to terminate
  while(Status()!=Down);
  // free buffers
  if(mpImage) delete mpImage;
  if(mpOutputMask) delete mpOutputMask;
  delete mMessage;
}

// Clear
void mbDevice::Clear(void) {
 // clear base
 sDevice::Clear();
 // my configuration
 mSlaveIoRanges.clear();
 mSlaveAddress.IpColonPort("localhost:502");
 mSyncWrite=true;
}


//Compile(void)
void mbDevice::Compile(void){
  //setup up internal data structure
  FD_DHV("mbDevice(" << mName << ")::Compile()");
  // call base
  sDevice::Compile();
  // implement consistency tests here
  mImageSize=mMaxBitAddress+1;
  for(unsigned int i=0; i< mSlaveIoRanges.size(); i++) {
    const IoRange& ior=mSlaveIoRanges.at(i);
    if(ior.mFdAddress + ior.mCount > mImageSize) 
      mImageSize=ior.mFdAddress + ior.mCount;
    if(ior.mCount>2000) {
      std::stringstream errstr;
      errstr << "image size must not exceed 2000 bits (image #" << i+1 << ")";
      throw Exception("mbDevice:Read", errstr.str(), 52); 
    } 
  }
  mImageSize = ( (mImageSize-1) / 16 ) *16 + 16;
  if(mImageSize<=0) mImageSize=16;
  /*
  if(mImageSize > mMaxBitAddress+1) fail=true;
  if(fail) {
    std::stringstream errstr;
    errstr << "Invalid remote address ranges";
    throw Exception("mbDevice:Compile()", errstr.str(), 52);  
  }
  */
  // 
  if(!mMasterRole) if(mSlaveAddress.Ip()!="localhost") {
    std::stringstream errstr;
    errstr << "Slave must have localhost as SlaveAddress";
    throw Exception("mbDevice:Compile()", errstr.str(), 52);  
  }     
  // (re-)initialize images
  if(mpImage) delete mpImage;
  mpImage = new char[mImageSize];
  memset(mpImage,0,mImageSize);
  pOutputImage=mpImage;
  pInputImage=mpImage;
  // initialize output mask
  mpOutputMask = new char[mImageSize];
  memset(mpOutputMask,0,mImageSize);
  for(int bit=0; bit< mImageSize; bit++) 
    if(!mOutputLevelIndexMap[bit].Empty()) 
      mpOutputMask[bit]=1;
  // debug
  //Write();
}


// programmatic config: append remote image
void mbDevice::AppendRemoteInputs(int mbid, int mbaddr, int cnt, int fdaddr) {
  IoRange ior;
  ior.mInputs=true;
  ior.mMbId=mbid;
  ior.mMbAddress=mbaddr;
  ior.mCount=cnt;
  ior.mFdAddress=fdaddr;
  mSlaveIoRanges.push_back(ior);
}

// programmatic config: append remote image
void mbDevice::AppendRemoteOutputs(int mbid, int mbaddr, int cnt, int fdaddr) {
  IoRange ior;
  ior.mInputs=false;
  ior.mMbId=mbid;
  ior.mMbAddress=mbaddr;
  ior.mCount=cnt;
  ior.mFdAddress=fdaddr;
  mSlaveIoRanges.push_back(ior);
}

// programmatic config: slave address
void mbDevice::SlaveAddress(const std::string& rAddr) {
  if(mState!=Down) return;
  mSlaveAddress.IpColonPort(rAddr);
}

//DoWrite(rTr,rLabel,pContext)
void mbDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel,  const Type* pContext) const {
  FD_DHV("mbDevice("<<mName<<")::DoWritePreface()");
  //call base
  sDevice::DoWritePreface(rTw,"",pContext);
  // role
  Token stoken;
  stoken.SetEmpty("Role");
  if(mMasterRole) {
    stoken.InsAttributeString("value","master");
  } else {
    stoken.InsAttributeString("value","slave");
  }
  rTw << stoken;
  // slave address
  Token ftoken;
  ftoken.SetEmpty("SlaveAddress");
  ftoken.InsAttributeString("value",mSlaveAddress.IpColonPort());
  rTw << ftoken;
  // ranges
  rTw.WriteBegin("RemoteImage");
  for(unsigned int i=0; i<mSlaveIoRanges.size(); i++) {
    Token rtoken;
    if(mSlaveIoRanges.at(i).mInputs)
      rtoken.SetEmpty("Inputs");
    else
      rtoken.SetEmpty("Outputs");
    if(mSlaveIoRanges.at(i).mMbId!=1) 
      rtoken.InsAttributeInteger("mbid",mSlaveIoRanges.at(i).mMbId);
    rtoken.InsAttributeInteger("mbaddr",mSlaveIoRanges.at(i).mMbAddress);
    if(mSlaveIoRanges.at(i).mFdAddress!=mSlaveIoRanges.at(i).mMbAddress) 
      rtoken.InsAttributeInteger("fdaddr",mSlaveIoRanges.at(i).mFdAddress);
    rtoken.InsAttributeInteger("count",mSlaveIoRanges.at(i).mCount);
    rTw << rtoken;
  }
  rTw.WriteEnd("RemoteImage");
}


//DoReadPreface(rTr,rLabel,pContext)
void mbDevice::DoReadPreface(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  //dummy for token-input
  FD_DHV("mbDevice("<<mName<<")::DoReadPreface()");
  //call base
  sDevice::DoReadPreface(rTr,"",pContext);
  // my global configuration
  Token token;
  while(rTr.Peek(token)) {
    // role
    if(token.IsBegin("Role")) {
      rTr.ReadBegin("Role");
      mMasterRole=true;
      if(!token.ExistsAttributeString("value")) {
        std::stringstream errstr;
        errstr << "Invalid role tag, value attribute missing" << rTr.FileLine();
        throw Exception("mpiDevice:Read", errstr.str(), 52);  
      }
      std::string val=token.AttributeStringValue("value");
      std::transform(val.begin(), val.end(), val.begin(), tolower);
      if(val=="master") mMasterRole =true;
      else if(val=="slave") mMasterRole =false;
      else {
        std::stringstream errstr;
        errstr << "Invalid value attribute in role tag" << rTr.FileLine();
        throw Exception("spiDevice:Read", errstr.str(), 52);  
      }
      rTr.ReadEnd("Role");
      continue;
    }
    // address
    if(token.IsBegin("SlaveAddress")) {
      FD_DHV("mBDevice::DoRead(): found slave address");
      rTr.ReadBegin("SlaveAddress");
      if(!token.ExistsAttributeString("value")) {
        std::stringstream errstr;
        errstr << "Invalid ip address" << rTr.FileLine();
        throw Exception("mbDevice:Read", errstr.str(), 52);  
      }
      mSlaveAddress.IpColonPort(token.AttributeStringValue("value"));
      rTr.ReadEnd("SlaveAddress");
      continue;
    }
    // process image
    if(token.IsBegin("RemoteImage")) {
      rTr.ReadBegin("RemoteImage");
      while(!rTr.Eos("RemoteImage")) {
	Token rtoken;
        rTr.Get(rtoken);
        if(!rtoken.IsBegin("Inputs")) 
        if(!rtoken.IsBegin("Outputs")) {
          std::stringstream errstr;
          errstr << "invalid io range" << rTr.FileLine();
          throw Exception("mbDevice:Read", errstr.str(), 52);  
        }
        IoRange iorange;
        iorange.mMbId=1;
        iorange.mFdAddress=-1;
        iorange.mInputs = rtoken.IsBegin("Inputs");
        if(rtoken.ExistsAttributeInteger("mbid")) 
          iorange.mMbId=rtoken.AttributeIntegerValue("mbid");
        if(!rtoken.ExistsAttributeInteger("mbaddr")) {
          std::stringstream errstr;
          errstr << "missing remote address" << rTr.FileLine();
          throw Exception("mbDevice:Read", errstr.str(), 52);  
        }
        iorange.mMbAddress=rtoken.AttributeIntegerValue("mbaddr");
        if(rtoken.ExistsAttributeInteger("fdaddr")) 
          iorange.mFdAddress=rtoken.AttributeIntegerValue("fdaddr");
        if(iorange.mFdAddress<0) iorange.mFdAddress=iorange.mMbAddress;
        iorange.mCount=1;
        if(rtoken.ExistsAttributeInteger("count")) 
          iorange.mCount=rtoken.AttributeIntegerValue("count");
        rTr.ReadEnd(rtoken.StringValue());
        if(iorange.mCount>2000) {
          std::stringstream errstr;
          errstr << "image size must not exceed 2000 bits" << rTr.FileLine();
          throw Exception("mbDevice:Read", errstr.str(), 52);  
        }
        mSlaveIoRanges.push_back(iorange);
      }
      rTr.ReadEnd("RemoteImage");
      continue;
    }
    // unknown: break
    break;
  }
}


// Start(void)
void mbDevice::Start(void) {
  // bail out
  if(mState!=Down) return;
  // initialize modbus/tcp io data
  mSlaveSocket=-1;
  mRequestCount=1;
  // as a slave, we listen for masters to connect
  if(!mMasterRole) {
    // clear connected masters
    mMasterSockets.clear();
    // open a tcp port to listen: create socket
    mSlaveSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mSlaveSocket<=0) {
      std::stringstream errstr;
      errstr << "Modbus fatal network error (cannot open server socket)";
      throw Exception("mbDevice::Start", errstr.str(), 553);
    }
    int reuse=1;
    faudes_setsockopt(mSlaveSocket,SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));  
    // open a tcp port to listen: set up address
    struct sockaddr_in slaveaddress;
    memset(&slaveaddress, 0, sizeof(slaveaddress));
    slaveaddress.sin_family = AF_INET;
    slaveaddress.sin_addr.s_addr = htonl(INADDR_ANY);  
    slaveaddress.sin_port = htons(mSlaveAddress.Port());     
    // open a tcp port to listen: bind socket to address
    if(bind(mSlaveSocket, (struct sockaddr *) &slaveaddress,sizeof(slaveaddress)) <0) {
      std::stringstream errstr;
      errstr << "Modbus fatal network error (cannot bind socket)";
      throw Exception("nDevice::Start", errstr.str(), 553);
    }
    // open a tcp port to listen: start to listen
    if(listen(mSlaveSocket, 77) < 0) {  // todo: max pending connections
      std::stringstream errstr;
      errstr << "Simplenet fatal network error (cannot listen from socket)";
      throw Exception("mbDevice::Start", errstr.str(), 553);
    }
  }
  // call base (incl. reset)
  sDevice::Start();
  // pessimistic master: let background thread figure presence of the slave
  if(mMasterRole) mState=StartUp;
}

// Stop()
void mbDevice::Stop(void) {
  //close serial interface
  if(mState != Up && mState != StartUp) return;
  FD_DHV("mbDevice(" << mName << ")::Stop()");
  // call base (this will join the thread)
  sDevice::Stop();
  // close connections to masters (effective only when we are slave)
  for(unsigned int i=0; i<mMasterSockets.size(); i++) {
    if(mMasterSockets.at(i)>0) {
      FD_DH("mbDevice::Stop(): closing master socket #" << mMasterSockets.at(i));
      faudes_closesocket(mMasterSockets.at(i));
    }
  }
  mMasterSockets.clear();
  // close my socket
  if(mSlaveSocket>0) {
    FD_DH("mbDevice::Stop(): closing slave socket");
    faudes_closesocket(mSlaveSocket);
  }
  mSlaveSocket=-1;
  // done
  FD_DH("mbDevice::Stop(): done");
}


// modbus access macros
#define MB_PDUOFF 7
#define MB_SETINT(p,v) { mMessage[p] = ((v)>>8); mMessage[p+1] = ((v) & 0xff); }
#define MB_GETINT(p)   ( ( mMessage[p] << 8) +  ((int) mMessage[p+1]) )
#define MB_SETBYTE(p,v) { mMessage[p] = (v);}
#define MB_GETBYTE(p)   ( mMessage[p] )



// helper: flush buffers
int mbDevice::MbFlushBuffers(void) {
// flush recv buffer
  while(1) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    fd_set mysocks;
    FD_ZERO(&mysocks);
    FD_SET(mSlaveSocket, &mysocks);
    int avail= select(mSlaveSocket+1, &mysocks, NULL, NULL, &tv);
    if(avail<=0) break;  
    if(!FD_ISSET(mSlaveSocket,&mysocks)) break;
    FD_DH("mbDevice::MbFlushBuffers(): flush recv buffer");
    char data;
    int rc = recv(mSlaveSocket, &data, 1, 0);
    if(rc==1) continue;
    FD_DH("mbDevice::MbFlushBuffers(): flush recv buffer: fatal error?");
    return -1;
  }
  FD_DHV("mbDevice::MbFlushBuffers(): ok");
  return 0;
}

// helper: send modbus request
int mbDevice::MbSendRequest(int id) {
  // flush buffers
  if(MbFlushBuffers()!=0) return -1;
  // setup mbab header
  mRequestCount++;
  MB_SETINT(0,mRequestCount);
  MB_SETINT(2,0);
  MB_SETINT(4,mMessageLen+1);
  MB_SETBYTE(6,id); 
  // sync send
  int from=0;
  int left=mMessageLen+MB_PDUOFF;
  while(left>0) {
    int rc=send(mSlaveSocket, mMessage+from, left, 0);
    if(rc<0) return rc;
    left-=rc;
    from+=rc;
  }
  return 0;
}


// helper: receive modbus response
int mbDevice::MbReceiveResponse(void) {
  // prepare relevant source
  fd_set mysocks;
  int mysocks_max=0;
  FD_ZERO(&mysocks);
  FD_SET(mSlaveSocket, &mysocks);
  if(mysocks_max< mSlaveSocket) mysocks_max=mSlaveSocket;
  // set moderate timeout 
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500000;
  // wait for message
  int avail=select(mysocks_max+1, &mysocks, NULL, NULL, &tv);
  // read availabe data (expect only one packet)
  mMessageLen=0;
  if(avail<=0) return -1;  
  if(!FD_ISSET(mSlaveSocket,&mysocks)) return -1;
  mMessageLen = recv(mSlaveSocket, mMessage, 260, 0);
  if(mMessageLen<7) return -1;
  int mbablen = MB_GETINT(4);
  if(mbablen+6 != mMessageLen) {
    FD_DH("mbDevice::MbReceiveResponse(): invalid MBAB header (size mismatch)");
    return -1;
  }
  return 0;
}


// helper: receive request
int mbDevice::MbReceiveRequest(int mastersock) {
  // read availabe data (expect only one packet)
  mMessageLen = recv(mastersock, mMessage, 260, 0);
  // test Modbus compliance
  if(mMessageLen<7) return -1; // perhaps connection closed
  int mbablen = MB_GETINT(4);
  if(mbablen+6 != mMessageLen) {
    FD_DH("mbDevice::MbReceiveRequest(): invalid MBAB header (size mismatch)");
    return -1;
  }
  // set net length
  mMessageLen-= MB_PDUOFF;
  return 0;
}

// helper: send modbus request
int mbDevice::MbSendResponse(int mastersock) {
  // update mbab header
  MB_SETINT(4,mMessageLen+1);
  // sync send
  int from=0;
  int left=mMessageLen+MB_PDUOFF;
  while(left>0) {
    int rc=send(mastersock, mMessage+from, left, 0);
    if(rc<0) return rc;
    left-=rc;
    from+=rc;
  }
  return 0;
}

// loopcall-back for serial comminucation
void mbDevice::DoLoopCallback(void) {

  // master role: try to connect to remote slave

  if((mMasterRole) && (mState==StartUp) && (mSlaveSocket<0)) {

    // dont congest network / dont mess up console 
    static int ctimer=0;
    ctimer+=CycleTime();
    if(ctimer< 1000000) return;
    ctimer=0;
    // report 
    FD_DH("mbDevice::DoLoopCallBack(): connecting to remote slave " << mSlaveAddress.IpColonPort());    
    // open a tcp port: create socket
    int slavesock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(slavesock<=0) {
      FD_DH("mbDevice::DoLoopCallBack(): connection to slave failed: internal err A0");
      return;
    }
    // open a tcp port: set up internet address
    unsigned long int slaveinaddr = INADDR_NONE;
    if(slaveinaddr==INADDR_NONE) {
      FD_DHV("mbDevice::DoLoopCallBack(): using provided address literaly");
      slaveinaddr = inet_addr(mSlaveAddress.Ip().c_str());
    }
    if(slaveinaddr==INADDR_NONE) {
      struct hostent *host;
      host = gethostbyname(mSlaveAddress.Ip().c_str());
      if(host!=0) {
        FD_DHV("mbDevice::DoLooCallBack(): retrieve alternative address by name-lookup");
        slaveinaddr = *(unsigned long int*) host->h_addr;
      }
    }
    if(slaveinaddr==INADDR_NONE) {
      FD_DH("mbDevice::DoLooCallBack():: connection to slave failed: invalid address " << mSlaveAddress.Ip());
      faudes_closesocket(slavesock);
      return;
    }
    // open a tcp port: set up socket address 
    struct sockaddr_in slaveaddress;
    memset(&slaveaddress, 0, sizeof(slaveaddress));
    slaveaddress.sin_family = AF_INET;
    slaveaddress.sin_addr.s_addr=slaveinaddr; 
    slaveaddress.sin_port = htons(mSlaveAddress.Port());     
    // make my socket nonblocking
    int rc = faudes_setsocket_nonblocking(slavesock, true);
    if(rc<0) {
      FD_DH("mbDevice::DoLoopCallBack():: connection to slave failed: internal error A1");
      faudes_closesocket(slavesock);
      return;
    }
    // try to connect
    int rcc= connect(slavesock, (struct sockaddr*) &slaveaddress, sizeof(slaveaddress));
    // wait for host to accept
#ifdef FAUDES_POSIX
    if(rcc<0) {
      if(errno!=EINPROGRESS) {
        FD_DH("mbDevice::DoLoopCallBack(): connection to slave failed: connect() errno " << errno);
      } else {
        FD_DH("mbDevice::DoLoopCallBack(): connection to slave: wait for host to accept");
        // sense success via select befor timeout
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        fd_set mysocks;
        FD_ZERO(&mysocks);
        FD_SET(slavesock, &mysocks);
        rcc= select(slavesock+1, NULL, &mysocks, NULL, &tv);
        rcc--; // map 1 to no err aka 0 ;-)
        if(rcc<0) FD_DH("mbDevice::DoLoopCallBack(): connection to slave failed: timeout");
      }
    }
#endif
#ifdef FAUDES_WINDOWS
    if(rcc<0) {
      int lerr = WSAGetLastError();
      if(lerr!=WSAEWOULDBLOCK) {
        FD_DH("mbDevice::DoLoopCallBack(): connection to slave failed: connect() errno " << lerr);
      } else {
        FD_DH("mbDevice::DoLoopCallBack(): wait for host to accept");
        // sense success via select befor timeout
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        fd_set mysocks;
        FD_ZERO(&mysocks);
        FD_SET(slavesock, &mysocks);
        rcc= select(slavesock+1, NULL, &mysocks, NULL, &tv);
        rcc--; // map 1 to no err aka 0 ;-)
        if(rcc<0) FD_DH("mbDevice::DoLoopCallBack(): connection to slave failed: timeout");
      }
    }
#endif
    // connection failed
    if(rcc<0) {
      FD_DH("mbDevice::DoLoopCallBack():: connection to slave failed: host unreachable");
      faudes_closesocket(slavesock);
      return;
    }
    // sense errors on socket level
    if(faudes_getsocket_error(slavesock)<0) {
      FD_DH("mbDevice::DoLoopCallBack():: connection to slave failed: internal error A2");
      faudes_closesocket(slavesock);
      return;
    }
    // restore blocking socket
    rc = faudes_setsocket_nonblocking(slavesock, false);
    if(rc<0) {
      FD_DH("mbDevice::DoLoopCallBack():: connection to slave failed: internal error A3");
      faudes_closesocket(slavesock);
      return;
    }
    // record success
    FD_DH("mbDevice::DoLoopCallBack(): connected to remote slave, using socket #" << slavesock);
    mSlaveSocket=slavesock;
  }


  // slave role: accept remote master connection

  if((!mMasterRole) && (mState==Up)) {

    // sense connection requests
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    fd_set mysocks;
    FD_ZERO(&mysocks);
    FD_SET(mSlaveSocket,&mysocks);
    int avail = select(mSlaveSocket+1, &mysocks, NULL, NULL, &tv);

    // accept
    if(avail==1) 
    if(FD_ISSET(mSlaveSocket,&mysocks)) {
      FD_DH("mbDevice::DoLoopCallBack(): accepting remote master to connect");
      struct sockaddr_in masteraddr;
      socklen_t masteraddr_len = sizeof(masteraddr);
      int mastersock=accept(mSlaveSocket, (struct sockaddr *) &masteraddr, &masteraddr_len );
      if(mastersock<0) {
        FD_DH("mbDevice::DoLoopCallback(): failed to accept incomming connection");
      } else {
        mMasterSockets.push_back(mastersock);
      }
    }
 
  }


  // master role: sync image with remote slave

  if((mMasterRole) && (mSlaveSocket>0) && (mState!=Down) && (mState!=ShutDown) ) {
    FD_DHV("mbDevice::DoLooCallBack(): update image from remote slave");
    // read all inputs
    for(unsigned int i=0; i<mSlaveIoRanges.size(); i++) {
      const IoRange& ior=mSlaveIoRanges.at(i);
      if(!ior.mInputs) continue;
      // assemble request<
      MB_SETBYTE(MB_PDUOFF,0x02); // read multiple digital inputs
      MB_SETINT(MB_PDUOFF+1,ior.mMbAddress); 
      MB_SETINT(MB_PDUOFF+3,ior.mCount); 
      mMessageLen=5;
      // send request
      FD_DHV("mbDevice::DoLoopCallBack(): sending request");
      if(MbSendRequest(ior.mMbId)!=0) {mState=StartUp; faudes_closesocket(mSlaveSocket); mSlaveSocket=-1; return;};
      FD_DHV("mbDevice::DoLoopCallBack(): read response");
      if(MbReceiveResponse()!=0)      {mState=StartUp; faudes_closesocket(mSlaveSocket); mSlaveSocket=-1; return;};
      FD_DHV("mbDevice::DoLoopCallBack(): received responde #" << mMessageLen);
      // interpret response
      if(MB_GETBYTE(MB_PDUOFF)==0x02) { // no error
        FD_DHV("mbDevice::DoLoopCallBack(): input image received");
	int count=MB_GETBYTE(MB_PDUOFF+1); // todo: verify
        count=ior.mCount;
        int src=MB_PDUOFF+2;
        int data=MB_GETBYTE(src);
        int addr=ior.mFdAddress;
        int shft=0x01;
        while(count) {
          if(!mpOutputMask[addr]) mpImage[addr]= (( data & shft) != 0);
          addr++; count--; shft = shft <<1;
          if(shft==0x100) { shft=0x01; data=MB_GETBYTE(++src);};
        }
      } else {
        FD_DH("mbDevice::DoLoopCallBack(): received error response on read inputs");
      }
    }
    // write all outputs
    for(unsigned int i=0; i<mSlaveIoRanges.size(); i++) {
      const IoRange& ior=mSlaveIoRanges.at(i);
      if(ior.mInputs) continue;
      // assemble request
      MB_SETBYTE(MB_PDUOFF,0x0f); // write multiple coils
      MB_SETINT(MB_PDUOFF+1,ior.mMbAddress); 
      MB_SETINT(MB_PDUOFF+3,ior.mCount); 
      int bcount= (ior.mCount-1)/8 +1;
      MB_SETBYTE(MB_PDUOFF+5,bcount);      
      int dst=MB_PDUOFF+6;
      int data=0x00;
      int shft=0x01;
      int addr=ior.mFdAddress;
      int count=ior.mCount;
      while(count) {
        if(mpImage[addr]) data |= shft;
        addr++; count--; shft = shft <<1;
        if(shft==0x100) { shft=0x01; MB_SETBYTE(dst,data); data=0x00; dst++;}
      }
      if(shft!=0x01) { MB_SETBYTE(dst,data);};
      mMessageLen=6+bcount;
      // send request
      FD_DHV("mbDevice::DoLoopCallBack(): sending request");
      if(MbSendRequest(ior.mMbId)!=0) {mState=StartUp; faudes_closesocket(mSlaveSocket); mSlaveSocket=-1; return;};
      if(MbReceiveResponse()!=0)      {mState=StartUp; faudes_closesocket(mSlaveSocket); mSlaveSocket=-1; return;};
      if(MB_GETBYTE(MB_PDUOFF)!=0x0f) { 
        FD_DH("mbDevice::DoLoopCallBack(): received error response on write coils");
      }
    }
  }


  // slave role: sync image with remote masters

  if((!mMasterRole) && (mState==Up)) {

    // prepare relevant sources 
    fd_set mysocks;
    int mysocks_max=0;
    FD_ZERO(&mysocks);
    for(unsigned int i=0; i< mMasterSockets.size(); i++) {
      int mastersock=mMasterSockets.at(i);
      if(mastersock<0) continue;
      FD_SET(mastersock, &mysocks);
      if(mysocks_max< mastersock) mysocks_max=mastersock;
    }

    // sense requests
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int avail=select(mysocks_max+1, &mysocks, NULL, NULL, &tv);

    // loop master requests
    if(avail>0)  
    for(unsigned int i=0; i< mMasterSockets.size(); i++) {
      int mastersock=mMasterSockets.at(i);
      if(mastersock<0) continue;
      if(!FD_ISSET(mastersock, &mysocks)) continue;
      FD_DHV("mbDevice::DoLoopCallback(): received message on  sock " <<  mastersock);
      if(MbReceiveRequest(mastersock)<0) {
        FD_DH("mbDevice::DoLoopCallback(): receive error on sock " <<  mastersock);
        faudes_closesocket(mastersock);
        mMasterSockets.at(i)=-1; // todo: remove
        continue;
      }
     
      // interpret request
      int fnct=MB_GETBYTE(MB_PDUOFF);
      int errcode = 0x01;
      // read inputs or coils
      if((fnct==0x01) || (fnct==0x02)) {
        FD_DHV("mbDevice::DoLoopCallback(): coil-read or input read request");
        int addr =  MB_GETINT(MB_PDUOFF+1);
        int count = MB_GETINT(MB_PDUOFF+3);
        int bcount= ((count-1)/8+1);
        FD_DHV("mbDevice::DoLoopCallback(): address range: @" << addr << " #" << count);
        // test validity
        errcode=0x00;
        if(addr+count>mImageSize) errcode=0x02;
        if(count>2000) errcode=0x02;
        // perform
        if(errcode==0x00) {    
          // fill in bits
          int dst=MB_PDUOFF+2;
          int data=0x00;
          int shft=0x01;
          while(count) {
            if(mpImage[addr]) data |= shft;
            addr++; count--; shft = shft <<1;
            if(shft==0x100) { shft=0x01; MB_SETBYTE(dst,data); dst++; data=0x00;}
          }
          if(shft!=0x01) { MB_SETBYTE(dst++,data);};
          MB_SETBYTE(MB_PDUOFF+1,bcount);
          // set nessage length
          mMessageLen=bcount+2;
        }
      }
      // read input registers or holding registers
      if((fnct==0x03) || (fnct==0x04)) {
        FD_DHV("mbDevice::DoLoopCallback(): register or holding register read request");
        int addr =  MB_GETINT(MB_PDUOFF+1);
        int count = MB_GETINT(MB_PDUOFF+3);
        FD_DHV("mbDevice::DoLoopCallback(): address range: @" << addr << " #" << count);
        // test validity
        errcode=0x00;
        if(16*addr+16*count>mImageSize) 
          errcode=0x02;
        // perform
        if(errcode==0x00) {    
          // set header length
          mMessageLen=2*count+2;
          MB_SETBYTE(MB_PDUOFF+1,2*count);
          // fill in bits
          int src= addr*16;
          int dst=MB_PDUOFF+2;
          for(;count>0; count--) {
            int shft=0x01;
            int lbyte=0x00;
            for(;src<mImageSize && shft!=0x100; src++, shft = shft << 1)
              if(mpImage[src]) lbyte |= shft;
            shft=0x01;
            int hbyte=0x00;
            for(;src<mImageSize && shft!=0x100; src++, shft = shft << 1)
              if(mpImage[src]) hbyte |= shft;
            MB_SETBYTE(dst,hbyte); dst++;
            MB_SETBYTE(dst,lbyte); dst++;
  	  }
        }
      }
      // write single coil
      if(fnct==0x05) {
        FD_DHV("mbDevice::DoLoopCallback(): write single coil request");
        int addr =  MB_GETINT(MB_PDUOFF+1);
        bool val = ( ((unsigned char) MB_GETBYTE(MB_PDUOFF+3))==0xff);
        FD_DHV("mbDevice::DoLoopCallback(): write single coil request: " << addr << " to " << val);
        // test
        errcode=0x00;
        if(addr>=mImageSize) errcode=0x02;
        // perform
        if(errcode==0x00) {    
          if(mpOutputMask[addr]) mpImage[addr] = val;
          mMessageLen=5;
        }
      }
      // write single register
      if(fnct==0x06) {
        FD_DHV("mbDevice::DoLoopCallback(): write holding register request");
        int addr =  MB_GETINT(MB_PDUOFF+1);
        int val = MB_GETINT(MB_PDUOFF+3);
        FD_DHV("mbDevice::DoLoopCallback(): set  @" << addr << " to " << val);
        // test validity
        errcode=0x00;
        if(16*addr+16 >mImageSize) 
          errcode=0x02;
        // perform
        if(errcode==0x00) {    
          // extract  bits
          int dst=16*addr;
          int hbyte= (val >> 8);    // :-)
          int lbyte= (val & 0xff);
          int shft;
          for(shft=0x01; shft!=0x100; shft = shft << 1, dst++)
            mpImage[dst] = (( lbyte & shft) != 0);
          for(shft=0x01; shft!=0x100; shft = shft << 1, dst++)
            mpImage[dst] = (( hbyte & shft) != 0);
          // setup reply
          mMessageLen=5;
        }
      }
      // write multiple coils
      if(fnct==0x0f) {
        FD_DHV("mbDevice::DoLoopCallback(): write multiple coils request");
        int addr =  MB_GETINT(MB_PDUOFF+1);
        int count = MB_GETINT(MB_PDUOFF+3);
        int bcount= MB_GETBYTE(MB_PDUOFF+5);
        FD_DHV("mbDevice::DoLoopCallback(): address range: @" << addr << " #" << count << "(" << bcount << ")");
        // test validity
        errcode=0x00;
        if(addr+count>mImageSize) errcode=0x02;
        if( (bcount < ((count-1)/8+1)) || (mMessageLen < 6+bcount) ) errcode=0x03;
        // perform
        if(errcode==0x00) {    
          // extract  bits
          int src=MB_PDUOFF+6;
          int data=0;
          int shft=0x100;
          while(count) {
            if(shft==0x100) { shft=0x01; data=MB_GETBYTE(src);src++;};
            if(!mpOutputMask[addr]) mpImage[addr]= (( data & shft) != 0);
            addr++; count--; shft = shft <<1;    
          }
          // setup reply
          mMessageLen=5;
        }
      }
      // write multiple holding registers
      if(fnct==0x10) {
        FD_DHV("mbDevice::DoLoopCallback(): write multiple holding registers request");
        int addr =  MB_GETINT(MB_PDUOFF+1);
        int count = MB_GETINT(MB_PDUOFF+3);
        int bcount= MB_GETBYTE(MB_PDUOFF+5);
        FD_DHV("mbDevice::DoLoopCallback(): address range: @" << addr << " #" << count << "(" << bcount << ")");
        // test validity
        errcode=0x00;
        if(16*addr+16*count>mImageSize) 
          errcode=0x02;
        if( bcount != 2* count) 
          errcode=0x03;
        // perform
        if(errcode==0x00) {    
          // extract  bits
          int src=MB_PDUOFF+6;
          int dst=16*addr;
          for(;count>0;count--) {
            int hbyte=MB_GETBYTE(src); src++;
            int lbyte=MB_GETBYTE(src); src++;
            int shft;
            for(shft=0x01; shft!=0x100; shft = shft << 1, dst++)
              mpImage[dst] = (( lbyte & shft) != 0);
            for(shft=0x01; shft!=0x100; shft = shft << 1, dst++)
              mpImage[dst] = (( hbyte & shft) != 0);
	  }
          // setup reply
          mMessageLen=5;
        }
      }
      // send reply
      if(errcode==0x00) {
        FD_DHV("mbDevice::DoLoopCallback(): sending reply #" << mMessageLen);
        MbSendResponse(mastersock);
      }
      // send error
      if(errcode!=0x00) {
        FD_DH("mbDevice::DoLoopCallback(): sending error reply, code " << errcode);
        MB_SETBYTE(MB_PDUOFF,  fnct | 0x80);
        MB_SETBYTE(MB_PDUOFF+1, errcode);
        mMessageLen=2;
        MbSendResponse(mastersock);
      }


    } // end: slave role loops all clients for requests
  } // end: slave role receiving requests

}


// DoReadSignalsPre(void)
bool mbDevice::DoReadSignalsPre(void) {
  return mSlaveSocket!=-1;
}


// DoReadSignalsPost(void)
void mbDevice::DoReadSignalsPost(void) {
}


//ReadSignal(int)
bool mbDevice::DoReadSignal(int bit){
  return pInputImage[bit];
}


// DoWriteSignalsPre(void)
bool mbDevice::DoWriteSignalsPre(void) {
  return mSlaveSocket!=-1;
}

// DoWrtieSignalsPost(void)
void mbDevice::DoWriteSignalsPost(void) {
}


//DoWriteSignal(int,int)
void mbDevice::DoWriteSignal(int bit, bool value){

  // Write one actuator value, adressed by bit number (0 to 63);
  //FD_DHV("mbDevice("<<mName<<")::DoWriteSignal(" << bit << ", " << value <<")");

  pOutputImage[bit]=value;

}


} // namespace



#endif // end modbus support

/** @file iop_serial.h Process image via serial line */

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2011, Thomas Moor.

*/


// include header
#include "iop_serial.h"

// only compile for use with spi configured
#ifdef FAUDES_IODEVICE_SERIAL

namespace faudes {

/*
 **********************************************
 **********************************************
 **********************************************

 implementation: serial helpers

 **********************************************
 **********************************************
 **********************************************
 */


// hardcoded block size (64bit)
#define PSIZE 8

// hardcoded timeout (usec) for serial transmission
#define PUSECPB ((int) 1000000.0* (8.0+1.0+1.0) / 115200.0)
#define PUSECXX PUSECPB


// open serial port, return fd or -1 on error
int serialOpen(const std::string& devname) {
  // open device / det filedescriptor
  int fd=-1;
  fd = open(devname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if(fd == -1) return -1;
  // set termio options (raw 115200 8N1, no flow control)
  // see also "Serial Programming Guide for POSIX Operating Systems" by Michael R. Sweet
  struct termios options;
  tcgetattr(fd, &options);
  cfsetispeed(&options, B115200);
  cfsetospeed(&options, B115200);
  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~CSIZE;        // 8 data bit
  options.c_cflag |= CS8;           
  options.c_cflag &= ~PARENB;       // no parity
  options.c_cflag &= ~CSTOPB;       // one stop bit
  options.c_cflag &= ~CRTSCTS;      // no harware flow control
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input (no line editing, no signals)
  options.c_iflag &= ~(IXON | IXOFF | IXANY);  // no software flow control
  options.c_iflag &= ~(INLCR | IGNCR |	ICRNL	| IUCLC	| IMAXBEL); // no CR/LF mapping and thelike
  options.c_oflag &= ~OPOST;        // raw output (no delays after CR etc)
  tcsetattr(fd, TCSANOW, &options);
  // configure via fcntl
  fcntl(fd, F_SETFL, FNDELAY);  // nonblocking read (should be by open option anyway)
  // done
  return fd; 
}

// close serial port
void serialClose(int fd) {
  close(fd);
}

// write block (-1 on error, else 0)
int serialWriteBlock(int fd, char* data, int len) {
  // debug 
  /*
  static char mdata[1024];
  if(len>1024) len=1024;
  for(int i=0; i<len; i++) mdata[i]= data[i]+'A';
  data=mdata;
  */
  while(len>0) {
    int n=write(fd, data, len);
    if(n<0) break;
    len-=n;
  }
  if(len!=0) 
   FD_DH("spiDevice()::serialWriteBlock(): cannot transmit");
  return len == 0 ? 0 : -1;
}

// flush input buffer
void serialFlush(int fd) {
  char d;
  int cnt=0;
  while(read(fd, &d,1)==1) cnt++;
  if(cnt!=0)  FD_DH("spiDevice()::serialFlush(): rec/drop #" << cnt);
}

// read block (return -1 on error, else 0)
// Note: this fnct reads the data if the amount of characters available 
// exactly matches the specified block length; in any other case,
// the read buffer is flushed and -1 is returned to indicate an
// error.
int serialReadBlock(int fd, char* data, int len) {
  int n1= read(fd, data,len);
  // error / no data at all
  if(n1<=0) return -1;
  // bytes missing, allow for transmission to complete
  if(n1<len){
    usleep( (len-n1)* PUSECPB + PUSECXX );
    int n2= read(fd, data+n1,len-n1);
    if(n2<=0) {
      FD_DH("spiDevice()::serialReadBlock(): rec/drop #" << n1)
      return -1;
    }
    n1+=n2;
  }
  // test for empty buffer
  int n3= read(fd, data,1);
  if(n3==1) {
    FD_DH("spiDevice()::serialReadBlock(): rec/drop #" << n1);
    serialFlush(fd);
    return -1;
  }
  // success
  return 0;
}



/*
 **********************************************
 **********************************************
 **********************************************

 implementation: spiDevice

 **********************************************
 **********************************************
 **********************************************
 */


// std faudes, incl dummy
FAUDES_TYPE_IMPLEMENTATION(SpiDevice,spiDevice,sDevice)

// autoregister (not functional, see xdevice constructor)
AutoRegisterType<spiDevice> gRtiRegisterSpiDevice("SpiDevice");

//constructor
spiDevice::spiDevice(void) : sDevice() {
  FD_DHV("spiDevice(" << mName << ")::spiDevice()");
  // have appropriate default label for token io
  mDefaultLabel = "SpiDevice";
  // pointer to internal I/O-image 
  mpImage=0;
  pOutputImage=0;
  pInputImage=0;
  mpOutputMask=0;
  // behavioural defaults
  mMaster=false;
  mSyncWrite=true;
}

//deconstructor
spiDevice::~spiDevice(void) {
  FD_DHV("spiDevice(" << mName << ")::~spiDevice()");
  Stop();
}

// Clear
void spiDevice::Clear(void) {
 // clear base
 sDevice::Clear();
 // my configuration
 mMaster=false;
 mDeviceFiles.clear();
 mPorts.clear();
 mSyncWrite=true;
}


//Compile(void)
void spiDevice::Compile(void){
  //setup up internal data structure
  FD_DHV("spiDevice(" << mName << ")::Compile()");
  // call base
  sDevice::Compile();
  // test for illegal address range
  if(mMaxBitAddress+1 > PSIZE*8) {
    std::stringstream errstr;
    errstr << "Invalid address range (must not exceed " << PSIZE*8 << ")";
    throw Exception("spiDevice:Compile", errstr.str(), 52);  
  }
  // slave must have exactly one serial device
  if((!mMaster) && (mDeviceFiles.size()!=1)) {
    std::stringstream errstr;
    errstr << "Slave must have exactly one device file specified";
    throw Exception("spiDevice:Compile()", errstr.str(), 52);  
  }
  // master must have at least  one serial device
  if((mMaster) && (mDeviceFiles.size()==0)) {
    std::stringstream errstr;
    errstr << "Master must have at least one device file specified";
    throw Exception("spiDevice:Compile()", errstr.str(), 52);  
  }
}


//DoWrite(rTr,rLabel,pContext)
void spiDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel,  const Type* pContext) const {
  FD_DHV("spiDevice("<<mName<<")::DoWritePreface()");
  //call base
  sDevice::DoWritePreface(rTw,"",pContext);
  // role
  Token ftoken;
  ftoken.SetEmpty("Role");
  if(mMaster) {
    ftoken.InsAttributeString("value","master");
  } else {
    ftoken.InsAttributeString("value","slave");
  }
  rTw << ftoken;
  // devicefiles
  for(unsigned int i=0; i<mDeviceFiles.size(); i++) {
    Token dtoken;
    ftoken.SetEmpty("DeviceFile");
    ftoken.InsAttributeString("value",mDeviceFiles.at(i));
    rTw << ftoken;
  }
}


//DoReadPreface(rTr,rLabel,pContext)
void spiDevice::DoReadPreface(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  //dummy for token-input
  FD_DHV("spiDevice("<<mName<<")::DoReadPreface()");
  //call base
  sDevice::DoReadPreface(rTr,"",pContext);
  // my global configuration
  Token token;
  while(rTr.Peek(token)) {
    // role
    if(token.IsBegin("Role")) {
      rTr.ReadBegin("Role");
      mMaster=false;
      if(!token.ExistsAttributeString("value")) {
        std::stringstream errstr;
        errstr << "Invalid role tag" << rTr.FileLine();
        throw Exception("spiDevice:Read", errstr.str(), 52);  
      }
      std::string val=token.AttributeStringValue("value");
      std::transform(val.begin(), val.end(), val.begin(), tolower);
      if(val=="master") mMaster =true;
      else if(val=="slave") mMaster =false;
      else {
        std::stringstream errstr;
        errstr << "Invalid role tag" << rTr.FileLine();
        throw Exception("spiDevice:Read", errstr.str(), 52);  
      }
      rTr.ReadEnd("Role");
      continue;
    }
    // device file
    if(token.IsBegin("DeviceFile")) {
      rTr.ReadBegin("DeviceFile");
      if(!token.ExistsAttributeString("value")) {
        std::stringstream errstr;
        errstr << "Invalid device tag" << rTr.FileLine();
        throw Exception("spiDevice:Read", errstr.str(), 52);  
      } 
      mDeviceFiles.push_back(token.AttributeStringValue("value"));
      rTr.ReadEnd("DeviceFile");
      continue;
    }
    // unknown: break
    break;
  }
}


// Start(void)
void spiDevice::Start(void) {
  //open wago-device
  if(mState!=Down) return;
  FD_DH("spiDevice(" << mName << ")::Start(): open devices #" << mDeviceFiles.size());
  // initialize serial line(s)
  for(unsigned int i=0; i<mDeviceFiles.size(); i++) {
    int fd=serialOpen(mDeviceFiles.at(i));
    if(fd<0) {
      std::stringstream errstr;
      errstr << "cannot open serial line " << mDeviceFiles.at(i);
      throw Exception("spiDevice()::Start()", errstr.str(), 552);
    }
    mPorts.push_back(fd);
  }
  // initialize images
  mpImage = new char[PSIZE];
  memset(mpImage,0,PSIZE);
  pOutputImage=mpImage;
  pInputImage=mpImage;
  // initialize output mask
  mpOutputMask = new char[PSIZE];
  memset(mpOutputMask,0,PSIZE);
  for(int bit=0; bit<=mMaxBitAddress; bit++) 
    if(!mOutputLevelIndexMap[bit].Empty()) 
      mpOutputMask[bit/8] |= (0x01 << (bit %8));
  // call base (incl. reset)
  sDevice::Start();
  // pessimistic: let background thread figure presence of other nodes
  mState=StartUp;
}

// Stop()
void spiDevice::Stop(void) {
  //close serial interface
  if(mState != Up && mState != StartUp) return;
  FD_DHV("spiDevice(" << mName << ")::Stop()");
  // call base
  sDevice::Stop();
  // close lines
  for(unsigned int i=0; i< mPorts.size(); i++)
    serialClose(mPorts.at(i));
  mPorts.clear();
  // invalidate images
  if(mpImage) delete mpImage;
  mpImage=0;
  pOutputImage=0;
  pInputImage=0;
  if(mpOutputMask) delete mpOutputMask;
  mpOutputMask=0;
  // down
  mState=Down;
}


// loopcall-back for serial comminucation
void spiDevice::DoLoopCallback(void) {
  // bail out
  if(mState!=Up && mState!=StartUp) return;
  //FD_DHV("spiDevice(" << mName << ")::DoLoopCallBack()");
  // master: send image block to each client and await reply
  if(mMaster) {
    FD_DHV("spiDevice()::DoLoopCallBack(): master send images #" << mPorts.size());
    for(unsigned int i=0; i< mPorts.size(); i++) {
      // discard input buffer
      serialFlush(mPorts.at(i));
      // write
      serialWriteBlock(mPorts.at(i),mpImage,PSIZE); 
      // await reply
      usleep(3000); // hardcoded, conservative ... here the cheap design shows :-(
      char buffer[PSIZE];
      int err=serialReadBlock(mPorts.at(i),buffer,PSIZE);
      if(err!=0) continue;
      FD_DHV("spiDevice()::DoLoopCallBack(): master received image");
      // copy to my image, except for my outputs
      for(int i=0; i<PSIZE; i++) 
        mpImage[i]= (buffer[i] & ~mpOutputMask[i]) | (mpImage[i] & mpOutputMask[i]);
    }
  }
  // slave: receive image block if available, send reply
  if(!mMaster) {
    //FD_DHV("spiDevice()::DoLoopCallBack(): slave await images #" << mPorts.size());
    // test for image
    char buffer[PSIZE];
    int err=serialReadBlock(mPorts.at(0),buffer,PSIZE);
    if(err==0) {
      FD_DHV("spiDevice()::DoLoopCallBack(): slave received image");
      // copy to my image, except for my outputs
      for(int i=0; i<PSIZE; i++) 
        mpImage[i]= (buffer[i] & ~mpOutputMask[i]) | (mpImage[i] & mpOutputMask[i]);
      // write
      serialWriteBlock(mPorts.at(0),mpImage,PSIZE); 
    }
  } // end: slave
}


// DoReadSignalsPre(void)
bool spiDevice::DoReadSignalsPre(void) {
  return pInputImage!=0;
}


// DoReadSignalsPost(void)
void spiDevice::DoReadSignalsPost(void) {
}


//ReadSignal(int)
bool spiDevice::DoReadSignal(int bit){
  // Read one input value, addressed by bit number (0 to 63);

  // Determine byte and bit address
  int byte = bit / 8;
  bit = bit % 8;

  // Read bit
  return ( pInputImage[byte] & (0x01 << (bit)) ) != 0x00;
}


// DoWriteSignalsPre(void)
bool spiDevice::DoWriteSignalsPre(void) {
  return pOutputImage!=0;
}

// DoWrtieSignalsPost(void)
void spiDevice::DoWriteSignalsPost(void) {
}


//DoWriteSignal(int,int)
void spiDevice::DoWriteSignal(int bit, bool value){

  // Write one actor value, adressed by bit number (0 to 63);
  FD_DHV("spiDevice("<<mName<<")::DoWriteSignal(" << bit << ", " << value <<")");

  // Determine byte and bit addresse.
  int byte = (bit) / 8;
  bit = (bit) % 8;

  // Write value to output-image using bit-operations
  if(value) pOutputImage[byte] |= (0x01 << (bit));
  else pOutputImage[byte] &= ~(0x01 << (bit));

}


} // namespace



#endif // end serial support

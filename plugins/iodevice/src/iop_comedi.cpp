 
/** @file iop_comedi.cpp Low-level access to physical device over comedi-driver*/

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008, Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/


// include header
#include "iop_comedi.h"

namespace faudes {

// only compile for comedi support
#ifdef FAUDES_IODEVICE_COMEDI

// std faudes, incl dummy
FAUDES_TYPE_IMPLEMENTATION(ComediDevice,cDevice,sDevice)

// autoregister
AutoRegisterType<cDevice> gRtiRegisterComediDevice("ComediDevice");

//constructor
cDevice::cDevice(void) : sDevice(), mDev(0) {
  FD_DHV("cDevice(" << mName << ")::cDevice()");
  // have appropriate default label for token io
  mDefaultLabel = "ComediDevice";
  // invalidate images
  mpInputImage=NULL;
  mpOutputImage=NULL;
}

//deconstructor
cDevice::~cDevice(void) {
  FD_DHV("cDevice(" << mName << ")::~cDevice()");
  Stop();
}

//DoWrite(rTr,rLabel,pContext)
void cDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel,  const Type* pContext) const {
  FD_DHV("cDevice("<<mName<<")::DoWritePreface()");
  //call base
  sDevice::DoWritePreface(rTw,"",pContext);
  // write mSystemFile
  Token sysf;
  sysf.SetBegin("DeviceFile");
  sysf.InsAttribute("value",mSystemFile);
  rTw << sysf;
}


//DoReadPreface(rTr,rLabel,pContext)
void cDevice::DoReadPreface(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  FD_DHV("cDevice("<<mName<<")::DoReadPreface()");
  //call base
  sDevice::DoReadPreface(rTr,"",pContext);

  // sense and digest pre 2.16 format
  Token token;
  rTr.Peek(token);
  if(token.IsString()) {
    mSystemFile = rTr.ReadString();
    return;
  }
  
  // loop my members
  while(true) {
    Token token;
    rTr.Peek(token);
    // system file
    if(token.IsBegin("DeviceFile")) {
      rTr.ReadBegin("DeviceFile", token);
      mSystemFile=token.AttributeStringValue("value");
      rTr.ReadEnd("DeviceFile");
      continue;
    }
    // break on unknown
    break;
  }
}


// Start(void)
void cDevice::Start(void) {
  //open comedi-device  
  if(mState!=Down) return;
  FD_DH("cDevice(" << mName << ")::Start(): open devices");
  mDev=comedi_open(mSystemFile.c_str());

  // throw exception if opening device failed
  if(!mDev) {
    std::stringstream errstr;
    errstr << "cannot open device /dev/comedi0 (inputs)";
    throw Exception("cDevice()::Start()", errstr.str(), 552);
  }

  // check some properties on input card
  for(int bit=0; bit<=mMaxBitAddress; bit++) {
 
    // map to subdevice/channel
    int sub=bit/32;
    int chan=bit%32;


    // dio config is not implemented for advantech boards (?)
    /*
    ComediInt32 res;
    // get info
    if(comedi_dio_get_config(mDev, sub, chan, &res) !=0) {
      std::stringstream errstr;
      errstr << "cannot open device " << mSystemFile << ": no digital io at bit address " << bit;
      throw Exception("cDevice()::Start()", errstr.str(), 552);
    }
    // is there a comedi input?
    if((!mInputPosEdgeIndexMap[bit].Empty()) || (!mInputNegEdgeIndexMap[bit].Empty())) {
      if(res != COMEDI_INPUT) {
        std::stringstream errstr;
        errstr << "cannot open device " << mSystemFile << ": no digital input at bit address " << bit;
        throw Exception("cDevice()::Start()", errstr.str(), 552);
      }
    }
    // is there a comedi output?
    if(!mOutputLevelIndexMap[bit].Empty()) {
      if(res != COMEDI_OUTPUT) {
        std::stringstream errstr;
        errstr << "cannot open device " << mSystemFile << ": no digital output at bit address " << bit;
        throw Exception("cDevice()::Start()", errstr.str(), 552);
      }
    }
    */

    // at least we can test whether we can read inputs
    lsampl_t val;
    if((!mInputPosEdgeIndexMap[bit].Empty()) || (!mInputNegEdgeIndexMap[bit].Empty())) {
      if(comedi_data_read(mDev,sub,chan,0,AREF_GROUND,&val)!=1) {
        std::stringstream errstr;
        errstr << "cannot open device " << mSystemFile << ": failed to test read from digital input at bit address " << bit;
        throw Exception("cDevice()::Start()", errstr.str(), 552);
      }
    }

  }

  // allocate image
  mComediSubdevs = (mMaxBitAddress / 32) + 1;
  if(mComediSubdevs<=0) mComediSubdevs=1;
  mpInputImage = new ComediInt32[mComediSubdevs];
  mpOutputImage = new ComediInt32[mComediSubdevs];
  // precompute mask
  for(int bit=0; bit<32; ++bit) mComediMask[bit] = 0x000000001U << bit;
  // call base
  sDevice::Start();
}


// Stop()
void cDevice::Stop(void) {
  //close comedi-device
  if(mState != Up && mState != StartUp) return;
  FD_DHV("cDevice(" << mName << ")::Stop()");
  // call base
  sDevice::Stop();
  // close device
  comedi_close(mDev);
  mDev=0;
  // invalidate buffers
  delete mpInputImage;
  delete mpOutputImage;
  mpInputImage=NULL;
  mpOutputImage=NULL;
}



// Input Hook: we use a bit image (libFAUDES 2.23)
bool cDevice::DoReadSignalsPre(void) {
  // read all bits to buffer
  for(int sub =0 ; sub<mComediSubdevs; ++sub) 
    comedi_dio_bitfield2(mDev, sub, 0, &mpInputImage[sub], 0);
  // never fail (i.e. ignore errors)
  return true;
}


// Input Hook: bit image does not need further attention
void cDevice::DoReadSignalsPost(void) {
}


//ReadSignal(int)
bool cDevice::DoReadSignal(int bit){
  // pre libFAUDES 2.23 bit-read (for reference)
  /*
  lsampl_t input=0;
  comedi_data_read(mDev,bit/32,bit%32,0,AREF_GROUND,&input);
  return (input!=0);
  */
  // read from buffer
  return (mpInputImage[bit/32] & mComediMask[bit%32]) != 0;
}


// Output Hook: we use bit-write
bool cDevice::DoWriteSignalsPre(void) { return true; }
void cDevice::DoWriteSignalsPost(void) {}


//WriteSignal(int,int)
void cDevice::DoWriteSignal(int bit, bool value){
  // write one actuator value, adressed by bit number (0 to 63);
  // ignore error
  lsampl_t output= (value ? 1 : 0);
  comedi_data_write(mDev,bit/32,bit%32,0,AREF_GROUND,output);
}


#endif // if comedi

} // namespace



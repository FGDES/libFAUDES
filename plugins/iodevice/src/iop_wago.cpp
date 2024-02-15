/** @file iop_wago.cpp provides access to wago-kbus*/

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009, Thomas Wittmann, Thomas Moor.
   Copyright (C) 2010, Thomas Moor.

*/


// include header
#include "iop_wago.h"

// only compile for use with wago-ipc
#ifdef FAUDES_IODEVICE_WAGO

// Include functions from "wago_ipc_kbus.c"
extern "C" {
  // Initialize libkbus
  extern int kbus_init(void);
  // Close libkbus
  extern int kbus_shutdown(void);
  // Start the kbus_thread used to build up the data-structures used to access the process-image.
  extern void start_kbus_thread(void);
  // Update a user-copy of the kbus process-image data-structures
  extern void kbus_update_image(void);
  // Test if failures occured which make reading form kbus impossible
  extern int kbus_running(void);
  // Obtain pointer to locked image 
  extern void *kbus_lock_input_image(void);
  // Obtain pointer to locked image 
  extern void *kbus_lock_output_image(void);
  // Release lock 
  extern void kbus_release_io_image(void);
}

namespace faudes {


// std faudes, incl dummy
FAUDES_TYPE_IMPLEMENTATION(WagoDevice,wDevice,sDevice)

// autoregister
AutoRegisterType<wDevice> gRtiRegisterWagoDevice("WagoDevice");

//constructor
wDevice::wDevice(void) : sDevice() {
  FD_DHV("wDevice(" << mName << ")::wDevice()");
  // have appropriate default label for token io
  mDefaultLabel = "WagoDevice";
  // pointer to kbus-I/O-image 
  pInputImage=0;
  pOutputImage=0;
  // no kbus-failure before kbus was even started
  mKbusOk=true;
}

//deconstructor
wDevice::~wDevice(void) {
  FD_DHV("wDevice(" << mName << ")::~wDevice()");
  Stop();
}

// Start(void)
void wDevice::Start(void) {
  // device is only functional in synchronous mode
  mSyncWrite=true;
  //open wago-device
  if(mState!=Down) return;
  FD_DH("wDevice(" << mName << ")::Start(): open devices");
  // initialize kbus
  int res = kbus_init();
  // set up kbus-thread
  start_kbus_thread();
  FD_DHV("wDevice(" << mName << ")::Start(): res = " << res );
  // throw execption if opening kbus failed
  if(res != 0) {
    std::stringstream errstr;
    errstr << "cannot open kbus";
    throw Exception("wDevice()::Start()", errstr.str(), 552);
  }
  // invalidate images
  pInputImage=0;
  pOutputImage=0;
  // call base (incl. reset)
  sDevice::Start();
  // pessimistic: assume  kbus-failure during start-up, let background thread figure
  mKbusOk=false;
  mState=StartUp;
}//end Start()

// Stop()
void wDevice::Stop(void) {
  //close kbus interface
  if(mState != Up && mState != StartUp) return;
  FD_DHV("wDevice(" << mName << ")::Stop()");
  // close kbus 
  kbus_shutdown();
  // call base
  sDevice::Stop();
  
}

//DoWrite(rTr,rLabel,pContext)
void wDevice::DoWritePreface(TokenWriter& rTw, const std::string& rLabel,  const Type* pContext) const {
  //dummy for token-output
  FD_DHV("wDevice("<<mName<<")::DoWritePreface()");
  //call base
  sDevice::DoWritePreface(rTw,"",pContext);
}


//DoReadPreface(rTr,rLabel,pContext)
void wDevice::DoReadPreface(TokenReader& rTr,const std::string& rLabel, const Type* pContext){
  //dummy for token-input
  FD_DHV("wDevice("<<mName<<")::DoReadPreface()");
  //call base
  sDevice::DoReadPreface(rTr,"",pContext);
}


// DoReadSignalsPre(void)
bool wDevice::DoReadSignalsPre(void) {

  static int okcnt=0;

  // We still have a valid input image: fine
  if(pInputImage) return true;

  // Prevent simultanuous reading and writing since this may
  // block the kbus thread.
  if(pOutputImage) return false;

  // Get input-image and lock kbus-mutex
  pInputImage=(char*) kbus_lock_input_image();
  // Test wether reading was valid
  int kok= kbus_running();   
  // If reading was not valid...
  if(kok==0) {
    // ... track and report ...
    if(mKbusOk) FD_WARN("wDevice(" << mName << ")::DoReadSignalsPre(): kbus error and friends ");    
    // ... invalidate input-image ...
    pInputImage=0;
    // ... releaset the mutex and ...
    kbus_release_io_image(); 
    // advertise failure
    mKbusOk=false;   
    okcnt=0;
  } 
  // If reading is valid ...
  if(kok!=0) {
    // ... wait for stable result ...
    if(!mKbusOk) {
      okcnt++;
      if(okcnt>50) {
         FD_WARN("wDevice(" << mName << ")::DoReadSignalsPre(): kbus recovered");    
         mKbusOk=true;
      }
      // ... release image while waiting.
      if(!mKbusOk) kbus_release_io_image();
    }
  }
  // return result
  return mKbusOk;
}


// DoReadSignalsPost(void)
void wDevice::DoReadSignalsPost(void) {
  // If the input-image is still valid, we assume that we hold the mutex
  // and, hence, must now release it.
  if(pInputImage)   kbus_release_io_image();
  // invalidate input-image.
  pInputImage=0;
}


//ReadSignal(int)
bool wDevice::DoReadSignal(int bit){
  // Read one input value, addressed by bit number (0 to 63);

  // Determine byte and bit address
  int byte = bit / 8;
  bit = bit % 8;

  // Read bit
  return ( pInputImage[byte] & (0x01 << (bit)) ) != 0x00;
}


// DoWriteSignalsPre(void)
bool wDevice::DoWriteSignalsPre(void) {
  // We have a valid input image: error, prevent simultaneous read/write
  if(pInputImage) return false;
  // We have a valid output image: fine
  if(pOutputImage) return true;
  // Get an image and lock it
  pOutputImage=(char*) kbus_lock_output_image();
  // Done
  return true;
}



// DoWrtieSignalsPost(void)
void wDevice::DoWriteSignalsPost(void) {
  // If there is an valid output-image, release kbus-mutex...
  if(pOutputImage)   kbus_release_io_image();
  // ...and invalidate image.
  pOutputImage=0;
}


//DoWriteSignal(int,int)
void wDevice::DoWriteSignal(int bit, bool value){
  // Write one actor value, adressed by bit number (0 to 63);
  
  // Determine byte and bit addresse.
  int byte = (bit) / 8;
  bit = (bit) % 8;
  // Report 
  FD_DHV("wDevice(" << mName << ")::DoWriteSignal(int): bit " << bit << " in byte " << byte);
 
  // Write value to output-image using bit-operations
  // Write a "1", or...
  if(value) pOutputImage[byte] |= (0x1 << (bit));
  // ... write a "0"
  else pOutputImage[byte] &= ~(0x1 << (bit));
}


} // namespace

#endif // end wago-support

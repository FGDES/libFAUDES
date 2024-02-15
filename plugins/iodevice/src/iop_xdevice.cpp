/** @file iop_xdevice.cpp Device container with vDevice interface */

/*
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008, Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/



#include "iop_xdevice.h"

#include "iop_simplenet.h"
#include "iop_comedi.h"
#include "iop_wago.h"
#include "iop_serial.h"
#include "iop_modbus.h"
#include "iop_d3ripURT.h" 
#include "iop_d3ripDART.h"

namespace faudes {

/*
 **********************************************
 **********************************************
 **********************************************

 implementation: xDevice

 **********************************************
 **********************************************
 **********************************************
 */

// std faudes
FAUDES_TYPE_IMPLEMENTATION(DeviceContainer,xDevice,vDevice)

// autoregister
volatile static AutoRegisterType<xDevice> gRtiRegisterDeviceContainer("DeviceContainer");

//constructor
xDevice::xDevice(void) : vDevice() {
  FD_DHV("xDevice(" << mName << ")::xDevice()");
  mName="IoDevices";
  // have appropriate default label
  mDefaultLabel="DeviceContainer";

  // auto register fix: 
  // for some reasons the global register objects do not work properly.
  // since the xdevice is autoregeistered by the rti, we ask the constructor
  // to register the other devices

#ifdef FAUDES_IODEVICE_SIMPLENET
  volatile static AutoRegisterType<nDevice> gRtiIORegisterSimplenetDevice("SimplenetDevice");
#endif
#ifdef FAUDES_IODEVICE_COMEDI
  volatile static AutoRegisterType<cDevice> gRtiIORegisterComediDevice("ComediDevice");
#endif
#ifdef FAUDES_IODEVICE_WAGO
  volatile static AutoRegisterType<wDevice> gRtiIORegisterWagoDevice("WagoDevice");
#endif
#ifdef FAUDES_IODEVICE_SERIAL
  volatile static AutoRegisterType<spiDevice> gRtiIORegisterSerialPiDevice("SpiDevice");
#endif
#ifdef FAUDES_IODEVICE_MODBUS
  volatile static AutoRegisterType<mbDevice> gRtiIORegisterSerialPiDevice("ModbusDevice");
#endif
#ifdef FAUDES_IODEVICE_D3RIP_URT
  volatile static AutoRegisterType<d3ripURTDevice> gRtiIORegisterD3ripUrtDevice("D3RipUrtDevice");
#endif
#ifdef FAUDES_IODEVICE_D3RIP_DART
  volatile static AutoRegisterType<d3ripDARTDevice> gRtiIORegisterD3ripDartDevice("D3RipDartDevice");
#endif

}

// destructor
xDevice::~xDevice(void) {
  FD_DHV("xDevice(" << mName << ")::~xDevice()");
  // loop to free devices.
  Iterator dit;
  for(dit=Begin();dit!=End();dit++){
    delete (*dit);
  }
}

//FromTokenReader(rTr)
xDevice* xDevice::FromTokenReader(TokenReader& rTr) {
  // construct from token reader
  FD_DHV("xDevice::FromTokenReader()");
  // allocate
  xDevice* dev=new xDevice();
  // read
  dev->DoRead(rTr);
  return dev;
}

//FromFile(rFilename)
xDevice* xDevice::FromFile(const std::string& rFileName) {
  // construct from file
  FD_DHV("xDevice()::FromFile(" << rFileName <<")");
  // peek first token
  TokenReader tr(rFileName);
  return FromTokenReader(tr);
}


//Status(void)
xDevice::DeviceState xDevice::Status(void){

  FD_DHV("xDevice(" << mName << ")::Status() ");
  //prepare result
  xDevice::DeviceState res;
  //vector to store states of all devices integrated in xdevice
  std::vector<xDevice::DeviceState> states;
  //vector iterator
  std::vector<DeviceState>::iterator vit;
  vit = states.begin();
  //device iterator
  iterator dit;
  //identify device state
  for(dit=Begin();dit!=End();dit++){
    //get state of current device and insert it in vector
    states.push_back((DeviceState)(*dit)->Status());
    //make sure it isnt the first entry
    if(vit != states.begin()){
      //if current entry doesnt equal previous
      //decide depending on last command
      if (*vit!=*vit--){
        //if last command was start: "StartUp"
        if(lastCommandWasStart){
          res = StartUp;
          break;
        }
        //if last command was stop: "ShutDown"
        if(!lastCommandWasStart){
          res = ShutDown;
          break;
        }
      }
    }
    //move on to next device
    vit++;
  }
  //if for-loop finished all entries of vector "states" are equal
  res =  *states.begin();
  return res;
}

// Start(void)
void xDevice::Start(void) {

  FD_DHV("xDevice(" << mName <<")::Start()");
  // start implies reset
  Reset();
  //start all existing devices
  Iterator dit;
  for(dit = Begin();dit!=End();dit++){
    (*dit)->Start();
  }
  //remember this command
  lastCommandWasStart = true;
}

// Stop(void)
void xDevice::Stop(void) {

  FD_DHV("xDevice(" << mName <<")::Stop()");
  // xDevice Reset
  Reset();
  // stop all existing devices
  Iterator dit;
  for(dit=Begin();dit!=End();dit++){
    (*dit)->Stop();
  }
  //remember this command
  lastCommandWasStart = false;
}

// Reset(void)
void xDevice::Reset(void){
  //clear dynamic data

  FD_DHV("xDevice(" << mName <<")::Reset()");
  // device-iterator
  Iterator dit;
  for(dit=Begin();dit!=End();dit++){
    (*dit)->Reset();
  }
  // call base to reset time and fifo
  vDevice::Reset();
}

// Clear(void)
void xDevice::Clear(void){
  //clear static data

  FD_DHV("xDevice(" << mName << ")::Clear()");
  // Stop running devices
  Stop();
  // device-iterator
  Iterator dit;
  // and clear all devices
  for(dit=Begin();dit!=End();dit++){
    (*dit)->Clear();
  }
  // destroy existing data
  for(dit=Begin();dit!=End();dit++){
    delete (*dit);
  }
  // compile with empty containers
  Compile();
}

// Insert(vDevice*)
void xDevice::Insert(vDevice* device){

  FD_DHV("xDevice("<<mName<<")::Insert(" << device << "):" <<  device->Name());
  // insert any vdevice, but no xdevice
  if(dynamic_cast<xDevice*>(device)) {
    std::stringstream errstr;
    errstr << "Attempt to insert xDevice into xDevice";
    throw Exception("xDevice::Compile", errstr.str(), 550);
  }
  // add to mDevices
  mDevices.push_back(device);
  // and update compiled data
  Compile();
}

//Insert(rFilename)
void xDevice::Insert(const std::string& rFileName){
  //inserts a device by filename
  FD_DHV("xDevice("<<mName<<")::Insert(rFileName)");
  //read device
  vDevice* pdev = vDevice::FromFile(rFileName);
  //insert device
  Insert(pdev);
  //and remember its name
  mDeviceNames.push_back(rFileName);
}


// Compile(void)
void xDevice::Compile(void) {
  //build up internal data-structures

  FD_DHV("xDevice("<<mName<<")::Compile()");
  Stop();
  // build up (event-idx,int)-map and memorize all existing events
  // prepare containers
  mInputs.Clear();
  mOutputs.Clear();
  mInputToDevice.clear();
  mOutputToDevice.clear();
  // temporary container
  EventSet tmpSenEvents;
  EventSet tmpActEvents;
  // event-iterator
  EventSet::Iterator eit;
  // device-iterator
  Iterator dit;
  // helper
  int j;
  // iterate over existing devices
  for(dit=Begin(), j=0;  dit!=End(); dit++,j++){
    // get events by Index
    tmpSenEvents = (*dit)->Inputs();
    tmpActEvents = (*dit)->Outputs();
    //insert output-events in map
    for(eit=tmpActEvents.Begin();eit!=tmpActEvents.End();eit++){
      // test if actual event already exists in map
      if(mOutputs.Exists(*eit)){
        //throw exeption
        std::stringstream errstr;
        errstr << "Event already exists!";
        throw Exception("xDevice()::Compile", errstr.str(), 550);
      }
      mOutputToDevice[*eit] = j;
    }
    // memorize events
    mInputs.InsertSet(tmpSenEvents);
    mOutputs.InsertSet(tmpActEvents);
    // tell the device which mutex/condition-pair to use
    (*dit)->UseCondition(pWaitMutex,pWaitCondition);
    (*dit)->UseBuffer(pBufferMutex,pInputBuffer);
  }//end iteration over existing devices
  // set time scale from first device
  if(Size()!=0) (*Begin())->TimeScale(mTimeScale);
}


//Configure(event,attr)
void xDevice::Configure(Idx event, const AttributeDeviceEvent& attr){
  (void) event; (void) attr;
  // cannot do vDevice style configuration of events

  FD_DHV("xDevice("<<mName<<")::Configure(Idx,attr): ");
  std::stringstream errstr;
  errstr << "Attempt to configure xDevice by event";
  throw Exception("xDevice::Configure(Idx,attr)", errstr.str(), 550);
}

//Configure(rEvents)
void xDevice::Configure(const EventSet& rEvents){
  (void) rEvents;
  // cannot do vDevice style configuration of events
  
  FD_DHV("xDevice("<<mName<<")::Configure(rEvents)");
  std::stringstream errstr;
  errstr << "Attempt to configure xDevice by events";
  throw Exception("xDevice::Configure", errstr.str(), 550);
}


//CurrentTime(void)
Time::Type xDevice::CurrentTime(void) {

  FD_DHV("xDevice("<<mName<<"): CurrentTime() ");
  // throw exception if there is no device
  if(Size()==0) {
    std::stringstream errstr;
    errstr << "xDevice owns no device!";
    throw Exception("xDevice::CurrentTime", errstr.str(),550);
  }
  // ask first device
  return (*Begin())->CurrentTime();
}


//CurrentTime(void)
long int xDevice::CurrentTimeMs(void) {
  //debug flag: say hello
  FD_DHV("xDevice("<<mName<<")::CurrentTimeMs() ");
  // throw exception if there is no device
  if(Size()==0) {
    std::stringstream errstr;
    errstr << "xDevice owns no device!";
    throw Exception("xDevice::CurrentTimeMs", errstr.str(),550);
  }
  // ask first device
  return (*Begin())->CurrentTimeMs();
}

//CurrentTime(now)
void xDevice::CurrentTime(Time::Type now) {
  //debug flag: say hello
  FD_DHV("xDevice("<<mName<<")::CurrentTime("<<now<<") ");
  // tell first device if such
  if(Size()!=0) (*Begin())->CurrentTime(now);
}

//CurrentTimeMs(now)
void xDevice::CurrentTimeMs(long int nowms) {
  //debug flag: say hello

  FD_DHV("xDevice("<<mName<<")::CurrentTimeMs("<<nowms<<") ");
  // tell first device if such
  if(Size()!=0) (*Begin())->CurrentTimeMs(nowms);
}

//DoWrite(rTr,rLabel)
void xDevice::DoWriteConfiguration(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rTw;

  FD_DHV("xDevice("<<mName<<")::DoWriteConfiguration()");
  // hard coded begin
  rTw.WriteBegin("Devices");
  // define device-iterator
  Iterator dit;
  //iterate over all devices 
  for(Idx i=0; i<Size(); i++) {
    //if device was read from an extra file
    if(mDeviceNames.at(i) != ""){ 
      //write path to TokenWriter
      FD_DHV("xDevice("<<mName<<")::DoWrite(): "<<mDeviceNames.at(i));
      rTw.WriteString(mDeviceNames.at(i)); //TODO: relativ path-names
    }
    //else device was directly read from base-config-file
    else if(mDeviceNames.at(i) == ""){
      //write configuration 
      FD_DHV("xDevice("<<mName<<")::DoWrite(): "<<mDeviceNames.at(i));
      (mDevices.at(i))->Write(rTw);
    }
  } 
  rTw.WriteEnd("Devices");
}


// ResetRequest()
bool xDevice::ResetRequest(void) {
  bool res=false;
  Iterator dit;
  for(dit=Begin();dit!=End();dit++){
    res = res | (*dit)->ResetRequest();
  }
  return res;
}

//DoRead(rTr,rLabel)
void xDevice::DoReadConfiguration(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;

  FD_DHV("xDevice("<<mName<<")::DoReadConfiguration()");
  //prepare token
  Token token;
  //prepare string 
  std::string filename = "";      //to store filename
  std::string dirname = "";  //to store directory
  std::string path;          //to store absolut path
  // have default section
  std::string label = rLabel;
  if(label=="") label = "Devices";
  // read begin
  rTr.ReadBegin(label);
  // fill mDevices with devices specified in given file
  while(!rTr.Eos("Devices")){
    //peek token
    rTr.Peek(token);
    // if Token is a String we assume its the name of a file containing a device
    if(token.Type()==Token::String) {
      //read Token
      rTr.Get(token);
      //import filename
      filename = token.StringValue();
      //build up path to base-file
      if(rTr.SourceMode()==TokenReader::File) dirname = ExtractDirectory(rTr.FileName());
      //build up path to specified file
      path = dirname.append(filename);
      //insert device
      Insert(path);
      continue;
    }
    // if its not a file it has to be a device
    else if(token.Type()==Token::Begin) {
      // read device
      vDevice* devp = vDevice::FromTokenReader(rTr);
      // insert device mDevices
      Insert(devp);
      //record that this device was read inline
      mDeviceNames.push_back("");
    }    
  }
  rTr.ReadEnd(label);
}


// WriteOutput(Idx)
void xDevice::WriteOutput(Idx output){
  FD_DHV("xDevice("<<mName<<")::WriteOutput()");
  // identify corresponding device(if idx is unique)
  int didx = mOutputToDevice[output];
  FD_DH("xDevice("<<mName<<")::WriteOutput(): " << output << " to " << didx);
  mDevices.at(didx)->WriteOutput(output);
}


// FlusgBufferes()
void xDevice::FlushOutputs(void) {
  // iterate over all participating devices
  for(Iterator dit=Begin();  dit!=End(); dit++)
    (*dit)->FlushOutputs();
}



} // name space



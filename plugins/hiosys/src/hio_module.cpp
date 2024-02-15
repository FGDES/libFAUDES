/** @file hio_module.cpp Class describing the I/O based hierarchy */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/

#include "hio_module.h"

namespace faudes { 

// Constructor
HioModule::HioModule(void){
}


  // Copy constructor
  HioModule::HioModule(const HioModule& rOtherHioModule) {
    mOpConstr = rOtherHioModule.OpConstr();
    mPlant = rOtherHioModule.Plant();
    mController = rOtherHioModule.Controller();
    mChildren = rOtherHioModule.Children();
    mEnvironment = rOtherHioModule.Environment();
    mEnvConstr = rOtherHioModule.EnvConstr();
    TypeHioModule(rOtherHioModule.TypeHioModule());
    mIndex = rOtherHioModule.Index();
    mName = rOtherHioModule.Name();
    Position(rOtherHioModule.Xpos(), rOtherHioModule.Ypos());
  }

  //HioModule::Clear()
  void HioModule::Clear()
  {
    mEnvConstr.Clear();
    mOpConstr.Clear();
    mController.Clear();
    mPlant.Clear();
    mIndex = 0;
    for (int i=0; i<5; i++)
      mType[i] = 0;
    mXpos = 0;
    mYpos = 0;
    mName = "";
    mChildren.clear();
  }

  // HioModule::Name()
  std::string HioModule::Name() const {
    return mName;
  }

  // HioModule::Name(rName)
  void HioModule::Name(const std::string& rName){
    mName = rName;
  }
	     
  // HioModule::Index()
  Idx HioModule::Index() const {
    return mIndex;
  }

  // HioModule::Index(Index)
  void HioModule::Index(const Idx Index){
    mIndex = Index;
  }
		 
  // HioModule::OpConstr(rOpConstr)
  void HioModule::OpConstr(const HioConstraint& rOpConstr){
    mOpConstr = rOpConstr;
  }
		 
  // HioModule::OpConstr()
  HioConstraint HioModule::OpConstr() const {
    return mOpConstr;
  }
	     
  // HioModule::Plant(HioPlant)
  void HioModule::Plant(const HioPlant& rHioPlant){
    mPlant = rHioPlant;
  }

  // HioModule::Plant()
  HioPlant HioModule::Plant() const {
    return mPlant;
  }

  // HioModule::Controller(HioController)
  void HioModule::Controller(const HioController& rHioController){
    mController = rHioController;
  }

  // HioModule::Controller()
  HioController HioModule::Controller() const {
    return mController;
  }
	
  // HioModule::Children(rChildren)
  void HioModule::Children(const std::vector<HioModule*>& rChildren) {
    mChildren = rChildren;
  }
	
  // HioModule::Children
  std::vector<HioModule*> HioModule::Children() const {
    return mChildren;
  }
	
  // HioModule::Environment(HioEnvironment)
  void HioModule::Environment(const HioEnvironment& rHioEnvironment){
    mEnvironment = rHioEnvironment;
  }

  // HioModule::Environment()
  HioEnvironment HioModule::Environment() const {
    return mEnvironment;
  }
	     
  // HioModule::EnvConstr(rEnvConstr)
  void HioModule::EnvConstr(const HioConstraint& rEnvConstr){
	     
    mEnvConstr = rEnvConstr;
  }    
		 
  // HioModule::EnvConstr()
  HioConstraint HioModule::EnvConstr() const {
    return mEnvConstr;
  }

  // HioModule::InsChild(rChild)
  void HioModule::InsChild(HioModule* rChild){
    mChildren.push_back(rChild);
  } 
		 
  /////////////////////////////////////////////////////////////////////
  // from here to end of file: design specific functions by M.Musunoi
  // will be outsourced to something like hiodesign.cpp
	
  // function to set mXpos and mYpos of the HioModule
  void HioModule::Position(const int Xpos, const int Ypos){
	     
    mXpos = Xpos;
    mYpos = Ypos;
  }

  // function to set the type of HioModule 
  void HioModule::TypeHioModule(int type[5]){
	     
    for (int i=0; i<=4; i++)
      mType[i] = type[i];
  }



  //ReadHioPlant: this function reads the models from the given library and
  //creates a general IOModulX which can be afterwards personalised. 
  void HioModule::ReadHioPlant(const std::string path)
  {
	    
    Generator OpConstr;
    Generator EnvConstr;

    std::string myType;
	    
    //read and set the IO Plant
    std::string HioPlantFile = path+"plant.gen";
    mPlant.Read(HioPlantFile.c_str());
	    
    //searching for the token reader "Type", and set the type of the HioPlant 
    TokenReader tReader(HioPlantFile.c_str());
    Token token;
    token.SetString("Type");
    tReader.ReadBegin("Type");
    while(tReader.Peek(token)) {
      // break on end
      if (token.Type() == Token::End) {
	break;
      }
      myType = tReader.ReadString();
      continue;
    }
	    
    //set mType 
    for(int i=0 ;i<=4;i++)
      {
	char tmpChar = myType.at(i);
	std::stringstream Str;
	Str << tmpChar;
	int d;
	Str >> d;
	            
	mType[i] = d;
      }
	            
    //read and set the Operator-Constraint    
    std::string LpFile = path+"constrP.gen";
    mOpConstr.Read(LpFile.c_str());
	    
	    
    //load and set the Environment-Constraint
    std::string LeFile = path+"constrE.gen";
    mEnvConstr.Read(LeFile.c_str());
	       
  } //end of  ReadHioPlant()


  //loadLibEnv: this function loads the model of interaction of IO-Plants with 
  //the environment and creates a general IOEnvironment. 
  void HioModule::ReadHioEnv(const std::string path)
	                
  {
    std::string EnvFile = path+"environment.gen";
    mEnvironment.Read(EnvFile.c_str());
  }


  //AdjusTimedGenerator: this function converts a generator by renaming all events from 
  //"CBx_..." to "CBi_...";
  void HioModule::AdjusTimedGenerator(const Generator& rOldGen,
				      const int i,
				      Generator& rResGen)
  {
    EventSet newAlph;
    TransSet::Iterator tit;
    StateSet::Iterator sit;
    EventSet::Iterator evit;
    std::string toSeti = ToStringInteger(i);
    std::string toSet = "CB"+toSeti+"_";
	    
	    
    //check if generator not already in converted form; if already converted, return
    //sperk: throw exception?
    for (evit = rOldGen.AlphabetBegin(); evit != rOldGen.AlphabetEnd(); ++evit)
      {        
	        
	std::string oldName = rOldGen.Alphabet().SymbolicName(*evit);
	//int loc1 = oldName.find(toSet, 0);
	//if( loc1 != std::string::npos )
	if( oldName.find(toSet, 0) != std::string::npos )
	  {
	    std::cout<<"At least one event already converted";
	    rResGen = rOldGen;
	    return;
	  }
      }

    // first state to be inserted in the new, empty rResult generator
    int state = 1;
	    
    //rename events an insert the resulting alphabet in rResult  
    AdjustAlphabet(rOldGen.Alphabet(), i, newAlph);
    rResGen.InjectAlphabet(newAlph);
	            
    //insert states in rResult;
    for (sit = rOldGen.StatesBegin(); sit != rOldGen.StatesEnd(); ++sit)
      {      
	rResGen.InsMarkedState(ToStringInteger(state));
	//if actual state also init state then mark it as init state in rResult too;
	if(rOldGen.ExistsInitState(state))
	  rResGen.SetInitState(ToStringInteger(state));
	                
	state++;
      }
	        
    //iterate through all states
    for (sit = rOldGen.StatesBegin(); sit != rOldGen.StatesEnd(); ++sit)
      {   
	//iterate through all transitions of the actual state; rename the events
	//of each transition and insert the new transition in rResult 
	for (tit = rOldGen.TransRelBegin(*sit); tit != rOldGen.TransRelEnd(*sit); ++tit)
	  {            
	    std::string oldName = rOldGen.EventName(tit->Ev);
	    int loc1 = oldName.find("_", 0);
	    std::string newName = oldName.erase(0,loc1+1);
	    newName = toSet + oldName; 
	                                     
	    Idx idx_event = rResGen.EventIndex(newName);
	    rResGen.SetTransition(tit->X1, idx_event, tit->X2);
	  }
      }
	    
  } //END ADJUSTGENERATOR


  // AdjustHioPlant(): convenience function (derived from AdjusTimedGenerator())to
  // allow also the conversion of HioPlants
  void HioModule::AdjustHioPlant(
				 const HioPlant& rOldHioPlant,
				 const int i,
				 HioPlant& rResGen)
  {
    EventSet newAlph;
    EventSet::Iterator evit;
    TransSet::Iterator tit;
    StateSet::Iterator sit;
    std::string toSeti = ToStringInteger(i);
    std::string toSet = "CB"+toSeti+"_";
	    
    //check if generator not already in converted form
    for (evit = rOldHioPlant.AlphabetBegin(); evit != rOldHioPlant.AlphabetEnd(); ++evit)
      {        
	        
	std::string oldName = rOldHioPlant.Alphabet().SymbolicName(*evit);
	//int loc1 = oldName.find(toSet, 0);
	//if( loc1 != std::string::npos )
	if( oldName.find(toSet, 0) != std::string::npos )
	  {
	    std::cout<<"At least one event already converted";
	    rResGen = rOldHioPlant;
	    return;
	  }
      }
    // first state to be inserted in the new, empty rResult generator
    int state = 1;
	    
    //rename events by preserving the events attributes
    //Yp-Events  
    AdjustAlphabet(rOldHioPlant.YpEvents(), i, newAlph);
    for(evit = newAlph.Begin(); evit != newAlph.End(); ++evit)
      rResGen.InsYpEvent(*evit);
    newAlph.Clear();
	    
    //Up-Events    
    AdjustAlphabet(rOldHioPlant.UpEvents(), i, newAlph);
    for(evit = newAlph.Begin(); evit != newAlph.End(); ++evit)
      rResGen.InsUpEvent(*evit);
    newAlph.Clear();
	    
    //Ye-Events
    AdjustAlphabet(rOldHioPlant.YeEvents(), i, newAlph);
    for(evit = newAlph.Begin(); evit != newAlph.End(); ++evit)
      rResGen.InsYeEvent(*evit);
    newAlph.Clear();
	    
    //Ue-Events
    AdjustAlphabet(rOldHioPlant.UeEvents(), i, newAlph);
    for(evit = newAlph.Begin(); evit != newAlph.End(); ++evit)
      rResGen.InsUeEvent(*evit);
    newAlph.Clear();
	       
    //insert states in rResult;
    for (sit = rOldHioPlant.StatesBegin(); sit != rOldHioPlant.StatesEnd(); ++sit)
      {      
	rResGen.InsMarkedState(ToStringInteger(state));
	//if actual state also init state then mark it as init state in rResult too;
	if(rOldHioPlant.ExistsInitState(state))
	  rResGen.SetInitState(ToStringInteger(state));
	                
	state++;
      }
    //iterate through all states
    for (sit = rOldHioPlant.StatesBegin(); sit != rOldHioPlant.StatesEnd(); ++sit)
      {   
	//iterate through all transitions of the actual state; rename the events
	//of each transition and insert the new transition in rResult 
	for (tit = rOldHioPlant.TransRelBegin(*sit); tit != rOldHioPlant.TransRelEnd(*sit); ++tit)
	  {            
	    std::string oldName = rOldHioPlant.EventName(tit->Ev);
	    int loc1 = oldName.find("_", 0);
	    std::string newName = oldName.erase(0,loc1+1);
	    newName = toSet + oldName; 
	                                     
	    Idx idx_event = rResGen.EventIndex(newName);
	    rResGen.SetTransition(tit->X1, idx_event, tit->X2);
	  }
      }
  } //End of AdjustHioPlant()
	  

  // AdjustAlph(): this function converts an alphabet by renaming all events from 
  // "CBx_..." to "CBi_...";
  // Warning: This function does not preserves the attributes of the events! 
  void HioModule::AdjustAlphabet(
				 const EventSet& rOldAlph, 
				 const int i,
				 EventSet& rResAlph)
  {    
    EventSet::Iterator evit;
    std::string toSeti = ToStringInteger(i);
    std::string toSet = "CB"+toSeti+"_";
	    
    //check if alphabet not already in converted form
    for (evit = rOldAlph.Begin(); evit != rOldAlph.End(); ++evit)
      {               
	std::string oldName = rOldAlph.SymbolicName(*evit);
	//int loc1 = oldName.find(toSet, 0);
	//if( loc1 != std::string::npos )
	if( oldName.find(toSet, 0) != std::string::npos )
	  {
	    std::cout<<"At least one event already converted";
	    rResAlph = rOldAlph;
	    return;
	  }
      }
	    
    //iterate through the alphabet and adjust every event
    for (evit = rOldAlph.Begin(); evit != rOldAlph.End(); ++evit)
      {        
	std::string oldName = rOldAlph.SymbolicName(*evit);
	int loc1 = oldName.find("_", 0);
	std::string newName1 = oldName.erase(0,loc1+1);
	std::string newName = toSet + oldName;
	        
	rResAlph.Insert(newName);
	               
      }
  } //End of AdjustAlphabet


  // AdjustEnvironmet: This function adjusts the master copy (IOEnviromntX) to
  // the two IO-Plants. 
  void HioModule::AdjustHioEnvironment(
				       const HioEnvironment& rHioEnvX,
				       const HioModule* rHioModule1,
				       const HioModule* rHioModule2,
				       HioEnvironment& rResEnv)
  {
    EventSet tmpAlph;
    EventSet::Iterator evit;
    StateSet::Iterator sit;
    TransSet::Iterator tit;
	    
	    
	    
    tmpAlph = rHioEnvX.Alphabet();
    int leftId, rightId; //
    int state = 1;
    // strings to replace the standard event names: CBx_, CBy_, CBxy_
    std::string toRepl_left, toRepl_right, toRepl_ext;
	        
    // check the relative position of the 2 HioModule to each other and prepare
    // the strings to be set, according to the Id of each IOMOdule
    if (rHioModule1->Xpos() <= rHioModule2->Xpos()) 
      {
	//HioModule1 at the left of HioModule2
	leftId = rHioModule1->Index();
	toRepl_left = "CB"+ToStringInteger(leftId)+"_";
	                             
	rightId = rHioModule2->Index();
	toRepl_right = "CB"+ToStringInteger(rightId)+"_";
	                             
	toRepl_ext = "CB"+ToStringInteger(leftId)+
	  ToStringInteger(rightId)+"_";
      }
    else
      {
	//HioModule1 at the right of HioModule2
	leftId = rHioModule2->Index();
	toRepl_left = "CB"+ToStringInteger(leftId)+"_";
	                                 
	rightId = rHioModule1->Index();
	toRepl_right = "CB"+ToStringInteger(rightId)+"_";
	                                 
	toRepl_ext = "CB"+ToStringInteger(leftId)+
	  ToStringInteger(rightId)+"_";
      }
	                                 
    //check if generator not already in converted form
    for (evit = rHioEnvX.AlphabetBegin(); evit != rHioEnvX.AlphabetEnd(); ++evit)
      {        
	        
	std::string oldName = rHioEnvX.Alphabet().SymbolicName(*evit);
	//int loc1 = oldName.find(toRepl_ext, 0);
	//if( loc1 != std::string::npos )
	if( oldName.find(toRepl_ext, 0) != std::string::npos )
	  {
	    std::cout<<"At least one event already converted";
	    rResEnv = rHioEnvX;
	    return;
	  }
      }
	    
    //**********************Adjust the Alphabet****************
    //iterate through all events and replace 
    //                 CBx_-events with toRepl_left
    //                 CBy_-events with toRepl_right
    //                 CBxy_-events with toRepl_ext
    for (evit = tmpAlph.Begin(); evit != tmpAlph.End(); ++evit)
      {
	//get actual event name
	std::string eventName = tmpAlph.SymbolicName(*evit);
	        
	// actual event is a "left-Plant" event? (Ye or Ue) 
	std::string::size_type loc1 = eventName.find("CBx_", 0);
	if (loc1 != std::string::npos)
	  {
	    eventName.erase(0, 4);
	    std::string newEvent = toRepl_left+eventName;
	    //std::string newEvent = eventName.replace(0,toRepl_left.size(), toRepl_left);
	    if(rHioEnvX.IsYe(*evit))
	      {
		rResEnv.InsYeEvent(newEvent);
	      }
	    else
	      {
		rResEnv.InsUeEvent(newEvent);
	      }
	  }
	else
	  {
	    // actual event is a "right-Plant" event?
	    std::string::size_type loc2 = eventName.find("CBy_", 0);
	    if (loc2 != std::string::npos)
	      {
		eventName.erase(0, 4);
		std::string newEvent = toRepl_right+eventName;
		if(rHioEnvX.IsYe(*evit))
		  {
		    rResEnv.InsYeEvent(newEvent);
		  }
		else
		  {
		    rResEnv.InsUeEvent(newEvent);
		  }
	      }
	        
	    else
	      {
		// actual event is a "external interaction" event? (YL or UL)
		std::string::size_type loc3 = eventName.find("CBxy_", 0);
		if (loc3 != std::string::npos)
		  {
		    eventName.erase(0, 5);
		    std::string newEvent = toRepl_ext+eventName;
		    if(rHioEnvX.IsYl(*evit))
		      {                                                                                             
			rResEnv.InsYlEvent(newEvent);
		      }
		    else
		      {
			rResEnv.InsUlEvent(newEvent);
		      }
		  }
	                         
	      }
	  }
      }//*************************** End Adjust Alphabet***************
	       
    // insert states into result generator                     
    for (sit = rHioEnvX.StatesBegin(); sit != rHioEnvX.StatesEnd(); ++sit)
      {      
	rResEnv.InsMarkedState(ToStringInteger(state));
	//if actual state also init state then mark it as init state in rResEnv too;
	if(rHioEnvX.ExistsInitState(state))
	  rResEnv.SetInitState(ToStringInteger(state));
	                
	state++; 
      } //End Insert States    
	           

	//***************************Adjust Transitions************************
	// iterate through all states and all transitions. For every trasition check the 
	// event name, and rename it as described in the sequence "Adjust Alphabet" 
    for (sit = rHioEnvX.StatesBegin(); sit != rHioEnvX.StatesEnd(); ++sit)
      {   
	//iterate through all transitions of the actuall state; 
	for (tit = rHioEnvX.TransRelBegin(*sit); tit != rHioEnvX.TransRelEnd(*sit); ++tit)
	  {            
	    std::string eventName = rHioEnvX.EventName(tit->Ev);
	             
	    // actual event is a "left-Plant" event?
	    std::string::size_type loc1 = eventName.find("CBx_", 0);
	    if (loc1 != std::string::npos)
	      {
		eventName.erase(0, 4);
		std::string newEvent = toRepl_left+eventName;
		eventName = newEvent;
	      }
	    else 
	      {
		// actual event is a "right-Plant" event?
		std::string::size_type loc2 = eventName.find("CBy_", 0);
		if (loc2 != std::string::npos)
		  {
		    eventName.erase(0, 4);
		    std::string newEvent = toRepl_right+eventName;
		    eventName = newEvent;
		  }
		else
		  {
		    // actual event is a "external interaction" event?
		    std::string::size_type loc3 = eventName.find("CBxy_", 0);
		    if (loc3 != std::string::npos)
		      {
			eventName.erase(0, 5);
			std::string newEvent = toRepl_ext+eventName;
			eventName = newEvent;
		      }
		  }
	      }
	                    
	    // insert transition into result                 
	    Idx idx_event = rResEnv.EventIndex(eventName);
	    rResEnv.SetTransition(tit->X1, idx_event, tit->X2);
	  } // end for-loop over transition
      } // end for-loop over states

  } // end of AdjustHioEnvironment()    


  // AdjustHioController: This function adjust an already computed IOController to 
  // apply with the two IO-Plants (that enforces a well-defined specification).
  void AdjustHioController(
			   const HioController& rHioController,
			   const HioModule* rHioModule1,
			   const HioModule* rHioModule2,
			   HioController& rResEnv)
  {
    EventSet tmpAlph;
    EventSet::Iterator evit;
    StateSet::Iterator sit;
    TransSet::Iterator tit;
	        
    tmpAlph = rHioController.Alphabet();
    int leftId, rightId; //
    int state = 1;
	    
    // strings to replace the standard event names: CBx_, CBy_, CBxy_
    std::string toRepl_left, toRepl_right, toRepl_ext;
	    
	            
    // check the relative position of the 2 HioModule to each other and prepare
    // the strings to be set, according to the Id of each IOMOdule
    if (rHioModule1->Xpos() <= rHioModule2->Xpos()) 
      {
	//HioModule1 at the left of HioModule2
	leftId = rHioModule1->Index();
	toRepl_left = "CB"+ToStringInteger(leftId)+"_";
	                             
	rightId = rHioModule2->Index();
	toRepl_right = "CB"+ToStringInteger(rightId)+"_";
	                             
	toRepl_ext = "CB"+ToStringInteger(leftId)+
	  ToStringInteger(rightId)+"_";
      }
    else
      {
	//HioModule1 at the right of HioModule2
	leftId = rHioModule2->Index();
	toRepl_left = "CB"+ToStringInteger(leftId)+"_";
	                                 
	rightId = rHioModule1->Index();
	toRepl_right = "CB"+ToStringInteger(rightId)+"_";
	                                 
	toRepl_ext = "CB"+ToStringInteger(leftId)+
	  ToStringInteger(rightId)+"_";
      }
	    
    //check if generator not already in converted form
    for (evit = rHioController.AlphabetBegin(); evit != rHioController.AlphabetEnd(); ++evit)
      {        
	        
	std::string oldName = rHioController.Alphabet().SymbolicName(*evit);
	//int loc1 = oldName.find(toRepl_ext, 0);
	//if( loc1 != std::string::npos )
	if( oldName.find(toRepl_ext, 0) != std::string::npos )
	  {
	    std::cout<<"At least one event already converted";
	    rResEnv = rHioController;
	    return;
	  }
      }
	    
    //**********************Adjust the Alphabet****************
    //iterate thru all events and replace 
    //                 CBx_-events with toRepl_left
    //                 CBy_-events with toRepl_right
    //                 CBxy_-events with toRepl_ext
    for (evit = tmpAlph.Begin(); evit != tmpAlph.End(); ++evit)
      {
	//get actual event name
	std::string eventName = tmpAlph.SymbolicName(*evit);
	        
	// actual event is a "left-Plant" event?
	std::string::size_type loc1 = eventName.find("CBx_", 0);
	if (loc1 != std::string::npos)
	  {
	    eventName.erase(0, 4);
	    std::string newEvent = toRepl_left+eventName;            
	    rResEnv.InsEvent(newEvent);
	  }
	else
	  {
	    // actual event is a "right-Plant" event?
	    std::string::size_type loc2 = eventName.find("CBy_", 0);
	    if (loc2 != std::string::npos)
	      {
		eventName.erase(0, 4);
		std::string newEvent = toRepl_right+eventName;
		rResEnv.InsEvent(newEvent);
	      }
	    else
	      {
		// actual event is a "external interaction" event?
		std::string::size_type loc3 = eventName.find("CBxy_", 0);
		if (loc3 != std::string::npos)
		  {
		    eventName.erase(0, 5);
		    std::string newEvent = toRepl_ext+eventName;
		    rResEnv.InsEvent(newEvent);
		  }
	      }
	  }
      }//*************************** End Adjust Alphabet***************
	       
    // insert states into result generator                     
    for (sit = rHioController.StatesBegin(); sit != rHioController.StatesEnd(); ++sit)
      {      
	rResEnv.InsMarkedState(ToStringInteger(state));
	//if actual state also init state then mark it as init state in rResEnv too;
	if(rHioController.ExistsInitState(state))
	  rResEnv.SetInitState(ToStringInteger(state));
	                
	state++; 
      } //End Insert States    
	           

	//***************************Adjust Transitions************************
	// iterate through all states and all transitions. For every trasition check the 
	// event name, and rename it as described in the sequence "Adjust Alphabet" 
    for (sit = rHioController.StatesBegin(); sit != rHioController.StatesEnd(); ++sit)
      {   
	//iterate through all transitions of the actuall state; 
	for (tit = rHioController.TransRelBegin(*sit); tit != rHioController.TransRelEnd(*sit); ++tit)
	  {            
	    std::string eventName = rHioController.EventName(tit->Ev);
	             
	    // actual event is a "left-Plant" event?
	    std::string::size_type loc1 = eventName.find("CBx_", 0);
	    if (loc1 != std::string::npos)
	      {
		eventName.erase(0, 4);
		std::string newEvent = toRepl_left+eventName;
		eventName = newEvent;
	      }
	    else 
	      {
		// actual event is a "right-Plant" event?
		std::string::size_type loc2 = eventName.find("CBy_", 0);
		if (loc2 != std::string::npos)
		  {
		    eventName.erase(0, 4);
		    std::string newEvent = toRepl_right+eventName;
		    eventName = newEvent;
		  }
		else
		  {
		    // actual event is a "external interaction" event?
		    std::string::size_type loc3 = eventName.find("CBxy_", 0);
		    if (loc3 != std::string::npos)
		      {
			eventName.erase(0, 5);
			std::string newEvent = toRepl_ext+eventName;
			eventName = newEvent;
		      }
		  }
	      }
	                    
	    // insert transition into result                 
	    Idx idx_event = rResEnv.EventIndex(eventName);
	    rResEnv.SetTransition(tit->X1, idx_event, tit->X2);
	  } // end for-loop over transition
      } // end for-loop over states

  } //End Adjust IOController


  // RenameHioModule: this function personalises a HioModule by renaming the content
  // of the single modules member
  void HioModule::RenameHioModule(const int i)
  {
    HioPlant tmpHioPlant;
    Generator tmpOpConstr, tmpEnvConstr;
    HioEnvironment tmpHioEnv;
	  
    //adjust the mPlant member     
    AdjustHioPlant(mPlant, i, tmpHioPlant);
	     
    //adjust the constraints
    AdjusTimedGenerator(mOpConstr, i, tmpOpConstr);
    AdjusTimedGenerator(mEnvConstr, i, tmpEnvConstr);
	     
    //set the members
    mPlant = tmpHioPlant;
    mOpConstr = tmpOpConstr;
    mEnvConstr = tmpEnvConstr;
	     
    //adjust the Id
    mIndex = i;
	     
    //adjust the name: 
    mName = "CB"+ToStringInteger(i);
	     
    //check if also an environment is available, if so adjust it too
    if(mChildren.size() != 0)
      {
	AdjustHioEnvironment(mEnvironment, mChildren[0], mChildren[1], tmpHioEnv);
	mEnvironment = tmpHioEnv;
      }
	      
  }// end of RenameHioModule()


  // chooseSpec: this function searches through an apappropriate folder for available
  // specifications.
  std::vector<std::string> HioModule::ChooseSpec(
						 const std::string path)
  {
    int capacity = 0;
    int size;
    std::string loc_path = "";
    std::set<std::string> tmpSpecSet;
    std::vector<std::string> listSpec, pathList;
    std::list<std::string>::iterator itstr;
	    
    //check to max capacity
    size = mChildren.size();
    std::cout<<"Number of children: "<<size<<std::endl;
    for(int i=0; i<size; i++)
      {
	//The "mType"-array stores at pos 0 the capacity of an plant model
	capacity = mChildren[i]->TypeHioModule()[0] + capacity;
      }
	    
    //prepare the local path to read from
    std::string cap = "SpecCB";
    std::vector<std::string> allPath;
    for (int i=1; i<=capacity;i++)
      {
	cap = cap + ToStringInteger(i);
	loc_path = path+cap+"/";
	allPath.push_back(loc_path);
      }
    //list the available specifications
    for (int i=0; i<capacity;i++)
      {
	// e.g.: resultin string = ../Spec/SpecCB123/
	loc_path = allPath[i];
	std::cout<<"Final path: "<<loc_path<<std::endl;
	//check folder for specification and show them to the user
	tmpSpecSet = ReadDirectory(loc_path);
	std::set< std::string >:: iterator fit=tmpSpecSet.begin();
	for(; fit!=tmpSpecSet.end(); fit++)
	  {
	    listSpec.push_back(*fit);
	    std::string tmpPath;
	    tmpPath = loc_path+*fit+"/";
	    pathList.push_back(tmpPath);
	  }
      }
	    
    std::cout<<"There are "<<listSpec.size()<<" spec. available"<<std::endl;
    size = listSpec.size();
    for(int i=0; i<size;i++)
      {
	std::cout<<"Option: "<<i+1<<":"<<listSpec[i]<<std::endl;
      }
    //delete....
    //int choise=1;
    //std::cin>>choise;
	    
    //encode specification type to the user
    //EncodeType(listSpec[choise-1].c_str());
	    
    //loc_path = loc_path+listSpec[choise-1]+"/";
	    
    //loc_path = pathList[choise-1];
    //ReadHioPlant(loc_path.c_str());
	    
    //std::cout<<"My type is: "<<mType[0]<<mType[1]<<mType[2]<<mType[3]<<mType[4]<<std::endl;
	    
    //do this as a final step....
    //RenameHioModule(mId);
    return listSpec;
	          
  }// end of chooseSpec()

  // function to compute the IOController
  void HioModule::Compute() {
	         
    // FindController: check if an IO-Controller is already available; if case 
    // of a positive result (print the IOController statistics) and inform the 
    // user about the found IO-Controller. The user can decide whether or not
    // the found IO-Controller will be used, or to compute a new IO-Controller.   
    if(FindController())
      {
	HioController tmpHioController;
	std::string loc_path, choice;

	loc_path = MyPath()+"ioController.gen";
	tmpHioController.Read(loc_path.c_str());
	tmpHioController.SWrite();
	std::cout<<"Controller found..! Accept(Y)? / New controller(N)?"<<std::endl;
	std::cin>>choice;
	     
	if(choice == "y" || choice == "Y")
	  {
	    mController = tmpHioController;
	    return;
	  }
	     
      }
    else
      {
	std::cout<<"Controller NOT found..!"<<std::endl;
      }
    Generator IOShufflemin, IOShuffle;     
    std::cout.flush()<<"Starting IOShuffle...1"<<std::endl;
    HioShuffle_Musunoi(mChildren[0]->Plant(), mChildren[1]->Plant(), 1, IOShufflemin);
	    
    std::cout.flush()<<"                  ...done!"<<std::endl;
    StateMin(IOShufflemin, IOShuffle);
	    
    //Start parallel composition with environment and load the local constraints
    Generator ioShCB12parEnv, locConstr;
	    
    Parallel(IOShuffle, mEnvironment, ioShCB12parEnv);
    //remove generator that are not in use anymore
    IOShuffle.Clear();
	     
    Generator tmp1, tmp2; // generator for intermediate result 
    Parallel(mChildren[0]->OpConstr(), mChildren[0]->EnvConstr(), tmp1);
    Parallel(mChildren[1]->OpConstr(), mChildren[1]->EnvConstr(), tmp2);
    Parallel(tmp1, tmp2, locConstr);
	    
    //remove generator that are not in use anymore
    tmp1.Clear();
    tmp2.Clear();
	     
	     
    //Start HIO_SYNTHESIS
	    
    Generator extConstr;
	     
    //external constraints
    Parallel(mOpConstr, mEnvConstr, extConstr);
	     
    std::cout<<"**********************//////\\\\\\*******************"<<std::endl;
    std::cout<<"Everything went fine so far...startig HIO_SYNTHESIS..."<<std::endl;
    std::cout<<"**********************\\\\\\//////*******************"<<std::endl;
	          
    //temp generators and event-sets for intermediate results
    Generator tmpController, tmpHioGen; 
    EventSet Yp, Up, Yc, Uc, Ye, Ue;
	    
    Yp = (mChildren[0]->Plant().YpEvents()+mChildren[1]->Plant().YpEvents());
    Up = (mChildren[0]->Plant().UpEvents()+mChildren[1]->Plant().UpEvents());
    Yc = (mPlant.YpEvents());
    Uc = (mPlant.UpEvents());
    Ye = (mPlant.YeEvents());
    Ue = (mPlant.UeEvents()); 
	     
    tmpHioGen.Assign(mPlant);
	     
    //start the hio-synthesis 
    HioSynth_Musunoi(ioShCB12parEnv, mPlant, extConstr, locConstr, Yp, Up, tmpController);
	     
    mController = tmpController;
	    
	     
    EventSet::Iterator evit;
    // As the result of the controller synthesis is a genarator, we set the
    // event properties in order to obtain a HioController  
    for(evit = Yc.Begin(); evit != Yc.End(); ++evit)
      mController.SetYc(*evit);

    for(evit = Uc.Begin(); evit != Uc.End(); ++evit)
      mController.SetUc(*evit);
	    
    Yp.DWrite(); 
    for(evit = Yp.Begin(); evit != Yp.End(); ++evit)
      mController.SetYp(*evit);              
	     
    for(evit = Up.Begin(); evit != Up.End(); ++evit)
      mController.SetUp(*evit);
	     
    std::cout.flush()<<"...Done"<<std::endl;
	    
    std::string filename = "Controller"+ToStringInteger(mIndex)+".gen";
    mController.Write(filename.c_str());
	    
	    
  } //End of Compute();


  //FindController(): this function searches in the folder of a choosen 
  //specification for an already computed IO-controller which enforces this
  //specification (sought: "ioController.gen")
  bool HioModule::FindController()
  {
	    
    //the path where to look up for the sought IO-Controller is based on the
    //type of the specification  
    std::set<std::string> folderContent; 
    std::string loc_path;
	    
    //find the local path
    loc_path = MyPath();
    folderContent = ReadDirectory(loc_path);
    std::set< std::string >:: iterator fit=folderContent.begin();
    for(; fit!=folderContent.end(); fit++)
      if (*fit== "ioController.gen")
	return true;
	                
    return false;
  } //End of FindController()


  // Save: this function saves the computed controller in the same folder with 
  // the specification that this controller enforces
  void HioModule::SaveController()
  {
    std::string loc_path;
    loc_path = MyPath();    
    loc_path = loc_path+"ioController.gen";
	    
    mController.Write(loc_path.c_str());
  } //End of SaveController()


  //MyPath: based on the type of the HioModule, this is a convenience function to
  // establish the local path where to Write/Read from.
  std::string HioModule::MyPath()
  {
    int capacity = mType[0];
    std::vector<std::string> folderContent;
    std::string cap, loc_path, sub_folder;
	    
	    
    for (int i=1; i<=capacity; i++)
      {
	cap = cap + ToStringInteger(i);
      }
	             
    for (int i=0; i<5;i++)
      {
	sub_folder = sub_folder+ToStringInteger(mType[i]);
      }
	        
    //path to search 
    loc_path = "Data/Spec/SpecCB"+cap+"/"+sub_folder+"/";
    return loc_path;        
  } //end MyPath()


  /**********************************************
   **********************************************
   *
   * functions to call properties of the HioModule
   **********************************************
   ***********************************************/               

  // function to call the x-Positon of the HioModule
  int HioModule::Xpos() const {
	      
    return mXpos;
  }


  // function to call the y-Positon of the HioModule
  int HioModule::Ypos() const {
	      
    return mYpos;
  }

  // function to call the type of the HioModule
  int* HioModule::TypeHioModule() const {
	    
    int* type = new int[5];   
    for (int i=0; i<=4; i++)
      type[i] = mType[i];
	    
    return type;
  }


  // function to encode to Type of the HioModule 
  void HioModule::EncodeType(const std::string type) {
	     
    std::cout<<"***********Capacity: "<<type.at(0)<<"******"<<std::endl;
    std::cout<<"*Take from left: "<<type.at(1)<<"* "<<" *Take from right: "<<type.at(2)<<"*"<<std::endl;
    std::cout<<"*Deliver to left: "<<type.at(3)<<"* "<<" *Deliver to right: "<<type.at(4)<<"*"<<std::endl;
	     
  }

  /*******************************************************************************
   ***************************END OF IOMODULE CLASS DEFINITION********************
   ******************************************************************************/
	 
	 
  // GroupHioModules: This function groups two or more HioModules to a new HioModule.
  void GroupHioModules(
		       const std::vector<HioModule*>& rChildren,
		       HioModule& rParentModule)
  {
    int size = rChildren.size();
    float Id =0;
    for (int i =0; i < size; i++)
      {
	rParentModule.InsChild(rChildren[i]);
	std::cout<<"insert #"<<i<<"- ModuleID: "<<rChildren[i]->Index();
	// Id is composed of children Id's
	Id = pow(10, size-i-1)*rChildren[i]->Index()+Id;
	std::cout<<".."<<Id<<std::endl;
      }
    std::cout<<"Id set!"<<std::endl;
    //read the environment describing the interaction between the grouped components    
    HioEnvironment HioEnv("Data/envCBxy.gen");
		
    //set the new Id of the module, and adjust the environment
    int intId = int(Id);
    rParentModule.Environment(HioEnv);
    rParentModule.RenameHioModule(intId);
	    
  }// end of GroupHioModule()    


  //CreateSpec()
  void CreateSpec(int mType[5], HioPlant& rHioSpec, Generator& constrP, Generator& constrE)
  {
	     
    // initiate several variables; variables to store information about the number
    // of the take and deliver actions and last known state.
    int loaded, fl, fr, tol, tor, i_lastState, index;
    std::string s_lastQy, s_lastQup, s_lastQue, s_lastState, s_lastQy_stby, s_newQy_stby; 
    std::string s_lastQupFL, s_lastQupFR, s_lastQup2L, s_lastQup2R, s_newQy_FR, s_newQy_FL, s_newQy_2L, s_newQy_2R;
	     
    s_lastQup = s_lastQupFL = s_lastQupFR = s_newQy_stby = "ErrQy"; 
    fl=fr=tol=tor=i_lastState=index=loaded=0;
	                   
    //Insert alphabet
    rHioSpec.InsYpEvent("CBx_free");
    rHioSpec.InsYpEvent("CBx_busy");
    rHioSpec.InsUpEvent("CBx_stby");
    rHioSpec.InsUpEvent("CBx_FL");
    rHioSpec.InsUpEvent("CBx_FR");
    rHioSpec.InsUpEvent("CBx_2L");
    rHioSpec.InsUpEvent("CBx_2R");
    rHioSpec.InsYeEvent("CBx_req_fl");
    rHioSpec.InsYeEvent("CBx_req_fr");
    rHioSpec.InsYeEvent("CBx_wpar_fl");
    rHioSpec.InsYeEvent("CBx_wpar_fr");
    rHioSpec.InsYeEvent("CBx_wpar_tl");
    rHioSpec.InsYeEvent("CBx_wpar_tr");
    rHioSpec.InsUeEvent("CBx_pack");
    rHioSpec.InsUeEvent("CBx_nack");
	     
    //Insert error state
    rHioSpec.InsMarkedState("ErrQy");
	     
    //1. Create init state and the first transitions; in order to save the last added
    //   state and/or transition, set the respective variable ("s_lastQup", "i_lastState")
    rHioSpec.InsInitState("1");
    rHioSpec.SetMarkedState("1");
    rHioSpec.InsMarkedState("2");
    rHioSpec.SetTransition("1", "CBx_free", "2");
    s_lastQup = "2";
    rHioSpec.SetTransition("2", "CBx_stby", "1");
    i_lastState = 2;

    /*************** Interpretation of mType[] *******
     ************* Capacity:        mType[0] *********
     ************* Take from left:  mType[1] *********
     ************* Take from right: mType[2] *********
     ************* Deliver to left: mType[3] *********
     ************* Deliver to left: mType[4] *********
     ************************************************/

	             
    //2. so called "Qy_stby" - states are of states were we decied if an other 
    //   "Take from..." or "Deliver to..." - cycle must be added to the specification.
    //   This are also states where the specification branch out, so it is important
    //   to keep this state in order to implement all transitions starting here.

    //   "index" - stores the information about the capacity, i.e. how many times we 
    //   need to add the "Take from..." "Deliver from" cycle 
    for(int i=0; i<mType[0]; i++)
      {
	s_lastQy_stby = s_newQy_stby;
	index++;
	     
	//if no piece loaded yet, deliver actions would lead to an Error state 
	if(loaded == 0)
	  {
	    rHioSpec.SetTransition(s_lastQup, "CBx_2L", "ErrQy");
	    s_lastQup2L = "ErrQy";
	    rHioSpec.SetTransition(s_lastQup, "CBx_2R", "ErrQy");
	    s_lastQup2R = "ErrQy";
	  }     
	     
	// the value of "fl" holds how many "Take from left"-cycles we already implemented
	// in the specification. If the fl < requested  "Take from left"- operations, 
	// implement an other one. 
	if(fl<mType[1])
	  {              
	    // if index = 1 then we are in the first implementation cycle
	    // so need to insert a new state. (in the next cycles, this
	    // state will have be already inserted -> see "else" statement) 
	    if(!(index > 1))
	      { 
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
	      }
	    else 
	      {
		// hold that the last inserted state is "Qy"-state that 
		// we arrive through a "Take from" - transition
		s_lastState = s_newQy_FL;
	      }
	                    
	    //insert the corresponding "Take from" - transition
	    rHioSpec.SetTransition(s_lastQup, "CBx_FL", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                    
	    //insert the corresponding "request from" - transition; 
	    //therefor we first need to insert a new state
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_req_fl", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                    
	    //insert the corresponding "acknowledge" - transition
	    //therefor we first need to insert a new state
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                    
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	                    
	    //insert the corresponding "busy" - transition
	    //therefor we first need to insert a new state
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_busy", s_lastState);
	    s_lastQupFL = s_lastState;
	    i_lastState++;
	                    
	    //update the status of the loaded number of pieces
	    loaded = index;
	    //update the number of implemented "Take from Left" - cycles
	    fl++;
	                    
	  }
	//Up must be free -> insert transition to error-state;
	else
	  {
	    rHioSpec.SetTransition(s_lastQup, "CBx_FL", "ErrQy");
	    s_lastQupFL = "ErrQy";
	  }
	                   
	                   
	//as "Take from left" or "Take from right" are simetrical, the implementation
	//is also the same as above
	// the value of "fr" holds how many "Take from right"-cycles we already implemented
	// in the specification. If the fr < requested  "Take from right"- operations, 
	// implement an other one. 
	if(fr<mType[2])
	  {
	    // if index = 1 then we are in the first implementation cycle
	    // so need to insert a new state. (in the next cycles, this
	    // state will have be already inserted -> see "else" statement)
	    if(!(index > 1))
	      { 
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
	      }
	    else 
	      {
		// hold that the last inserted state is "Qy"-state that 
		// we arrive through a "Take from" - transition
		s_lastState = s_newQy_FR;
	      }
	                         
	    //insert the corresponding "Take from" - transition
	    rHioSpec.SetTransition(s_lastQup, "CBx_FR", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                    
	    //insert the corresponding "request from" - transition; 
	    //therefor we first need to insert a new state
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_req_fr", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                    
	    //insert the corresponding "acknowledge" - transition
	    //therefor we first need to insert a new state
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                    
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	                    
	    //insert the corresponding "busy" - transition
	    //therefor we first need to insert a new state       
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_busy", s_lastState);
	    s_lastQupFR = s_lastState;
	    i_lastState++;
	                    
	    //update the status of the loaded number of pieces 
	    loaded = index;
	    //update the number of implemented "Take from Right" - cycles
	    fr++;
	  }
	//Up must be free -> insert transition to error-state;
	else
	  {
	    rHioSpec.SetTransition(s_lastQup, "CBx_FR", "ErrQy");
	    s_lastQupFR = "ErrQy";
	  }

	// as far as the last QupFL or QupFR did not lead to an error state (i.e. the 
	// requested number of  "Take from..." achieved) insert a new stbyState in order 
	// to start the new cycle of "Take from..."
	if((s_lastQupFL != "ErrQy") ||(s_lastQupFR != "ErrQy"))
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    s_newQy_stby = s_lastState;
	    i_lastState++;

	  }

	//here is the standby case handeled, if before a "Take from left" - command occured
	if(s_lastQupFL != "ErrQy")
	  {
	    // insert the stby transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQupFL, "CBx_stby", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;                   
	                                   
	    // insert the corresponding environment transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fl", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	               
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_stby);
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	               
	  }

	//here is the standby case handeled, if before a "Take from right" - command occured
	if(s_lastQupFR != "ErrQy")
	  {
	    // insert the stby transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQupFR, "CBx_stby", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;                   
	               
	    // insert the corresponding environment transition                    
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fr", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	               
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_stby);
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	  }

	//setup the stanby-loop
	if((s_lastQupFL != "ErrQy") ||(s_lastQupFR != "ErrQy"))
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_newQy_stby, "CBx_busy", s_lastState);
	    s_lastQup = s_lastState;
	    i_lastState++;
	    rHioSpec.SetTransition(s_lastState, "CBx_stby", s_newQy_stby);
	  }
	                       


	//insert shared 2L-state if the number of "Deliver to Left" transitions is not 
	//already acheived. By shared we mean the "Deliver to Left" transition
	//starting from the stby state, where no communication with the environment occurs   
	if(tol<mType[3])
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    s_newQy_2L = s_lastState;
	    i_lastState++;
	  }
	else tol++;

	//insert 'shared' 2R-state if the number of "Deliver to Right" transitions is not 
	//already acheived; By shared we mean the "Deliver to Right" transition
	//starting from the stby state, where no communication with the environment occurs;
	if(tor<mType[4])
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    s_newQy_2R = s_lastState;
	    i_lastState++;  
	  }
	else tor++;
	     
	// if at least one piece is available (fl or fr != 0) continue with the
	// implementation of the 2L action
	if(tol<mType[3] && (fl !=0 ||fr !=0))
	  {
	    // if the last FL led to an Error State, then don't implement
	    // the "Deliver to..." action (as this would start from an 
	    // Error State - ErrQy)
	    if(s_lastQupFL != "ErrQy")
	      {
		//insert the corresponding "Deliver to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQupFL, "CBx_2L", s_lastState);
		s_lastQy = s_lastState;
		i_lastState++;                   
	                                   
		//insert the corresponding "request to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fl", s_lastState);
		s_lastQue = s_lastState;
		i_lastState++;
	                                   
		//Enivronment acknowledge                    
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_2L);
		//Ue must be free -> insert transition to error-state
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");              
	      }
	                    
	    // if the last FR led to an Error State, then don't implement
	    // the "Deliver to..." action (as this would start from an 
	    // Error State - ErrQy)               
	    if(s_lastQupFR != "ErrQy")
	      {
		//insert the corresponding "Deliver to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);              
		rHioSpec.SetTransition(s_lastQupFR, "CBx_2L", s_lastState);
		s_lastQy = s_lastState;
		i_lastState++;                   
	                                   
		//insert the corresponding "request to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fr", s_lastState);
		s_lastQue = s_lastState;
		i_lastState++;
	                                   
		//Enivronment acknowledge                    
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_2L);
		//Ue must be free -> insert transition to error-state
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	                                   
	                                   
	      }
	    // Both deliver actions (2L, 2R) contiue on
	    // the same path
	    // Environment receieved the piece
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_newQy_2L, "CBx_wpar_tl", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                                   
	    if(loaded ==1)
	      {
		//up to here only one piece loaded -> go back to init state
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", "1");
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	      }
	    else
	      {
		//if more then one piece loaded go to the last stby state
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_lastQy_stby);
		//Ue must be free -> insert transition to error-state
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	      }
	                                   
	    //increase number of delivered pieces
	    tol++;
	     
	  }
	     
	// Uc must be free: if last Qup was an Error state, still insert the "Deliver to..."
	// action leading also to an Error state  
	else
	  {
	    if(s_lastQupFL != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQupFL, "CBx_2L", "ErrQy");
	      }
	    if(s_lastQupFR != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQupFR, "CBx_2L", "ErrQy");
	      }
	  }
	     
	// if at least one piece is available (fl or fr != 0) continue with the
	// implementation of the 2R action       
	if(tor<mType[4] && (fl !=0 ||fr !=0))
	  {
	    // if the last FL led to an Error State, then don't implement
	    // the "Deliver to..." action (as this would start from an 
	    // Error State - ErrQy)
	    if(s_lastQupFL != "ErrQy")
	      {              
		//insert the corresponding "Deliver to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQupFL, "CBx_2R", s_lastState);
		s_lastQy = s_lastState;
		i_lastState++;                   
	                                   
		//insert the corresponding "request to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fl", s_lastState);
		s_lastQue = s_lastState;
		i_lastState++;
	                                   
		//Enivronment acknowledge                                   
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_2R);
		//Ue must be free -> insert transition to error-state
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	                                                                      
	      }
	                    
	    // if the last FR led to an Error State, then don't implement
	    // the "Deliver to..." action (as this would start from an 
	    // Error State - ErrQy)               
	    if(s_lastQupFR != "ErrQy")
	      {
		//insert the corresponding "Deliver to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQupFR, "CBx_2R", s_lastState);
		s_lastQy = s_lastState;
		i_lastState++;                   
	                                   
		//insert the corresponding "request to..." - transition
		s_lastState = ToStringInteger(i_lastState+1);
		rHioSpec.InsMarkedState(s_lastState);
		rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fr", s_lastState);
		s_lastQue = s_lastState;
		i_lastState++;
	                    
		//Enivronment acknowledge
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_2R);
		//Ue must be free -> insert transition to error-state
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	                                                                      
	      }
	    // Both deliver actions (2L, 2R) contiue on
	    // the same path
	    // Environment receieved the piece
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_newQy_2R, "CBx_wpar_tr", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                                   
	    if(loaded ==1)
	      {
		//up to here only one piece loaded -> go back to init state
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", "1");
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	      }
	    //if more then one piece loaded go to the last stby state
	    else
	      {
		rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_lastQy_stby);
		//Ue must be free -> insert transition to error-state
		rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	      }
	                                   
	    // increase number of delivered pieces               
	    tor++;
	     
	  }
	// Uc must be free: if last Qup was an Error state, still insert the "Deliver to..."
	// action leading also to an Error state 
	else
	  {
	                                   
	    if(s_lastQupFL != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQupFL, "CBx_2R", "ErrQy");
	      }
	    if(s_lastQupFR != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQupFR, "CBx_2R", "ErrQy");
	      }
	  }

	//insert the next FL action after an FL or FR action already occured; 
	if(fl<mType[1])
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    s_newQy_FL = s_lastState;
	    i_lastState++;
	  }

	//insert the next FL action after an FL or FR action already occured; 
	if(fr<mType[2])
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    s_newQy_FR = s_lastState;
	    i_lastState++;
	  }

	// if last action was a FL, implement the corresponding environment events (wpar_fl)
	// before continue with the next take from action
	if((s_lastQupFL != "ErrQy") && fl <mType[1])
	     
	  {
	    //insert the "Take from..." transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQupFL, "CBx_FL", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                   
	    //insert the environment transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fl", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                                        
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_FL);
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");
	               
	  }
	//Up must be free -> insert transition to error-state;
	else
	  if(fl<mType[1])
	    {
	      rHioSpec.SetTransition(s_lastQupFL, "CBx_FL", "ErrQy");
	    }
	// if last action was a FL, implement the corresponding environment events (wpar_fl)
	// before continue with the next take from action
	if((s_lastQupFL != "ErrQy") && fr <mType[2])
	     
	  {
	    //insert the "Take from..." transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQupFL, "CBx_FR", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                   
	    //insert the environment transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fl", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                    
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_FR);
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");

	  }
	//Up must be free -> insert transition to error-state;
	else
	  if(fl<mType[1])
	    {
	      rHioSpec.SetTransition(s_lastQupFL, "CBx_FR", "ErrQy");
	    }


	// if last action was a FR, implement the corresponding environment events (wpar_fl)
	// before continue with the next take from action
	if((s_lastQupFR != "ErrQy") && fl <mType[1])
	     
	  {
	    //insert the "Take from..." transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQupFR, "CBx_FL", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                    
	    //insert the environment transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fr", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;
	                    
	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_FL);
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");

	  }
	//Up must be free -> insert transition to error-state;
	else
	  if (fr<mType[2])
	    {
	      rHioSpec.SetTransition(s_lastQupFR, "CBx_FL", "ErrQy");
	    }
	                   
	// if last action was a FR, implement the corresponding environment events (wpar_fl)
	// before continue with the next take from action
	if((s_lastQupFR != "ErrQy") && fr <mType[2])
	     
	  {
	    //insert the "Take from..." transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQupFR, "CBx_FR", s_lastState);
	    s_lastQy = s_lastState;
	    i_lastState++;
	                   
	    //insert the environment transition
	    s_lastState = ToStringInteger(i_lastState+1);
	    rHioSpec.InsMarkedState(s_lastState);
	    rHioSpec.SetTransition(s_lastQy, "CBx_wpar_fr", s_lastState);
	    s_lastQue = s_lastState;
	    i_lastState++;

	    rHioSpec.SetTransition(s_lastQue, "CBx_pack", s_newQy_FR);
	    //Ue must be free -> insert transition to error-state
	    rHioSpec.SetTransition(s_lastQue, "CBx_nack", "ErrQy");                               
	  }
	//Up must be free -> insert transition to error-state;
	else
	  if (fr<mType[2])
	    {
	      rHioSpec.SetTransition(s_lastQupFR, "CBx_FR", "ErrQy");
	    }

	  

	// insert the remaining "Deliver to..." transition;
	if(tol <= mType[3])
	  rHioSpec.SetTransition(s_lastQup, "CBx_2L", s_newQy_2L);
	else
	  // Up must be free -> insert transition to error-state;
	  rHioSpec.SetTransition(s_lastQup, "CBx_2L", "ErrQy");
	       
	// insert the remaining "Deliver to..." transition;
	if(tor <= mType[4])
	  rHioSpec.SetTransition(s_lastQup, "CBx_2R", s_newQy_2R);
	else
	  // Up must be free -> insert transition to error-state;
	  rHioSpec.SetTransition(s_lastQup, "CBx_2R", "ErrQy");
	    
	// finaly, if the numer of maximal capacity is achieved, insert remaining transition
	// in order to ensure the I/O plant format of the specification;
	if(index == mType[0])
	  {
	    if(s_lastQup != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQup, "CBx_FR", "ErrQy");
		rHioSpec.SetTransition(s_lastQup, "CBx_FL", "ErrQy");
	      }
	         
	    if(s_lastQupFL != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQupFL, "CBx_FL", "ErrQy");
		rHioSpec.SetTransition(s_lastQupFL, "CBx_FR", "ErrQy");
	      }
	         
	    if(s_lastQupFR != "ErrQy")
	      {
		rHioSpec.SetTransition(s_lastQupFR, "CBx_FR", "ErrQy");
		rHioSpec.SetTransition(s_lastQupFR, "CBx_FL", "ErrQy");
	      }

	    rHioSpec.SWrite();

	    // for the creaded specification create now also the constraints which describe
	    // the condition of completeness and Yp-liveness
	    CreateConstraint(mType, constrP, constrE);

	  }// end if

      }// end for-loop: see comment at begin of the  (main)loop -> 
    // "index" - stores the information about the capacity, i.e. how many
    // times we need to add the "Take from..." "Deliver from" cycle 

  }// end createSpec() 


  // This function creates constraints which describe the condition of completeness
  // and Yp-liveness of a Specification. The implementation follows the same algorithm
  // as the CreateSpec() function, and has the same limitation: it is only for use 
  // with a specific model
  void CreateConstraint(int mType[5], Generator& constrP, Generator& constrE)
  {
	    
    int i_lastState, fl, fr, tol, tor;
    i_lastState = fl = fr = tol = tor = 0;
	    
    std::string s_lastState, s_lastQup, s_lastQy, s_newQy, s_return;
    s_lastState = s_lastQup = s_lastQy = s_newQy = s_return = "";
	    
    // the environment constraint constrE: this is an automata with one state,
    // one event and no transitions
    constrE.InsEvent("CBx_nack");
    constrE.InsInitState("1");
    constrE.SetMarkedState("1");
	    
    // the operator constraint constrP
    constrP.InsEvent("CBx_free");
    constrP.InsEvent("CBx_busy");
    constrP.InsEvent("CBx_stby");
    constrP.InsEvent("CBx_FL");
    constrP.InsEvent("CBx_FR");
    constrP.InsEvent("CBx_2L");
    constrP.InsEvent("CBx_2R"); 
	    
    // insert the states and transitions that are independent of the type of the 
    // constraint
    constrP.InsInitState("1");
    constrP.SetMarkedState("1");
    s_return = "1";
    s_lastQy = "1";
    constrP.InsMarkedState("2");
    constrP.SetTransition("1", "CBx_free", "2");
    constrP.SetTransition("1", "CBx_busy", "2");
    i_lastState = 2;
    s_lastQup = "2";

    // start the main loop   
    for (int i=0; i<mType[0]; i++)
      {
	   
	//insert shared state Qy
	if(fl<mType[1] || fr<mType[2])
	  {
	    s_lastState = ToStringInteger(i_lastState+1);
	    constrP.InsMarkedState(s_lastState);
	    s_newQy = s_lastState;
	    constrP.SetTransition(s_lastQup, "CBx_stby", s_lastQy);
	    i_lastState++;

	  }
	// insert "Take from left" transition if number of needed transitions not 
	// already achieved 
	if(fl<mType[1])
	  {
	    constrP.SetTransition(s_lastQup, "CBx_FL", s_newQy);
	    fl++;
	  }
	    
	// insert "Take from right" transition if number of needed transitions not 
	// already achieved
	if(fr<mType[2])
	  {
	    constrP.SetTransition(s_lastQup, "CBx_FR", s_newQy);
	    fr++;
	  }
	// i>0 only after we implemented at least one "Take from..." action; if i==0
	// "Deliver to..." action will be not implemented, as a deliver action is not 
	// possible yet
	if(i>0)
	  {
	    //insert 2L-Trans
	    if(tol<mType[3])
	      {
		// insert corresponding "Deliver to..." transition
		// and increase the number of implemented "Deliver
		// to..." transitions
		constrP.SetTransition(s_lastQup, "CBx_2L", s_return);
		tol++;
	      }

	    //insert 2R-Trans
	    if(tor<mType[4])
	      {
		// insert corresponding "Deliver to..." transition
		// and increase the number of implemented "Deliver
		// to..." transitions
		constrP.SetTransition(s_lastQup, "CBx_2R", s_return);
		tor++;
	      }
	                           
	    // store the last state
	    s_return = s_lastQy;
	  }
	                     
	// implement stby loop
	s_lastState = ToStringInteger(i_lastState+1);
	constrP.InsMarkedState(s_lastState);
	constrP.SetTransition(s_newQy, "CBx_free", s_lastState);
	constrP.SetTransition(s_newQy, "CBx_busy", s_lastState);
	constrP.SetTransition(s_lastState, "CBx_stby", s_newQy);
	s_lastQup = s_lastState;
	i_lastState++;
	      
	// as i starts at 0, check if the limit is achieved and implemet the remaining 
	// "Deliver to..." transitions
	if(i+1 == mType[0])
	  {
	    if(tol<mType[3])
	      {
		constrP.SetTransition(s_lastQup, "CBx_2L", s_return);
		tol++;
	      }

	    if(tor<mType[4])
	      {
		constrP.SetTransition(s_lastQup, "CBx_2R", s_return);
		tor++;
	      }
	  }
	// store the last state
	s_lastQy = s_newQy;
	 
      }//end for-loop - see comment at begin of the loop -> 
    // "index" - stores the information about the capacity, i.e. how many
    // times we need to add the "Take from..." "Deliver from" cycle 
	        
	          
  }// End create Constraint
                        
} // end namespace faudes*******end namespace faudes********end namespace faudes

/********************************END*******************************************/

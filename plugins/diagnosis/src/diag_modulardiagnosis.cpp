/** @file diag_modulardiagnosis.cpp Functions to test modular diagnosability and compute modular diagnosers.
*/

/*

Copyright Tobias Barthel, Klaus Schmidt, Thomas Moor

*/

#include "diag_modulardiagnosis.h"


using namespace std;

namespace faudes {

///////////////////////////////////////////////////////////////////////////////
// Functions for modular diagnosability
///////////////////////////////////////////////////////////////////////////////


// IsModularDiagnosable(): RTI wrapper
bool IsModularDiagnosable(const SystemVector& rGsubs, const GeneratorVector& rKsubs, string& rReportString) {
  FD_DD("IsModularDiagnosable()");

  // if the vectors have different size, the input is invalid
  if(rGsubs.Size() != rKsubs.Size() ){
    stringstream errstr;
    errstr << "Number of specifications (" << rKsubs.Size() << ") does not equal number of subsystems ("<< rGsubs.Size() << ")" << endl;
    throw Exception("IsModularDiagnosable()", errstr.str(), 304);	
  }
	
  // reorganize as std vector
  std::vector<const System*> gen;
  std::vector<const Generator*> spec;
  Idx i; 

  for(i = 0; i < rGsubs.Size(); ++i) 
    gen.push_back(&rGsubs.At(i));
  for(i = 0; i < rKsubs.Size(); ++i)
    spec.push_back(&rKsubs.At(i));
	
  // invoke function
  return IsModularDiagnosable(gen, spec, rReportString);
}



// IsModularDiagnosable()
bool IsModularDiagnosable(const vector< const System* >& rGSubs, const vector< const Generator* >& rKSubs, std::string& rReportString) {
  
  FD_DD("IsModularDiagnosable()");
	
  // clear report
  rReportString.clear();
  // verify dimensions  
  if(rGSubs.size() != rKSubs.size()) {
    stringstream errstr;
    errstr << "Number of specifications (" << rKSubs.size() << ") does not equal number of subsystems ("<< rGSubs.size() << ")" << endl;
    throw Exception("IsModularDiagnosable()", errstr.str(), 304);
  }
  // assemble shared events
  EventSet sigmaCup;
  EventSet sigmaCap;
  vector<EventSet> sigmaiCapVector;
  for(unsigned int i = 0; i < rGSubs.size(); ++i) // alphabet union
    sigmaCup.InsertSet(rGSubs.at(i)->Alphabet());

  FD_DD("all events: " + sigmaCup.ToString());
  for(EventSet::Iterator eit=sigmaCup.Begin(); eit!=sigmaCup.End(); eit++) { // overall shared events
    int cnt=0;
    for(unsigned int i = 0; i < rGSubs.size(); ++i) {
      if(rGSubs.at(i)->ExistsEvent(*eit)) 
	cnt++;
      if(cnt>1){ 
	sigmaCap.Insert(*eit); 
	break;
      }
    }
  }
  sigmaCup.Name("Sigma_cup");
  sigmaCap.Name("Sigma_cap");
  FD_DD("Shared Events: " << sigmaCap.ToString());
  // shared events per component
  for(unsigned int i = 0; i < rGSubs.size(); ++i)
    sigmaiCapVector.push_back(sigmaCap * rGSubs.at(i)->Alphabet() );
	
  // Compute abstraction alphabet for each component such that loop-preserving observer is fulfilled
  vector<EventSet> sigmaAbstVector;
  vector<System> genAbstVector;
  EventSet sigmaAbst;
  for(Idx i = 0; i < rGSubs.size(); ++i){
    sigmaAbstVector.push_back(EventSet() );
    LoopPreservingObserver(*rGSubs.at(i), sigmaiCapVector.at(i), sigmaAbstVector[i]);
    sigmaAbst = sigmaAbst + sigmaAbstVector.at(i); // overall abstraction alphabet
    // Compute the abstraction of each subsystem
    genAbstVector.push_back(System() );
    Project(*rGSubs.at(i),sigmaAbstVector.at(i),genAbstVector[i]);
    FD_DD("AbstractionAlphabet of Automaton " + ToStringInteger(i) + " " + sigmaAbstVector.at(i).ToString());
  }
  // Verify modular diagnosability for each subsystem
  EventSet obsEvents;
  System plant, plantAbst, spec;
  cParallel(genAbstVector,plantAbst); // abstracted plant
  bool diagnosable = true;
  for(unsigned int i = 0; i < rGSubs.size(); ++i){	
    plant.Clear();
    Parallel(*rGSubs.at(i),plantAbst,plant); // plant for verification
    Parallel(plant,*rKSubs.at(i),spec);
    plant.ClrObservable(plant.Alphabet() ); 
    plant.SetObservable(rGSubs.at(i)->ObservableEvents() ); // only observable events of the subsystem are observable
    std::string reportString;
    if(IsLanguageDiagnosable(plant,spec,reportString) == false ){
      FD_DD("Plant " + ToStringInteger(i) + " fails");
      diagnosable = false;
      rReportString += "Subsystem " + ToStringInteger(i) + " fails ";
    }
    else{
      FD_DD("Plant " + ToStringInteger(i) + " works");
      rReportString += "Subsystem " + ToStringInteger(i) + " works ";
    }
  }
  return diagnosable;
}


// IsModularDiagnosable(): RTI wrapper
bool ModularDiagnoser(const SystemVector& rGsubs, const GeneratorVector& rKsubs, GeneratorVector& rDiagSubs, string& rReportString) {
  FD_DD("ModularDiagnoser");

  // if the vectors have different size, the input is invalid
  if(rGsubs.Size() != rKsubs.Size() ){
    stringstream errstr;
    errstr << "Number of specifications (" << rKsubs.Size() << ") does not equal number of subsystems ("<< rGsubs.Size() << ")" << endl;
    throw Exception("ModularDiagnoser()", errstr.str(), 304);	
  }


  // reorganize as std vector
  std::vector<const System*> gen;
  std::vector<const Generator*> spec;
  Idx i; 
  for(i = 0; i < rGsubs.Size(); ++i) 
    gen.push_back(&rGsubs.At(i));
  for(i = 0; i < rKsubs.Size(); ++i)
    spec.push_back(&rKsubs.At(i));
	
  // prepare result
  std::vector<Diagnoser*> diag;

  // invoke function
  bool diagnosable = ModularDiagnoser(gen, spec, diag, rReportString);

  // retrieve result (no copy, using explicit reference)
  for(i = 0; i < diag.size(); i++)
    rDiagSubs.Append(diag.at(i) );
  rDiagSubs.TakeOwnership();

  // done
  return diagnosable;
}


// ModularDiagnoser(rGsubs, rKsubs, rDiagsubs, rReportString)
bool ModularDiagnoser(const std::vector< const System* >& rGSubs, const std::vector< const Generator* >& rKSubs, std::vector<Diagnoser*>& rDiagSubs, std::string& rReportString){
  
  FD_DD("ModularDiagnoser()");
	
  // clear report
  rReportString.clear();

  // verify dimensions  
  if(rGSubs.size() != rKSubs.size()) {
    stringstream errstr;
    errstr << "Number of specifications (" << rKSubs.size() << ") does not equal number of subsystems ("<< rGSubs.size() << ")" << endl;
    throw Exception("ModularDiagnoser()", errstr.str(), 304);
  }

  // assemble shared events
  EventSet sigmaCup;
  EventSet sigmaCap;
  vector<EventSet> sigmaiCapVector;
  for(unsigned int i = 0; i < rGSubs.size(); ++i) // alphabet union
    sigmaCup.InsertSet(rGSubs.at(i)->Alphabet());

  FD_DD("all events: " + sigmaCup.ToString());
  for(EventSet::Iterator eit=sigmaCup.Begin(); eit!=sigmaCup.End(); eit++) { // overall shared events
    int cnt=0;
    for(unsigned int i = 0; i < rGSubs.size(); ++i) {
      if(rGSubs.at(i)->ExistsEvent(*eit)) 
	cnt++;
      if(cnt>1){ 
	sigmaCap.Insert(*eit); 
	break;
      }
    }
  }
  sigmaCup.Name("Sigma_cup");
  sigmaCap.Name("Sigma_cap");
  FD_DD("Shared Events: " << sigmaCap.ToString());
  // shared events per component
  for(unsigned int i = 0; i < rGSubs.size(); ++i)
    sigmaiCapVector.push_back(sigmaCap * rGSubs.at(i)->Alphabet() );
	
  // Compute abstraction alphabet for each component such that loop-preserving observer is fulfilled
  vector<EventSet> sigmaAbstVector;
  vector<System> genAbstVector;
  EventSet sigmaAbst;
  for(Idx i = 0; i < rGSubs.size(); ++i){
    sigmaAbstVector.push_back(EventSet() );
    LoopPreservingObserver(*rGSubs.at(i), sigmaiCapVector.at(i), sigmaAbstVector[i]);
    sigmaAbst = sigmaAbst + sigmaAbstVector.at(i); // overall abstraction alphabet
    // Compute the abstraction of each subsystem
    genAbstVector.push_back(System() );
    Project(*rGSubs.at(i),sigmaAbstVector.at(i),genAbstVector[i]);
    FD_DD("AbstractionAlphabet of Automaton " + ToStringInteger(i) + " " + sigmaAbstVector.at(i).ToString());
  }
  // Verify modular diagnosability for each subsystem
  EventSet obsEvents;
  System plant, plantAbst, spec;
  cParallel(genAbstVector,plantAbst); // abstracted plant
  bool diagnosable = true;
  for(unsigned int i = 0; i < rGSubs.size(); ++i){	
    plant.Clear();
    Parallel(*rGSubs.at(i),plantAbst,plant); // plant for verification
    Parallel(plant,*rKSubs.at(i),spec);
    plant.ClrObservable(plant.Alphabet() ); 
    plant.SetObservable(rGSubs.at(i)->ObservableEvents() ); // only observable events of the subsystem are observable
    std::string reportString;
    rDiagSubs.push_back( new Diagnoser() );

    if(IsLanguageDiagnosable(plant,spec,reportString) == false ){
      FD_DD("Plant " + ToStringInteger(i) + " fails");
      diagnosable = false;
      rReportString += "Subsystem " + ToStringInteger(i) + " fails ";
    }
    else{
      FD_DD("Plant " + ToStringInteger(i) + " works");
      LanguageDiagnoser(plant,spec,*rDiagSubs[i]);
      rReportString += "Subsystem " + ToStringInteger(i) + " works ";

    }
  }
  return diagnosable;
}

///////////////////////////////////////////////////////////////////////////////
// RTI wrapper
///////////////////////////////////////////////////////////////////////////////

// IsModularDiagnosable()
bool IsModularDiagnosable(const SystemVector& rGsubs, const GeneratorVector& rKsubs) {
  std::string ignore;
  return IsModularDiagnosable(rGsubs, rKsubs, ignore);
}



// ModularDiagnoser()
bool ModularDiagnoser(const SystemVector& rGsubs, const GeneratorVector& rKsubs, GeneratorVector& rDiagsubs) {
  std::string ignore;
  return ModularDiagnoser(rGsubs, rKsubs, rDiagsubs, ignore);
}

///////////////////////////////////////////////////////////////////////////////
// Auxiliary Functions  
///////////////////////////////////////////////////////////////////////////////

// cParallel()
void cParallel(const vector<System>& rGens, System& rResGen) {
  unsigned int i = 0;
  rResGen.Clear();
  if (rGens.size() == 0) return;
  rResGen = rGens.at(0);
  
  for (i = 1; i < rGens.size(); ++i) {
    aParallel(rResGen,rGens.at(i),rResGen);  
  }  
}


} // namespace faudes

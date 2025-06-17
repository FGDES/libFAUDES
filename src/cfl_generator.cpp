/** @file cfl_generator.cpp Class vGenerator */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2007, 2010, 2025  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNES FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


#include "cfl_generator.h"
#include <stack>

//local debug
//#undef FD_DG
//#define FD_DG(m) FD_WARN(m)

namespace faudes {


// msObjectCount (static) 
Idx vGenerator::msObjectCount = 0;

// State names default (static) 
bool vGenerator::msStateNamesEnabledDefault = true;

// Reindex default (static) 
bool vGenerator::msReindexOnWriteDefault = false;


// static default prototypes (construct on first use design pattern)
const EventSet& vGenerator::AlphabetVoid(void) {
  static EventSet fls;
  return fls;
}
const StateSet& vGenerator::StatesVoid(void) {
  static StateSet fls;
  return fls;
}
const TransSet& vGenerator::TransRelVoid(void) {
  static TransSet  fls;
  return fls;
}
const AttributeVoid& vGenerator::GlobalVoid(void) {
  static AttributeVoid fls;
  return fls;
}




// constructor
vGenerator::vGenerator(void) : 
  // base
  ExtType(),
  // have std symboltables
  mpStateSymbolTable(&mStateSymbolTable),
  mpEventSymbolTable(GlobalEventSymbolTablep()),
  // other members
  mStateNamesEnabled(msStateNamesEnabledDefault),  
  mReindexOnWrite(msReindexOnWriteDefault),  
  // initialise heap members
  mpAlphabet(0),
  mpStates(0),
  mpTransRel(0),
  mpGlobalAttribute(0),
  // initialise prototypes
  pAlphabetPrototype(&AlphabetVoid()),
  pStatesPrototype(&StatesVoid()),
  pTransRelPrototype(&TransRelVoid()),
  pGlobalPrototype(&GlobalVoid())
{
  FAUDES_OBJCOUNT_INC("Generator");
  FD_DG("vGenerator(" << this << ")::vGenerator()");
  // track generator objects
  msObjectCount++; 
  mId = msObjectCount;
  // overwrite base defaults
  mObjectName="Generator",
  // allocate core members
  NewCore();
  // fix std names
  mInitStates.Name("InitStates");
  mMarkedStates.Name("MarkedStates");
}

// copy constructor
vGenerator::vGenerator(const vGenerator& rOtherGen) :
  // base
  ExtType(),
  // have std symboltables
  mpStateSymbolTable(&mStateSymbolTable),
  mpEventSymbolTable(GlobalEventSymbolTablep()),
  // other members
  mStateNamesEnabled(msStateNamesEnabledDefault),  
  mReindexOnWrite(msReindexOnWriteDefault),  
  // initialise heap members
  mpAlphabet(0),
  mpStates(0),
  mpTransRel(0),
  mpGlobalAttribute(0),
  // initialise prototypes
  pAlphabetPrototype(&AlphabetVoid()),
  pStatesPrototype(&StatesVoid()),
  pTransRelPrototype(&TransRelVoid()),
  pGlobalPrototype(&GlobalVoid())
{
  FAUDES_OBJCOUNT_INC("Generator");
  FD_DG("vGenerator(" << this << ")::vGenerator(" << &rOtherGen << ")");
  // track generator objects
  msObjectCount++; 
  mId = msObjectCount;
  // overwrite base defaults
  mObjectName="Generator",
  // allocate core members
  NewCore();
  // perform copy
  DoAssign(rOtherGen);
}

// construct from file
vGenerator::vGenerator(const std::string& rFileName) : 
  // base
  ExtType(),
  // have std symboltables
  mpStateSymbolTable(&mStateSymbolTable),
  mpEventSymbolTable(GlobalEventSymbolTablep()),
  // other members
  mStateNamesEnabled(msStateNamesEnabledDefault),  
  mReindexOnWrite(msReindexOnWriteDefault),  
  // initialise heap members
  mpAlphabet(0),
  mpStates(0),
  mpTransRel(0),
  mpGlobalAttribute(0),
  // initialise prototypes
  pAlphabetPrototype(&AlphabetVoid()),
  pStatesPrototype(&StatesVoid()),
  pTransRelPrototype(&TransRelVoid()),
  pGlobalPrototype(&GlobalVoid())
{
  FAUDES_OBJCOUNT_INC("Generator");
  FD_DG("vGenerator(" << this << ")::vGenerator(" << rFileName << ")");
  // track generator objects
  msObjectCount++; 
  mId = msObjectCount;
  // overwrite base defaults
  mObjectName="Generator",
  // allocate core members
  NewCore();
  // fix std names
  mInitStates.Name("InitStates");
  mMarkedStates.Name("MarkedStates");
  // set defaults for file io
  mStateNamesEnabled=true;
  // do read
  Read(rFileName,"Generator");
  // restore defaults
  mStateNamesEnabled=msStateNamesEnabledDefault;
}

// construct on heap
vGenerator* vGenerator::New(void) const {
  FD_DG("vGenerator(" << this << ")::New()");
  // allocate
  vGenerator* res = new vGenerator();
  // fix configuration
  res->EventSymbolTablep(mpEventSymbolTable);
  res->mStateNamesEnabled=mStateNamesEnabled;
  res->mReindexOnWrite=mReindexOnWrite;  
  return res;
}

// construct on heap
vGenerator* vGenerator::Copy(void) const {
  FD_DG("vGenerator(" << this << ")::Copy()");
  // copy construct
  vGenerator* res = new vGenerator(*this);
  return res;
}

// cast
const Type* vGenerator::Cast(const Type* pOther) const {
  return dynamic_cast< const vGenerator* > (pOther);
}


// destruct
vGenerator::~vGenerator(void) {
  FAUDES_OBJCOUNT_DEC("Generator");
  // free my members
  DeleteCore();
}

// configure attribute types
void vGenerator::ConfigureAttributeTypes(const AttributeVoid* pNewGlobalPrototype, 
    const StateSet* pNewStatesPrototype, const EventSet* pNewAlphabetPrototype, 
    const TransSet* pNewTransRelPrototype) {
  FD_DG("vGenerator(" << this << ")::ConfigureAtributes(..)");
  pGlobalPrototype= pNewGlobalPrototype;
  pStatesPrototype= pNewStatesPrototype;
  pAlphabetPrototype= pNewAlphabetPrototype;
  pTransRelPrototype= pNewTransRelPrototype;
  FD_DG("vGenerator(" << this << ")::ConfigureAtributes(): done");
}

// indicate new core
void vGenerator::UpdateCore(void) {
  // fix std names
  if(mpAlphabet) mpAlphabet->Name("Alphabet");
  if(mpStates) mpStates->Name("States");
  if(mpTransRel) mpTransRel->Name("TransRel");
  // fix symbol table
  if(mpAlphabet) EventSymbolTablep(mpEventSymbolTable);
}

// protected helper: allocate core on heap
void vGenerator::NewCore(void) {
  DeleteCore();
  // allocate (use prototypes, fallback to void attributes)
  if(pAlphabetPrototype) mpAlphabet= pAlphabetPrototype->New();
  else mpAlphabet = new EventSet();
  if(pStatesPrototype) mpStates=pStatesPrototype->New();
  else mpStates = new StateSet();
  if(pTransRelPrototype) mpTransRel= pTransRelPrototype->New();
  else mpTransRel = new TransSet();
  if(pGlobalPrototype) mpGlobalAttribute=pGlobalPrototype->New();
  else mpGlobalAttribute = new AttributeVoid();
  // update callback
  UpdateCore();
}
  
// protected helper: free core from heap
void vGenerator::DeleteCore(void) {
  if(mpAlphabet) delete mpAlphabet; 
  if(mpStates) delete mpStates; 
  if(mpTransRel) delete mpTransRel; 
  if(mpGlobalAttribute) delete mpGlobalAttribute;   
  mpAlphabet=0;
  mpStates=0;
  mpTransRel=0;
  mpGlobalAttribute=0;
  // update callback
  UpdateCore();
}



// copy from other vGenerator (try to convert attributes)
void vGenerator::DoAssign(const vGenerator& rGen) {
  FD_DG("vGenerator(" << this << ")::DoAssign(" << &rGen << ")");
  // bail out on match
  if(&rGen==this) return;
  // prepare result (call clear for virtual stuff)
  Clear();
  // have same event symboltable
  EventSymbolTablep(rGen.mpEventSymbolTable);
  // copy state symboltable
  StateSymbolTable(rGen.mStateSymbolTable);
  // set other members
  Name(rGen.Name());
  StateNamesEnabled(rGen.StateNamesEnabled());
  ReindexOnWrite(rGen.ReindexOnWrite());
  InjectInitStates(rGen.mInitStates);
  InjectMarkedStates(rGen.mMarkedStates);
  // core members, try attributes
  InjectStates(*rGen.mpStates);
  InjectAlphabet(*rGen.mpAlphabet);
  InjectTransRel(*rGen.mpTransRel);
  GlobalAttributeTry(*rGen.mpGlobalAttribute);
#ifdef FAUDES_DEBUG_CODE
  if(!Valid()) {
    FD_DG("vGenerator()::Copy(): invalid generator");
    DWrite(); 
    abort();
  }
#endif
  FD_DG("vGenerator(" << this << ")::DoAssign(" << &rGen << "): done");
} 

// copy from other vGenerator (try to convert attributes)
vGenerator& vGenerator::Assign(const Type& rSrc) {
  FD_DG("vGenerator(" << this << ")::Assign([type] " << &rSrc << ")");
  // bail out on match
  if(&rSrc==this) return *this;
  // run DoAssign if object casts to a generator
  const vGenerator* vgen=dynamic_cast<const vGenerator*>(&rSrc);
  if(vgen) {
    DoAssign(*vgen);
    return *this;
  }  
  // clear to default
  Clear();
  return *this;
}

// copy from other vGenerator (clear attributes)
vGenerator& vGenerator::AssignWithoutAttributes(const vGenerator& rGen) {
  FD_DG("vGenerator(" << this << ")::Assign(" << &rGen << ")");
  // prepare result (call clear for virtual stuff)
  Clear();
  // have same event symboltable
  EventSymbolTablep(rGen.mpEventSymbolTable);
  // copy state symboltable
  StateSymbolTable(rGen.mStateSymbolTable);
  // set other members
  Name(rGen.Name());
  StateNamesEnabled(rGen.StateNamesEnabled());
  ReindexOnWrite(rGen.ReindexOnWrite());
  InjectInitStates(rGen.mInitStates);
  InjectMarkedStates(rGen.mMarkedStates);
  // core members, ignore attributes
  mpStates->AssignWithoutAttributes(rGen.States());
  mpAlphabet->AssignWithoutAttributes(rGen.Alphabet());
  mpTransRel->AssignWithoutAttributes(rGen.TransRel());
#ifdef FAUDES_DEBUG_CODE
  if(!Valid()) {
    FD_DG("vGenerator()::Copy(): invalid generator");
    DWrite(); 
    abort();
  }
#endif
  return *this;
} 


// Move(gen) destructive copy 
void vGenerator::Move(vGenerator& rGen) {
  FD_DG("vGenerator(" << this << ")::Move(" << &rGen << ")");
  // test types
  bool tmm=false;
  const Type* pt;
  const Type* opt;
  pt=&Alphabet();
  opt=&rGen.Alphabet();
  if(typeid(*pt)!=typeid(*opt)) tmm=true;
  pt=&States();
  opt=&rGen.States();
  if(typeid(*pt)!=typeid(*opt)) tmm=true;
  pt=&TransRel();
  opt=&rGen.TransRel();
  if(typeid(*pt)!=typeid(*opt)) tmm=true;
  pt=&GlobalAttribute();
  opt=&rGen.GlobalAttribute();
  if(typeid(*pt)!=typeid(*opt)) tmm=true;
  // use copy on mismatch to convert   
  if(tmm) { 
    FD_DG("vGenerator(" << this << ")::Move(" << &rGen << "): using std copy");
    rGen.DoAssign(*this);
    Clear();
    return;
  }
  // prepare result (call clear for virtual stuff)
  rGen.Clear();
  // have same event symboltable
  rGen.EventSymbolTablep(mpEventSymbolTable);
  // copy state symboltable (todo: make this pointer based?)
  rGen.StateSymbolTable(mStateSymbolTable);
  // copy members
  rGen.Name(Name());
  rGen.StateNamesEnabled(StateNamesEnabled());
  rGen.ReindexOnWrite(ReindexOnWrite());
  rGen.InjectInitStates(InitStates());
  rGen.InjectMarkedStates(MarkedStates());
  // delete destination core
  if(rGen.mpStates) delete rGen.mpStates;
  if(rGen.mpAlphabet) delete rGen.mpAlphabet;
  if(rGen.mpTransRel) delete rGen.mpTransRel;
  if(rGen.mpGlobalAttribute) delete rGen.mpGlobalAttribute;
  // move and invalidate core members
  rGen.mpStates=mpStates;
  rGen.mpAlphabet=mpAlphabet;
  rGen.mpTransRel=mpTransRel;
  rGen.mpGlobalAttribute=mpGlobalAttribute;
  mpStates=0;
  mpAlphabet=0;
  mpTransRel=0;
  mpGlobalAttribute=0;
  // register core update
  rGen.UpdateCore();
  // install new empty core members
  NewCore();
  // clear myself (incl derived classes members)
  Clear();
}

// operator =
vGenerator& vGenerator::operator = (const vGenerator& rOtherGen) {
  FD_DG("vGenerator(" << this << ")::operator = " << &rOtherGen);
  FD_DG("vGenerator(" << this << ")::operator =  types " << typeid(*this).name() << " <= " << typeid(rOtherGen).name());
  return Assign(rOtherGen);
}

// Version(idx)
void vGenerator::Version(Idx version, vGenerator& rResGen) const {
  FD_DG("vGenerator(" << this << ")::Version(" << version << ")");
  std::ostringstream o;
  o << version;
  Version(o.str(),rResGen);
}

// Version(string)
void vGenerator::Version(const std::string& rVersion, vGenerator& rResGen) const {
  FD_DG("vGenerator(" << this << ")::Version(" << rVersion << ")");
  // second arg must not be us 
  if(this==&rResGen) {
    std::stringstream errstr;
    errstr << "Destination must not match source.";
    throw Exception("vGenerator::Version(string)", errstr.str(), 96);
  }
  // prepare Empty generator
  rResGen.Clear();
  rResGen.GlobalAttribute(GlobalAttribute());
  EventSet::Iterator eit;
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  std::map<Idx,Idx> eventoldnewmap;
  rResGen.Name(Name()+"_"+rVersion);
  // create versioned mAlphabet
  for (eit = AlphabetBegin(); eit != AlphabetEnd(); ++eit) {
    Idx newevent= rResGen.InsEvent(EventName(*eit)+"_"+rVersion); 
    eventoldnewmap[*eit] = newevent;
    rResGen.EventAttribute(newevent,EventAttribute(*eit));
  }
  // create new stateset
  for (lit = StatesBegin(); lit != StatesEnd(); ++lit) {
    rResGen.InsState(*lit);
    rResGen.StateAttribute(*lit,StateAttribute(*lit)); //would be faster if directly copied ?
    if (StateName(*lit) != "") rResGen.StateName(*lit,StateName(*lit));
  }
  // created versioned transrel
  for (tit = TransRelBegin(); tit != TransRelEnd(); ++tit) {
    Transition trans=Transition(tit->X1, eventoldnewmap[tit->Ev], tit->X2);
    rResGen.SetTransition(trans);
    rResGen.TransAttribute(trans, TransAttribute(*tit));
  }
  // set i/m states
  rResGen.mInitStates=mInitStates;
  rResGen.mMarkedStates=mMarkedStates;
  // behavioural flags
  rResGen.mStateNamesEnabled = mStateNamesEnabled;
  rResGen.mReindexOnWrite = mReindexOnWrite;
}


// Version(string,string)
void vGenerator::Version(const std::string& rPattern, const std::string& rReplacement, vGenerator& rResGen) const {
  FD_DG("vGenerator(" << this << ")::Version(" << rPattern << ", " << rReplacement << ", ...)");
  // second arg must not be us 
  if(this==&rResGen) {
    std::stringstream errstr;
    errstr << "Destination must not match source.";
    throw Exception("vGenerator::Version(string,string)", errstr.str(), 96);
  }
  // ignore invalid pattern
  if(rPattern.empty()) {
    rResGen.Assign(*this);
    return;
  }
  // trivial case
  if(rPattern==rReplacement) {
    rResGen.Assign(*this);
    return;
  }
  // prepare Empty generator
  rResGen.Clear();
  rResGen.GlobalAttribute(GlobalAttribute());
  rResGen.Name(Name()+"_v");
  EventSet::Iterator eit;
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  std::map<Idx,Idx> eventoldnewmap;
  // create versioned mAlphabet
  std::string newstring;
  std::string::size_type pos = 0;
  int patternlength=rPattern.size();
  int replacementlength=rReplacement.size();
  for (eit = AlphabetBegin(); eit != AlphabetEnd(); ++eit) {
    // search for all pattern occurences in event name and replace
    newstring=EventName(*eit);
    pos=0;
    while( (pos = newstring.find(rPattern, pos)) != std::string::npos ) {
        newstring.replace(pos, patternlength, rReplacement);
        //pos++;
        pos=pos+replacementlength;
    }
    Idx newevent= rResGen.InsEvent(newstring); 
    eventoldnewmap[*eit] = newevent;
    rResGen.EventAttribute(newevent,EventAttribute(*eit));
  }
  // have a nice neme too
  newstring=Name();
  pos=0;
  while( (pos = newstring.find(rPattern, pos)) != std::string::npos ) {
    newstring.replace(pos, patternlength, rReplacement);
    pos=pos+replacementlength;
  }
  rResGen.Name(newstring);
  // create new stateset
  for(lit = StatesBegin(); lit != StatesEnd(); ++lit) {
    rResGen.InsState(*lit);
    rResGen.StateAttribute(*lit,StateAttribute(*lit)); //would be faster if directly copied ?
    if(StateName(*lit) != "") rResGen.StateName(*lit,StateName(*lit));
  }
  // created versioned transrel
  for(tit = TransRelBegin(); tit != TransRelEnd(); ++tit) {
    Transition trans=Transition(tit->X1, eventoldnewmap[tit->Ev], tit->X2);
    rResGen.SetTransition(trans);
    rResGen.TransAttribute(trans, TransAttribute(*tit));
  }
  // set i/m states
  rResGen.mInitStates=mInitStates;
  rResGen.mMarkedStates=mMarkedStates;
  // behavioural flags
  rResGen.mStateNamesEnabled = mStateNamesEnabled;
  rResGen.mReindexOnWrite = mReindexOnWrite;
}


// Valid()
bool vGenerator::Valid(void) const {
  FD_DG("vGenerator(" << this << ")::Valid()");
  // core members on heap 
  if(mpAlphabet==0) return false;
  if(mpStates==0) return false;
  if(mpTransRel==0) return false;
  if(mpGlobalAttribute==0) return false;
  // transitions to be known
  TransSet::Iterator tit;
  StateSet::Iterator lit;
  for(tit = TransRelBegin(); tit != TransRelEnd(); ++tit) {
    if(! ExistsState(tit->X1)) return false;
    if(! ExistsEvent(tit->Ev)) return false;
    if(! ExistsState(tit->X2)) return false;
  }
  // init states to be known
  for(lit = InitStatesBegin(); lit != InitStatesEnd(); ++lit) 
    if(! ExistsState(static_cast<Idx>(*lit))) return false;
  for(lit = MarkedStatesBegin(); lit != MarkedStatesEnd(); ++lit) 
    if(! ExistsState(static_cast<Idx>(*lit))) return false;
  // sets to have proper names
  if(mpAlphabet->Name() != "Alphabet") return false;
  if(mpStates->Name() != "States") return false;
  if(mInitStates.Name() != "InitStates") return false;
  if(mMarkedStates.Name() != "MarkedStates") return false;
  // event symbol table
  NameSet::Iterator eit;
  for(eit = AlphabetBegin(); eit != AlphabetEnd(); eit ++) {
    if(EventName(*eit)=="") return false;
  }
  // done
  return true;
}

// AlphabetSize()
Idx vGenerator::AlphabetSize(void) const {
  return mpAlphabet->Size();
}

// Size()
Idx vGenerator::Size(void) const {
  return mpStates->Size();
}

// Clear()
void vGenerator::Clear(void) {
  FD_DG("vGenerator(" << this << ")::Clear()");
  mpAlphabet->Clear();
  mpStates->Clear();
  mpStateSymbolTable->Clear();
  mpTransRel->Clear();
  mInitStates.Clear();
  mMarkedStates.Clear();
  ClearGlobalAttribute();
  FD_DG("vGenerator(" << this << ")::Clear(): done");
}

// ClearGlobalAttribute()
void vGenerator::ClearGlobalAttribute(void) {
  mpGlobalAttribute->Clear(); 
}

// ClearStateAttributes()
void vGenerator::ClearStateAttributes(void) {
  mpStates->ClearAttributes();
}

// ClearEventAttributes()
void vGenerator::ClearEventAttributes(void) {
  mpAlphabet->ClearAttributes();
}

// ClearTransAttributes()
void vGenerator::ClearTransAttributes(void) {
  mpTransRel->ClearAttributes();
}

// ClearAttributes()
void vGenerator::ClearAttributes(void) {
  ClearGlobalAttribute();
  ClearStateAttributes();
  ClearEventAttributes();
  ClearTransAttributes();
}


// ClearStates()
void vGenerator::ClearStates(void) {
  mpStates->Clear();
  mpTransRel->Clear();
  mInitStates.Clear();
  mMarkedStates.Clear();
  mpStateSymbolTable->Clear();
}

// TransRelSize()
Idx vGenerator::TransRelSize(void) const {
  return mpTransRel->Size();
}

// InitStatesSize()
Idx vGenerator::InitStatesSize(void) const {
  return mInitStates.Size();
}

// MarkedStatesSize()
Idx vGenerator::MarkedStatesSize(void) const {
  return mMarkedStates.Size();
}

// AlphabetEmpty()
bool vGenerator::AlphabetEmpty(void) const {
  return mpAlphabet->Empty();
}

// Empty()
bool vGenerator::Empty(void) const {
  return mpStates->Empty();
}

// TransRelEmpty()
bool vGenerator::TransRelEmpty(void) const {
  return mpTransRel->Empty();
}

// InitStatesEmpty()
bool vGenerator::InitStatesEmpty(void) const {
  return mInitStates.Empty();
}

// MarkedStatesEmpty()
bool vGenerator::MarkedStatesEmpty(void) const {
  return mMarkedStates.Empty();
}


// ClearMinStateIndexMap()
void vGenerator::ClearMinStateIndexMap(void) const {
  FD_DG("vGenerator::ClearMinStateIndexMap()");
  // fake const
  vGenerator* fakeconst = const_cast<vGenerator*>(this);
  fakeconst->mMinStateIndexMap.clear();
}

// MinStateIndexMap()
const std::map<Idx,Idx>& vGenerator::MinStateIndexMap(void) const {
  return mMinStateIndexMap;
}


// SetMinStateIndexMap()
void vGenerator::SetMinStateIndexMap(void) const {
  FD_DG("vGenerator::SetMinStateIndexMap()");
  // fake const
  vGenerator* fakeconst = const_cast<vGenerator*>(this);
  // clear map
  fakeconst->ClearMinStateIndexMap();
  StateSet::Iterator it;
  Idx minindex = 0;
  // if generator states get names
  if(StateNamesEnabled()) {
    // named initial states first
    for(it = InitStatesBegin(); it != InitStatesEnd(); ++it) {
      if(StateName(static_cast<Idx>(*it)) != "") {
	fakeconst->mMinStateIndexMap[*it] = ++minindex;
      }
    }
    // then all other named states
    for(it = StatesBegin(); it != StatesEnd(); ++it) {
      if(mMinStateIndexMap.count(*it) == 0) {
	if(StateName(static_cast<Idx>(*it)) != "") {
	  fakeconst->mMinStateIndexMap[*it] = ++minindex;
	}
      }
    }
    // at last all anonymous states
    for(it = StatesBegin(); it != StatesEnd(); ++it) {
      if(mMinStateIndexMap.count(*it) == 0) {
	fakeconst->mMinStateIndexMap[*it] = ++minindex;
      }
    }
  }
  // if generator states are all anonymous
  else {
    // all initial states first
    for(it = InitStatesBegin(); it != InitStatesEnd(); ++it) {
      fakeconst->mMinStateIndexMap[*it] = ++minindex;
    }
    // then the rest
    for(it = StatesBegin(); it != StatesEnd(); ++it) {
      if(mMinStateIndexMap.count(*it) == 0) {
	fakeconst->mMinStateIndexMap[*it] = ++minindex;
      }
    }
  }
#ifdef FAUDES_DEBUG_CONTAINER
  std::map<Idx,Idx>::const_iterator _it;
  for(_it = mMinStateIndexMap.begin(); _it != mMinStateIndexMap.end(); ++_it) {
    FD_DC("vGenerator::MinStateIndexMap: " << _it->first
	  << " <-- " << SStr(_it->second));
  }
#endif
}


// MinStateIndex(index)
Idx vGenerator::MinStateIndex(Idx index) const {
  std::map<Idx,Idx>::const_iterator minit;
  minit = mMinStateIndexMap.find(index);
  if(minit != mMinStateIndexMap.end()) {
    return minit->second;
  } 
  return index;
}


// Max StateIndex
Idx vGenerator::MaxStateIndex(void) const {
  if(mpStates->Empty()) return 0;
  return *(--(mpStates->End()));
}

// Re-enumerate states to obtain a consecutive state set
void vGenerator::MinStateIndex(void) {
  // bail out on trivial
  // if(MaxStateIndex()==Size()) return;  tmoor 201206: dont bail out to be consistent with token IO
  // prepare buffer and index map
  vGenerator* dst = New();
  dst->InsEvents(Alphabet());
  SetMinStateIndexMap();
  // doit: states
  StateSet::Iterator sit;
  Idx s;
  for(sit=StatesBegin(); sit!=StatesEnd(); ++sit) {
    s=MinStateIndex(*sit);
    dst->InsState(s);
    dst->StateAttribute(s,StateAttribute(*sit));
    if(StateNamesEnabled()) 
      dst->StateName(s,StateName(*sit));
  }
  for(sit=InitStatesBegin(); sit!=InitStatesEnd(); ++sit) {
    s=MinStateIndex(*sit);
    dst->SetInitState(s);
  }
  for(sit=MarkedStatesBegin(); sit!=MarkedStatesEnd(); ++sit) {
    s=MinStateIndex(*sit);
    dst->SetMarkedState(s);
  }
  // doit: transitions
  TransSet::Iterator tit;
  for(tit=TransRelBegin(); tit!=TransRelEnd(); tit++) {
    Transition dt(MinStateIndex(tit->X1),tit->Ev,MinStateIndex(tit->X2));
    dst->SetTransition(dt);
    dst->TransAttribute(dt,TransAttribute(*tit));
  }
  // move relevant core members
  StateSymbolTable(dst->mStateSymbolTable);
  delete mpStates;
  delete mpTransRel;
  mpStates=dst->mpStates;
  mpTransRel=dst->mpTransRel;
  dst->mpStates=0;
  dst->mpTransRel=0;
  // move other members
  mInitStates=dst->InitStates();
  mMarkedStates=dst->MarkedStates();
  // update callback
  UpdateCore();
  // delete buffer
  delete dst;
  // invalidate map
  ClearMinStateIndexMap();
}


// EventSymbolTablep() const
SymbolTable* vGenerator::EventSymbolTablep(void) const {
  return mpEventSymbolTable;
}

// GlobalEventSymbolTablep() const
SymbolTable* vGenerator::GlobalEventSymbolTablep(void) {
  return SymbolTable::GlobalEventSymbolTablep();
}

// EventSymbolTablep(pSymTab) 
void vGenerator::EventSymbolTablep(SymbolTable* pSymTab) {
  mpEventSymbolTable=pSymTab;
  // todo: set symboltable in mpAlphabet
}

// EventSymbolTablep(rOtherGen) 
void vGenerator::EventSymbolTablep(const vGenerator& rOtherGen) {
  EventSymbolTablep(rOtherGen.EventSymbolTablep());
}

// EventIndex(rName)
Idx vGenerator::EventIndex(const std::string& rName) const {
  return mpEventSymbolTable->Index(rName);
}

// EventName(index)
std::string vGenerator::EventName(Idx index) const {
  return mpEventSymbolTable->Symbol(index);
}

// EventName(index, name)
void vGenerator::EventName(Idx index, const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::EventName(" 
      << index << ",\"" << rName << "\")");
#ifdef FAUDES_CHECKED
  if (! ExistsEvent(index)) {
    std::stringstream errstr;
    errstr << "event \"" << index << "\" not found in generator \""
	   << Name() << "\"";
    throw Exception("vGenerator::EventName(name)", errstr.str(), 89);
  }
#endif
  mpEventSymbolTable->SetEntry(index, rName);
}

// UniqueEventName(rName)
std::string vGenerator::UniqueEventName(const std::string& rName) const {
  std::string name=rName;
  if(name=="") name="ev";
  return mpEventSymbolTable->UniqueSymbol(name) ;
}


// EventRename
bool vGenerator::EventRename(Idx event, const std::string& rNewName) {
  // consistency
  FD_DG("vGenerator(" << this << ")::EventRename(" << EStr(event) << ", " << rNewName << ")");
#ifdef FAUDES_CHECKED
  if (! ExistsEvent(event)) {
    std::stringstream errstr;
    errstr << "event \"" << event << "\" not found in generator \""
	   << Name() << "\"";
    throw Exception("vGenerator::EventReame(name)", errstr.str(), 89);
  }
#endif
  // prepare formal result
  bool res=ExistsEvent(rNewName);
  // insert new event
  Idx newidx=InsEvent(rNewName);
  // bail out if events are the same
  if(newidx==event) return true;
  // copy event attribute
  if(!res) EventAttribute(newidx,EventAttribute(event));
  // store new transitions (with their attributes)
  TransSet* newtrans= TransRel().New();
  // iterate over transitions
  TransSet::Iterator tit= TransRelBegin();
  while(tit != TransRelEnd()) {
    if(tit->Ev!=event) {++tit; continue;}
    Transition trans= Transition(tit->X1, newidx, tit->X2);
    newtrans->Insert(trans);
    newtrans->Attribute(trans,TransAttribute(*tit));
    ClrTransition(tit++);
  }
  // merge transitions
  for(tit=newtrans->Begin(); tit!=newtrans->End(); tit++) {
    SetTransition(*tit);
    TransAttribute(*tit,newtrans->Attribute(*tit));
  }
  // free temp
  delete newtrans;
  // remore original event
  DelEvent(event);
  // done
  FD_DG("vGenerator(" << this << ")::EventRename(" << EStr(event) << ", " << rNewName << "):OK");
  return res;
}

// EventRename
bool vGenerator::EventRename(const std::string& rOldName, const std::string& rNewName) {
  Idx oev = EventIndex(rOldName);
  return EventRename(oev,rNewName);
}

// NewEventSet()
EventSet vGenerator::NewEventSet(void) const {
  EventSet res;
  res.SymbolTablep(mpEventSymbolTable);
  return res;
}

// NewEventSetp()
EventSet* vGenerator::NewEventSetp(void) const {
  EventSet* res = new EventSet();
  res->SymbolTablep(mpEventSymbolTable);
  return res;
}


// StateSymbolTable() const
const SymbolTable& vGenerator::StateSymbolTable(void) const {
  return mStateSymbolTable;
}

// StateSymbolTable(rSymTab) 
void vGenerator::StateSymbolTable(const SymbolTable& rSymTab) {
  mStateSymbolTable=rSymTab;
  mpStateSymbolTable=&mStateSymbolTable;
}

// StateIndex(rName)
Idx vGenerator::StateIndex(const std::string& rName) const {
   return mpStateSymbolTable->Index(rName);
}

// StateName(index)
std::string vGenerator::StateName(Idx index) const {
  return mpStateSymbolTable->Symbol(index);
}

// StateName(index, name)
void vGenerator::StateName(Idx index, const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::StateName(" 
      << index << ",\"" << rName << "\")");
#ifdef FAUDES_CHECKED
  if (! ExistsState(index)) {
    std::stringstream errstr;
    errstr << "state name \"" << rName << "\" not found in generator \""
	   << Name() << "\"";
    throw Exception("vGenerator::StateName(name)", errstr.str(), 90);
  }
#endif
  mpStateSymbolTable->SetEntry(index, rName);
}


// ClearStateNames()
void vGenerator::ClearStateNames(void) {
  FD_DG("vGenerator(" << this << ")::ClearStateNames()");
  mpStateSymbolTable->Clear();
}


// ClrStateName(index)
void vGenerator::ClrStateName(Idx index) {
  FD_DG("Generator(" << this << ")::ClrStateName(\"" << index << "\")");
#ifdef FAUDES_CHECKED
  if (! ExistsState(index)) {
    std::stringstream errstr;
    errstr << "state \"" << index << "\" not found in generator \""
	   << Name() << "\"";
    throw Exception("vGenerator::ClrStateName(name)", errstr.str(), 90);
  }
#endif
  mpStateSymbolTable->ClrEntry(index);
}

// ClrStateName(rName)
void vGenerator::ClrStateName(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::ClrStateName(\"" << rName << "\")");
  Idx index = StateIndex(rName);
  ClrStateName(index);
}


// StateNamesEnabled()
bool vGenerator::StateNamesEnabled(void) const {
  return mStateNamesEnabled;
}

// StateNamesEnabled(flag)
void vGenerator::StateNamesEnabled(bool flag) {
  mStateNamesEnabled = flag;
  if(!flag) ClearStateNames();
}

// StateNamesEnabled(flag)
void vGenerator::StateNamesEnabledDefault(bool flag) {
  msStateNamesEnabledDefault = flag;
}

// SetDefaultStateNames()
void vGenerator::SetDefaultStateNames(void) {
  FD_DG("vGenerator(" << this << ")::SetDefaultStateNames()");
  mpStateSymbolTable->Clear();
  StateSet::Iterator it;
  for (it = StatesBegin(); it != StatesEnd(); ++it) {
    std::string dname;
    dname = std::string("S")+ToStringInteger(*it);
    mpStateSymbolTable->SetEntry(*it,dname);
  }
}

// EnforceStateNames(rTemplate)
void vGenerator::EnforceStateNames(const std::string& rTemplate) {
  FD_DG("vGenerator(" << this << ")::EnforceStateNames(temp)");
  StateSet::Iterator it;
  for (it = StatesBegin(); it != StatesEnd(); ++it) {
    if(StateName(*it)=="") {
      std::string name=UniqueStateName(rTemplate + "_1");
      StateName(*it,name);
    }
  }
}

// UniqueStateName(rName)
std::string vGenerator::UniqueStateName(const std::string& rName) const {
  std::string name=rName;
  if(name=="") name="st";
  return mpStateSymbolTable->UniqueSymbol(name) ;
}


// iterator AlphabetBegin() const
EventSet::Iterator vGenerator::AlphabetBegin(void) const {
  return mpAlphabet->Begin();
}

// iterator AlphabetEnd() const
EventSet::Iterator vGenerator::AlphabetEnd(void) const {
  return mpAlphabet->End();
}

// iterator StatesBegin() const
StateSet::Iterator vGenerator::StatesBegin(void) const {
  return mpStates->Begin();
}

// iterator StatesEnd() const
StateSet::Iterator vGenerator::StatesEnd(void) const {
  return mpStates->End();
}

// iterator TransRelBegin() const
TransSet::Iterator vGenerator::TransRelBegin(void) const {
  return mpTransRel->Begin();
}

// iterator TransRelEnd() const
TransSet::Iterator vGenerator::TransRelEnd(void) const {
  return mpTransRel->End();
}

// iterator TransRelBegin(x1) const
TransSet::Iterator vGenerator::TransRelBegin(Idx x1) const {
  return mpTransRel->Begin(x1);
}

// iterator TransRelEnd(x1) const
TransSet::Iterator vGenerator::TransRelEnd(Idx x1) const {
  return mpTransRel->End(x1);
}

// iterator TransRelBegin(x1, ev) const
TransSet::Iterator vGenerator::TransRelBegin(Idx x1, Idx ev) const {
  return mpTransRel->Begin(x1, ev);
}

// iterator TransRelEnd(x1, ev) const
TransSet::Iterator vGenerator::TransRelEnd(Idx x1, Idx ev) const {
  return mpTransRel->End(x1, ev);
}

// iterator FindTransition(trans)
TransSet::Iterator vGenerator::FindTransition(const Transition& rTrans) const {
  return mpTransRel->Find(rTrans);
}

// iterator FindTransition(x1, ex x2)
TransSet::Iterator vGenerator::FindTransition(Idx x1, Idx ev, Idx x2) const {
  return mpTransRel->Find(Transition(x1, ev, x2));
}

// iterator FindTransition(x1, ev, x2) 
TransSet::Iterator vGenerator::FindTransition(
   const std::string& rX1, const std::string& rEv, const std::string& rX2) const 
{
  return mpTransRel->Find(StateIndex(rX1), EventIndex(rEv), StateIndex(rX2));
}

// iterator ExistsTransition(trans)
bool vGenerator::ExistsTransition(const Transition& rTrans) const {
  return mpTransRel->Exists(rTrans);
}

// iterator ExistsTransition(x1, ex x2)
bool vGenerator::ExistsTransition(Idx x1, Idx ev, Idx x2) const {
  return mpTransRel->Exists(Transition(x1, ev, x2));
}

// iterator ExistsTransition(x1, ev, x2)
bool vGenerator::ExistsTransition(
  const std::string& rX1, const std::string& rEv, const std::string& rX2) const 
{
  return mpTransRel->Exists(StateIndex(rX1), EventIndex(rEv), StateIndex(rX2));
}

// iterator ExistsTransition(x1, ev)
bool vGenerator::ExistsTransition(Idx x1, Idx ev) const {
  return mpTransRel->ExistsByX1Ev(x1, ev);
}

// iterator ExistsTransition(x1)
bool vGenerator::ExistsTransition(Idx x1) const {
  return mpTransRel->ExistsByX1(x1);
}


// idx InitState() const
Idx vGenerator::InitState(void) const {
  if(mInitStates.Size()!=1) return 0;
  return *mInitStates.Begin();
}



// iterator InitStatesBegin() const
StateSet::Iterator vGenerator::InitStatesBegin(void) const {
  return mInitStates.Begin();
}

// iterator InitStatesEnd() const
StateSet::Iterator vGenerator::InitStatesEnd(void) const {
  return mInitStates.End();
}

// iterator MarkedStatesBegin()
StateSet::Iterator vGenerator::MarkedStatesBegin(void) const {
  return mMarkedStates.Begin();
}

// iterator MarkedStatesEnd() const
StateSet::Iterator vGenerator::MarkedStatesEnd(void) const {
  return mMarkedStates.End(); 
}

// InjectAlphabet(newalphabet)
void vGenerator::InjectAlphabet(const EventSet& rNewAlphabet) {
  FD_DG("vGenerator::InjectAlphabet()  " << rNewAlphabet.ToString());
#ifdef FAUDES_CHECKED
  if(rNewAlphabet.SymbolTablep()!=mpEventSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("vGenerator::InjectAlphabet", errstr.str(), 88);
  }
#endif
  *mpAlphabet = rNewAlphabet;
  mpAlphabet->Name("Alphabet");
}

// RestrictAlphabet(newalphabet)
void vGenerator::RestrictAlphabet(const EventSet& rNewAlphabet) {
  FD_DG("vGenerator::RestrictAlphabet()  " << rNewAlphabet.ToString());
#ifdef FAUDES_CHECKED
  if(rNewAlphabet.SymbolTablep()!=mpEventSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("vGenerator::RestrictAlphabet", errstr.str(), 88);
  }
#endif
  mpAlphabet->RestrictSet(rNewAlphabet);
  mpTransRel->RestrictEvents(rNewAlphabet);
}

// InsEvent(index)
bool vGenerator::InsEvent(Idx index) {
  FD_DG("vGenerator(" << this << ")::InsEvent(" << index << ")");
  return mpAlphabet->Insert(index);
}

// InsEvent(rName)
Idx vGenerator::InsEvent(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::InsEvent(\"" << rName << "\")");
  return mpAlphabet->Insert(rName);
}

// InsEvents(events)
void vGenerator::InsEvents(const EventSet& events) {
  mpAlphabet->InsertSet(events);
}

// DelEvent(index)
bool vGenerator::DelEvent(Idx index) {
  FD_DG("vGenerator(" << this << ")::DelEvent(" << index << ")");
  mpTransRel->EraseByEv(index);
  return mpAlphabet->Erase(index);
}

// DelEvent(rName)
bool vGenerator::DelEvent(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::DelEvent(\"" << rName << "\")");
  Idx index = mpAlphabet->Index(rName);
  mpTransRel->EraseByEv(index);
  return mpAlphabet->Erase(index);
}

// DelEvents(events)
void vGenerator::DelEvents(const EventSet& rEvents) {
  FD_DG("vGenerator(" << this << ")::DelEvents(\"" 
	<< rEvents.ToString() << "\")");
  EventSet::Iterator it;
  for (it = rEvents.Begin(); it != rEvents.End(); ++it) {
    DelEvent(*it);
  }
}

// DelEventFromAlphabet(index)
bool vGenerator::DelEventFromAlphabet(Idx index) {
  FD_DG("vGenerator(" << this << ")::DelEventFromAlphabet(" 
	<< index << ")");
  return mpAlphabet->Erase(index);
}

// InsState()
Idx vGenerator::InsState(void) {
  FD_DG("vGenerator(" << this << ")::InsState()");
  return mpStates->Insert();
}

// InsState(index)
bool vGenerator::InsState(Idx index) {
  FD_DG("vGenerator(" << this << ")::InsState(" << index << ")");
  return mpStates->Insert(index);
}

// InsState(rName)
Idx vGenerator::InsState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::InsState(\"" << rName << "\")");
  Idx index=mpStates->Insert();
  StateName(index,rName);
  return index;
}

// InsStates(states)
void vGenerator::InsStates(const StateSet& states) {
  mpStates->InsertSet(states);
}

// InjectState(index)
void vGenerator::InjectState(Idx index) {
  FD_DG("vGenerator(" << this << ")::InjectState(\"" << SStr(index) << "\")");
  mpStates->Insert(index);
}

// InjectStates(rNewStates)
void vGenerator::InjectStates(const StateSet& rNewStates) {
  FD_DG("vGenerator(" << this << ")::InjectStates(" << rNewStates.ToString() << ")");
  *mpStates=rNewStates;
  mpStates->Name("States");
  mpStateSymbolTable->RestrictDomain(*mpStates);
  FD_DG("vGenerator(" << this << ")::InjectStates(): report " << mpStates->ToString());
}

// InsInitState()
Idx vGenerator::InsInitState(void) {
  FD_DG("vGenerator(" << this << ")::InsInitState()");
  Idx index;
  index = InsState();
  mInitStates.Insert(index);
  return index;
}

// InsInitState(name)
Idx vGenerator::InsInitState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::InsInitState(\"" << rName << "\")");
  Idx index;
  index = InsState(rName);
  mInitStates.Insert(index);
  return index;
}

// InsInitState(name)
bool vGenerator::InsInitState(Idx index) {
  bool res=InsState(index);
  mInitStates.Insert(index);
  return res;
}

// InsInitStates(states)
void vGenerator::InsInitStates(const StateSet& states) {
  mpStates->InsertSet(states);
  mInitStates.InsertSet(states);
}

// InsMarkedState()
Idx vGenerator::InsMarkedState(void) {
  FD_DG("vGenerator(" << this << ")::InsMarkedState()");
  Idx index;
  index = InsState();
  mMarkedStates.Insert(index);
  return index;
}

// InsInitState(name)
bool vGenerator::InsMarkedState(Idx index) {
  bool res=InsState(index);
  mMarkedStates.Insert(index);
  return res;
}

// InsMarkedState(rName)
Idx vGenerator::InsMarkedState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::InsMarkedState(\"" << rName << "\")");
  Idx index;
  index = InsState(rName);
  mMarkedStates.Insert(index);
  return index;
}

// InsMarkedStates(states)
void vGenerator::InsMarkedStates(const StateSet& states) {
  mpStates->InsertSet(states);
  mMarkedStates.InsertSet(states);
}


// DelState(index)
bool vGenerator::DelState(Idx index) {
  FD_DG("vGenerator(" << this << ")::DelState(" << index << ")");
  // mInitStates
  mInitStates.Erase(index);
  // mstates
  mMarkedStates.Erase(index);
  // transrel 
  mpTransRel->EraseByX1OrX2(index);
  // symbolic name
  mpStateSymbolTable->ClrEntry(index);
  // finally ... remove the state
  return mpStates->Erase(index);
}

// DelState(rName)
bool vGenerator::DelState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::DelState(\"" << rName << "\")");
  Idx index;
  index = StateIndex(rName);
#ifdef FAUDES_CHECKED
  if (index == 0) {
    std::stringstream errstr;
    errstr << "state name \"" << rName << "\" not found in generator \""
	   << Name() << "\"";
    throw Exception("vGenerator::DelState(name)", errstr.str(), 90);
  }
#endif
  return DelState(index);
}

// DelStates(rDelStates)
void vGenerator::DelStates(const StateSet& rDelStates) {
  FD_DG("vGenerator(" << this << ")::DelStates(" 
	<< rDelStates.ToString() << ")");
  StateSet::Iterator cit;
  StateSet::Iterator cit_end;
  // symbolic state names
  for (cit = rDelStates.Begin(); cit != rDelStates.End(); ++cit) {
    mpStateSymbolTable->ClrEntry(*cit);
  }
  // statesets
  mpStates->EraseSet(rDelStates);
  mInitStates.EraseSet(rDelStates);
  mMarkedStates.EraseSet(rDelStates);
  // mpTransRel:
  mpTransRel->EraseByX1OrX2(rDelStates);
}

// DelStateFromStates(index)
bool vGenerator::DelStateFromStates(Idx index) {
  FD_DG("vGenerator(" << this << ")::DelStateFromStates(" << index << ")");
  // mStates + global
  return mpStates->Erase(index);
}

// DelStateFromStates(pos)
StateSet::Iterator vGenerator::DelStateFromStates(StateSet::Iterator pos) {
  FD_DG("vGenerator(" << this << ")::DelState(" << *pos << ")");
  return mpStates->Erase(pos);
}

// RestrictStates(rStates)
void vGenerator::RestrictStates(const StateSet& rStates) {
  FD_DG("vGenerator(" << this << ")::RestrictStates(" 
	<< rStates.ToString() << ")");

  StateSet::Iterator cit;
  StateSet::Iterator cit_end;
  // symbolic state names
  for(cit = StatesBegin(); cit != StatesEnd(); ++cit) {
    if(!rStates.Exists(*cit)) mpStateSymbolTable->ClrEntry(*cit);
  }
  // statesets
  mpStates->RestrictSet(rStates);
  mInitStates.RestrictSet(rStates);
  mMarkedStates.RestrictSet(rStates);
  // mpTransRel:
  mpTransRel->RestrictStates(rStates);
}


// SetInitState(index)
void vGenerator::SetInitState(Idx index) {
  FD_DG("vGenerator(" << this << ")::SetInitState(" << index << ")");
#ifdef FAUDES_CHECKED
  if (! mpStates->Exists(index)) {
    std::stringstream errstr;
    errstr << "vGenerator::SetMarkedState: index " << index 
	   << " not in stateset";
    throw Exception("vGenerator::SetInitState(..)", errstr.str(), 91);
  }
#endif
  mInitStates.Insert(index);
}

// SetInitState(rName)
void vGenerator::SetInitState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::SetInitState(\"" << rName << "\")");
  Idx index = StateIndex(rName);
#ifdef FAUDES_CHECKED
  if (index == 0) {
    std::stringstream errstr;
    errstr << "State name \"" << rName << "\" not known in Generator";
    throw Exception("vGenerator::SetInitState(..)", errstr.str(), 90);
  }
#endif
  SetInitState(index);
}

// InjectInitStates(rNewInitStates)
void vGenerator::InjectInitStates(const StateSet& rNewInitStates) {
  FD_DG("vGenerator(" << this << ")::InjectInitStates(" 
	<< rNewInitStates.ToString() << ")");
  mInitStates = rNewInitStates;
  mInitStates.Name("InitStates");
}

// ClrInitState(index)
void vGenerator::ClrInitState(Idx index) {
  FD_DG("vGenerator(" << this << ")::ClrInitState(" << index << ")");
#ifdef FAUDES_CHECKED
  if (! mpStates->Exists(index)) {
    std::stringstream errstr;
    errstr << "vGenerator::SetMarkedState: index " << index 
	   << " not in stateset";
    throw Exception("vGenerator::ClrInitState(..)", errstr.str(), 91);
  }
#endif
  mInitStates.Erase(index);
}

// ClrInitState(rName)
void vGenerator::ClrInitState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::ClrInitState(\"" << rName << "\")");
  Idx index = StateIndex(rName);
#ifdef FAUDES_CHECKED
  if (index == 0) {
    std::stringstream errstr;
    errstr << "State name \"" << rName << "\" not known in Generator";
    throw Exception("vGenerator::ClrInitState(..)", errstr.str(), 90);
  }
#endif
  ClrInitState(index);
}

// ClrInitState(pos)
StateSet::Iterator vGenerator::ClrInitState(StateSet::Iterator pos) {
  FD_DG("vGenerator(" << this << ")::ClrInitState(" << *pos << ")");
  return mInitStates.Erase(pos);
}

// ClearInitStates()
void vGenerator::ClearInitStates(void) {
  mInitStates.Clear();
}

// SetMarkedState(index)
void vGenerator::SetMarkedState(Idx index) {
  FD_DG("vGenerator(" << this << ")::SetMarkedState(" << index << ")");
#ifdef FAUDES_CHECKED
  if (! mpStates->Exists(index)) {
    std::stringstream errstr;
    errstr << "vGenerator::SetMarkedState: index " << index 
	   << " not in stateset";
    throw Exception("vGenerator::SetMarkedState(..)", errstr.str(), 91);
  }
#endif
  mMarkedStates.Insert(index);
}

// SetMarkedState(rName)
void vGenerator::SetMarkedState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::SetMarkedState(\"" << rName << "\")");
  Idx index = StateIndex(rName);
#ifdef FAUDES_CHECKED
  if (index == 0) {
    std::stringstream errstr;
    errstr << "State name \"" << rName << "\" not known in Generator";
    throw Exception("vGenerator::SetMarkedState(..)", errstr.str(), 90);
  }
#endif
  SetMarkedState(index);
}

// InjectMarkedStates(rNewMarkedStates)
void vGenerator::InjectMarkedStates(const StateSet& rNewMarkedStates) {
  FD_DG("vGenerator(" << this << ")::InjectMarkedStates(" 
	<< rNewMarkedStates.ToString() << ")");
  mMarkedStates = rNewMarkedStates;
  mMarkedStates.Name("MarkedStates");
}

// ClrMarkedState(index)
void vGenerator::ClrMarkedState(Idx index) {
  FD_DG("vGenerator(" << this << ")::ClrMarkedState(" << index << ")");
#ifdef FAUDES_CHECKED
  if (! mpStates->Exists(index)) {
    std::stringstream errstr;
    errstr << "vGenerator::ClrMarkedState: index " << index 
	   << " not in stateset";
    throw Exception("vGenerator::ClrMarkedState(..)", errstr.str(), 91);
  }
#endif
  mMarkedStates.Erase(index);
}

// ClrMarkedState(rName)
void vGenerator::ClrMarkedState(const std::string& rName) {
  FD_DG("vGenerator(" << this << ")::ClrMarkedState(\"" << rName << "\")");
  Idx index = StateIndex(rName);
#ifdef FAUDES_CHECKED
  if (index == 0) {
    std::stringstream errstr;
    errstr << "State name \"" << rName << "\" not known in Generator";
    throw Exception("vGenerator::ClrMarkedState(..)", errstr.str(), 90);
  }
#endif
  ClrMarkedState(index);
}

// ClrMarkedState(pos)
StateSet::Iterator vGenerator::ClrMarkedState(StateSet::Iterator pos) {
  FD_DG("vGenerator(" << this << ")::ClrMarkedState(" << *pos << ")");
  return mMarkedStates.Erase(pos);
}

// ClearMarkedStates()
void vGenerator::ClearMarkedStates(void) {
  mMarkedStates.Clear();
}

// InjectTransition(newtrans)
void vGenerator::InjectTransition(const Transition& rTrans) {
  FD_DG("vGenerator::InjectTransition(" << TStr(rTrans) << ")");
  mpTransRel->Inject(rTrans);
}

// InjectTransRel(newtransrel)
void vGenerator::InjectTransRel(const TransSet& rNewTransrel) {
  FD_DG("vGenerator::InjectTransRel(...)");
  *mpTransRel=rNewTransrel;
  mpTransRel->Name("TransRel");
}

// SetTransition(rX1, rEv, rX2)
bool vGenerator::SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2) {
  FD_DG("vGenerator(" << this << ")::SetTransition(\""
	<< rX1 << "\", \"" << rEv << "\", \"" << rX2 << "\")");
  Idx x1 = StateIndex(rX1);
  Idx x2 = StateIndex(rX2);
#ifdef FAUDES_CHECKED
  if (x1 == 0) {
    FD_ERR("vGenerator::SetTransition: state " << rX1 
	   << " not in stateset");
    std::stringstream errstr;
    errstr << "State name " << rX1 << " not found in Generator";
    throw Exception("vGenerator::SetTransition(..)", errstr.str(), 90);
  }
  if (! mpAlphabet->Exists(rEv)) {
    FD_ERR("vGenerator::SetTransition: event " << rEv << " not in alphabet");
    std::stringstream errstr;
    errstr << "Event name " << rEv << " not found in event domain of Generator";
    throw Exception("vGenerator::SetTransition(..)", errstr.str(), 95);
  }
  if (x2 == 0) {
    FD_ERR("vGenerator::SetTransition: state " << rX2 << " not in stateset");
    std::stringstream errstr;
    errstr << "State name " << rX2 << " not found in Generator";
    throw Exception("vGenerator::SetTransition(..)", errstr.str(), 90);
  }
#endif
  return SetTransition(Transition(x1, EventIndex(rEv), x2));
}


// SetTransition(x1, ev, x2)
bool vGenerator::SetTransition(Idx x1, Idx ev, Idx x2) {
  return SetTransition(Transition(x1,ev,x2));
}

// SetTransition(rTransition)
bool vGenerator::SetTransition(const Transition& rTransition) {
  FD_DG("vGenerator(" << this << ")::SetTransition(" << rTransition.X1 << "," 
	<< rTransition.Ev << "," << rTransition.X2 << ")");
#ifdef FAUDES_CHECKED
  if (! mpStates->Exists(rTransition.X1)) {
    std::stringstream errstr;
    errstr << "vGenerator::SetTransition: state " << SStr(rTransition.X1) 
	   << " not in stateset";
    throw Exception("vGenerator::SetTransition(..)", errstr.str(), 95);
  }
  if (! mpAlphabet->Exists(rTransition.Ev)) {
    std::stringstream errstr;
    errstr << "vGenerator::SetTransition: event " << EStr(rTransition.Ev) 
	   << " not in alphabet ";
    throw Exception("vGenerator::SetTransition(..)", errstr.str(), 95);
  }
  if (! mpStates->Exists(rTransition.X2)) {
    std::stringstream errstr;
    errstr << "vGenerator::SetTransition: state " << SStr(rTransition.X2) 
	   << " not in stateset";
    throw Exception("vGenerator::SetTransition(..)", errstr.str(), 95);
  }
#endif
  return mpTransRel->Insert(rTransition);
}



// ClrTransition.X1, ev, x2)
void vGenerator::ClrTransition(Idx x1, Idx ev, Idx x2) {
  FD_DG("vGenerator(" << this << ")::ClrTransition(" 
	<< x1 << "," << ev << "," << x2 << ")");
  mpTransRel->Erase(x1, ev, x2);
}

// ClrTransition(rTransition)
void vGenerator::ClrTransition(const Transition& rTransition) {
  FD_DG("vGenerator(" << this << ")::ClrTransition(" << TStr(rTransition) << ")");
  mpTransRel->Erase(rTransition);
}

// ClrTransition(it)
TransSet::Iterator vGenerator::ClrTransition(TransSet::Iterator it) {
  FD_DG("vGenerator(" << this << ")::ClrTransition(" << TStr(*it)<< ")" );
  return mpTransRel->Erase(it);
}

// ClrTransitions(X1, ev)
void vGenerator::ClrTransitions(Idx x1, Idx ev) {
  FD_DG("vGenerator(" << this << ")::ClrTransition(" 
	<< x1 << "," << ev << ")");
  mpTransRel->EraseByX1Ev(x1, ev);
}

// ClrTransitions(X1)
void vGenerator::ClrTransitions(Idx x1) {
  FD_DG("vGenerator(" << this << ")::ClrTransition(" 
	<< x1  << ")");
  mpTransRel->EraseByX1(x1);
}

// TransAttribute(trans, attr)
void vGenerator::TransAttribute(const Transition& rTrans, const Type& rAttr) {
  FD_DG("vGenerator(" << this << ")::TransAttribute(" 
	<< TStr(rTrans) << ",\"" << rAttr.ToString() << "\")");
  mpTransRel->Attribute(rTrans, rAttr);
}

// TransAttributep(trans)
AttributeVoid* vGenerator::TransAttributep(const Transition& rTrans)  {
  return mpTransRel->Attributep(rTrans);
}


// TransAttribute(trans)
const AttributeVoid& vGenerator::TransAttribute(const Transition& rTrans) const {
  return mpTransRel->Attribute(rTrans);
}

// ClrTransAttribute(trans)
void vGenerator::ClrTransAttribute(const Transition& rTrans)  {
  mpTransRel->ClrAttribute(rTrans);
}


// ClearTransRel()
void vGenerator::ClearTransRel(void) {
  mpTransRel->Clear();
}

// EventAttribute(index, attr)
void vGenerator::EventAttribute(Idx index, const Type& rAttr) {
  FD_DG("vGenerator(" << this << ")::EventAttribute(" 
	<< EStr(index) << ",\"" << rAttr.ToString() << "\")");
  mpAlphabet->Attribute(index, rAttr);
}


// EventAttributes(set)
void vGenerator::EventAttributes(const EventSet& rEventSet) {
  FD_DG("vGenerator(" << this << ")::EventAttributes(" 
	<< rEventSet.ToString() << "\")");
  mpAlphabet->Attributes(rEventSet);
}

// ClrEventAttribute(index)
void vGenerator::ClrEventAttribute(Idx index) {
  FD_DG("vGenerator(" << this << ")::ClrEventAttribute(\"" << EStr(index) << "\")");
  mpAlphabet->ClrAttribute(index);
}

// StateAttribute(index, attr)
void vGenerator::StateAttribute(Idx index, const Type& rAttr) {
  FD_DG("vGenerator(" << this << ")::StateAttribute(" 
	<< SStr(index) << ",\"" << rAttr.ToString() << "\")");
  mpStates->Attribute(index, rAttr);
}

// ClrStateAttribute(index)
void vGenerator::ClrStateAttribute(Idx index) {
  FD_DG("vGenerator(" << this << ")::ClrStateAttribute(\"" << index << "\")");
  mpStates->ClrAttribute(index);
}

// ExistsEvent(index)
bool vGenerator::ExistsEvent(Idx index) const {
  return mpAlphabet->Exists(index);
}

// ExistsEvent(rName)
bool vGenerator::ExistsEvent(const std::string& rName) const {
  return mpAlphabet->Exists(rName);
}

// FindEvent(index) const
EventSet::Iterator vGenerator::FindEvent(Idx index) const {
  return mpAlphabet->Find(index);
}

// FindEvent(rName)
EventSet::Iterator vGenerator::FindEvent(const std::string& rName) const {
  return mpAlphabet->Find(rName);
}

// ExistsState(index)
bool vGenerator::ExistsState(Idx index) const {
  return mpStates->Exists(index);
}

// ExistsName(name)
bool vGenerator::ExistsState(const std::string& rName) const {
  return ExistsState(StateIndex(rName));
}

// FindState(rName) const
StateSet::Iterator vGenerator::FindState(const std::string& rName) const {
  return mpStates->Find(mpStateSymbolTable->Index(rName));
}

// FindState(index) const
StateSet::Iterator vGenerator::FindState(Idx index) const {
  return mpStates->Find(index);
}

// ExistsInitState(index)
bool vGenerator::ExistsInitState(Idx index) const {
  return mInitStates.Exists(index);
}

// FindInitState(index)
StateSet::Iterator vGenerator::FindInitState(Idx index) const {
  return mInitStates.Find(index);
}

// ExistsMarkedState(index)
bool vGenerator::ExistsMarkedState(Idx index) const {
  return mMarkedStates.Exists(index);
}

// FindMarkedState(index)
StateSet::Iterator vGenerator::FindMarkedState(Idx index) const {
  return mMarkedStates.Find(index);
}

// EventAttribute(index)
const AttributeVoid&  vGenerator::EventAttribute(Idx index) const {
  return mpAlphabet->Attribute(index);
}

// EventAttributep(index)
AttributeVoid* vGenerator::EventAttributep(Idx index) {
  return mpAlphabet->Attributep(index);
}

// EventAttribute(rName)
const AttributeVoid&  vGenerator::EventAttribute(const std::string& rName) const {
  return EventAttribute(EventIndex(rName));
}

// EventAttributep(rName)
AttributeVoid* vGenerator::EventAttributep(const std::string& rName) {
  return EventAttributep(EventIndex(rName));
}

// StateAttribute(index)
const AttributeVoid&  vGenerator::StateAttribute(Idx index) const {
  return mpStates->Attribute(index);
}

// StateAttributep(index)
AttributeVoid*  vGenerator::StateAttributep(Idx index) {
  return mpStates->Attributep(index);
}

// GlobalAttribute(attr)
void vGenerator::GlobalAttribute(const Type& rAttr) {
  FD_DG("vGenerator(" << this << ")::GlobalAttribute(" 
	<< rAttr.ToString() << "\")");
  // set to void is ok for silient ignore
  if(typeid(rAttr)==typeid(AttributeVoid)) return;
  // error:
  std::stringstream errstr;
  errstr << "cannot cast global attribute " << rAttr.ToString() << " for generator " << Name();
  throw Exception("vGenerator::GlobalAttribute", errstr.str(), 63);
}

// GlobalAttributeTry(attr)
void vGenerator::GlobalAttributeTry(const Type& rAttr) {
  FD_DG("vGenerator(" << this << ")::GlobalAttributeTry(" 
	<< rAttr.ToString() << "\")");
  // ignore
}

// GlobalAttribute()
const AttributeVoid& vGenerator::GlobalAttribute(void) const {
  FD_DG("vGenerator(" << this << ")::GlobalAttribute()");
  return *mpGlobalAttribute;
}

// GlobalAttributep()
AttributeVoid* vGenerator::GlobalAttributep(void) {
  FD_DG("vGenerator(" << this << ")::GlobalAttributep()");
  return mpGlobalAttribute;
}


// Alphabet()
const EventSet& vGenerator::Alphabet(void) const {
  return *mpAlphabet;
}

// States()
const StateSet& vGenerator::States(void) const {
  return *mpStates;
}

// TransRel()
const TransSet& vGenerator::TransRel(void) const {
  return *mpTransRel;
}


// TransRel(res)
void vGenerator::TransRel(TransSetX1EvX2& res) const { mpTransRel->ReSort(res); }
void vGenerator::TransRel(TransSetEvX1X2& res) const { mpTransRel->ReSort(res); }
void vGenerator::TransRel(TransSetEvX2X1& res) const { mpTransRel->ReSort(res); }
void vGenerator::TransRel(TransSetX2EvX1& res) const { mpTransRel->ReSort(res); }
void vGenerator::TransRel(TransSetX2X1Ev& res) const { mpTransRel->ReSort(res); }
void vGenerator::TransRel(TransSetX1X2Ev& res) const { mpTransRel->ReSort(res); }


Transition vGenerator::TransitionByNames(
    const std::string& rX1, const std::string& rEv, const std::string& rX2) const {
  return Transition(StateIndex(rX1),EventIndex(rEv),StateIndex(rX2));
}

// InitStates()
const StateSet& vGenerator::InitStates(void) const {
  return mInitStates;
}

// MarkedStates()
const StateSet& vGenerator::MarkedStates(void) const {
  return mMarkedStates;
}

// MinimizeAlphabet()
void vGenerator::MinimizeAlphabet(void) {
  mpAlphabet->NameSet::EraseSet(UnusedEvents());
}

// UsedEvents()
EventSet vGenerator::UsedEvents(void) const {
  EventSet resultset = NewEventSet();
  TransSet::Iterator it;
  for (it = mpTransRel->Begin(); it != mpTransRel->End(); ++it) {
    resultset.Insert(it->Ev);
  }
  return resultset;
}

// UnusedEvents()
EventSet vGenerator::UnusedEvents(void) const {
  return *mpAlphabet - UsedEvents();
}

// ActiveEventSet(x1)
EventSet vGenerator::ActiveEventSet(Idx x1) const {
  EventSet result = NewEventSet();
  TransSet::Iterator it;
  for (it = TransRelBegin(x1); it != TransRelEnd(x1); ++it) {
    result.Insert(it->Ev);
  }
  return result;
}

// ActiveTransSet(x1)
TransSet vGenerator::ActiveTransSet(Idx x1) const {
  TransSet result;
  TransSet::Iterator it;
  for (it = TransRelBegin(x1); it != TransRelEnd(x1); ++it) {
    result.Insert(*it);
  }
  return result;
}

// TransRelStates()
StateSet vGenerator::TransRelStates(void) const {
  StateSet states;
  TransSet::Iterator it;
  for (it=mpTransRel->Begin(); it != mpTransRel->End(); ++it) {
    states.Insert(it->X1);
    states.Insert(it->X2);
  }
  return states;
}

// SuccessorStates(x1)
StateSet vGenerator::SuccessorStates(Idx x1) const {
  return mpTransRel->SuccessorStates(x1);
}

// SuccessorStates(x1,ev)
StateSet vGenerator::SuccessorStates(Idx x1, Idx ev) const {
  return mpTransRel->SuccessorStates(x1,ev);
}


// idx SuccessorState() const
Idx vGenerator::SuccessorState(Idx x1, Idx ev) const {
  TransSet::Iterator it = mpTransRel->Begin(x1,ev);
  TransSet::Iterator it_end = mpTransRel->End(x1,ev);
  if(it==it_end) return 0;
  Idx res=(*it).X2;
#ifdef FAUDES_CHECKED
  it++;
  if(it!=it_end) {
    std::stringstream errstr;
    errstr << "successor state does not exist uniquely" << std::endl;
    throw Exception("vGenerator::SuccessorState", errstr.str(), 92);
  }
#endif
  return res;
}


// AccessibleSet()
StateSet vGenerator::AccessibleSet(void) const {
  // initialize todo stack
  std::stack<Idx> todo;
  StateSet::Iterator sit;
  for(sit = InitStatesBegin(); sit != InitStatesEnd(); ++sit) 
    todo.push(*sit);
  // loop variables
  StateSet accessibleset;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  // loop
  while(!todo.empty()) {
    // pop
    Idx x1=todo.top();
    todo.pop();
    // sense known
    if(accessibleset.Exists(x1)) continue; 
    // record
    accessibleset.Insert(x1);
    // iterate/push
    tit=TransRelBegin(x1);
    tit_end=TransRelEnd(x1);
    for(; tit != tit_end; ++tit) 
      if(tit->X2 != x1) 
        todo.push(tit->X2);
  }
  // done
  accessibleset.Name("AccessibleSet");
  return accessibleset;
}

// Accessible()
bool vGenerator::Accessible(void) {
  StateSet accessibleset = AccessibleSet();
  StateSet not_accessible = *mpStates - accessibleset;
  DelStates(not_accessible);
  // return true if there is an initial state
  if (!mInitStates.Empty()) {
    FD_DF("vGenerator::accessible: generator is accessible");
    return true;
  }
  FD_DF("vGenerator::accessible: generator is accessible but empty");
  return false;
}

// IsAccessible()
bool vGenerator::IsAccessible(void) const {
  if(AccessibleSet() == *mpStates) {
    FD_DF("vGenerator::accessible: generator is accessible");
    return true;
  }
  FD_DF("vGenerator::accessible: generator is not accessible");
  return false;
}

// CoaccessibleSet()
StateSet vGenerator::CoaccessibleSet(void) const {
  // build reverse transition relation
  TransSetX2EvX1 rtrel;
  TransRel(rtrel);
  // initialize todo stack
  StateSet::Iterator sit;
  std::stack<Idx> todo;
  for(sit = MarkedStatesBegin(); sit != MarkedStatesEnd(); ++sit) 
    todo.push(*sit);
  // loop variables
  StateSet coaccessibleset;
  TransSetX2EvX1::Iterator tit;
  TransSetX2EvX1::Iterator tit_end;
  // loop
  while(!todo.empty()) {
    // pop
    Idx x2=todo.top();
    todo.pop();
    // sense known
    if(coaccessibleset.Exists(x2)) continue; 
    // record
    coaccessibleset.Insert(x2);
    // iterate/push
    tit=rtrel.BeginByX2(x2);
    tit_end=rtrel.EndByX2(x2);
    for(; tit != tit_end; ++tit) 
      if(tit->X1 != x2) 
        todo.push(tit->X1);
  }
  // done
  coaccessibleset.Name("CoaccessibleSet");
  return coaccessibleset;
}

// Coaccessible()
bool vGenerator::Coaccessible(void) {
  StateSet coaccessibleset = CoaccessibleSet();
  StateSet not_coaccessible = *mpStates - coaccessibleset;
  DelStates(not_coaccessible);
  // return true if there is a marked state
  if (! mMarkedStates.Empty()) {
    FD_DF("vGenerator::coaccessible: generator is coaccessible");
    return true;
  }
  FD_DF("vGenerator::coaccessible: generator is not coaccessible");
  return false;
}

// IsCoaccessible()
bool vGenerator::IsCoaccessible(void) const {
  if(CoaccessibleSet() == *mpStates) {
    FD_DF("vGenerator::coaccessible: generator is coaccessible");
    return true;
  }
  FD_DF("vGenerator::coaccessible: generator is not coaccessible");
  return false;
}

// TrimSet()
StateSet vGenerator::TrimSet(void) const {
  FD_DF("vGenerator::trimset: trim states: " 
	<< StateSet(AccessibleSet() * CoaccessibleSet()).ToString());
  StateSet res=AccessibleSet() * CoaccessibleSet();
  res.Name("TrimSet");
  return res;
}

// Trim()
bool vGenerator::Trim(void) {
  FD_DF("vGenerator::trim: generator states: " << mpStates->ToString());
  // better: compute sets and do one state delete
  bool accessiblebool = Accessible();
  bool coaccessiblebool = Coaccessible();
  FD_DF("vGenerator::trim: trim states: " << mpStates->ToString());
  if(accessiblebool && coaccessiblebool) {
    FD_DF("vGenerator::Trim(): generator is nontrivial");
    return true;
  }
  FD_DF("vGenerator::Trim(): generator is trivial");
  return false;
}


// IsTrim()
bool vGenerator::IsTrim(void) const {
  bool res=true;
  if(!IsAccessible()) res=false;
  else if(!IsCoaccessible()) res=false;
  FD_DF("vGenerator::IsTrim(): result " << res);
  return res;
}


// BlockingStates()
StateSet vGenerator::BlockingStates(void) const {
  FD_DF("vGenerator::BlockingSet: blocking states: " 
	<< StateSet(AccessibleSet() - CoaccessibleSet()).ToString());
  return AccessibleSet() - CoaccessibleSet();
}



// IsComplete
bool vGenerator::IsComplete(const StateSet& rStates) const {
  FD_DG("IsComplete(" << Name() << ")");
  
  // loop over provided states
  StateSet::Iterator sit=rStates.Begin();
  StateSet::Iterator sit_end=rStates.End();
  for(;sit!=sit_end;sit++){
    TransSet::Iterator tit=TransRelBegin(*sit);
    TransSet::Iterator tit_end=TransRelEnd(*sit);
    if(tit==tit_end) break;
  }
  // return true if no terminal state has been found
  return sit==sit_end;
}


// IsComplete w.r.t. rSigmaO
bool vGenerator::IsComplete(const EventSet& rSigmaO) const {
  FD_DC("IsComplete(" << Name() << ")");
  // find good states: where some rEvent is enabled
  std::stack<Idx> todo;
  TransSet::Iterator tit=TransRelBegin();
  TransSet::Iterator tit_end=TransRelEnd();
  while(tit!=tit_end){
    if(!rSigmaO.Exists(tit->Ev)) { ++tit; continue; }
    todo.push(tit->X1);
    tit=TransRelEnd(tit->X1);
  }
  // build reverse transition relation
  TransSetX2EvX1 rtrel;
  TransRel(rtrel);
  // reverse reachability analysis for more good states
  StateSet good;
  while(!todo.empty()) {
    // pop
    Idx x2=todo.top();
    todo.pop();
    // sense known
    if(good.Exists(x2)) continue; 
    // record
    good.Insert(x2);
    // iterate/push
    TransSetX2EvX1::Iterator tit=rtrel.BeginByX2(x2);
    TransSetX2EvX1::Iterator tit_end=rtrel.EndByX2(x2);
    for(; tit != tit_end; ++tit) 
      if(tit->X1 != x2) todo.push(tit->X1);
  }
  FD_DG("IsComplete(" << Name() << "): done");
  // done
  return good == States();
}


// IsComplete
bool vGenerator::IsComplete(void) const {
  return IsComplete(States());
}

// Complete
bool vGenerator::Complete(void) {
  // states to remove
  StateSet termset = TerminalStates();
  // iterative search 
  bool done;
  do {
    // over all states (could make use of reverse transrel here)
    StateSet::Iterator sit = States().Begin();
    StateSet::Iterator sit_end = States().End();
    done=true;
    for(; sit!=sit_end; ++sit) {
      if(termset.Exists(*sit)) continue;
      TransSet::Iterator tit = TransRelBegin(*sit);
      TransSet::Iterator tit_end = TransRelEnd(*sit);
      for (; tit != tit_end; ++tit) {
        if(!termset.Exists(tit->X2)) break;
      }
      if(tit==tit_end) {
        termset.Insert(*sit);
	done=false;
      }
    }
  } while(!done);
  // remove those states
  DelStates(termset);
  // done
  return !InitStates().Empty();
}

// Complete w.r.t. rSigmaO 
bool vGenerator::Complete(const EventSet& rSigmaO) {

  // prepare reverse transition relation
  TransSetX2EvX1 rtrel;
  TransRel(rtrel);

  // initialize nu-iteration
  StateSet domain=States();

  // nu-loop 
  while(true){

    // initialize mu-iteration
    StateSet target;
    TransSet::Iterator tit=TransRelBegin();
    TransSet::Iterator tit_end=TransRelEnd();
    while(tit!=tit_end) {
      if(!rSigmaO.Exists(tit->Ev)) { ++tit; continue; } 
      if(!domain.Exists(tit->X2)) { ++tit; continue; } 
      target.Insert(tit->X1);
      tit=TransRelEnd(tit->X1);
    }

    // mu-loop: find good states by reverse reachability
    StateSet good;
    StateSet::Iterator sit;
    std::stack<Idx> todo;
    for(sit = target.Begin(); sit != target.End(); ++sit) 
    todo.push(*sit);
    while(!todo.empty()) {
      // pop
      Idx x2=todo.top();
      todo.pop();
      // sense known
      if(good.Exists(x2)) continue; 
      // record
      good.Insert(x2);
      // iterate/push
      TransSetX2EvX1::Iterator tit=rtrel.BeginByX2(x2);
      TransSetX2EvX1::Iterator tit_end=rtrel.EndByX2(x2);
      for(; tit != tit_end; ++tit) 
        if(tit->X1 != x2) todo.push(tit->X1);
    }

    // nu-break: target will not change on update
    if(domain <= good) break;
 
    // nu-update: restrict target to have a successor within domain
    domain = domain * good;

  }

  // remove other states
  RestrictStates(domain);

  // done
  return !InitStates().Empty();
}



// TerminalStates()
StateSet vGenerator::TerminalStates(const StateSet& rStates) const {
  FD_DG("Generator::TerminalStates(" << Name() << ")");

  // prepare result
  StateSet res;
  
  // loop states
  StateSet::Iterator sit=rStates.Begin();
  StateSet::Iterator sit_end=rStates.End();
  for(;sit!=sit_end;sit++){
    TransSet::Iterator tit=TransRelBegin(*sit);
    TransSet::Iterator tit_end=TransRelEnd(*sit);
    if(tit==tit_end) res.Insert(*sit);
  }
  // return terminal states
  res.Name("TerminalStates");
  return res;
}

// TerminalStates()
StateSet vGenerator::TerminalStates(void) const {
  return TerminalStates(States());
}




// IsDeterministic()
bool vGenerator::IsDeterministic(void) const {
  // if there is more than one initial state ... nondet
  if (InitStatesSize() > 1) {
    FD_DG("vGenerator::IsDeterministic: more than one initial state");
    return false;
  }
  // if there is a state/event pair with non-unique successor ... nondet
  if (TransRelSize() < 2) return true;
  TransSet::Iterator it1;
  TransSet::Iterator it2;
  for (it1 = TransRelBegin(), it2 = it1++; it1 != TransRelEnd(); it2 = it1++) {
    if ((it1->X1 == it2->X1) && (it1->Ev == it2->Ev)) {
      FD_DG("IsDeterministic(): at least one state "
	    << "contains more than on transition with same event: " 
	    << TStr(*it1));
     return false;
    }
  }
  // in all other cases generator is deterministic
  return true;
}


// ReindexOnWrite()
bool vGenerator::ReindexOnWrite(void) const {
  return mReindexOnWrite;
}

// ReindexOnWrite(flag)
void vGenerator::ReindexOnWrite(bool flag) {
  mReindexOnWrite = flag;
}

// ReindexOnWrite(flag)
void vGenerator::ReindexOnWriteDefault(bool flag) {
  msReindexOnWriteDefault = flag;
}

// ReindexOnWrite(flag)
bool vGenerator::ReindexOnWriteDefault(void) {
  return msReindexOnWriteDefault;
}



// DoWrite()
void vGenerator::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  // pre 2.20 behaviour: re-index on file output
  /*
  if(rTw.FileMode()) 
    SetMinStateIndexMap();
  */
  // post 2.20 beaviour: re-index by configuration
  if(ReindexOnWrite())
    SetMinStateIndexMap();
  // figure section
  std::string label=rLabel;
  if(label=="") label="Generator"; 
  FD_DG("vGenerator(" << this << ")::DoWrite(): section " << label);
  // write generator
  Token btag;
  btag.SetBegin(label);
  if(mObjectName!=label) btag.InsAttributeString("name",mObjectName);
  rTw.Write(btag);
  rTw << "\n";
  SWrite(rTw);
  rTw << "\n";
  mpAlphabet->Write(rTw);
  rTw << "\n";
  WriteStates(rTw);
  rTw << "\n";
  WriteTransRel(rTw);
  rTw << "\n";
  WriteStateSet(rTw, mInitStates);
  rTw << "\n";
  WriteStateSet(rTw, mMarkedStates);
  rTw << "\n";
  mpGlobalAttribute->Write(rTw,"",this);
  rTw << "\n";
  rTw.WriteEnd(label);
  // end of reindex
  ClearMinStateIndexMap();
}

// DoDWrite()
void vGenerator::DoDWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(label=="") label="Generator"; 
  FD_DG("vGenerator(" << this << ")::DoDWrite(): section " << label);
  // write generator
  rTw.WriteBegin(label);
  rTw << mObjectName;
  rTw << "\n";
  rTw << "\n";
  SWrite(rTw);
  rTw << "\n";
  mpAlphabet->DWrite(rTw);
  rTw << "\n";
  DWriteStateSet(rTw, *mpStates);
  rTw << "\n";
  DWriteTransRel(rTw);
  rTw << "\n";
  DWriteStateSet(rTw, mInitStates);
  rTw << "\n";
  DWriteStateSet(rTw, mMarkedStates);
  rTw << "\n";
  mpGlobalAttribute->DWrite(rTw,"",this);
  rTw << "\n";
  rTw.WriteEnd(label);
  rTw << "\n";
}

// DoXWrite()
void vGenerator::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  // Set up outer tag
  std::string label=rLabel;
  std::string ftype=TypeName();
  if(label=="") label="Generator";
  Token btag;
  btag.SetBegin(label);
  if(Name()!=label && Name()!="") btag.InsAttributeString("name",Name());
  //2.24e: make ftype mandatory for XML format to allow for more flexible faudes/plain format
  //if(ftype!=label && ftype!="") btag.InsAttributeString("ftype",ftype); // pre 2.22e  
  btag.InsAttributeString("ftype",ftype); // 2.24e
  FD_DG("vGenerator(" << this << ")::DoXWrite(..): section " << btag.StringValue() << " #" << Size());
  rTw.Write(btag);
  // Optional re-indexing
  if(mReindexOnWrite) SetMinStateIndexMap();
  // Write comcponents
  rTw << "\n";
  mpAlphabet->XWrite(rTw,"Alphabet");
  rTw << "\n";
  XWriteStateSet(rTw, *mpStates,"StateSet");
  rTw << "\n";
  XWriteTransRel(rTw);
  rTw << "\n";
  mpGlobalAttribute->XWrite(rTw,"",this);
  if(!mpGlobalAttribute->IsDefault())
    rTw << "\n";
  // Write end
  rTw.WriteEnd(btag.StringValue());
  // End of re-index
  ClearMinStateIndexMap();
}

// WriteAlphabet() 
void vGenerator::WriteAlphabet(void) const {
  TokenWriter tw(TokenWriter::Stdout);
  WriteAlphabet(tw);
}

// AlphabetToString()
std::string vGenerator::AlphabetToString(void) const {
  TokenWriter tw(TokenWriter::String);
  WriteAlphabet(tw);
  return tw.Str();
}

// WriteAlphabet(rTw&) 
void vGenerator::WriteAlphabet(TokenWriter& rTw) const { 
  mpAlphabet->Write(rTw);
}

// WriteStateSet(rStateSet&) 
void vGenerator::WriteStateSet(const StateSet&  rStateSet) const {
  TokenWriter tw(TokenWriter::Stdout);
  WriteStateSet(tw,rStateSet);
}

// StateSetToString()
std::string vGenerator::StateSetToString(const StateSet&  rStateSet) const {
  TokenWriter tw(TokenWriter::String);
  WriteStateSet(tw,rStateSet);
  return tw.Str();
}

// StateSetToText()
std::string vGenerator::StateSetToText(const StateSet&  rStateSet) const {
  TokenWriter tw(TokenWriter::String);
  tw.Endl(true);
  WriteStateSet(tw,rStateSet);
  return tw.Str();
}


// WriteStateSet(rTw&, rStateSet&) 
void vGenerator::WriteStates(TokenWriter& rTw) const {
  // have my section
  rTw.WriteBegin("States");
  // test whether we reindex
  bool reindex=mMinStateIndexMap.size()>0;
  // if we reindex, setup reverse map to write in strategic order
  // to allow for consisten read (i.e. states with symbolic name first, starting
  // with index 1); this is the plain faudes file format from 2005
  if(reindex) {
    // reverse map
    std::map<Idx,Idx> reversemap;
    std::map<Idx,Idx>::const_iterator minit;
    StateSet::Iterator sit;
    for (sit = StatesBegin(); sit != StatesEnd(); ++sit) 
      reversemap[MinStateIndex(*sit)] = *sit;
    // iterate states to write 
    for(minit = reversemap.begin(); minit != reversemap.end(); ++minit) {
      // identify anonymous block (consecutive state indices)
      std::map<Idx,Idx>::const_iterator conit=minit;
      Idx start = conit->first;
      Idx anoncount = 0;
      for(; conit != reversemap.end(); ++conit) {
        if(StateName(conit->second) != "") break;
        if(!StateAttribute(conit->second).IsDefault()) break;
        if(conit->first != start+anoncount) break;
        ++anoncount;
      }
      // write anonymous block
      if(anoncount > FD_CONSECUTIVE) {
        rTw.WriteBegin("Consecutive");
        rTw << start;
        rTw << start+anoncount-1;
        rTw.WriteEnd("Consecutive");
        minit=conit;
      } 
      // break loop
      if(minit == reversemap.end()) break;
      // write non anonymous state name/idx
      std::string statename = StateName(minit->second);
      if (statename != "") rTw << statename;
      else rTw << minit->first;
      // write state attribute
      const AttributeVoid& attr=StateAttribute(minit->second);
      attr.Write(rTw);
    }
  }
  // if we dont reindex, write symbolic names with index suffix to
  // enable a consistent read; this was introduced with libfaudes 2.20j
  if(!reindex) {
    // iterate states to write 
    bool symexpl = (States().MaxIndex() != States().Size());
    StateSet::Iterator sit;
    for(sit = StatesBegin(); sit != StatesEnd(); ++sit) {
      // identify anonymous block (consecutive state indices)
      StateSet::Iterator conit=sit;
      Idx start = *conit;
      Idx anoncount = 0;
      for(; conit != StatesEnd(); ++conit) {
        if(StateName(*conit) != "") break;
        if(!StateAttribute(*conit).IsDefault()) break;
        if(*conit != start+anoncount) break;
        ++anoncount;
      }
      // write anonymous block
      if(anoncount > FD_CONSECUTIVE) {
        rTw.WriteBegin("Consecutive");
        rTw << start;
        rTw << start+anoncount-1;
        rTw.WriteEnd("Consecutive");
        sit=conit;
      } 
      // break loop
      if(sit == StatesEnd()) break;
      // write index or name with index suffix
      std::string statename = StateName(*sit);
      if((statename != "") && symexpl) {
        rTw << (statename + "#"+ToStringInteger(*sit)) ;
      } else if(statename != "") {
        rTw << statename;
      } else {
        rTw << *sit;
      }
      // write attribute
      const AttributeVoid& attr=StateAttribute(*sit);
      attr.Write(rTw);
    }
  }
  // write end tag
  rTw.WriteEnd("States");
}


// WriteStateSet(rTw&, rStateSet&) 
void vGenerator::WriteStateSet(TokenWriter& rTw, const StateSet&  rStateSet, const std::string& rLabel) const {
  // begin tag
  std::string label=rLabel;
  if(label.empty()) label=rStateSet.Name();
  rTw.WriteBegin(label);
  // test whether we reindex
  bool reindex=mMinStateIndexMap.size()>0;
  // if we reindex, setup reverse map to write in strategic order;
  // reading back is no issue for external states, however, we would like
  // to provoke large consecutive blocks as a benefit from re-indexing
  if(reindex) {
    // reverse map
    std::map<Idx,Idx> reversemap;
    std::map<Idx,Idx>::const_iterator minit;
    StateSet::Iterator sit;
    for (sit = rStateSet.Begin(); sit != rStateSet.End(); ++sit) 
      reversemap[MinStateIndex(*sit)] = *sit;
    // iterate states to write 
    for(minit = reversemap.begin(); minit != reversemap.end(); ++minit) {
      // identify anonymous block (consecutive state indices)
      std::map<Idx,Idx>::const_iterator conit=minit;
      Idx start = conit->first;
      Idx anoncount = 0;
      for(; conit != reversemap.end(); ++conit) {
        if(StateName(conit->second) != "") break;
        if(!StateAttribute(conit->second).IsDefault()) break;
        if(conit->first != start+anoncount) break;
        ++anoncount;
      }
      // write anonymous block
      if(anoncount > FD_CONSECUTIVE) {
        rTw.WriteBegin("Consecutive");
        rTw << start;
        rTw << start+anoncount-1;
        rTw.WriteEnd("Consecutive");
        minit=conit;
      } 
      // break loop
      if(minit == reversemap.end()) break;
      // write non anonymous state name/idx
      std::string statename = StateName(minit->second);
      if (statename != "") rTw << statename;
      else rTw << minit->first;
      // write state attribute
      const AttributeVoid& attr=rStateSet.Attribute(minit->second);
      attr.Write(rTw);
    }
  }
  // if we dont reindex, we dont need the reverse map; note that
  // external stateset will never have an explicit symbol table
  if(!reindex) {
    // iterate states to write 
    StateSet::Iterator sit;
    for(sit = rStateSet.Begin(); sit != rStateSet.End(); ++sit) {
      // identify anonymous block (consecutive state indices)
      StateSet::Iterator conit=sit;
      Idx start = *conit;
      Idx anoncount = 0;
      for(; conit != rStateSet.End(); ++conit) {
        if(StateName(*conit) != "") break;
        if(!StateAttribute(*conit).IsDefault()) break;
        if(*conit != start+anoncount) break;
        ++anoncount;
      }
      // write anonymous block
      if(anoncount > FD_CONSECUTIVE) {
        rTw.WriteBegin("Consecutive");
        rTw << start;
        rTw << start+anoncount-1;
        rTw.WriteEnd("Consecutive");
        sit=conit;
      } 
      // break loop
      if(sit == rStateSet.End()) break;
      // write non anonymous state name/idx
      std::string statename = StateName(*sit);
      if (statename != "") rTw << statename;
      else rTw << *sit;
      // write attribute
      const AttributeVoid& attr=rStateSet.Attribute(*sit);
      attr.Write(rTw);
    }
  }
  // write end tag
  rTw.WriteEnd(label);
}


// DWriteStateSet(rTw&, rStateSet&) 
void vGenerator::DWriteStateSet(TokenWriter& rTw, const StateSet& rStateSet) const {
  rTw.WriteBegin(rStateSet.Name());
  StateSet::Iterator sit;
  for(sit = rStateSet.Begin(); sit != rStateSet.End(); ++sit) {
    rTw << SStr(*sit);
    const AttributeVoid& attr=rStateSet.Attribute(*sit);
    attr.Write(rTw);
  }
  rTw.WriteEnd(rStateSet.Name());
}


// XWriteStateSet(rTw&, rStateSet&) 
void vGenerator::XWriteStateSet(TokenWriter& rTw, const StateSet&  rStateSet, const std::string& rLabel) const {
  // figure label
  std::string label=rLabel;
  if(label=="") label=rStateSet.Name();
  if(label=="") label="StateSet";
  rTw.WriteBegin(label);
  // build reverse index map of states to write ( fileidx->idx )
  // -- this ensures named states to be written first; see SetMinStateIndexMap()
  // -- this is required to figure consecutive blocks
  std::map<Idx,Idx> reversemap;
  std::map<Idx,Idx>::const_iterator minit;
  StateSet::Iterator sit;
  for (sit = rStateSet.Begin(); sit != rStateSet.End(); ++sit) {
    reversemap[MinStateIndex(*sit)] = *sit;
  }
  // iterate states to write 
  for(minit = reversemap.begin(); minit != reversemap.end(); ++minit) {
    // identify anonymous block (consecutive state indices, no names, no attributes)
    std::map<Idx,Idx>::const_iterator conit=minit;
    Idx start = conit->first;
    Idx anoncount = 0;
    for(; conit != reversemap.end(); ++conit) {
      if(StateName(conit->second) != "") break;
      if(!StateAttribute(conit->second).IsDefault()) break;
      if(ExistsInitState(conit->second)) break;
      if(ExistsMarkedState(conit->second)) break;
      if(conit->first != start+anoncount) break;
      ++anoncount;
    }
    // write anonymous block
    if(anoncount > FD_CONSECUTIVE) {
      Token contag;
      contag.SetEmpty("Consecutive");
      contag.InsAttributeInteger("from",start);
      contag.InsAttributeInteger("to",start+anoncount-1);
      rTw.Write(contag);
      minit=conit;
    } 
    // break loop
    if(minit == reversemap.end() )
      break;
    // prepare state token
    Token sttag;    
    std::string statename = StateName(minit->second);
    Idx index=minit->first;
    sttag.SetBegin("State");
    sttag.InsAttributeInteger("id",index);
    if(statename!="")
    if(&rStateSet==mpStates) 
      sttag.InsAttributeString("name",statename);
    rTw.Write(sttag);
    // marking
    if(ExistsInitState(minit->second))   { sttag.SetEmpty("Initial"); rTw.Write(sttag);}; 
    if(ExistsMarkedState(minit->second)) { sttag.SetEmpty("Marked");  rTw.Write(sttag);}; 
    // attribute
    const AttributeVoid& attr=rStateSet.Attribute(minit->second);
    if(!attr.IsDefault()) attr.XWrite(rTw);
    // done
    rTw.WriteEnd("State");
  }
  rTw.WriteEnd(label);
}


// StatesToString()
std::string vGenerator::StatesToString(void) const {
  return StateSetToString(*mpStates);
}

// StatesToText()
std::string vGenerator::StatesToText(void) const {
  return StateSetToText(*mpStates);
}

// MarkedStatesToString()
std::string vGenerator::MarkedStatesToString(void) const {
  return StateSetToString(mMarkedStates);
}

// InitStatesToString()
std::string vGenerator::InitStatesToString(void) const {
  return StateSetToString(mInitStates);
}


// WriteTransRel() 
void vGenerator::WriteTransRel(void) const {
  TokenWriter tw(TokenWriter::Stdout);
  WriteTransRel(tw);
}

// TransRelToString()
std::string vGenerator::TransRelToString(void) const {
  TokenWriter tw(TokenWriter::String);
  WriteTransRel(tw);
  return tw.Str();
}

// TransRelToText()
std::string vGenerator::TransRelToText(void) const {
  TokenWriter tw(TokenWriter::String);
  tw.Endl(true);
  WriteTransRel(tw);
  return tw.Str();
}

// WriteTransRel(rTw&)
void vGenerator::WriteTransRel(TokenWriter& rTw) const {
  TransSet::Iterator tit;
  int oldcolumns = rTw.Columns();
  rTw.Columns(3);
  rTw.WriteBegin("TransRel");
  bool smalltransrel = (Size() < FD_SMALLTRANSREL);

  // loop all transitions
  for(tit = mpTransRel->Begin(); tit != mpTransRel->End(); ++tit) {

    // write x1
    Idx x1=MinStateIndex(tit->X1);
    if (smalltransrel) {
      std::string x1name = StateName(tit->X1);
      if (x1name != "") {
	rTw << x1name;
      } else {
	rTw << x1;
      }
    } else {
      rTw << x1;
    }

    // write ev
    rTw << EventName(tit->Ev);

    // write x2
    Idx x2=MinStateIndex(tit->X2);
    if (smalltransrel) {
      std::string x2name = StateName(tit->X2);
      if (x2name != "") {
	rTw << x2name;
      } else {
	rTw << x2;
      }
    } else {
      rTw << x2;
    }

    // write attributes
    TransAttribute(*tit).Write(rTw);

  }
  rTw.WriteEnd("TransRel");
  rTw.Columns(oldcolumns);
}

// DWriteTransRel(rTw&)
void vGenerator::DWriteTransRel(TokenWriter& rTw) const {
  TransSet::Iterator tit;
  int oldcolumns = rTw.Columns();
  rTw.Columns(3);
  rTw.WriteBegin("TransRel");
  // iterate all transitions
  for (tit = mpTransRel->Begin(); tit != mpTransRel->End(); ++tit) {
    // write x1
    std::ostringstream ox1;
    Idx x1= tit->X1;
    std::string x1name = StateName(x1);
    if (x1name != "") {
      ox1 << x1name << "[" << x1 << "]";
    } else {
      ox1 << x1;
    }
    rTw << ox1.str();
    // write ev
    std::ostringstream oev;
    Idx ev= tit->Ev;
    std::string evname = EventName(ev);
    oev << evname << "[" << ev << "]";
    rTw << oev.str();
    // write x2
    std::ostringstream ox2;
    Idx x2= tit->X2;
    std::string x2name = StateName(x2);
    if (x2name != "") {
      ox2 << x2name << "[" << x2 << "]";
    } else {
      ox2 << x2;
    }
    rTw << ox2.str();
    // attribute
    mpTransRel->Attribute(*tit).Write(rTw);
  }
  rTw.WriteEnd("TransRel");
  rTw.Columns(oldcolumns);
}


// XWriteTransRel(rTw&)
void vGenerator::XWriteTransRel(TokenWriter& rTw) const {
  TransSet::Iterator tit;
  rTw.WriteBegin("TransitionRelation");

  // loop all transitions
  for(tit = mpTransRel->Begin(); tit != mpTransRel->End(); ++tit) {

    // retrieve values x1
    Idx x1=MinStateIndex(tit->X1);
    Idx x2=MinStateIndex(tit->X2);
    std::string ev=EventName(tit->Ev);

    // prepare tag
    Token trtag;
    trtag.SetEmpty("Transition");
    trtag.InsAttributeInteger("x1",x1);
    trtag.InsAttributeString("event",ev);
    trtag.InsAttributeInteger("x2",x2);

    // consider attribute
    const AttributeVoid& attr=TransAttribute(*tit);
    // case a: indeed no attribute value
    if(attr.IsDefault()) {
      rTw.Write(trtag);
    } 
    // calse b: with attribute
    else {
      trtag.ClrEnd();
      rTw.Write(trtag);
      attr.XWrite(rTw);
      rTw.WriteEnd(trtag.StringValue());
    }

  }
  rTw.WriteEnd("TransitionRelation");
}

// DoSWrite(rTw&)
void vGenerator::DoSWrite(TokenWriter& rTw) const
{
  Type::DoSWrite(rTw);
  rTw.WriteComment(" States:        " + ToStringInteger(Size()) );
  rTw.WriteComment(" Init/Marked:   " + ToStringInteger(mInitStates.Size()) 
	      + "/" + ToStringInteger(mMarkedStates.Size()));
  rTw.WriteComment(" Events:        " + ToStringInteger(mpAlphabet->Size()) );
  rTw.WriteComment(" Transitions:   " + ToStringInteger(mpTransRel->Size()) );
  rTw.WriteComment(" StateSymbols:  " + ToStringInteger(mpStateSymbolTable->Size()) );
  rTw.WriteComment(" Attrib. E/S/T: " + ToStringInteger(mpAlphabet->AttributesSize()) 
	      + "/" + ToStringInteger(mpStates->AttributesSize())
	      + "/" + ToStringInteger(mpTransRel->AttributesSize()) );
  rTw.WriteComment("");
}

// DotWrite(rFileName)
void vGenerator::DotWrite(const std::string& rFileName) const {
  FD_DG("vGenerator(" << this << ")::DotWrite(" << rFileName << ")");
  if(ReindexOnWrite())
    SetMinStateIndexMap();
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  try {
    std::ofstream stream;
    stream.exceptions(std::ios::badbit|std::ios::failbit);
    stream.open(rFileName.c_str());
    stream << "// dot output generated by libFAUDES vGenerator" << std::endl;
    stream << "digraph \"" << Name() << "\" {" << std::endl;
    stream << "  rankdir=LR" << std::endl;
    stream << "  node [shape=circle];" << std::endl;
    stream << std::endl;
    stream << "  // initial states" << std::endl;
    int i = 1;
    for (lit = InitStatesBegin(); lit != InitStatesEnd(); ++lit) {
      std::string xname= StateName(*lit);
      if(xname=="") xname=ToStringInteger(MinStateIndex(*lit));
      stream << "  dot_dummyinit_" << i << " [shape=none, label=\"\", width=\"0.0\", height=\"0.0\" ];" << std::endl;
      stream << "  dot_dummyinit_" << i << " -> \"" << xname << "\";" << std::endl; 
      i++;
    }
    stream << std::endl;
    stream << "  // mstates" << std::endl;
    for (lit = MarkedStatesBegin(); lit != MarkedStatesEnd(); ++lit) {
      std::string xname= StateName(*lit);
      if(xname=="") xname=ToStringInteger(MinStateIndex(*lit));
      stream << "  \"" << xname << "\" [shape=doublecircle];" << std::endl;
    }
    stream << std::endl;
    stream << "  // rest of stateset" << std::endl;
    for (lit = StatesBegin(); lit != StatesEnd(); ++lit) {
      if (! (ExistsInitState(*lit) || ExistsMarkedState(*lit)) ) {
	std::string xname= StateName(*lit);
	if(xname=="") xname=ToStringInteger(MinStateIndex(*lit));
	stream << "  \"" << xname << "\";" << std::endl;
      }
    }
    stream << std::endl;
    stream << "  // transition relation" << std::endl;
    for(tit = TransRelBegin(); tit != TransRelEnd(); ++tit) {
      std::string x1name= StateName(tit->X1);
      if(x1name=="") x1name=ToStringInteger(MinStateIndex(tit->X1));
      std::string x2name= StateName(tit->X2);
      if(x2name=="") x2name=ToStringInteger(MinStateIndex(tit->X2));
      stream << "  \"" << x1name  << "\" -> \"" << x2name
	     << "\" [label=\"" << EventName(tit->Ev) << "\"];" << std::endl;
    }
    stream << "}" << std::endl;
    stream.close();
  }
  catch (std::ios::failure&) {
    throw Exception("vGenerator::DotWrite", 
		    "Exception opening/writing dotfile \""+rFileName+"\"", 2);
  }
  ClearMinStateIndexMap();
}

// DDotWrite(rFileName)
void vGenerator::DDotWrite(const std::string& rFileName) const {
  FD_DG("vGenerator(" << this << ")::DDotWrite(" << rFileName << ")");
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  try {
    std::ofstream stream;
    stream.exceptions(std::ios::badbit|std::ios::failbit);
    stream.open(rFileName.c_str());
    stream << "digraph \"" << Name() << "\" {" << std::endl;
    stream << "  rankdir=LR" << std::endl;
    stream << "  node [shape=circle];" << std::endl;
    stream << std::endl;
    stream << "  // istates" << std::endl;
    int i = 1;
    for (lit = InitStatesBegin(); lit != InitStatesEnd(); ++lit) {
      stream << "  dot_dummyinit_" << i << " [shape=none, label=\"\" ];" << std::endl;
      stream << "  dot_dummyinit_" << i << " -> \"" 
	     << SStr(*lit)  << "\";" << std::endl; 
      i++;
    }
    stream << std::endl;
    stream << "  // mstates" << std::endl;
    for (lit = MarkedStatesBegin(); lit != MarkedStatesEnd(); ++lit) {
      stream << "  \"" << SStr(*lit) << "\" [shape=doublecircle];" << std::endl;
    }
    stream << std::endl;
    stream << "  // rest of stateset" << std::endl;
    for (lit = StatesBegin(); lit != StatesEnd(); ++lit) {
      // if not in mInitStates or mMarkedStates
      if (! (ExistsInitState(*lit) || ExistsMarkedState(*lit)) ) {
	stream << "  \"" << SStr(*lit) << "\";" << std::endl;
      }
    }
    stream << std::endl;
    stream << "  // transition relation" << std::endl;
    for (tit = TransRelBegin(); tit != TransRelEnd(); ++tit) {
      stream << "  \"" << SStr(tit->X1) 
	     << "\" -> \"" << SStr(tit->X2)
	     << "\" [label=\"" << EventName(tit->Ev) << "\"];" << std::endl;
    }
    stream << "}" << std::endl;
    stream.close();
  }
  catch (std::ios::failure&) {
    throw Exception("vGenerator::DDotWrite", 
		    "Exception opening/writing dotfile \""+rFileName+"\"", 2);
  }
}


// XDotWrite(rFileName)
void vGenerator::XDotWrite(const std::string& rFileName) const {
  FD_DG("vGenerator(" << this << ")::XDotWrite(" << rFileName << ")");
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  try {
    std::ofstream stream;
    stream.exceptions(std::ios::badbit|std::ios::failbit);
    stream.open(rFileName.c_str());
    stream << "digraph \"___" << Name() << "___\" {" << std::endl;
    stream << "  rankdir=LR" << std::endl;
    stream << "  node [shape=circle];" << std::endl;
    stream << std::endl;
    stream << "  // stateset" << std::endl;
    for (lit = InitStatesBegin(); lit != InitStatesEnd(); ++lit) {
      stream << "  \"s" << *lit << "\";" << std::endl;
    }
    for (lit = MarkedStatesBegin(); lit != MarkedStatesEnd(); ++lit) {
      stream << "  \"s" << *lit << "\";" << std::endl;
    }
    for (lit = StatesBegin(); lit != StatesEnd(); ++lit) {
      if(ExistsInitState(*lit)) continue;
      if(ExistsMarkedState(*lit)) continue;
      stream << "  \"s" << *lit << "\";" << std::endl;
    }
    stream << std::endl;
    stream << "  // transition relation" << std::endl;
    for (tit = TransRelBegin(); tit != TransRelEnd(); ++tit) {
      stream << "  \"s" << tit->X1 
	     << "\" -> \"s" << tit->X2
	     << "\" [label=\"e" << tit->Ev << "\" " << "polyline" << "];" << std::endl;
    }
    stream << "}" << std::endl;
    stream.close();
  }
  catch (std::ios::failure&) {
    throw Exception("vGenerator::XDotWrite", 
		    "Exception opening/writing dotfile \""+rFileName+"\"", 2);
  }
}

// DoRead(tr)
void vGenerator::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  std::string label=rLabel;
  if(label=="") label="Generator"; 
  FD_DG("vGenerator(" << this << ")::DoRead(): file " << rTr.FileName() << "  section " << label);
  // clear old stuff
  Clear();
  // find Generator tag 
  Token btag;
  rTr.ReadBegin(label, btag);
  int seclev=rTr.Level();
  // hypothesis: format is either "relaxed native 2.24e" or "xml"
  bool native=true;
  bool xml= true;
  // the ftype attribute is mandatory to indicate xml format (as of 2.24e)
  if(btag.ExistsAttributeString("ftype")) { native=false;}
  // try name by relaxed native 2.24e
  if(native) {
    FD_DG("vGenerator(" << this << ")::DoRead(): relaxed native header")
    std::string name="Generator";
    // figure name: as attribute
    if(btag.ExistsAttributeString("name")) 
      name=btag.AttributeStringValue("name");
    // figure name: as string token
    Token token;
    rTr.Peek(token);
    if(token.IsString()) { name=rTr.ReadString(); xml=false; }
    Name(name);
  }
  // try core sets by relaxed native 2.24e
  if(native) {
    FD_DG("vGenerator(" << this << ")::DoRead(): relaxed native core")
    // read alphabet (optional)
    ReadAlphabet(rTr);
    // read stateset (optional)
    ReadStates(rTr);
    // read transrel (required --- if not present, we might have mis-sensed pre 2.24e xml)
    Token token;
    rTr.Peek(token);
    FD_DG("vGenerator(" << this << ")::DoRead(): " << token.Str());
    if(token.IsBegin("TransRel") || token.IsBegin("T")) {
      ReadTransRel(rTr);
    } else {
      native=false;
      rTr.Reset(seclev);
      Clear();
    }
  }
  // try extra sets by relaxed native 2.24e
  if(native) {
    FD_DG("vGenerator(" << this << ")::DoRead(): native extra items")
    // read istates (optional)
    Token token;
    rTr.Peek(token); 
    if(token.IsBegin("InitStates"))
      {ReadStateSet(rTr, "InitStates", mInitStates); xml=false;}
    if(token.IsBegin("I"))
      {ReadStateSet(rTr, "I", mInitStates); xml=false;}
    mInitStates.Name("InitStates");
    // read mstates (optional)
    rTr.Peek(token); 
    if(token.IsBegin("MarkedStates"))
      {ReadStateSet(rTr, "MarkedStates", mMarkedStates); xml=false; }
    if(token.IsBegin("M"))
      {ReadStateSet(rTr, "M", mMarkedStates); xml=false;}
    mMarkedStates.Name("MarkedStates");
    // read attribute
    mpGlobalAttribute->Read(rTr,"",this);
  }
  // if we survived, its not xml
  if(native) xml=false;
  // read strict xml format
  if(xml) {
    FD_DG("vGenerator(" << this << ")::DoRead(): xml")
    // figure generator name
    std::string name="generator";
    if(btag.ExistsAttributeString("name")) 
      name=btag.AttributeStringValue("name");
    Name(name);
    // read alphabet 
    mpAlphabet->Read(rTr,"Alphabet");
    // read state set, incl. init/marked attribute
    XReadStateSet(rTr, *mpStates, "StateSet");
    mpStates->Name("States");
    // read trans rel
    XReadTransRel(rTr);
    // read attribute
    mpGlobalAttribute->Read(rTr,"",this);
    // fix labels
    mInitStates.Name("InitStates");
    mMarkedStates.Name("MarkedStates");
  }
  // read end 
  rTr.ReadEnd(label);
  FD_DG("vGenerator(" << this << ")::DoRead(): done");
}

// ReadAlphabet(rTr) 
void vGenerator::ReadAlphabet(TokenReader& rTr) {
  FD_DG("vGenerator(" << this << ")::ReadAlphabet(\"" 
	<< rTr.FileName() << "\")");
  Token token;
  rTr.Peek(token);
  if(token.IsBegin("Alphabet"))
    mpAlphabet->Read(rTr,"Alphabet");
  if(token.IsBegin("A"))
    mpAlphabet->Read(rTr,"A");
  mpAlphabet->Name("Alphabet");
}

// ReadStates(tr) 
void vGenerator::ReadStates(TokenReader& rTr) {
  FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\")");
  // HELPERS:
  Token token;
  std::string label="";
  rTr.Peek(token);
  if(token.IsBegin("States")) label=token.StringValue();
  if(token.IsBegin("S")) label=token.StringValue();
  if(label=="") return;
  AttributeVoid* attrp = mpStates->AttributeType()->New();
  FD_DG("vGenerator(" << this << ")::ReadStates(..): attribute type " << typeid(*attrp).name());
  // ALGORITHM:
  mpStates->Clear();
  mpStates->Name("States");
  mStateSymbolTable.Clear();
  // track occurence of explicit symboltable
  bool symimpl=false;
  bool symexpl=false;
  Idx  symnext=1;
  // loop section
  rTr.ReadBegin(label);  
  while(!rTr.Eos(label)) {
    // peek
    rTr.Peek(token);
    // read state by index
    if(token.IsInteger()) {
      rTr.Get(token);
      Idx index = token.IntegerValue();
      FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\"): by index " << index);
      if(mpStates->Exists(index)) {
        delete attrp;
	std::stringstream errstr;
	errstr << "Token " << token.IntegerValue() << " appears twice in stateset"
	       << rTr.FileLine();
	throw Exception("vGenerator::ReadStates", errstr.str(), 80);
      }
      // read attribute
      attrp->Read(rTr,"",this);
      // skip unknown attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      InsState(index); 
      StateAttribute(index,*attrp);
      symnext++;
      continue;
    } 
    // read state by name
    if(token.IsString()) {
      rTr.Get(token);
      FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\"): by name " << token.StringValue());
      // interpret name, sense index suffx if present
      std::string statename=token.StringValue();
      Idx index=symnext;
      std::size_t pos= statename.find_first_of('#');
      if(pos==std::string::npos) symimpl=true;
      if(pos!=std::string::npos) symexpl=true;
      if(pos!=std::string::npos && pos < statename.size()-1) {
	std::string suffix=statename.substr(pos+1);
        index=ToIdx(suffix);
        statename=statename.substr(0,pos);
        FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\"): extracted suffix from  " << token.StringValue() << ": " << statename << " idx " << index);
      }      
      // no doublets
      if(ExistsState(statename) || ExistsState(index)) {
        delete attrp;
	std::stringstream errstr;
	errstr << "State " << statename << "(idx " << index <<") appears twice in stateset"
	       << rTr.FileLine();
	throw Exception("vGenerator::ReadStates", errstr.str(), 80);
      }
      // read attribute
      attrp->Read(rTr,"",this);
      // skip unknown attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      InsState(index); 
      StateName(index,statename);
      StateAttribute(index,*attrp);
      symnext++;
      continue;
    } 
    // read consecutive block of anonymous states
    if(token.IsBegin("Consecutive")) {
      rTr.ReadBegin("Consecutive");
      Token token1,token2;
      rTr.Get(token1);
      rTr.Get(token2);
      FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\"): consecutive range");
      if ((!token1.IsInteger()) || (!token2.IsInteger())) {
        delete attrp;
	std::stringstream errstr;
	errstr << "Invalid range of consecutive states"  << rTr.FileLine();
	throw Exception("vGenerator::ReadStates", errstr.str(), 80);
      }
      for(Idx index = (Idx) token1.IntegerValue(); index <= (Idx) token2.IntegerValue(); ++index) {
	if(mpStates->Exists(index)) {
          delete attrp;
	  std::stringstream errstr;
	  errstr << "Index " << index << " appears twice in stateset"
		 << rTr.FileLine();
	  throw Exception("vGenerator::ReadStates", errstr.str(), 80);
	}
	InsState(index);
        symnext++;
      }
      rTr.ReadEnd("Consecutive");
      continue;
    }
    // cannot process token
    delete attrp;
    std::stringstream errstr;
    errstr << "Invalid token" << rTr.FileLine();
    throw Exception("vGenerator::ReadStates", errstr.str(), 80);
  }
  rTr.ReadEnd(label);
  // test consistent explicit symboltable
  if(symimpl && symexpl) {
    delete attrp;
    std::stringstream errstr;
    errstr << "StateSet with inconsitent explicit symboltable" << rTr.FileLine();
    throw Exception("vGenerator::ReadStates", errstr.str(), 80);
  }
  // dispose attribute
  delete attrp;
  FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\"): done");
}


// ReadStateSet(tr, rLabel, rStateSet) 
void vGenerator::ReadStateSet(TokenReader& rTr, const std::string& rLabel, StateSet& rStateSet)  const {
  FD_DG("vGenerator(" << this << ")::ReadStateSet(\"" << rLabel<< "\")");
  // HELPERS:
  Token token;
  AttributeVoid* attrp = rStateSet.AttributeType()->New();
  FD_DG("vGenerator(" << this << ")::ReadStateSet(..): attribute type " << typeid(*attrp).name());
  // ALGORITHM:
  rStateSet.Clear();
  rTr.ReadBegin(rLabel);  
  rStateSet.Name(rLabel);
  // loop section
  while(!rTr.Eos(rLabel)) {
    // peek
    rTr.Peek(token);
    
    // read state by index
    if(token.IsInteger()) {
      rTr.Get(token);
      Idx index = token.IntegerValue();
      if(!ExistsState(index)) {
        delete attrp;
	std::stringstream errstr;
	errstr << "Token " << token.IntegerValue() << " not in generator stateset"
	       << rTr.FileLine();
	throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
      }
      // read attribute
      attrp->Read(rTr,"",this);
      // skip unknown attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      rStateSet.Insert(index); 
      rStateSet.Attribute(index,*attrp);
      continue;
    }
    
    // read state by name
    if(token.IsString()) {
      rTr.Get(token);
      // test validity of symbolic name (no suffixes allowed here ... simply ignore)
      std::string statename=token.StringValue();
      std::size_t pos= statename.find_first_of('#');
      if(pos!=std::string::npos) {
        delete attrp;
	std::stringstream errstr;
	errstr << "invalid symbolic name: " << token.StringValue() 
           << " (no suffix allowed in external state sets)" << rTr.FileLine();
	throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
      }
      Idx index =StateIndex(statename);
      if(index==0) {
        delete attrp;
	std::stringstream errstr;
	errstr << "Symbolic name " << token.StringValue() << " not in stateset"
	       << rTr.FileLine();
	throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
      }
      // read attribute
      attrp->Read(rTr,"",this);
      // skip unknown attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      rStateSet.Insert(index); 
      rStateSet.Attribute(index,*attrp);
      continue;
    }
    
    // read state in XML style
    if(token.IsBegin() && token.StringValue() == "State") {
      rTr.Get(token);
      std::string name="";
      if(token.ExistsAttributeString("name"))
        name=token.AttributeStringValue("name");
      Idx index=0;
      if(token.ExistsAttributeInteger("id"))
        index=token.AttributeIntegerValue("id");
      FD_DG("vGenerator::ReadStateSet(): got idx " << index << " " << name);
      // reconstruct index from name if possible
      if(index==0) {
        index=StateIndex(name);
      }
      // failed to figure index
      if(index==0) {
        delete attrp;
        std::stringstream errstr;
        errstr << "Cannot figure index for state token " << token.Str() << rTr.FileLine();
        throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
      }
      // dont allow doublets
      if(rStateSet.Exists(index)) {
        delete attrp;
        std::stringstream errstr;
        errstr << "Doublet state from token " << token.Str() << rTr.FileLine();
        throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
      }
      // record state
      rStateSet.Insert(index);
      // record name if we read the core set
      if(&rStateSet==mpStates) 
      if(name!="") {
         mpStateSymbolTable->SetEntry(index, name);
      }
      // test for attributes 
      if(!rTr.Eos("State")) {	
        FD_DG("vGenerator(" << this << ")::ReadStates(\"" << rTr.FileName() << "\"): attribute ?");
        attrp->Read(rTr,"",this);
        rStateSet.Attribute(index,*attrp);
      } 
      // read end
      rTr.ReadEnd("State");
      continue;
    }

    // read consecutve block of anonymous states
    if(token.IsBegin() && token.StringValue() == "Consecutive") {
      Token ctag;
      rTr.ReadBegin("Consecutive",ctag);
      Idx idx1=0;
      Idx idx2=0;
      Token token1,token2;
      rTr.Peek(token1);
      // figure range a) XML    
      if(token1.IsEnd() && token.StringValue() == "Consecutive") {
        if(ctag.ExistsAttributeInteger("from"))
          idx1= (Idx) ctag.AttributeIntegerValue("from");
        if(ctag.ExistsAttributeInteger("to"))
          idx2= (Idx) ctag.AttributeIntegerValue("to");
      }
      // figure range a) native
      if(token1.IsInteger()) {
        rTr.Get(token1);
        rTr.Get(token2);
        if(!token1.IsInteger() || !token2.IsInteger()) {
          delete attrp;
 	  std::stringstream errstr;
	  errstr << "Invalid range of consecutive states"  << rTr.FileLine();
	  throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
        }
	idx1=token1.IntegerValue();
	idx2=token2.IntegerValue();
      }
      // validate range
      if(idx1==0 || idx2 < idx1) {
        delete attrp;
	std::stringstream errstr;
	errstr << "Invalid range of consecutive states"  << rTr.FileLine();
	throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
      }
      // perform range
      FD_DG("vGenerator(" << this << ")::ReadStateSet(\"" << rTr.FileName() << "\"): consecutive range " << idx1 << " to " << idx2);    
      for(Idx index = idx1; index <= idx2; ++index) {
	if(!ExistsState(index)) {
          delete attrp;
	  std::stringstream errstr;
	  errstr << "range not in generator stateset" << rTr.FileLine();
	  throw Exception("vGenerator::ReadStateSet", errstr.str(), 80);
	}
	rStateSet.Insert(index);
      }
      rTr.ReadEnd("Consecutive");
      continue;
    }
    
    // Ignore other sections
    if(token.IsBegin()) {
      rTr.ReadEnd(token.StringValue());
      continue;
    }
    
    // cannot process token
    delete attrp;
    std::stringstream errstr;
    errstr << "Section " << rLabel << ": Invalid token" << rTr.FileLine() << ": " << token.Str();
    throw Exception("vGenerator::ReadStateSet", errstr.str(), 50);
  }
  rTr.ReadEnd(rLabel);
  // dispose attribute
  delete attrp;
}

// XReadStateSet(tr, rLabel, rStateSet) 
void vGenerator::XReadStateSet(TokenReader& rTr, StateSet& rStateSet, const std::string& rLabel)  const {
  FD_DG("vGenerator(" << this << ")::XReadStateSet(\"" << rLabel<< "\")");
  AttributeVoid* attrp = rStateSet.AttributeType()->New();
  FD_DG("vGenerator(" << this << ")::ReadStateSet(..): attribute type " << typeid(*attrp).name());
  // Clear my set
  rStateSet.Clear();
  // Figure section
  std::string label=rLabel;
  if(label=="") label=rStateSet.Name();
  if(label=="") label="StateSet";
  // Read begin
  Token btag;
  rTr.ReadBegin(label,btag);  
  // Use name if provided (std is no name)
  rStateSet.Name("");
  if(btag.ExistsAttributeString("name"))
    rStateSet.Name(btag.AttributeStringValue("name"));
  // Scan my section
  while(!rTr.Eos(label)) {
    Token sttag;
    rTr.Get(sttag);
    // Read consecutive block of anonymous states
    if(sttag.IsBegin("Consecutive")) {
      // Get range
      Idx idx1=0;
      Idx idx2=0;
      if(sttag.ExistsAttributeInteger("from"))
        idx1= (Idx) sttag.AttributeIntegerValue("from");
      if(sttag.ExistsAttributeInteger("to"))
        idx2= (Idx) sttag.AttributeIntegerValue("to");
      // Insist in valid range
      if(idx1==0 || idx2 < idx1) {
        delete attrp;
	std::stringstream errstr;
	errstr << "Invalid range of consecutive states"  << rTr.FileLine();
	throw Exception("vGenerator::XReadStates", errstr.str(), 80);
      }
      FD_DG("vGenerator(" << this << ")::XReadStates(\"" << rTr.FileName() << "\"): consecutive range " << idx1 << " to " << idx2);
      for(Idx index = idx1; index <= idx2; ++index) {
	if(rStateSet.Exists(index)) {
          delete attrp;
	  std::stringstream errstr;
	  errstr << "Doublet state index " << index << " " << rTr.FileLine();
	  throw Exception("vGenerator::XReadStates", errstr.str(), 80);
	}
	rStateSet.Insert(index);
      }
      // Done: consecutive
      rTr.ReadEnd("Consecutive");
      continue;
    }
    // Ignore other sections
    if(!sttag.IsBegin("State")) {
      if(sttag.IsBegin()) 
        rTr.ReadEnd(sttag.StringValue());
      continue;
    }
    // Its a state
    std::string name="";
    if(sttag.ExistsAttributeString("name"))
      name=sttag.AttributeStringValue("name");
    Idx index=0;
    if(sttag.ExistsAttributeInteger("id"))
      index=sttag.AttributeIntegerValue("id");
    FD_DG("vGenerator::XReadStateSet(): got idx " << index << " " << name);
    // reconstruct index from name if possible
    if(index==0) {
      index=StateIndex(name);
    }
    // failed to figure index
    if(index==0) {
      delete attrp;
      std::stringstream errstr;
      errstr << "Cannot figure index for state token " << sttag.Str() << rTr.FileLine();
      throw Exception("vGenerator::XReadStateSet", errstr.str(), 80);
    }
    // dont allow doublets
    if(rStateSet.Exists(index)) {
      delete attrp;
      std::stringstream errstr;
      errstr << "Doublet state from token " << sttag.Str() << rTr.FileLine();
      throw Exception("vGenerator::XReadStateSet", errstr.str(), 80);
    }
    // record state
    rStateSet.Insert(index);
    // record name if we read the core set
    if(&rStateSet==mpStates) 
    if(name!="") {
       mpStateSymbolTable->SetEntry(index, name);
    }
    // test for attributes and such
    while(!rTr.Eos("State")) {
      Token token;
      rTr.Peek(token);
      // marking (if this is the main state set)
      Token mtag; 
      if(token.IsBegin("Initial") && (&rStateSet==mpStates)) { 
        rTr.ReadBegin("Initial",mtag);  
        bool v=true;
        if(mtag.ExistsAttributeInteger("value"))
          v=mtag.AttributeIntegerValue("value");
        if(v) const_cast<vGenerator*>(this)->SetInitState(index);
        rTr.ReadEnd("Initial");
        continue;
      }
      if(token.IsBegin("Marked") && (&rStateSet==mpStates)) { 
        rTr.ReadBegin("Marked",mtag);  
        bool v=true;
        if(mtag.ExistsAttributeInteger("value"))
          v=mtag.AttributeIntegerValue("value");
        if(v) const_cast<vGenerator*>(this)->SetMarkedState(index);
        rTr.ReadEnd("Marked");
        continue;
      }
      // read attribute
      FD_DG("vGenerator(" << this << ")::XReadStates(\"" << rTr.FileName() << "\"): attribute ?");
      attrp->Read(rTr,"",this);
      rStateSet.Attribute(index,*attrp);
      // break on first unknown/undigested
      break;
    } 
    // read end
    rTr.ReadEnd("State");
  }
  rTr.ReadEnd(label);
  // dispose attribute
  delete attrp;
  FD_DG("vGenerator(" << this << ")::XReadStates(\"" << rTr.FileName() << "\"): done");
}



// ReadTransRel(tr) 
void vGenerator::ReadTransRel(TokenReader& rTr) {
  FD_DG("vGenerator(" << this << ")::ReadTransRel(\"" << rTr.FileName() << "\")");
  AttributeVoid* attrp = mpTransRel->AttributeType()->New();
  FD_DG("vGenerator(" << this << ")::ReadTransRel(..): attribute type " << typeid(*attrp).name());

  // sense begin
  std::string label="";
  Token token;
  rTr.Peek(token);
  if(token.IsBegin("TransRel")) label="TransRel";
  if(token.IsBegin("T")) label="T";
  if(label=="") {
    std::stringstream errstr;
    errstr << "Reading TransRel failed in " << rTr.FileLine() << ": no valid begin token";
    throw Exception("vGenerator::ReadTransRel", errstr.str(), 50); 
  }
  rTr.ReadBegin(label);

  // 2.24e: allow for new states or new events if not specified so far
  bool isx = StateSet().Empty();
  bool isn = StateSet().Empty();
  bool ien = Alphabet().Empty();

  // read section
  try {
  while(!rTr.Eos(label)) {

    // local vars
    Token x1t;
    Token x2t;
    Token evt;

    // clear my transition
    Idx x1 = 0, ev = 0, x2 = 0;

    // 1: the x1 token
    rTr >> x1t;
    if(x1t.IsInteger()) {
      x1=x1t.IntegerValue();
      if((!ExistsState(x1)) &&  isx) {InsState(x1); isn=false; };
    } else if(x1t.IsString()) {
      x1=StateIndex(x1t.StringValue());
      if(x1==0 && isn) { x1=InsState(x1t.StringValue()); isx=false; };
    } else break;

    // 2: the event token
    rTr >> evt;
    if(evt.IsString()) {
      evt.StringValue();
      ev=EventIndex(evt.StringValue());
      if((!ExistsEvent(ev)) &&  ien) {ev = InsEvent(evt.StringValue());};
    } else break;

    // 3: the x2 token
    rTr >> x2t;
    if(x2t.IsInteger()) {
      x2=x2t.IntegerValue();
      if((!ExistsState(x2)) &&  isx) {InsState(x2); isn=false; };
    } else if(x2t.IsString()) {
      x2=StateIndex(x2t.StringValue());
      if(x2==0 && isn) { x2=InsState(x2t.StringValue()); isx=false; };
    } else break;

    // 4: attributes
    attrp->Read(rTr,"",this);

    // 5: skip unknown attributes
    AttributeVoid::Skip(rTr);

    // check values
    if(!ExistsState(x1)){
      std::stringstream errstr;
      errstr << "invalid state x1 " << x1t.StringValue() << " " << rTr.FileLine();
      throw Exception("vGenerator::ReadTransRel", errstr.str(), 85);
    } 
    if(!ExistsState(x2)){
      std::stringstream errstr;
      errstr << "invalid state x2 " << x2t.StringValue() << " " << rTr.FileLine();
      throw Exception("vGenerator::ReadTransRel", errstr.str(), 85);
    } 
    if(!ExistsEvent(ev)) {
      std::stringstream errstr;
      errstr << "invalid event " << evt.StringValue() << " " << rTr.FileLine();
      throw Exception("vGenerator::ReadTransRel", errstr.str(), 85);
    } 

    // insert transition
    Transition trans=Transition(x1,ev,x2);
    SetTransition(trans);
    TransAttribute(trans,*attrp);

  } // end while
  } // end try
    
  catch (faudes::Exception& oex) {
    delete attrp;
    std::stringstream errstr;
    errstr << "Reading TransRel failed in " << rTr.FileLine() << " " << oex.What();
    throw Exception("vGenerator::ReadTransRel", errstr.str(), 50); 
  }

  // read end token  
  rTr.ReadEnd(label);   

  // done
  FD_DG("vGenerator(" << this << ")::ReadTransRel(\"" << rTr.FileName() << "\"): done");

  // dispose attribute
  delete attrp;
}


// XReadTransRel(tr) 
void vGenerator::XReadTransRel(TokenReader& rTr) {
  FD_DG("vGenerator(" << this << ")::XReadTransRel()");
  AttributeVoid* attrp = mpTransRel->AttributeType()->New();
  FD_DG("vGenerator(" << this << ")::ReadTransRel(..): attribute type " << typeid(*attrp).name());
  // Clear my set
  mpTransRel->Clear();
  mpTransRel->Name("TransRel");
  // Read begin
  Token btag;
  rTr.ReadBegin("TransitionRelation",btag);  
  // Scan my section
  while(!rTr.Eos("TransitionRelation")) {
    Token trtag;
    rTr.Get(trtag);
    // Ignore other sections
    if(!trtag.IsBegin("Transition")) {
      if(trtag.IsBegin()) 
        rTr.ReadEnd(trtag.StringValue());
      continue;
    }
    // Its a transition
    Idx x1=0;
    Idx x2=0;
    Idx ev=0;
    std::string evname;
    std::string x1name;
    std::string x2name;
    if(trtag.ExistsAttributeInteger("x1"))
      x1=trtag.AttributeIntegerValue("x1");
    if(trtag.ExistsAttributeInteger("x2"))
      x2=trtag.AttributeIntegerValue("x2");
    if(trtag.ExistsAttributeString("x1"))
      x1name=trtag.AttributeStringValue("x1");
    if(trtag.ExistsAttributeString("x2"))
      x2name=trtag.AttributeStringValue("x2");
    if(trtag.ExistsAttributeString("event"))
      evname=trtag.AttributeStringValue("event");
    // Interpret names
    ev=EventIndex(evname);
    if(x1==0) x1=StateIndex(x1name);
    if(x2==0) x2=StateIndex(x2name);
    // Report
    FD_DG("vGenerator::XReadTransRel(): got transition " << TStr(Transition(x1,ev,x2)));
    // test 1: components specified
    if(x1==0 || x2==0 || ev==0) {
      delete attrp;
      std::stringstream errstr;
      errstr << "Invalid transition at token " << trtag.Str() << rTr.FileLine();
      throw Exception("vGenerator::XReadTransRel", errstr.str(), 80);
    }
    // test 2: no doublets
    if(ExistsTransition(x1,ev,x2)) {
      delete attrp;
      std::stringstream errstr;
      errstr << "Doublet transition at token " << trtag.Str() << rTr.FileLine();
      throw Exception("vGenerator::XReadTransRel", errstr.str(), 80);
    }
    // test 3: components must exist
    if(!ExistsState(x1)){
      delete attrp;
      std::stringstream errstr;
      errstr << "Invalid state x1 " << x1 << " " << rTr.FileLine();
      throw Exception("vGenerator::XReadTransRel", errstr.str(), 80);
    } 
    if(!ExistsState(x2)){
      delete attrp;
      std::stringstream errstr;
      errstr << "Invalid state x2 " << x2 << " " << rTr.FileLine();
      throw Exception("vGenerator::XReadTransRel", errstr.str(), 80);
    } 
    if(!ExistsEvent(ev)){
      delete attrp;
      std::stringstream errstr;
      errstr << "Invalid event  " << evname << " " << rTr.FileLine();
      throw Exception("vGenerator::XReadTransRel", errstr.str(), 80);
    } 
    // record transition
    SetTransition(x1,ev,x2);
    // test for attribute
    Token token;
    rTr.Peek(token);
    if(!token.IsEnd("Transition")) {
      // read attribute
      attrp->Read(rTr,"",this);
      TransAttribute(Transition(x1,ev,x2),*attrp);
    } 
    // read end
    rTr.ReadEnd("Transition");
  }
  rTr.ReadEnd("TransitionRelation");
  // dispose attribute
  delete attrp;
  FD_DG("vGenerator(" << this << ")::XReadTransRel(\"" << rTr.FileName() << "\"): done");
}

// EStr(index)
std::string vGenerator::EStr(Idx index) const {
  std::string name = EventName(index);
  return name+"#"+faudes::ToStringInteger(index);
}

// SStr(index)
std::string vGenerator::SStr(Idx index) const {
  std::string name = StateName(index);
  if(name == "") name=ToStringInteger(index);
  return name+"#"+faudes::ToStringInteger(index);
}

// TStr(index)
std::string vGenerator::TStr(const Transition& trans) const {
  return SStr(trans.X1) + "--(" + EStr(trans.Ev) + ")-->" + SStr(trans.X2);
}


// GraphWrite(rFileName,rOutFormat)
void vGenerator::GraphWrite(const std::string& rFileName, const std::string& rOutFormat,
   const std::string& rDotExec) const {
  FD_DG("vGenerator::GraphWrite(...): " << typeid(*this).name());
  // generate temp dot input file
  std::string dotin = CreateTempFile();
  if(dotin == "") {
    std::stringstream errstr;
    errstr << "Exception opening temp file";
    throw Exception("vGenerator::GraphWrite", errstr.str(), 2);
  }
  try{
    DotWrite(dotin);
  } 
  catch (faudes::Exception& exception) {  
    FileDelete(dotin);
    std::stringstream errstr;
    errstr << "Exception writing dot input file";
    throw Exception("vGenerator::GraphWrite", errstr.str(), 2);
  }
  try{
    faudes::ProcessDot(dotin,rFileName,rOutFormat,rDotExec);
  } 
  catch (faudes::Exception& exception) {  
    FileDelete(dotin);
    std::stringstream errstr;
    errstr << "Exception processing dot file";
    throw Exception("vGenerator::GraphWrite", errstr.str(), 3);
  }
  FileDelete(dotin);
}

// rti wrapper
bool IsAccessible(const vGenerator& rGen) {
  return rGen.IsAccessible();
}

// rti wrapper
bool IsCoaccessible(const vGenerator& rGen) {
  return rGen.IsCoaccessible();
}

// rti wrapper
bool IsTrim(const vGenerator& rGen) {
  return rGen.IsTrim();
}


// rti wrapper
bool IsComplete(const vGenerator& rGen) {
  return rGen.IsComplete();
}

// rti wrapper
bool IsComplete(const vGenerator& rGen, const StateSet& rStateSet) {
  return rGen.IsComplete(rStateSet);
}

// rti wrapper
bool IsComplete(const vGenerator& rGen, const EventSet& rSigmaO) {
  return rGen.IsComplete(rSigmaO);
}

// rti wrapper
bool IsDeterministic(const vGenerator& rGen) {
  return rGen.IsDeterministic();
}


// rti wrapper
void Accessible(vGenerator& rGen) {
  rGen.Accessible();
}

// rti wrapper
void Accessible(const vGenerator& rGen, vGenerator& rRes) {
  rRes=rGen;
  rRes.Accessible();
}  

// rti wrapper
void Coaccessible(vGenerator& rGen) {
  rGen.Coaccessible();
}

// rti wrapper
void Coaccessible(const vGenerator& rGen, vGenerator& rRes) {
  rRes=rGen;
  rRes.Coaccessible();
}  

// rti wrapper
void Complete(vGenerator& rGen) {
  rGen.Complete();
}

// rti wrapper
void Complete(const vGenerator& rGen, vGenerator& rRes) {
  rRes=rGen;
  rRes.Complete();
}  

// rti wrapper
void Complete(vGenerator& rGen, const EventSet& rSigmaO) {
  rGen.Complete(rSigmaO);
}

// rti wrapper
void Complete(const vGenerator& rGen, const EventSet& rSigmaO, vGenerator& rRes) {
  rRes=rGen;
  rRes.Complete(rSigmaO);
}  

// rti wrapper
void Trim(vGenerator& rGen) {
  rGen.Trim();
}

// rti wrapper
void Trim(const vGenerator& rGen, vGenerator& rRes) {
  rRes=rGen;
  rRes.Trim();
}  


// rti wrapper
void MarkAllStates(vGenerator& rGen) {
  rGen.InjectMarkedStates(rGen.States());
}

// rti wrapper
void AlphabetExtract(const vGenerator& rGen, EventSet& rRes) {
  // This function is an ideal debugging case for lbp_function.cpp.
  // FD_WARN("AlphabetExtract(): gen at " << &rGen << " sig at " << &rRes);
  // FD_WARN("AlphabetExtract(): gen id " << typeid(rGen).name() << " sig id " << typeid(rRes).name());
  rRes = rGen.Alphabet();
  // rRes.Write();
}

// rti convenience function
void SetIntersection(const GeneratorVector& rGenVec, EventSet& rRes) {
  // prepare result
  rRes.Clear();
  // ignore empty
  if(rGenVec.Size()==0) return;
  // copy first
  rRes.Assign(rGenVec.At(0).Alphabet());
  // perform intersecttion 
  for(GeneratorVector::Position i=1; i<rGenVec.Size(); i++) 
    SetIntersection(rGenVec.At(i).Alphabet(),rRes,rRes);
}


// rti convenience function
void SetUnion(const GeneratorVector& rGenVec, EventSet& rRes) {
  // prepare result
  rRes.Clear();
  // ignore empty
  if(rGenVec.Size()==0) return;
  // copy first
  rRes.Assign(rGenVec.At(0).Alphabet());
  // perform intersecttion 
  for(GeneratorVector::Position i=1; i<rGenVec.Size(); i++) 
    SetUnion(rGenVec.At(i).Alphabet(),rRes,rRes);
}

// rti convenience function
void SetIntersection(const vGenerator& rGenA, const vGenerator& rGenB, EventSet& rRes) {
  SetIntersection(rGenA.Alphabet(),rGenB.Alphabet(),rRes);
}

// rti convenience function
void SetUnion(const vGenerator& rGenA, const vGenerator& rGenB, EventSet& rRes) {
  SetUnion(rGenA.Alphabet(),rGenB.Alphabet(),rRes);
}

// rti convenience function
void SetDifference(const vGenerator& rGenA, const vGenerator& rGenB, EventSet& rRes) {
  SetDifference(rGenA.Alphabet(),rGenB.Alphabet(),rRes);
}


} // name space


/***** NOTES ****/

/*
class A {
public:
  virtual A& operator=(const A& src) { std::cout << "A=A/"; return *this; };
  virtual A& assign(const A& src) { std::cout << "A=A/"; return *this; };
  int a;
};
class B : public A {
public:
  virtual A& operator=(const A& src) { std::cout<<"B=A/"; return *this;};
  virtual A& assign(const A& src) { std::cout<<"B=A/"; return *this; };
  int b;
};

  

int main() {

  B mD1;
  B mD2;
  mD1=mD2;   // << fails here, dont know why
  mD1.assign(mD2);
  A& rD1 = mD1;
  A& rD2 = mD2;
  rD1=rD2;
}

*/

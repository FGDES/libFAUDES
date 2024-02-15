/* tp_timecontraint.cpp -- model of timeconstraints in timed automata */


/* Timeplugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  B Schlein
   Copyright (C) 2007  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/



#include "tp_timeconstraint.h"

namespace faudes {


/********************************************************************

 Implementation of ClockSet

********************************************************************/


// std faudes type (cannot do New() with macro)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,ClockSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,ClockSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,ClockSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,ClockSet,NameSet)

// ClockSet(void);
ClockSet::ClockSet(void) : NameSet() { 
  // overwrite default static symboltable 
  mpSymbolTable=GlobalClockSymbolTablep();
  NameSet::Name("Clocks"); 
  FD_DC("ClockSet("<<this<<")::ClockSet() with csymtab "<< SymbolTablep()); 
}

// ClockSet(clockset)
ClockSet::ClockSet(const ClockSet& rOtherSet) : NameSet(rOtherSet) {
  FD_DC("ClockSet(" << this << ")::ClockSet(rOtherSet " << &rOtherSet << ")");
}

// ClockSet(file);
ClockSet::ClockSet(const std::string& rFilename, const std::string& rLabel) : NameSet() {
  // overwrite default static symboltable 
  mpSymbolTable= GlobalClockSymbolTablep(); 
  NameSet::Name("Clocks"); 
  // read file
  NameSet::Read(rFilename,rLabel);
}

// Clockset::New()
ClockSet* ClockSet::New(void) const {
  ClockSet* res = new ClockSet();
  res->mpSymbolTable=mpSymbolTable;
  return res;
}

// DoAssign()
void ClockSet::DoAssign(const ClockSet& rSourceSet) {
  // call base
  NameSet::DoAssign(rSourceSet);
}

// DoEqual()
bool ClockSet::DoEqual(const ClockSet& rOtherSet) const {
  // call base
  return NameSet::DoEqual(rOtherSet);
}



// ClockSet::GlobalClockSymbolTablep
// (initialize on first use pattern)
SymbolTable* ClockSet::GlobalClockSymbolTablep(void) {
  static SymbolTable fls;
  return &fls; 
}






/********************************************************************

 Implementation of ElemConstraint

********************************************************************/


// helper (conversion from operatorname to string
std::string ElemConstraint::OperatorName(Operator op){
  switch(op){
  case LessThan: return "LT";  
  case GreaterThan: return "GT";  
  case LessEqual: return "LE";  
  case GreaterEqual: return "GE";
  }  
  return "Inalid";
}                


// Constructor
ElemConstraint::ElemConstraint(void) {
  Set(0,LessThan,0);
}
  
// Constructor
ElemConstraint::ElemConstraint(Idx clockindex, Operator op, const Time::Type timeconst) {
  Set(clockindex,op,timeconst);
}
  

// Set(clockindex, op, timeconst)
void  ElemConstraint::Set(Idx clockindex, Operator op, const Time::Type timeconst) {
  mClockIndex = clockindex;
  mCompOperator = op;
  mTimeConstant = timeconst;
}

// Clock(clockindex)
Idx ElemConstraint::Clock(Idx clockindex) {
  mClockIndex=clockindex;
  return mClockIndex;
}

// Clock()
Idx ElemConstraint::Clock(void) const {
  return mClockIndex;
}

// CompOperator(newOp)
void ElemConstraint::CompOperator(Operator newOp) {
  mCompOperator = newOp;
}


// CompOperator()
ElemConstraint::Operator ElemConstraint::CompOperator(void) const {
  return mCompOperator;
}


// TimeConstant(newTimeConst)
void ElemConstraint::TimeConstant (Time::Type newTimeConst) {
  mTimeConstant = newTimeConst;
}

// TimeConstant()
Time::Type ElemConstraint::TimeConstant(void) const {
  return mTimeConstant;
}

// Str() const
std::string ElemConstraint::Str(void) const {
  std::stringstream resstream;
  resstream << "(" << mClockIndex << " "
	    << OperatorName(mCompOperator) << " " << mTimeConstant << ")";
  std::string result = resstream.str();
  return result;
}

// operator==
bool ElemConstraint::operator== (const ElemConstraint & otherClockConstraint) const {
  return ( mClockIndex == otherClockConstraint.mClockIndex
	   && mCompOperator == otherClockConstraint.mCompOperator
	   && mTimeConstant == otherClockConstraint.mTimeConstant );
}

// operator!=
bool ElemConstraint::operator!= (const ElemConstraint & otherClockConstraint) const {
  return !(operator==(otherClockConstraint));
}

// operator <
bool ElemConstraint::operator < (const ElemConstraint& otherElemConstraint) const {
  if (mClockIndex < otherElemConstraint.mClockIndex) return true;
  if (mClockIndex > otherElemConstraint.mClockIndex) return false;
  if (mCompOperator < otherElemConstraint.mCompOperator) return true;
  if (mCompOperator > otherElemConstraint.mCompOperator) return false;
  if (mTimeConstant < otherElemConstraint.mTimeConstant) return true;
  return false;
}


/********************************************************************

 Implementation of TimeConstraint

********************************************************************/



// empty constructor
TimeConstraint::TimeConstraint(void) {
  FD_DC("TimeConstraint(" << this << ")::TimeConstraint()");
  mpClockSymbolTable=ClockSet::GlobalClockSymbolTablep();
  mName="TimeConstraint";
}

// read file constructor
TimeConstraint::TimeConstraint(const std::string& rFilename, const std::string& rLabel) {
  FD_DC("TimeConstraint(" << this << ")::TimeConstraint(" << rFilename << ")");
  mpClockSymbolTable=ClockSet::GlobalClockSymbolTablep();
  Read(rFilename, rLabel);
}

// constructor
TimeConstraint::TimeConstraint(const TimeConstraint& rOtherTimeConstraint) {
  FD_DC("TimeConstraint(" << this << ")::TimeConstraint(other)");
  mName=rOtherTimeConstraint.mName;
  mpClockSymbolTable= rOtherTimeConstraint.mpClockSymbolTable;
  mClockConstraints = rOtherTimeConstraint.ClockConstraints();
}


// Destructor
TimeConstraint::~TimeConstraint(void) {
}

// ClockSymbolTablep()
SymbolTable* TimeConstraint::ClockSymbolTablep(void) const {
  return mpClockSymbolTable;
}

// ClockSymbolTablep(pSymTab)
void TimeConstraint::ClockSymbolTablep(SymbolTable* pSymTab) {
  if(mClockConstraints.empty()) {
    mpClockSymbolTable=pSymTab;
  } else {
    // TODO: implement reindex
    FD_ERR("TimeConstraint::SymboltTable(pSymTab): "
	   << "set SymbolTable not implemented!!");
    abort();
  }   
}

// Empty()
bool TimeConstraint::Empty(void) const {
  return mClockConstraints.empty();
}

// Size of set of ElemConstraints
Idx TimeConstraint::Size(void) const {
  return (Idx)mClockConstraints.size();
}

// InsClock()
Idx TimeConstraint::InsClock(const std::string& rClockName) const {
  return mpClockSymbolTable->InsEntry(rClockName);
}

// ClockName(clockindex)
std::string TimeConstraint::ClockName(Idx clockindex) const {
  return mpClockSymbolTable->Symbol(clockindex);
}

// ClockIndex(clockname)
Idx TimeConstraint::ClockIndex(const std::string& rClockName) const {
  return mpClockSymbolTable->Index(rClockName);
}


// EStr(ElemConstraint) const
std::string TimeConstraint::EStr(const ElemConstraint& rElemConstraint) const {
  std::stringstream resstream;
  resstream << "(" << ClockName(rElemConstraint.Clock()) << "[" <<  rElemConstraint.Clock() 
	    << "] " << ElemConstraint::OperatorName(rElemConstraint.CompOperator()) << " " 
	    << rElemConstraint.TimeConstant() << ")";
  std::string result = resstream.str();
  return result;
}

// Insert(rNewConstr)
TimeConstraint::Iterator TimeConstraint::Insert(const ElemConstraint& rNewConstr) {
  FD_DC("TimeConstraint(" << this << ")::Insert(" << rNewConstr.Str() << ")");
  if(rNewConstr.Clock()<=0 || ClockName(rNewConstr.Clock())=="") {
    std::stringstream errstr;
    errstr << "Invalid ElemConstraint: \"" << rNewConstr.Str();
    throw Exception("TimeConstraint::Insert", errstr.str(), 55);
  }
  return mClockConstraints.insert(rNewConstr).first;
}

// Insert(clock, op, timeconst)
TimeConstraint::Iterator TimeConstraint::Insert(Idx clockindex, Operator op, const Time::Type timeconst) {
  FD_DC("TimeConstraint(" << this << ")::Insert(" 
	<< clockindex << " " << ElemConstraint::OperatorName(op) << " " << timeconst << ")");
  ElemConstraint newconstr(clockindex,op,timeconst);
  return Insert(newconstr);
}


// Insert(clock, op, timeconst)
TimeConstraint::Iterator TimeConstraint::Insert(
						const std::string clockname, 
						Operator op, 
						const Time::Type timeconst) 
{
  FD_DC("TimeConstraint(" << this << ")::Insert(\"" 
	<< clockname << "\" " << ElemConstraint::OperatorName(op) << " " << timeconst << ")");
  Idx clockindex = InsClock(clockname);
  return Insert(clockindex,op,timeconst);
}

// Insert(rNewConstraints)
void TimeConstraint::Insert(const std::list<ElemConstraint>& rNewConstraints) {
  FD_DC("TimeConstraint(" << this << ")::Insert(const std::list<ElemConstraint>&)");
  // HELPERS
  std::list<ElemConstraint>::const_iterator it;
  // ALGORITHM
  for(it = rNewConstraints.begin(); it != rNewConstraints.end(); it++)
    Insert(*it);
}


// Insert(rOtherTimeConstraint)
void TimeConstraint::Insert(const TimeConstraint& rOtherTimeConstraint) {
  FD_DC("TimeConstraint(" << this << ")::Insert(" << rOtherTimeConstraint.ToString() << ")");
  // HELPERS
  Iterator it;    
  // ALGORITHM
  if(mpClockSymbolTable != rOtherTimeConstraint.mpClockSymbolTable) {
    FD_ERR("TimeConstraint::Insert "
	   << "SymbolTable mismatch aka not implemented!!");
    abort();
  }
  for(it = rOtherTimeConstraint.Begin(); it != rOtherTimeConstraint.End(); it++) {
    Insert(*it);
  }
}

// ClockConstraints()
std::set<ElemConstraint> TimeConstraint::ClockConstraints(void) const {
  return mClockConstraints;
}


// EraseByClock(clock)
bool TimeConstraint::EraseByClock(Idx clock) {
  FD_DC("TimeConstraint(" << this << ")::EraseByClock(" << clock << ") const");
  // HELPERS
  iterator lit,uit;
    
  // ALGORITHM
  lit= mClockConstraints.lower_bound(ElemConstraint(clock,ElemConstraint::GreaterEqual,0));
  uit= mClockConstraints.lower_bound(ElemConstraint(clock+1,ElemConstraint::GreaterEqual,0));

  if(lit==mClockConstraints.end()) 
    return false;
 
  mClockConstraints.erase(lit,uit);
  return true;
}


// Erase(it)
TimeConstraint::Iterator TimeConstraint::Erase(Iterator it) {
  FD_DC("TimeConstraint(" << this << ")::Erase(" << it->Str() << ") const");
  if(it==End()) return it;
  iterator del= it; //cit
  it++;
  mClockConstraints.erase(del);
  return it;
}


// Erase(rElemConstr)
bool TimeConstraint::Erase(const ElemConstraint& rElemConstr) {
  FD_DC("TimeConstraint(" << this << ")::Erase(" << rElemConstr.Str() << ") const");
  // HELPERS
  iterator it;
  // ALGORITHM
  it = mClockConstraints.find(rElemConstr);
  if(it == End()) return false;
  mClockConstraints.erase(it);
  return true;
}

// Erase(clock, op, timeconst)
bool TimeConstraint::Erase(Idx clockindex, Operator op, const Time::Type timeconst) {
  FD_DC("TimeConstraint(" << this << ")::Erase(" 
	<< clockindex << " " << ElemConstraint::OperatorName(op) << " " << timeconst << ")");
  ElemConstraint newconstr(clockindex,op,timeconst);
  return Erase(newconstr);
}


// Erase(clock, op, timeconst)
bool TimeConstraint::Erase(const std::string& clockname,  Operator op, const Time::Type timeconst) 
{
  FD_DC("TimeConstraint(" << this << ")::Erase(\"" 
	<< clockname << "\" " << ElemConstraint::OperatorName(op) << " " << timeconst << ")");
  Idx clockindex = ClockIndex(clockname);
  return Erase(clockindex,op,timeconst);
}

// Exists(rElemConstr)
bool TimeConstraint::Exists(const ElemConstraint& rElemConstr) const {
  FD_DC("TimeConstraint(" << this << ")::ExistsElConstr(" << rElemConstr.Str() << ") const");
  // HELPERS
  Iterator it;
  // ALGORITHM
  it = mClockConstraints.find(rElemConstr); 
  return (it != End()) ;
}


// Deletes all ElemConstraints
void TimeConstraint::Clear(void) {
  FD_DC("TimeConstraint(" << this << ")::Clear() const");
  mClockConstraints.clear();
}



// Iterator Begin() const
TimeConstraint::Iterator TimeConstraint::Begin(void) const {
  return mClockConstraints.begin();
}


// iterator End() const
TimeConstraint::Iterator TimeConstraint::End(void) const {
  return mClockConstraints.end();
}

// reverse iterator RBegin()
TimeConstraint::RIterator TimeConstraint::RBegin(void) const {
  return mClockConstraints.rbegin();
}

// reverse iterator REnd() const
TimeConstraint::RIterator TimeConstraint::REnd(void) const {
  return mClockConstraints.rend();
}

// iterator Begin(clock) const
TimeConstraint::Iterator TimeConstraint::Begin(Idx clock) const {
  return mClockConstraints.lower_bound(ElemConstraint(clock,ElemConstraint::GreaterEqual,0));
}

// iterator End(clock) const
TimeConstraint::Iterator TimeConstraint::End(Idx clock) const {
  return mClockConstraints.lower_bound(ElemConstraint(clock+1,ElemConstraint::GreaterEqual,0));
}

// returns ClockSet filled with clocks used by ElemConstraints
ClockSet TimeConstraint::ActiveClocks(void) const {
  FD_DC("TimeConstraint(" << this << ")::ActiveClocks() const");
  //Helpers
  ClockSet result;
  result.SymbolTablep(mpClockSymbolTable);
  Iterator it;
  // Algorithm
  for(it = Begin(); it != End(); it++) 
    result.Insert(it->Clock());
  return result;
}

// valid timeinterval for given clock 
TimeInterval TimeConstraint::Interval(const std::string& clockname) const{
  Idx clockindex = ClockIndex(clockname);
  return Interval(clockindex);
}

// valid timeinterval for given clock 
TimeInterval TimeConstraint::Interval(Idx clockindex) const{
  FD_DC("TimeConstraint(" << this << ")::Interval(" << clockindex <<") const");
  TimeInterval res;
  TimeInterval tint; 
  Iterator it;
  for(it = Begin(clockindex); it != End(clockindex); it++) {
    FD_DC("TimeConstraint(" << this << ")::Interval: elemconstraint: " << it->Str());
    tint.SetFull();
    if(it->CompOperator() == ElemConstraint::LessThan) {
      tint.UB(it->TimeConstant());
      tint.UBincl(false);
    }
    if(it->CompOperator() == ElemConstraint::LessEqual) {
      tint.UB(it->TimeConstant());
      tint.UBincl(true);
    }
    if(it->CompOperator() == ElemConstraint::GreaterThan) {
      tint.LB(it->TimeConstant());
      tint.LBincl(false);
    }
    if(it->CompOperator() == ElemConstraint::GreaterEqual) {
      tint.LB(it->TimeConstant());
      tint.LBincl(true);
    }
    FD_DC("TimeConstraint(" << this << ")::Interval: interval: " << tint.Str());
    res.Intersect(tint);
  }
  return res;
}
 

// set valid timeinterval for given clock 
void TimeConstraint::Interval(const std::string& clockname, const TimeInterval& rInterval) {
  Idx clockindex = InsClock(clockname);
  Interval(clockindex,rInterval);
}

// set valid timeinterval for given clock 
void TimeConstraint::Interval(Idx clockindex, const TimeInterval& rInterval) {
  FD_DC("TimeConstraint(" << this << ")::Interval(" << clockindex <<", " << rInterval.Str() << ") ");
  EraseByClock(clockindex);
  if(rInterval.LBinf()==false) {
    ElemConstraint newconstraint;
    newconstraint.Clock(clockindex);
    if(rInterval.LBincl())
      newconstraint.CompOperator(ElemConstraint::GreaterEqual);
    else
      newconstraint.CompOperator(ElemConstraint::GreaterThan);
    newconstraint.TimeConstant(rInterval.LB());
    Insert(newconstraint);
  }
  if(rInterval.UBinf()==false) {
    ElemConstraint newconstraint;
    newconstraint.Clock(clockindex);
    if(rInterval.UBincl())
      newconstraint.CompOperator(ElemConstraint::LessEqual);
    else
      newconstraint.CompOperator(ElemConstraint::LessThan);
    newconstraint.TimeConstant(rInterval.UB());
    Insert(newconstraint);
  }
}


// Minimize()
void TimeConstraint::Minimize(void) {
  ClockSet aclocks=ActiveClocks();
  ClockSet::Iterator cit;
  for(cit=aclocks.Begin(); cit != aclocks.End(); cit++) {
    TimeInterval tint=Interval(*cit);
    Interval(*cit, tint);
  }
}

// operator==
bool TimeConstraint::operator== (const TimeConstraint & rOther) const {
  ClockSet aclocks=ActiveClocks();
  aclocks.InsertSet(rOther.ActiveClocks());
  ClockSet::Iterator cit;
  for(cit=aclocks.Begin(); cit != aclocks.End(); cit++) {
    TimeInterval tint=Interval(*cit);
    if(rOther.Interval(*cit)!=tint) return false;
  }
  return true;  
}

// operator!=
bool TimeConstraint::operator!= (const TimeConstraint & rOther) const {
  return !(operator==(rOther));
}


// Write()
void TimeConstraint::Write(void) const {
  TokenWriter tw(TokenWriter::Stdout);
  Write(tw);
}

// Write(rFilename, rLabel, openmode)
void TimeConstraint::Write(const std::string& rFilename, const std::string& rLabel,
			   std::ios::openmode openmode) const {
  try {
    TokenWriter tw(rFilename, openmode);
    Write(tw, rLabel);
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << rFilename << "\"";
    throw Exception("TimeConstraint::Write", errstr.str(), 2);
  }
}

// Write(tw)
void TimeConstraint::Write(TokenWriter& tw) const {
  Write(tw, Name());
}


// Write(tw, rLabel)
void TimeConstraint::Write(TokenWriter& tw, const std::string& rLabel) const {
  Token token;
  Iterator it;
  int oldcolumns = tw.Columns();
  tw.Columns(3);
  tw.WriteBegin(rLabel);
  for (it = Begin(); it != End(); ++it) {
    // 1. clock
    if(ClockName(it->Clock()) != "") {
      token.SetString(ClockName(it->Clock()));
      tw << token;
    } else {
      token.SetInteger(it->Clock());
      tw << token;
    }
    // 2. operator
    token.SetString(ElemConstraint::OperatorName(it->CompOperator()));
    tw << token;
    // 3. timeconst
    token.SetFloat((Float) it->TimeConstant());
    tw << token;      
  }
  tw.WriteEnd(rLabel);
  tw.Columns(oldcolumns);
}


// ToString()
std::string TimeConstraint::ToString(void) const {
  TokenWriter tw(TokenWriter::String);
  Write(tw);
  return tw.Str();
}


// DWrite()
void TimeConstraint::DWrite(void) const {
  TokenWriter tw(TokenWriter::Stdout);
  DWrite(tw);
}

// DWrite(tw)
void TimeConstraint::DWrite(TokenWriter& tw) const {
  Token token;
  Iterator it;
  tw.WriteBegin(Name());
  for (it = Begin(); it != End(); ++it) {
    tw << EStr(*it);
  }
  tw.WriteEnd(Name());
}

// Read(rFilename, rLabel)
void TimeConstraint::Read(const std::string& rFilename, const std::string& rLabel) {
  TokenReader tr(rFilename);
  Read(tr,rLabel);
}

// Read(rTr, rLabel)
void TimeConstraint::Read(TokenReader& rTr, const std::string& rLabel) {
  Clear();
  Name(rLabel);
  rTr.ReadBegin(rLabel);
 
  std::string clockname;
  Time::Type  timeconst;
  ElemConstraint::Operator compop;
  Token token;
  while(rTr.Peek(token)) {
    // 0. check for end
    if (token.Type() == Token::End) {
      break;
    }
    // 1. read clock
    rTr >> token;
    if (token.Type() != Token::String) {
      std::stringstream errstr;
      errstr << "Invalid clock" << rTr.FileLine();
      throw Exception("TimeConstraint::Read", errstr.str(), 56);
    }    
    clockname=token.StringValue();
    // 2. read operator
    rTr >> token;
    if (token.Type() != Token::String) {
      std::stringstream errstr;
      errstr << "Invalid operator" << rTr.FileLine();
      throw Exception("TimeConstraint::Read", errstr.str(), 56);
    }    
    if(token.StringValue() == "LE") {
      compop = ElemConstraint::LessEqual;
    } else if(token.StringValue() == "GE") {
      compop = ElemConstraint::GreaterEqual;
    } else if(token.StringValue() == "LT") {
      compop = ElemConstraint::LessThan;
    } else if(token.StringValue() == "GT") {
      compop = ElemConstraint::GreaterThan; 
    } else {
      std::stringstream errstr;
      errstr << "Invalid operator value " << rTr.FileLine();
      throw Exception("TimedTransSet::ReadTimeConstraint", errstr.str(), 56);
    }
    // 3. read timeconst
    rTr >> token;
    if (!token.IsFloat()) {
      std::stringstream errstr;
      errstr << "Invalid timeconstant" << rTr.FileLine();
      throw Exception("TimeConstraint::Read", errstr.str(), 56);
    }    
    timeconst=(Time::Type) token.FloatValue();
    // 4. set constraint
    Insert(clockname,compop,timeconst);
  } // while not end
  rTr.ReadEnd(rLabel);
}


// End Implementation of TimeConstraint



} // namespace faudes

/* tp_timeinterval.cpp -- model of interval */

/* Timeplugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2007  Ruediger Berndt
   Copyright (C) 2007  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/

#include "tp_timeinterval.h"

using namespace faudes;



/* canonical */
void TimeInterval::Canonical(void) {
  // make left closed 
  if((!mbLBincl)&&(!LBinf())) {
    LB(mstLB+Time::Step());
    LBincl(true);
  }
  // make right open
  if(UBincl()) {
    UB(mstLB+Time::Step());
    UBincl(false);  
  }
}

/* bool Empty const() */
bool TimeInterval::Empty() const {
  if(mstLB<mstUB) 
    return false;
  if(mstLB==mstUB) 
    if((mbLBincl==true) && (mbUBincl==true)) 
      return false;	
  return true;
}

/* bool In(time) */
bool TimeInterval::In(Time::Type time) const {
  if(time<mstLB) return false;
  if(time> mstUB) return false;
  if(time == mstLB) if(  ! mbLBincl) return false;
  if(time == mstUB) if(! mbUBincl) return false;
  return true;
}

/* PositiveLeftShift */
void TimeInterval::PositiveLeftShift(Time::Type time) {
  if(!UBinf()) UB(mstUB-time);
  if(!LBinf()) LB(mstLB-time);
  if( LBinf() || mstLB <= 0){
    LB(0);
    mbLBincl=true;
  }
}

/* std::string Str() const */ 
std::string TimeInterval::Str(void) const {

  std::string res;
  std::stringstream resstream;

  if(Empty()) {
    resstream << "[empty]";
    res=resstream.str();
    return(res);
  }

  if(mbLBincl) resstream << "[";
  else resstream << "(";
  if(LBinf()) resstream << "-inf";
  else resstream << mstLB;
  resstream << ", ";
  if(UBinf()) resstream << "inf";
  else resstream << mstUB;
  if(mbUBincl) resstream << "]";
  else resstream << ")";
  res=resstream.str();
  return(res);

}

/* void Intersect(const TimeInterval& rOtherInterval) */
void TimeInterval::Intersect(const TimeInterval& rOtherInterval) {

  FD_DC("TimeInterval::Intersect(otherinterval): "  << Str()<<" & " <<
	rOtherInterval.Str() );

  // other upper bound matches
  if(UB()==rOtherInterval.UB()) {
    if(!rOtherInterval.UBincl()) UBincl(false);
  } 
  // other upper bound wins
  if(UB() > rOtherInterval.UB()) {
    UB(rOtherInterval.UB());
    UBincl(rOtherInterval.UBincl());
  }
  // other lower bound matches
  if(LB()==rOtherInterval.LB()) {
    if(!rOtherInterval.LBincl()) LBincl(false);
  }
  // other lower bound wins
  if(LB() < rOtherInterval.LB()) {
    LB(rOtherInterval.LB());
    LBincl(rOtherInterval.LBincl());
  }
	
  FD_DC("TimeInterval::Intersect( ... ), ret: "  <<  Str());
}


/* TimeInterval Intersect(const TimeInterval& rInterval1, const TimeInterval& rInterval2) */
TimeInterval TimeInterval::Intersect(const TimeInterval& rInterval1, const TimeInterval& rInterval2) {
  FD_DC("TimeInterval::Intersect("  << rInterval1.Str()<<", " <<
	rInterval2.Str() << ")");
  TimeInterval res=rInterval1;
  res.Intersect(rInterval2);
  FD_DC("TimeInterval::Intersect( ... ), ret: "  <<  res.Str());
  return(res);
}


/* void  Merge(const TimeInterval& rOtherInterval) */
void TimeInterval::Merge(const TimeInterval& rOtherInterval) {
  // other upper bound matches
  if(UB()==rOtherInterval.UB()) {
    if(rOtherInterval.UBincl()) UBincl(true);
  } 
  // other upper bound wins
  if(UB() < rOtherInterval.UB()) {
    UB(rOtherInterval.UB());
    UBincl(rOtherInterval.UBincl());
  } 
  // other lower bound matches
  if(LB()==rOtherInterval.LB()) {
    if(rOtherInterval.LBincl() ) LBincl(true);
  }
  // other lower bound wins
  if(LB()<rOtherInterval.LB()) {
    LB(rOtherInterval.LB());
    LBincl(rOtherInterval.LBincl());
  }	
}

/* TimeInterval Merge(const TimeInterval& rInterval1, const TimeInterval& rInterval2) */
TimeInterval TimeInterval::Merge(const TimeInterval& rInterval1, const TimeInterval& rInterval2) {
  TimeInterval res=rInterval1;
  res.Merge(rInterval2);
  return res;
}

/** @file sp_densityfnct.cpp  Discrete density function approximation */


/* 
   Copyright (C) 2008  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt
*/

#include "sp_densityfnct.h"
#include "sp_executor.h"

#include <cmath>

namespace faudes {

/*
***********************************************************
***********************************************************
***********************************************************

implementation of density function approximattion

***********************************************************
***********************************************************
***********************************************************
*/

// construct
DiscreteDensityFunction::DiscreteDensityFunction(void) : 
  mCount(0), mMaxValue(0), mMinValue(0), mMaxTime(0), mMinTime(0), mSum(0), mSquareSum(0), mAverage(0), mVariance(0), mQuantile05(0), mQuantile95(0) 
{ 
  mEntryZero.timeint.SetPositive();
  mEntryZero.value=0;
}


//Write(rTw);
void DiscreteDensityFunction::Write(TokenWriter& rTw) const {
  Token token;
  rTw.WriteBegin("Density");
  rTw << Name();
  int oldcolumns = rTw.Columns();
  rTw.Columns(3);
  rTw << "\n";
  for(CIterator mit=Begin(); mit!=End(); mit++) {
    if(mit->second.value==0) continue;
    rTw.WriteFloat(mit->second.timeint.LB());
    rTw.WriteFloat(mit->second.timeint.UB());
    rTw.WriteFloat(mit->second.value);
  }
  rTw.Columns(oldcolumns);
  rTw.WriteEnd("Density");
}

// ToString()
std::string DiscreteDensityFunction::ToString(void) const {
  TokenWriter tw(TokenWriter::String);
  Write(tw);
  return tw.Str();
}

// Write()
void DiscreteDensityFunction::Write(void) const {
  TokenWriter tw(TokenWriter::Stdout);
  Write(tw);
}    

//Read(rTr)
void DiscreteDensityFunction::Read(TokenReader& rTr) {
  Clear();
  rTr.ReadBegin("Density");
  rTr.ReadEnd("Density");
}


// reset all
void DiscreteDensityFunction::Clear(void) {
 FD_DX("DiscreteDensityFunction::Clear()");
 mValueMap.clear();
 mMinValue=0;
 mMaxValue=0;
 mMinTime=0;
 mMaxTime=0;
 mSum=0;
 mSquareSum=0;
 mAverage=0;
 mVariance=0;
 mQuantile05=0;
 mQuantile95=0;
 mCount=0;
}

// const fake version
void DiscreteDensityFunction::Compile(void) const {
  DiscreteDensityFunction* fakeconst = const_cast<DiscreteDensityFunction*>(this);
  fakeconst->CompileNonConst();
}

// compute (assume right open intervals) 
void DiscreteDensityFunction::CompileNonConst(void) {
  FD_DX("DiskreteDensityFunction::Compile(" << mName << ")");
  if(mValueMap.size()<1) return; // error
  // normalize
  double integral=0;
  for(Iterator mit=Begin() ;mit!=End(); mit++) {
    integral+= (mit->second.timeint.UB()-mit->second.timeint.LB())* mit->second.value;
  };
  if(integral==0) return; // error
  double norm=1/integral;
  for(Iterator mit=Begin() ;mit!=End(); mit++) {
    mit->second.value*= norm;
  };
  // find min, max, and sums
  mMinValue=-1;
  mMaxValue=-1;
  mMinTime=-1;
  mMaxTime=-1;
  mSum=0;
  mSquareSum=0;
  for(Iterator mit=Begin() ;mit!=End(); mit++) {
    if(mMinValue<0 || (mit->second.value < mMinValue)) mMinValue=mit->second.value;
    if(mMaxValue<0 || (mit->second.value > mMaxValue)) mMaxValue=mit->second.value;
    double time = (mit->second.timeint.UB() -1  + mit->second.timeint.LB()) / 2.0; // discrete time
    double prob = (mit->second.timeint.UB() - mit->second.timeint.LB()) * mit->second.value; 
    mSum=mSum + time * prob;
    mSquareSum=mSquareSum + time*time*prob;
  }
  // min and max
  if(mMinValue<0) mMinValue=0;
  if(mMaxValue<0) mMaxValue=0;
  mMinTime=Begin()->second.timeint.LB();
  mMaxTime=(--End())->second.timeint.UB() -1; // discrete time;
  // avaerage and variance
  mAverage= mSum;
  mVariance=sqrt(fabs(mSquareSum - mSum*mSum)); // fix!!
  // quantile 05  (todo: inspect/test/fix)
  mQuantile05=mMinTime;
  integral=0;
  double len=0, area=0;
  Iterator mit;
  for(mit=Begin() ;mit!=End(); mit++) {
    len = mit->second.timeint.UB()-mit->second.timeint.LB(); 
    area=len* mit->second.value;
    if(integral + area >= 0.05) break;
    integral = integral + area;
  }
  if((mit!=End()) && (integral + area >= 0.05)) {
    if(mit->second.value>0.01) 
      mQuantile05 = mit->second.timeint.LB() + (0.05-integral)/mit->second.value;
    else 
      mQuantile05 = mit->second.timeint.LB() + len/2;
  }
  if(mQuantile05<=mMinTime) mQuantile05=mMinTime;
  // quantile 95  (todo: inspect/test/fix)
  mQuantile95=mMaxTime;
  integral=0;
  len=0, area=0;
  for(mit=End();mit!=Begin(); ) {
    mit--;
    len = mit->second.timeint.UB()-mit->second.timeint.LB(); 
    area=len* mit->second.value;
    if(integral + area >= 0.05) break;
    integral = integral + area;
  }
  if(integral +area >= 0.05) {
    if(mit->second.value>0.01) 
      mQuantile95 = mit->second.timeint.UB() - (0.05-integral)/mit->second.value;
    else 
      mQuantile95 = mit->second.timeint.UB() - len/2;
  }
  if(mQuantile95>=mMaxTime) mQuantile95=mMaxTime;
}


// get entry, perhaps fake
const DiscreteDensityFunction::Entry& DiscreteDensityFunction::EntryAt(Time::Type time) const {
  CIterator mit= At(time);
  if(mit!=End()) return mit->second;
  return mEntryZero;
}


// get value
double DiscreteDensityFunction::Value(Time::Type time) const { return EntryAt(time).value; }
const TimeInterval& DiscreteDensityFunction::TimeInt(Time::Type time) const { return EntryAt(time).timeint; }

// pretty string (should resample)
std::string DiscreteDensityFunction::Str(void) const {
  std::stringstream ss;
  ss << "% Discrete Density \"" << mName <<"\"" <<  " characteristics:" << std::endl;
  ss << "% time " << MinTime() << "/" << MaxTime() << std::endl;
  ss << "% value " << MinValue() << "/" << MaxValue() << std::endl;
  ss << "% quant " << Quantile05() << "/" << Quantile95() << std::endl;
  ss << "% stat  " << Average() << "/" << Variance() << std::endl;
  for(CIterator mit=Begin();  mit!=End(); mit++) {
    if(mit->second.value==0) continue;
    ss << "% " << ExpandString(mit->second.timeint.Str(),FD_NAMELEN) << ": " 
       << ExpandString(ToStringFloat(mit->second.value), FD_NAMELEN)  << ": ";
    double pc=mit->second.value;
    double sc= MaxValue()/50.0;
    for(; pc>0; pc-=sc) ss << "#";
    ss << " " << std::endl;
  }
  std::string res= ss.str();
  return res;
}


/*
***********************************************************
***********************************************************
***********************************************************

implementation of sampled density function approximattion

***********************************************************
***********************************************************
***********************************************************
*/



// construct
SampledDensityFunction::SampledDensityFunction(void) : DiscreteDensityFunction(), 
  mDim(100), mCountSum(0), mCountSquareSum(0) 
{ 
}



// clear all
void SampledDensityFunction::Clear(void) {
  FD_DX("SampledDensityFunction::Clear()");
  DiscreteDensityFunction::Clear(); 
  mCount=0; 
  mCountSum=0; 
  mCountSquareSum=0;
  mCountMap.clear();
}

// add one sample
void SampledDensityFunction::Sample(Time::Type duration) {
  // report
  FD_DX("SampledDensityFunction::Sample(" << Name() << "): duration " << duration);
  FD_DX(SStr());
  // bail out on negative duration (error)
  if(duration<0) return;
  // record
  mCount++;
  mCountSum+=duration;
  mCountSquareSum+=duration*duration;
  // cases ...
  CountIterator mit = mCountMap.lower_bound(duration);
  // ... do we have a range? just count
  if(mit!=mCountMap.end()) {
    if(mit->second.timeint.In(duration)) {
      FD_DX("SampledDensityFunction::Sample(): range found, count");
      mit->second.count+=1;
      return;
    }
  }
  //  insert tailord support
  FD_DX("SampledDensityFunction::Sample(): insert tailored support");
  CountEntry tent;
  tent.timeint.UB(duration);
  tent.timeint.LB(duration);
  tent.timeint.UBincl(true);
  tent.timeint.LBincl(true);
  tent.count=1;
  mCountMap[duration]=tent;
  // dim ok? done 
  if(mCountMap.size()<=mDim)
    return;
  // merge intervals
  FD_DX("SampledDensityFunction::Sample(): merge");
  CountIterator mit1=mCountMap.begin();
  CountIterator mit2=mCountMap.begin();
  CountIterator mmit;
  double minarea = -1;
  for(mit2++; mit2!=mCountMap.end(); mit1++, mit2++) {
    Time::Type dur  =  mit2->second.timeint.UB() - mit1->second.timeint.LB();
    double area =  dur * (mit2->second.count + mit1->second.count);
    if(area < minarea || minarea <0) { minarea=area; mmit=mit1;}
  }
  if(mit2==mCountMap.end()) return; // error!
  // merge intervals
  mit2=mmit;
  mit2++;
  mmit->second.timeint.Merge(mit2->second.timeint);
  mmit->second.count += mit2->second.count;
  mCountMap.erase(mit2);
}


// compute (incl normalize)
void SampledDensityFunction::CompileNonConst(void) {
  FD_DX("SampledDensityFunction::Compile(" << mName << ")");
  FD_DX(SStr());
  if(mCountMap.size()<1) return; // error

  // convert count
  double count=mCount;

  // clear main data
  mValueMap.clear();

  // copy
  if(count<=0) return; // error
  for(CountIterator mit=mCountMap.begin() ;mit!=mCountMap.end(); mit++) {
    Entry tent;
    tent.timeint = mit->second.timeint;
    tent.value = ((double) mit->second.count); 
    mValueMap[tent.timeint.LB()]=tent;
  } 

  // fix bounds: insert (assume all closed)
  if(mCountMap.size()<mDim/2) 
  for(Iterator mit=Begin() ;mit!=End(); mit++) {
    Iterator nit=mit;
    nit++;
    if(nit!=End()) 
    if(mit->second.timeint.UB() + 1 != nit->second.timeint.LB()) { // todo: float time
      Entry tent;
      tent.timeint.LB(mit->second.timeint.UB() + 1);
      tent.timeint.UB(nit->second.timeint.LB() - 1);
      tent.timeint.LBincl(false);
      tent.timeint.UBincl(false);
      tent.value = 0;
      mValueMap[tent.timeint.LB()]=tent;
    }    
  }

  // fix bounds: extend over gaps (assume all closed, turn to right open)
  for(Iterator mit=Begin() ;mit!=End(); mit++) {
    Iterator nit=mit;
    nit++;
    mit->second.timeint.UBincl(false);
    mit->second.timeint.LBincl(true);
    mit->second.timeint.UB(mit->second.timeint.UB()+1);
    if(nit!=End()) 
    if(mit->second.timeint.UB() != nit->second.timeint.LB()) {
      double middle = 0.5*(mit->second.timeint.UB() + nit->second.timeint.LB());
      Time::Type dmiddle=((Time::Type) middle);
      if(dmiddle <= mit->second.timeint.LB()) dmiddle= nit->second.timeint.LB();
      mit->second.timeint.UB(dmiddle);
      nit->second.timeint.LB(dmiddle);
    }
    if(nit==End()) 
    if(mit->second.timeint.UB() <= mit->second.timeint.LB())
       mit->second.timeint.UB(mit->second.timeint.LB()+1);

    mit->second.value=mit->second.value / 
       (mit->second.timeint.UB() - mit->second.timeint.LB());
  }

  // compile base
  DiscreteDensityFunction::CompileNonConst();

  // fix characteristics
  mAverage=mCountSum/count;
  mVariance=sqrt(1.0/ count * fabs(
         mCountSquareSum - (1.0/count*((double)mCountSum)*((double)mCountSum))) );

  FD_DX(DiscreteDensityFunction::Str());
}



// pretty string
std::string SampledDensityFunction::SStr(void) const {
  std::stringstream ss;
  ss << "Sampled Density \"" << mName <<"\"";
  for(CCountIterator mit=mCountMap.begin();  mit!=mCountMap.end(); mit++) {
    ss << "(-- " << mit->second.timeint.Str() << " " << mit->second.count << " --)";
  }
  return ss.str();
}


} // namespace faudes


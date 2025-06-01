/*
FAU Dscrete Event Systems Library (libfaudes)

Copyright (C) 2025  Thomas Moor
Exclusive copyright is granted to Klaus Schmidt

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
To use a wide range of third party omega-automata tools, we implement
serialisation from and to the HOA file format. To read HOA files, we
use the cpphoafparser library, authored/copyrighted by

Joachim Klein <klein@tcs.inf.tu-dresden.de>
David Mueller <david.mueller@tcs.inf.tu-dresden.de>

The original sources have been distributed under LGPL v2.1 conditions
and we include them with libFAUDES.
*/


#ifndef FAUDESXMLOUT_H
#define FAUDESXMLOUT_H

#include <iostream>

#include "cpphoafparser/consumer/hoa_consumer.hh"
#include "cpphoafparser/parser/hoa_parser_helper.hh"
#include "libfaudes.h"

namespace cpphoafparser {

/**
 * A HOAConsumer implementation that provides output in libFAUDES XML format.
 * We construct a faudes type object, record relevant comsumabilities and use
 * libFAUDES XML seriliasation for output. Technically, we used cpphoafparser's
 * HOAComsumerPrint at as template.
 */

class HOAConsumerFaudes : public HOAConsumer {
public:

  /** Constructor, providing a reference to the output stream */
  HOAConsumerFaudes(faudes::TokenWriter& tw) : rTw(tw) {}

  virtual bool parserResolvesAliases() override {
    return false;
  }

  virtual void notifyHeaderStart(const std::string& version) override {
    // should check version?
    // std::cerr << "HOA: " << version << std::endl;
    mRAut.Clear();
  }

  virtual void setNumberOfStates(unsigned int numberOfStates) override {
    //std::cerr << "States: " << numberOfStates << std::endl;
    for(int i=0;i<numberOfStates;++i) mRAut.InsState(i+1);
  }

  virtual void addStartStates(const int_list& stateConjunction) override {
    /*
    std::cerr << "% Start: ";
    bool first = true;
    for (unsigned int state : stateConjunction) {
      if (!first) std::cerr << " & ";
      first=false;
      std::cerr << state;
    }
    */
    std::cerr << std::endl;
    for(unsigned int state : stateConjunction) 
       mRAut.InsInitState(state+1);
  }

  virtual void addAlias(const std::string& name, label_expr::ptr labelExpr) override {
    std::cerr << "% Alias: @" << name << " " << *labelExpr << std::endl;
    // cannot deal with aliases yet
    //mRAut.InsEvent(name);
  }

  // faudes event label from bits
  std::string bits2label(uint32_t bits) {
    std::string res="ev";;
    int i;
    for(i=mApc-1;i>=0;--i) {
      if(bits & (1L << i)) res+="1";
      else res+= "0";
    }
    return res;
  }

  virtual void setAPs(const std::vector<std::string>& aps) override {
    /*
    std::cerr << "% AP: " << aps.size();
    for (const std::string& ap : aps) {
      std::cerr << " ";
      HOAParserHelper::print_quoted(std::cerr, ap);
    }
    std::cerr << std::endl;
    */
    // should perhaps record?
    // set up alphabet
    mApc=aps.size();
    int evcnt= 1L << mApc;
    for(uint32_t i=0;i<evcnt;++i)
      mRAut.InsEvent(bits2label(i));    
  }

  virtual void setAcceptanceCondition(unsigned int numberOfSets, acceptance_expr::ptr accExpr) override {
    //std::cerr << "Acceptance: " << numberOfSets << " " << *accExpr << std::endl;
    mRAut.RabinAcceptance().Size((numberOfSets+1)/2);
    // error on odd?
  }

  virtual void provideAcceptanceName(const std::string& name, const std::vector<IntOrString>& extraInfo) override {
    /*
    std::cerr << "acc-name: " << name;
    for (const IntOrString& extra : extraInfo) {
      std::cerr << " " << extra;
    }
    std::cerr << std::endl;
    */
    // can on ly deal with Rabin
    if(name=="Rabin") {
      mRabin=true;
    }
  }

  virtual void setName(const std::string& name) override {
    mRAut.Name(name);
  }

  virtual void setTool(const std::string& name, std::shared_ptr<std::string> version) override {
    /*
    std::cerr << "tool: ";
    HOAParserHelper::print_quoted(std::cerr, name);
    if (version) {
      std::cerr << " ";
      HOAParserHelper::print_quoted(std::cerr, *version);
    }
    std::cerr << std::endl;
    */
  }

  virtual void addProperties(const std::vector<std::string>& properties) override {
    /*
    std::cerr << "properties:";
    for (const std::string& property : properties) {
      std::cerr << " " << property;
    }
    std::cerr << std::endl;
    */
  }

  virtual void addMiscHeader(const std::string& name, const std::vector<IntOrString>& content) override {
    /*
    std::cerr << name << ":";
    for (const IntOrString& extra : content) {
      std::cerr << " " << extra;
    }
    std::cerr << std::endl;
    */
  }

  virtual void notifyBodyStart() override {
    //std::cerr << "--BODY--" << std::endl;
  }

  virtual void addState(unsigned int id,
                        std::shared_ptr<std::string> info,
                        label_expr::ptr labelExpr,
                        std::shared_ptr<int_list> accSignature) override {
    /*
    std::cerr << "State: ";
    if (labelExpr) {
      std::cerr << "[" << *labelExpr << "] ";
    }
    std::cerr << id;
    if (info) {
      std::cerr << " ";
      HOAParserHelper::print_quoted(std::cerr, *info);
    }
    if (accSignature) {
      std::cerr << " {";
      bool first = true;
      for (unsigned int acc : *accSignature) {
        if (!first) std::cerr << " ";
        first = false;
        std::cerr << acc;
      }
      std::cerr << "}";
    }
    std::cerr << std::endl;
    */
    // have the state (redundant)
    mRAut.InsState(id+1);
    // record to acceptance condition
    if (accSignature) {
      for (unsigned int acc : *accSignature) {
	faudes::RabinPair& rpair = mRAut.RabinAcceptance().At(acc/2);
	bool rnoti = acc%2;
	if(rnoti) rpair.RSet().Insert(id+1);
	else rpair.ISet().Insert(id+1);
      }
    }
    // reset event index
    mImplicitLabel=0;
  }

  virtual void addEdgeImplicit(unsigned int stateId,
                               const int_list& conjSuccessors,
                               std::shared_ptr<int_list> accSignature) override {
    /*
    bool first = true;
    for (unsigned int succ : conjSuccessors) {
      if (!first) std::cerr << "&";
      first = false;
      std::cerr << succ;
    }
    if (accSignature) {
      std::cerr << " {";
      first = true;
      for (unsigned int acc : *accSignature) {
        if (!first) std::cerr << " ";
        first = false;
        std::cerr << acc;
      }
      std::cerr << "}";
    }
    std::cerr << std::endl;
    */
    faudes::Idx ev=mRAut.EventIndex(bits2label(mImplicitLabel));
    for (unsigned int succ : conjSuccessors) 
      mRAut.SetTransition(stateId+1,ev,succ+1);
    mImplicitLabel++;;
  }
  
  virtual void addEdgeWithLabel(unsigned int stateId,
                                label_expr::ptr labelExpr,
                                const int_list& conjSuccessors,
                                std::shared_ptr<int_list> accSignature) override {
    if (labelExpr) {
      std::cerr << "[" << *labelExpr << "] ";
    }

    bool first = true;
    for (unsigned int succ : conjSuccessors) {
      if (!first) std::cerr << "&";
      first = false;
      std::cerr << "consuc" << succ;
    }

    if (accSignature) {
      std::cerr << " {";
      first = true;
      for (unsigned int acc : *accSignature) {
        if (!first) std::cerr << " ";
        first = false;
        std::cerr << acc;
      }
      std::cerr << "}";
    }
    std::cerr << std::endl;
  }

  virtual void notifyEndOfState(unsigned int stateId) override {
    // nothing to do
  }

  virtual void notifyEnd() override {
    // invert ISets
    faudes::RabinAcceptance::Iterator rit=mRAut.RabinAcceptance().Begin();
    for(;rit!=mRAut.RabinAcceptance().End();++rit) 
      rit->ISet()=mRAut.States() - rit->ISet();
    // done  
    mRAut.Write(rTw);
    rTw.Flush();
  }

  virtual void notifyAbort() override {
    std::cerr << "--ABORT--" << std::endl;
    std::cerr.flush();
  }

  virtual void notifyWarning(const std::string& warning) override {
    std::cerr << "Warning: " << warning << std::endl;
  }

private:

  /** Reference to the output token stream  */
  faudes::TokenWriter& rTw;

  /** Payload: faudestype objects */
  faudes::RabinAutomaton mRAut;

  /** Payload: track my requirements */
  bool mRabin=false;

  /** Payload: parsing results */
  int mApc=0;
  uint32_t mImplicitLabel=0;
    
};

}

#endif

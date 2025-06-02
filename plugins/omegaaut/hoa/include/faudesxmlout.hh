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
#include "cpphoafparser/parser/hoa_parser_exception.hh"
#include "cpphoafparser/util/implicit_edge_helper.hh"
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
  HOAConsumerFaudes(faudes::TokenWriter& tw) : rTw(tw), mpImplEdgeHlp(new ImplicitEdgeHelper(0)) {}

  /** Constructs a HOAParserExeption to indicate an unsupported but presumably relevent feature */
  HOAParserException error(const std::string& msg) {
    std::stringstream ss;
    ss << "HOAConsumerFaudes: unsupported feature: " << msg;
    return HOAParserException(ss.str());
  }

  virtual bool parserResolvesAliases() override {
    return false;
  }

  virtual void notifyHeaderStart(const std::string& version) override {
    // should we the check version?
    mRAut.Clear();
  }

  virtual void setNumberOfStates(unsigned int numberOfStates) override {
    for(unsigned int i=0;i<numberOfStates;++i) mRAut.InsState(i+1);
  }

  virtual void addStartStates(const int_list& stateConjunction) override {
    std::cerr << std::endl;
    for(unsigned int state : stateConjunction) 
       mRAut.InsInitState(state+1);
  }

  virtual void addAlias(const std::string& name, label_expr::ptr labelExpr) override {
    std::cerr << "% Alias: @" << name << " " << *labelExpr << std::endl;
    throw error("aliases");
  }

  // faudes event label from bits
  // (alt: use decimals, or hexadezimals)
  std::string bits2label(uint32_t bits) {
    std::string res="ev";;
    int i;
    for(i=mApCount-1;i>=0;--i) {
      if(bits & (1L << i)) res+="1";
      else res+= "0";
    }
    return res;
  }

  virtual void setAPs(const std::vector<std::string>& aps) override {
    // record the names
    int i=0;
    for (const std::string& ap : aps) 
      mApSymbols[i++]=ap;
    // set up alphabet
    mApCount=aps.size();
    unsigned int evcnt= 1L << mApCount;
    for(uint32_t i=0;i<evcnt;++i)
      mRAut.InsEvent(bits2label(i));    
  }

  virtual void setAcceptanceCondition(unsigned int numberOfSets, acceptance_expr::ptr accExpr) override {
    if(mRabin) {
      mRAut.RabinAcceptance().Size((numberOfSets+1)/2);
      return;
    }
    throw error("Acc befor acc-name");
  }

  virtual void provideAcceptanceName(const std::string& name, const std::vector<IntOrString>& extraInfo) override {
    if(name=="Rabin") {
      mRabin=true;
      return;
    }
    // can only deal with Rabin
    throw error("acc-name must be Rabin");
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
    mpImplEdgeHlp.reset(new ImplicitEdgeHelper(mApCount));
  }

  virtual void addState(unsigned int id,
                        std::shared_ptr<std::string> info,
                        label_expr::ptr labelExpr,
                        std::shared_ptr<int_list> accSignature) override {
    if (labelExpr) {
      throw error("state label expression");
    }
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
    // reset event index for implicit edges to come
    mpImplEdgeHlp->startOfState(id);
  }

  virtual void addEdgeImplicit(unsigned int stateId,
                               const int_list& conjSuccessors,
                               std::shared_ptr<int_list> accSignature) override {
    if (accSignature) {
      throw error("transition marking");
    }
    uint32_t edgeIndex = mpImplEdgeHlp->nextImplicitEdge();
    faudes::Idx ev=mRAut.EventIndex(bits2label(edgeIndex));
    for (unsigned int succ : conjSuccessors) 
      mRAut.SetTransition(stateId+1,ev,succ+1);
  }
  
  virtual void addEdgeWithLabel(unsigned int stateId,
                                label_expr::ptr labelExpr,
                                const int_list& conjSuccessors,
                                std::shared_ptr<int_list> accSignature) override {
    throw error("explicit edge lable");
    /*
    if (labelExpr) {
      std::cerr << "[" << *labelExpr << "] ";
    }
    */

  }

  virtual void notifyEndOfState(unsigned int stateId) override {
    mpImplEdgeHlp->endOfState();
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

  /** Payload: parsing results */
  bool mRabin=false;
  bool mBuechi=false;
  int mApCount=0;
  std::map<int,std::string> mApSymbols;
  std::shared_ptr<ImplicitEdgeHelper> mpImplEdgeHlp;
    
};

}

#endif

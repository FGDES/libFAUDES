/** 

@file pd_interface.i 

SWIG interface for pushdown plugin.
**/


/*
**************************************************
**************************************************
**************************************************

Pushdown interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module pushdown
#ifndef SwigModule
#define SwigModule "SwigPushdown"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy pushdown to faudes name space
for k,v in pairs(pushdown) do faudes[k]=v end
}
#endif

/*
**************************************************
**************************************************
**************************************************

Pushdown interface: plugin data types

**************************************************
**************************************************
**************************************************
*/




SwigHelpTopic("PushdownGenerator","PushdownGenerator methods");




class AttributePushdownState : public AttributeFlags {
public:

  /** Constructor */
  AttributePushdownState(void);
  ~AttributePushdownState();
  virtual bool IsDefault(void) const;
};


class AttributePushdownGlobal : public AttributeVoid {
public:

  /** Constructor */
  AttributePushdownGlobal(void);
  virtual bool IsDefault(void) const;
};


typedef  std::set<std::pair<std::vector<Idx>,std::vector<Idx> > > PopPushSet;

class AttributePushdownTransition : public AttributeVoid {
 public:
 
  /** Constructor */
  AttributePushdownTransition(void);
  virtual bool  IsDefault(void) const;
  const PopPushSet& PopPush() const { return mPopPush;}
  bool ClrPopPush(const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
  
};


class StackSymbol{
  public:
  
  /** Constructor */
  StackSymbol(void);
  StackSymbol(std::string symbol);
};

class StackSymbolSet : public NameSet {
public:

  /** Constructor */
  StackSymbolSet(void);
  StackSymbolSet(const StackSymbolSet& rOtherSet);
};


// PushdownGenerator: define attribute members as a macro
%define SwigTpdGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)
  
    bool SetTransition(const std::string& x1, const std::string& ev, const std::string& pop,	const std::string& push, const std::string& x2);
	bool ClrTransition(const std::string& x1, const std::string& ev, const std::string& pop,	const std::string& push, const std::string& x2);	
	bool ExistsTransition(const std::string& x1, const std::string& ev,	const std::string& pop,	const std::string& push, const std::string& x2);
	
  	Idx InsLambdaStackSymbol(void);
  	
    Idx StackSymbolsSize(void) const;
    
    std::string StackSymbolName(Idx index) const;
    
    void StackSymbolName(Idx index, const std::string& rName) const;
    
    Idx StackSymbolIndex(const std::string& rName) const;

    Idx InsStackSymbol(const std::string& rName);
    
    Idx SetStackBottom(const std::string& rName);
    
    Idx StackBottom() const;

    bool DelStackSymbol(Idx index);
    bool DelStackSymbol(const std::string& rName);

    bool ExistsStackSymbol(Idx index) const;
    bool ExistsStackSymbol(const std::string& rName) const;

    std::string UniqueStackSymbolName(const std::string& rName) const;

   	Idx TransRelSize() const;
   	
   	
    bool SetTransition(const Transition& rTrans, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
    bool SetTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
    bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);
    bool SetTransition(Idx x1, Idx ev, Idx x2, const std::vector<std::string>& rPop, const std::vector<std::string>& rPush);
    bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
    
    bool ClrTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
    bool ClrTransition(const Transition& rTrans, const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
    
    bool ExistsTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2) const;
    bool ExistsTransition(Idx x1, Idx ev, Idx x2) const;
    bool ExistsTransition(const Transition& rTrans) const;
    bool ExistsTransition(Idx x1, Idx ev) const;
    bool ExistsTransition(Idx x1) const;
    bool ExistsTransition(Idx x1, Idx ev, Idx x2, const std::vector<Idx>& pop, const std::vector<Idx>& push) const;
    bool ExistsTransition(const Transition& rTrans, const std::vector<Idx>& pop, const std::vector<Idx>& push) const;
    const PopPushSet& PopPush(const Transition& rTrans) const;

    bool IsStackSymbolLambda(Idx index) const;
    
    bool IsEventLambda(Idx index) const;
    
    void DotWrite(const std::string& rFileName) const;
    void DotWrite(const std::string& rFileName, bool printInfo, bool lr) const;
    
    void DotRead(const std::string& rFileName);

%enddef

	SwigHelpEntry("PushdownGenerator","Set Transition","bool SetTransition(x1, pop, push, x2)");
	SwigHelpEntry("PushdownGenerator","Clear Transition","bool ClrTransition(x1, pop, push, x2)");
	SwigHelpEntry("PushdownGenerator","Test Transition","bool ExistsTransition(x1, pop, push, x2)");
  
  	SwigHelpEntry("PushdownGenerator","Insert lambda stack symbol","Idx InsLambdaStackSymbol()");
  	SwigHelpEntry("PushdownGenerator","Number of stack symbol","Idx StackSymbolsSize()");
    SwigHelpEntry("PushdownGenerator","Name of stack symbol","std::string StackSymbolName(Idx)");
    SwigHelpEntry("PushdownGenerator","Set name of stack symbol","std::string StackSymbolName(Idx,std::string)");
    SwigHelpEntry("PushdownGenerator","Insert stack symbol","Idx InsStackSymbol(std::string)");
    SwigHelpEntry("PushdownGenerator","Index of stack symbol","std::string StackSymbolIndex(std::string)");
    SwigHelpEntry("PushdownGenerator","Set stack bottom","Idx SetStackBottom(std::string)");
    SwigHelpEntry("PushdownGenerator","Index of stack bottom","Idx StackBottom()");
    SwigHelpEntry("PushdownGenerator","Delete stack symbol","bool DelStackSymbol(Idx)");
    SwigHelpEntry("PushdownGenerator","Delete stack symbol","bool DelStackSymbol(std::string)"); 
    SwigHelpEntry("PushdownGenerator","Test stack symbol","bool ExistsStackSymbol(Idx)");
    SwigHelpEntry("PushdownGenerator","Test stack symbol","bool ExistsStackSymbol(std::string)");
    SwigHelpEntry("PushdownGenerator","Get unique stack symbol name","std::string UniqueStackSymbolName(std::string)");
   	SwigHelpEntry("PushdownGenerator","Number of Transitions","Idx TransRelSize()");
    SwigHelpEntry("PushdownGenerator","Test if stack symbol is lambda","bool IsStackSymbolLambda(Idx)");
    SwigHelpEntry("PushdownGenerator","Test if event is lambda","bool IsEventLambda(Idx)");
    SwigHelpEntry("PushdownGenerator","Write DOT file","void DotWrite(string)");
    SwigHelpEntry("PushdownGenerator","Read DOT file","void DotRead(string)");



// PushdownGenerator template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TpdGenerator :public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TpdGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTcGeneratorMembers(TpdGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  
  // Additional constructor
  //TpdGenerator(void);
  //TpdGenerator(const TpdGenerator& rOtherGen);
  //TpdGenerator(const vGenerator& rOtherGen);
  //TpdGenerator(const std::string& rFileName);
  //virtual ~TpdGenerator();
  //virtual TpdGenerator* New(void) const;
  //virtual TpdGenerator* Copy(void) const;
    
  // TpdGenerator members not inherited from TaGenerator
  SwigTpdGeneratorMembers(TpdGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  System class to swig as PushdownGenerator
%template(PushdownGenerator) TpdGenerator<AttributePushdownGlobal, AttributePushdownState, AttributeCFlags,AttributePushdownTransition>;

// Tell swig that our generator is also known as PushdownGenerator
typedef TpdGenerator<AttributePushdownGlobal, AttributePushdownState, AttributeCFlags,AttributePushdownTransition>
  PushdownGenerator;

typedef TaTransSet<AttributePushdownTransition> PdTransSet;





/*
**************************************************
**************************************************
**************************************************

Pushdown interface: plugin function(s)

**************************************************
**************************************************
**************************************************
*/

// Add topic to mini help system
SwigHelpTopic("Pushdown","Pushdown plugin methods");

//SwigHelpEntry("Pushdown","Constructors","PushdownGenerator PushdownGenerator()");



void PushdownConstructController(const PushdownGenerator& rSpec,const System& rPlant, PushdownGenerator& rRes, bool debug = false);
SwigHelpEntry("Pushdown","Construct a minimal restrictive sound controller","void PushdownConstructController(PushdownGenerator,System)");

void PushdownBlockfree(const PushdownGenerator& rPd, PushdownGenerator& rResPd, bool debug = false);
SwigHelpEntry("Pushdown","Make a pushdown generator nonblocking","void PushdownBlockfree(PushdownGenerator inPd,PushdownGenerator result)");

void PushdownAccessible(const PushdownGenerator& pd, PushdownGenerator& resPd, bool coacc = true, bool debug = false);
SwigHelpEntry("Pushdown","Construct the accessible part of a pushdown generator","void PushdownAccessible(PushdownGenerator inPd, PushdownGenerator result)");

StateSet Transient(const PushdownGenerator& pd);
SwigHelpEntry("Pushdown","Get all states that are the starting state of a lambda reading transition","StateSet Transient(PushdownGenerator)");

PushdownGenerator Rnce(const PushdownGenerator& pd, const System& s);
SwigHelpEntry("Pushdown","Remove non-controllable ears from a generator.","PushdownGenerator Rnce(PushdownGenerator, System)");

PushdownGenerator Times(const System& reg, const PushdownGenerator& pd);
SwigHelpEntry("Pushdown","synchronous product generator of a pushdown generator and a regular generator","PushdownGenerator Times(System, PushdownGenerator)");

void IntersectEvents(const System& s, const PushdownGenerator& pd, PushdownGenerator& rPd);
SwigHelpEntry("Pushdown","Adds the intersection of events of the first two generators to the result generator.","void IntersectEvents(System, PushdownGenerator, PushdownGenerator result)");

PushdownGenerator Split(const PushdownGenerator& pd);
SwigHelpEntry("Pushdown","Splits the states of a pushdown generator into heads and ears.","PushdownGenerator Split(PushdownGenerator)");

void CorrectEvents(const PushdownGenerator& correctPd, PushdownGenerator& pd);
SwigHelpEntry("Pushdown","Sets controllability and observability flags for a pushdown generator's events to the event flags of another pushdown generator","void CorrectEvents(PushdownGenerator correctPd, PushdownGenerator pd)");

  
//convencience definition for a vector which contains each transformation from PDA to SPDA by function SPDA
typedef std::vector<std::vector<std::pair<Idx,MergeTransition> > > TransformationHistoryVec;





bool PushdownTrim(PushdownGenerator& rPd, Idx n, bool debug=false);
SwigHelpEntry("Pushdown","Make generator trim","bool PushdownTrim(PushdownGenerator, uint n)");

PushdownGenerator Nda(const PushdownGenerator& pd);
SwigHelpEntry("Pushdown","Remodels the generator to prevent double (or multiple) acceptance of the same input string (Input must have passed SPDA()).","PushdownGenerator Nda(PushdownGenerator)");

Grammar Sp2Lr(const PushdownGenerator& rPd, Idx n=0, bool ignorReducible = false);
SwigHelpEntry("Pushdown","Transform a simple pushdown generator into a context free grammar. Input must have passed SPDA() and Nda() !","Grammar Sp2Lr(PushdownGenerator)");

Grammar Rnpp(const Grammar& gr);
SwigHelpEntry("Pushdown","Remove all productions from a grammar that are nonproductive","Grammar Rnpp(Grammar)");

Grammar Rup(const Grammar& gr);
SwigHelpEntry("Pushdown","Remove all unreachable productions and nonterminals from the grammar","Grammar Rnpp(Grammar)");

PushdownGenerator RemoveMultPop(const PushdownGenerator& pd);
SwigHelpEntry("Pushdown","Removes transitions popping more than one stack symbol.","PushdownGenerator RemoveMultPop(PushdownGenerator)");

uint InTransitionSize(Idx trg,const PushdownGenerator& rPd);
SwigHelpEntry("Pushdown","Getting the number of incoming transitions of a state","uint InTransitionSize(Idx, PushdownGenerator)");

void MergeAdjacentTransitions(PushdownGenerator& rPd);
SwigHelpEntry("Pushdown","Merge adjacent transitions","void MergeAdjacentTransitions(PushdownGenerator)");

PushdownGenerator RemoveLambdaPop(const PushdownGenerator& pd);
SwigHelpEntry("Pushdown","Removes all transitions popping lambda.","PushdownGenerator RemoveLambdaPop(PushdownGenerator)");


System SystemFromDot(const std::string& filename);
SwigHelpEntry("Pushdown","Create a system from given dot file.","PushdownGenerator SystemFromDot(std::string filename)");


void ToFile(const PushdownGenerator& pd,std::string name, bool printDOT =true, bool printPNG=true);
void ToFile(const std::string& tex,std::string name);


/*
**************************************************
**************************************************
**************************************************

Pushdown interface: Grammar topics

**************************************************
**************************************************
**************************************************
*/

class Grammar{

  public:
  
  std::string Str() const;
};




/*
**************************************************
**************************************************
**************************************************

Help topics

**************************************************
**************************************************
**************************************************
*/

//SwigHelpTopic("PushdownGenerator","PushdownGenerator methods");

//SwigHelpEntry("PushdownGenerator","Constructors","PushdownGenerator PushdownGenerator()");






// Include rti generated functioninterface 
#if SwigModule=="SwigPushdown"
%include "../../../include/rtiautoload.i"
#endif

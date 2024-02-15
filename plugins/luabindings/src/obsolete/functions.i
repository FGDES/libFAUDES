/** @file functions.i @brief luabindings for core generator functions */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Thomas Moor
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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


/*
**************************************************
**************************************************
**************************************************

Functions misc

**************************************************
**************************************************
**************************************************
*/


// **** Version (does not participate in rti)
// Have function interface for a generator member
%{
void LuaVersion(const Generator& rGen, const std::string& ver, Generator& rRes) 
 { rGen.Version(ver,rRes);}
void LuaVersion(const Generator& rGen, const std::string& pat, const std::string& rep, Generator& rRes) 
 { rGen.Version(pat,rep,rRes);}
%}
// Tell SWIG with nice target name
%rename(Version) LuaVersion;
void LuaVersion(const Generator& rGen, const std::string& ver, Generator& rRes);
void LuaVersion(const Generator& rGen, const std::string& pat, const std::string& rep, Generator& rRes);



/*
**************************************************
**************************************************
**************************************************

Functions from regex and friends

**************************************************
**************************************************
**************************************************
*/

// Tell SWIG (non-det versions dont participate with rti)
void LanguageConcatenateNonDet(const Generator& rGen1, const Generator& rGen2, Generator& rRes);
void LanguageUnionNonDet(const Generator& rGen1, const Generator& rGen2, Generator& rResGen);
void KleeneClosureNonDet(Generator& rGen);
void ProjectNonDet(Generator& rGen, const EventSet& rAlph);
void ProjectNonDetScc(Generator& rGen, const EventSet& rAlph);



/*
**************************************************
**************************************************
**************************************************

Functions related to EventSet

**************************************************
**************************************************
**************************************************
*/

// Have function interface with Alphabets
%{
void LuaEventSetUnion(const EventSet& rAlph1, const EventSet& rAlph2, EventSet& rRes) 
 { rRes = rAlph1; rRes.InsertSet(rAlph2); }
void LuaEventSetIntersection(const EventSet& rAlph1, const EventSet& rAlph2, EventSet& rRes)
 { rRes = rAlph1; rRes.RestrictSet(rAlph2); }
void LuaEventSetDifference(const EventSet& rAlph1, const EventSet& rAlph2, EventSet& rRes) 
 { rRes = rAlph1; rRes.EraseSet(rAlph2); }
%}



// Tell SWIG with nice target names (depreciated, use rti versions instead)
%rename(EventSetUnion) LuaEventSetUnion;
%rename(EventSetIntersection) LuaEventSetIntersection;
%rename(EventSetDifference) LuaEventSetDifference;
void LuaEventSetUnion(const EventSet& rAlph1, const EventSet& rAlph2, EventSet& rRes);
void LuaEventSetIntersection(const EventSet& rAlph1, const EventSet& rAlph2, EventSet& rRes);
void LuaEventSetDifference(const EventSet& rAlph1, const EventSet& rAlph2, EventSet& rRes);

/*
**************************************************
**************************************************
**************************************************

Parallel related data type: reverse composition map

**************************************************
**************************************************
**************************************************
*/

/*
Note:
- this is a minimal interface 
- this is not a full faudes type (no file io)
*/

class ProductCompositionMap : public Type {
public:
  // construct/destruct
  ProductCompositionMap(void);
  ProductCompositionMap(const ProductCompositionMap& rOther);
  ~ProductCompositionMap(void);
  // access map
  Idx CompState(Idx s1, Idx s2) const;
  Idx Arg1State(Idx s12) const;
  Idx Arg2State(Idx s12) const;
  // extend for Lua string conversion
  %extend {
    std::string __str__(void) { 
      EventSet eset;
      std::stringstream str;
      str << "["; 
      std::map< std::pair<Idx,Idx> , Idx >::const_iterator i;
      for(i=$self->StlMap().begin(); i!=$self->StlMap().end(); i++) {
        if(i!=$self->StlMap().begin()) str << "; "; 
        str << "(" << i->first.first << ", " << i->first.second << ")-->(" << i->second << ")";
      };
      str << "]"; 
      return str.str();
    };
  };
};


/*
**************************************************
**************************************************
**************************************************

SCC interface

**************************************************
**************************************************
**************************************************
*/

/*
Note:
- this is a minimal interface 
*/

class SccFilter {

public:

  SccFilter(void);
  ~SccFilter(void);
  void Clear(void);
  void StatesAvoid(const StateSet& rStatesAvoid);
  void StatesRequire(const StateSet& rStatesRequire);
  void EventsAvoid(const EventSet& rEventsAvoid);
  void IgnoreTrivial(bool flag);  
  void FindFirst(bool flag);  
  const StateSet& StatesAvoid(void) const;
  const StateSet& StatesRequire(void) const;

};


bool ComputeScc(const Generator& rGen, const SccFilter& rFilter, StateSet& rScc);
bool ComputeScc(const Generator& rGen, const SccFilter& rFilter, Idx q0, StateSet& rScc);
bool ComputeNextScc(const Generator& rGen, SccFilter& rFilter, StateSet& rScc);
bool HasScc(const Generator& rGen,const SccFilter& rFilter);


/*
**************************************************
**************************************************
**************************************************

Help topics

**************************************************
**************************************************
**************************************************
*/



SwigHelpTopic("Functions","Core library functions");

SwigHelpEntry("Functions","regular expressions", " LanguageConcatenateNonDet(+In+ Generator Lm1, +In+ Generator Lm2, +Out+ Generator LRes)");
SwigHelpEntry("Functions","regular expressions", " LanguageUnion(+In+ Generator Lm1, +In+ Generator Lm2, +Out+ Generator LRes)");
SwigHelpEntry("Functions","regular expressions", " LanguageUnionNonDet(+In+ Generator Lm1, +In+ Generator Lm2, +Out+ Generator LRes)");
SwigHelpEntry("Functions","regular expressions", " LanguageIntersection(+In+ Generator Lm1, +In+ Generator Lm2, +Out+ Generator LRes)");
SwigHelpEntry("Functions","regular expressions", " KleeneClosureNonDet(+InOut+ Generator LArg)");



SwigHelpEntry("Functions","event sets"," EventSetUnion(alph1_arg, alph2_arg, alph_res) [depreciated; use AlphabetUnion instead]");
SwigHelpEntry("Functions","event sets"," EventSetIntersection(alph1_arg, alph2_arg, alph_res) [depreciated; use AlphabetIntersection instead]");
SwigHelpEntry("Functions","event sets"," EventSetDifference(alph1_arg, alph2_arg, alph_res) [depreciated; use AlphabetDifference instead]");

SwigHelpEntry("Functions","generator misc"," Version(+In+ Generator G, +In+ String Subscript, +Out+ Generator GRes)");
SwigHelpEntry("Functions","generator misc"," Version(+In+ Generator G, +In+ String Pattern, +In+ String Replacement, +Out+ Generator GRes)");


SwigHelpEntry("Functions","generator misc","Boolean ComputeScc(+In+ Generator G, +In+ SccFilter sf, +Out+ StateSet SCC)");
SwigHelpEntry("Functions","generator misc","Boolean ComputeScc(+In+ Generator G, +In+ SccFilter sf, +In+ q0 +Out+ StateSet SCC)");
SwigHelpEntry("Functions","generator misc","Boolean ComputeNextScc(+In+ Generator G, +InOut+ SccFilter sf, +Out+ StateSet SCC)");
SwigHelpEntry("Functions","generator misc","Boolean HasScc(+In+ Generator G, +In+ SccFilter sf)");

SwigHelpEntry("Functions","language misc"," ProjectNonDet(+InOut+ Generator LArg, +In+ EventSet Sigma0)");
SwigHelpEntry("Functions","language misc"," ProjectNonDetScc(+InOut+ Generator LArg, +In+ EventSet Sigma0)");




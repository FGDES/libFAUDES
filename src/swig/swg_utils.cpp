/** @file swg_utils.cpp  utility functions for bindings*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008-2025 Thomas Moor
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

#include "swg_utils.h"


namespace faudes {

// programatically throw exceptions
void faudes_throw_exception(const std::string& msg) {
  throw Exception("faudes script", msg, 49);
}


// global switches
bool faudes_statenames = true;
std::string faudes_dotpath = "dot";

// access functions
void faudes_statenames_on(void) {faudes_statenames=true; Generator::StateNamesEnabledDefault(true); }
void faudes_statenames_off(void) {faudes_statenames=false; Generator::StateNamesEnabledDefault(false); }
void faudes_dotexecpath(const std::string& filename)  { faudes_dotpath=filename; }
std::string faudes_dotexecpath()  { return faudes_dotpath; }
std::string  faudes_version(void) { return VersionString()+" "+PluginsString(); }
std::string  faudes_build(void) { return BuildString(); }

// helper: run dot for test
bool faudes_dotready(void) {
  // cache value
  static bool ready=false;  
  static bool known=false;
  if(known) return ready;    
  // test for dot binary
  std::string testdot = faudes_dotpath + " -V";
  ready = (system(testdot.c_str()) == 0);
  known = true;
  return ready;
}

//global help dictionary topic -> key -> text
std::map< std::string, std::string > faudes_dictionary_topics;
std::map< std::string, std::map<std::string, std::vector<std::string> > > faudes_dictionary_entries;

// insert topic to dictionary
void faudes_dict_insert_topic(const std::string& topic, const std::string& text) {
  faudes_dictionary_topics[topic]=text;
}

// insert entry to dictionary
void faudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry) {
  std::string k=key;
  if(k.length()>0) 
    k.at(0)=toupper(k.at(0));
  faudes_dictionary_entries[topic][k].push_back(entry);
  if(topic!="")
  if(faudes_dictionary_topics[topic]=="") 
    faudes_dictionary_topics[topic]=topic;
}


// main help text
void faudes_help(void) {
  std::stringstream sstr;

  // section: intro
  sstr << std::endl
    << "libFAUDES bindings provide access to libFAUDES data structures and functions. " << std::endl
    << "For detailed information, please consult the libFAUDES documentation."<< std::endl
    << std::endl
    << "All libFAUDES bindings are in the module 'faudes', ie access is via 'faudes.*'." << std::endl
    << std::endl
    << "Interface:" << std::endl
    << std::endl;

  // section: topics
  std::map< std::string, std::string >::iterator tit; 
  for(tit=faudes_dictionary_topics.begin(); tit!=faudes_dictionary_topics.end();tit++) {
    std::string left  = "  faudes.Help(\"" + tit->first +"\")";
    std::string right = tit->second;
    while(left.length()<37) left+=" ";
    sstr << left << right << std::endl;
  }
  sstr << std::endl;

  // section: behavioural
  sstr 
    << "Configuration:" << std::endl
    << std::endl
    << "  faudes.StateNamesOn()              enable automatic state names" << std::endl
    << "  faudes.StateNamesOff()             disable automatic state names" << std::endl
    << "  faudes.DotExecPath(\"filename\")     path of dot executable" << std::endl
    << "  faudes.Version()                   return libfaudes version string" << std::endl
    << std::endl
    << "Console Commands:" << std::endl
    << std::endl
    << "  faudes.Print(\"message\")            print as faudes debugging message" << std::endl
    << "  faudes.Error(\"message\")            abort script with error message" << std::endl
    << "  faudes.Verbosity(int)                set verbositry level " << std::endl;

  // do print to stderr
  Print(0,sstr.str()); // verb 0 <> always
}

// section text
void faudes_help(const std::string& topic) {
  std::stringstream sstr;

  // section: intro
  sstr  
    << std::endl
    << "libFAUDES help topic: \"" << topic << "\"" << std::endl
    << std::endl;

  // section: list entries
  std::map< std::string, std::map< std::string, std::vector<std::string> > >::iterator tit; 
  tit = faudes_dictionary_entries.find(topic);
  if(tit!=faudes_dictionary_entries.end()) {
    std::map< std::string, std::vector<std::string> >::iterator kit;   
    for(kit = tit->second.begin(); kit != tit->second.end(); kit++) {
      if(kit != tit->second.begin()) sstr << std::endl;
      const std::string& line = kit->first;
      sstr << "                 *** " << line << " ***" << std::endl;
      for(unsigned int i=0; i< kit->second.size(); i++) {
        const std::string& line = kit->second[i];
	std::size_t sep = line.find_first_of(' ');
        if(sep==std::string::npos) sep=0;
        while(sep<20) { sstr << " "; sep++; };
        sstr << line << std::endl;
      }
    }
  } else {
    sstr << "  (no entries) " << std::endl;
  }

  // do print to stderr
  Print(0,sstr.str()); // verb 0 <> always
}

}//namespace  


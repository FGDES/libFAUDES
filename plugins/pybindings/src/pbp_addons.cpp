// my headers
#include "corefaudes.h"
#include "pbp_addons.h"

// python header
#include "Python.h"

namespace faudes {

    
// write to Python's sys.stdout
void faudes_pprint(const char* msg) {
  PySys_FormatStdout("%s",msg);
}

  
// redirect libFAUDES console
class PythonConsole : public ConsoleOut{
public:
  // singleton access
  static PythonConsole* G(void);
  // singelton destructor
  static void Destruct(void);
private:
  // construct/destruct
  PythonConsole();
  virtual ~PythonConsole();
protected:
  // reimplement faudes dowrite
  void DoWrite(const std::string& message, long int cntnow, long int cntdone, int verb=0);
  // single instance
  static PythonConsole* mpVInstance;
};


// static singleton incl construction
PythonConsole* PythonConsole::mpVInstance = NULL;

//  explcit activation
void faudes_redirect(bool on) {
  static PythonConsole* gSinglePythonConsole=NULL;
  if(on) {
    gSinglePythonConsole=PythonConsole::G();
  } else {
    PythonConsole::Destruct();
  }
}
  
// private construct
PythonConsole::PythonConsole() : ConsoleOut() {
  //FD_WARN("faudes::ConsoleOut(): redirect to sys.stdout") 
  faudes::ConsoleOut::G()->Redirect(this);
};
// private destruct 
PythonConsole::~PythonConsole()  {
  //FD_WARN("faudes::ConsoleOut(): redirect off"); 
  faudes::ConsoleOut::G()->Redirect(0);
  mpVInstance=NULL;
};
// access/construct singleton
PythonConsole* PythonConsole::G(void) { 
  if(!mpVInstance) mpVInstance = new PythonConsole();
  return mpVInstance;
}
// destruct singleton
void PythonConsole::Destruct(void) {
  if(!mpVInstance) return;
  delete mpVInstance;
  mpVInstance=0;
}
// faudes hook ...
// ... is rather fragile due to interference between logging and loop call back
// ... dont use debugging macros FD_xxx here, they will mess up logging
void PythonConsole::DoWrite(const std::string& message,long int cntnow, long int cntdone, int verb) {
  (void) cntnow; (void) cntdone;
  // we're on mute
  if(Verb()<verb) {
    return;
  }
  // its a file
  if(IsFile()) {
    ConsoleOut::DoWrite(message,cntnow,cntdone,verb);
    return;
  }
  // its the console we want to grab
  faudes_pprint(message.c_str());
}

  
  

}//namespace

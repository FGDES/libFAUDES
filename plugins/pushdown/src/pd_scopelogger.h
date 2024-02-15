/** @file pd_scopelogger.h Create Logfile */

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2014  Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_SCOPELOGGER_H
#define FAUDES_PD_SCOPELOGGER_H

#include "corefaudes.h"
#include "pd_include.h"

#include <ctime>


namespace faudes {

#ifdef FAUDES_PD_PRINT_LOGFILE
#define FUNCTION(x) ScopeLogger l_##x##_scope(x);
#else
#define FUNCTION(x)
#endif

/**
 * Class to create a log file which contains function calls with their runtime
 */
class ScopeLogger{

private:
  static int indent;
  static std::ofstream outputFile;
  static time_t starttime;

  public:
  	  std::string msg;
  	  time_t startFuntime;
	  static std::vector<ScopeLogger*> vFunTrace;
	  static std::stringstream stFunDone;

  /**
   * Constructor
   */
  ScopeLogger( std::string const & msg );

  /**
    * Deconstructor
    */
  ~ScopeLogger();

  /*
   * Print current stack trace
   */
  void StackTrace();

}; // end class ScopeLogger


} // namespace faudes

#endif

/** @file pd_scopelogger.cpp Create Logfile */

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2014  Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_scopelogger.h"

namespace faudes {

/********************************************************************

 Implementation of ScopeLogger

 ********************************************************************/

//Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d: %X", &tstruct);

	return buf;
}

// ****************************
//	class ScopeLogger
// ***************************

int ScopeLogger::indent = 0;
std::ofstream ScopeLogger::outputFile(FAUDES_PD_LOGFILE);
std::vector<ScopeLogger*> ScopeLogger::vFunTrace;
std::stringstream ScopeLogger::stFunDone;
time_t ScopeLogger::starttime = 0;

//constructor
ScopeLogger::ScopeLogger(std::string const & msg) :
		msg(msg), startFuntime(time(0)) {

	if (starttime == 0)
		starttime = time(0);

	outputFile << std::string(indent++, ' ') << "> " << msg << " ("
			<< currentDateTime() << ") " << std::endl;
	vFunTrace.push_back(this);
}

//deconstructor
ScopeLogger::~ScopeLogger() {
	int totalTime = static_cast<int>(difftime(time(0), starttime));
	std::stringstream stotalTime;
	stotalTime << totalTime / 60 << "min " << totalTime % 60 << "s";

	int funTime = static_cast<int>(difftime(time(0), startFuntime));
	std::stringstream sfunTime;
	sfunTime << funTime / 60 << "min " << funTime % 60 << "s";

	if (!vFunTrace.empty())
		vFunTrace.pop_back();

	if (stFunDone.str().empty())
		stFunDone << "Done: ";
	stFunDone << msg << "(" << totalTime << "s)";

	outputFile << std::string(--indent, ' ') << "< " << msg << " ("
			<< currentDateTime() << ") function " << sfunTime.str() << ", total: "
			<< stotalTime.str() << std::endl;
}

//print stack trace
void ScopeLogger::StackTrace() {

	if (!ScopeLogger::stFunDone.str().empty())
		std::cout << ScopeLogger::stFunDone.str() << std::endl;

	if (ScopeLogger::vFunTrace.empty())
		return;

	std::stringstream trace;
	trace << "Trace: ";

	for (std::vector<ScopeLogger*>::iterator ilog =
			ScopeLogger::vFunTrace.begin();
			ilog != ScopeLogger::vFunTrace.end(); ++ilog) {
		trace << (*ilog)->msg << "(" << difftime(time(0), (*ilog)->startFuntime)
				<< "s) ";
	}

	std::cout << trace.str() << std::endl;
}

} // namespace faudes

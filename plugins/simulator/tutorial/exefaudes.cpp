/** @file exefaudes.cpp  Example application for executor class  

This application reads the specified generator file to initialize an
Executor and then runs the generator interactively. For more advanced
simulation, see the tutoriyl simfaudes.cpp. 

@ingroup Tutorials

@include exefaudes.cpp

*/


#include "libfaudes.h"

using namespace faudes;
using namespace std;

int main(int argc, char* argv[])
{
  // first argument has to be a .gen-file
  if(argc!=2) {
    cerr<<"usage: ./exefaudes <gen-File>"<<endl;
    return(-1);
  }

  // executor objekt
  Executor sim(argv[1]);

  // until user breaks simulation
  while(true) {

    // report current state
    cout << "##########################################" << endl;
    cout << "# interactive simulation of " << sim.Generator().Name() << endl;
    cout << "# at state " << endl;
    cout << "(time " << sim.CurrentTime() << ") " ;
    cout << sim.CurrentTimedStateStr() << endl;

    // request feasibility information
    TimeInterval etime=sim.EnabledTime();
    TimeInterval einterval=sim.EnabledInterval();
    EventSet eevents=sim.EnabledEvents();

    // report deadlock
    if(sim.IsDeadlocked()) {
      cout<<">> Simulation is deadlocked."<<endl;
      cout<<">> Abort (a) or reset simulation (r)?"<<endl;
      std::string choice;
      cin >> choice;
      if(choice=="a") break;
      else if(choice=="r") {
        sim.Reset();
        continue;
      } else {
        cerr<<">> ignoring invalid choice"<<endl;
        continue;
      }
    }

    // report feasibility: a) no events
    if(eevents.Empty()) {
      cout << ">> Time may pass up to  " << etime.Str() << endl;
      cout << ">> No events are enabled for " << einterval.Str() << endl;
      cout << ">> Abort (a), or let time pass (t): ";
    }
    // report feasibility: b) events nonempty
    else {
      cout << ">> Time may pass up to  " << etime.Str() << endl;
      cout << ">> The following events are enabled for time " << einterval.Str() << endl;
      eevents.DWrite();
      cout<<">> Abort (a), let time pass (t) or execute transition (e): ";
    }
    std::string choice;
    cin>>choice;
    if(choice=="a") {
      break;
    } else if(choice=="e") {
      cout<< ">> Event index: ";
      Idx event;
      cin >> event;
      if(eevents.Exists(event)==false) {
        cout<< ">> ignoring invalid event index "<<endl;
        continue;
      } else {
        sim.ExecuteEvent(event);
        continue;
      }
    } else if (choice=="t") {
      cout<<">> Let time pass t: ";
      Time::Type time;
      cin>>time;
      if(! etime.In(time)) {
        cout<<">> ignoring invalid time "<<endl;
        continue;
      }
      else sim.ExecuteTime(time);
    } else {
      cout<<">> ignoring invalid choice "<<endl;
      continue;
    }
  } // end while


  return(0);
}

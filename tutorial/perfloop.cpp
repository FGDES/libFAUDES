/** @file perfloop.cpp  Elementary template for performance evaluation

(c) 2016 Katja Pelaic
(c) 2016 Thomas Moor

*/

#include "libfaudes.h"

using namespace faudes;

/*
Construct a random automaton by setting the parameters:

- scnt  (number of states)
- evcnt (number of events)
- tcnt  (number of transitions per state)

*/
void RandomGenerator(int scnt, int evcnt, int tcnt, Generator& rRes) {
  // clear result
  rRes.Clear();
  // insert states (consecutive idx 1...scnt)
  for(int i=1; i <= scnt; ++i) {
    rRes.InsState(i);
  }
  rRes.SetInitState(1);
  // insert events (non consecutive idx, record map ev-numver --> ev-idx) 
  std::map<int, Idx>    map_evidx;   
  for(int i=0; i < evcnt; ++i) {
    map_evidx[i]=rRes.InsEvent("e" + ToStringInteger(i));
  }
  // insert random transitions (quietly ignore doublets)
  // loop over all states 
  StateSet:: Iterator sit= rRes.StatesBegin();
  for(; sit != rRes.StatesEnd(); ++sit) {
    int tc = faudes::ran_uniform_int(0, 2*tcnt + 1);
    for(int i = 0; i < tc; ++i) {
      Idx tx2 = ran_uniform_int(1,scnt+1);
      faudes::Idx tev = map_evidx[ran_uniform_int(0, evcnt)];
      rRes.SetTransition(*sit, tev, tx2);
    }
  }
  //rRes.SWrite();
}



// print usage info and exit
void usage_exit(const std::string& message="") {
  if(message!="") {
    std::cout << "perfloop: " << message << std::endl;
    std::cout << "" << std::endl;
    exit(-1);
  }
  std::cout << "perfloop: version " << VersionString() << std::endl;
  std::cout << "" << std::endl;
  std::cout << "perfloop: usage: " << std::endl;
  std::cout << "  perfloop [-q][-cs <nnn>][-ce <nnn>][-ct <nn>] [-n <nn>] " << std::endl;
  std::cout << "where " << std::endl;
  std::cout << "  -q:  less console output " << std::endl;
  std::cout << "  -cs <nnn>: number of states <nnn> " << std::endl;
  std::cout << "  -ce <nnn>: number of events <nnn> " << std::endl;
  std::cout << "  -ct <nnn>: number of transitions <nnn> per state" << std::endl;
  std::cout << "  -n  <nnn>: repetition of test" << std::endl;
  std::cout << "  -s: use zero seed for random number generator" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "" << std::endl;
  exit(-1);
}


// main program: parge options and run loop
int main(int argc, char* argv[]){

  // command line parameter defaults
  int param_q = 0;   // normal console output
  int param_cs = 10; // 10 states
  int param_ce = 3;  // 3 events
  int param_ct = 5;  // 5 transition per state
  int param_n = 1;   // 1 run 
  int param_s = 0;   // random seed

  // primitive commad line parsing
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: quiet
    if((option=="-q") || (option=="--quiet")) {
      param_q=1;
      continue;
    }
    // option: state count
    if((option=="-cs") || (option=="--states")) {
      i++; if(i>=argc) usage_exit();
      param_cs= ToIdx(argv[i]);
      if(param_cs <=0) usage_exit("positive state count required");
      continue;
    }
    // option: event count
    if((option=="-ce") || (option=="--events")) {
      i++; if(i>=argc) usage_exit();
      param_ce= ToIdx(argv[i]);
      if(param_ce <=0) usage_exit("positive event count required");
      continue;
    }
    // option: trans. dens.
    if((option=="-ct") || (option=="--transition-density")) {
      i++; if(i>=argc) usage_exit();
      param_ct= ToIdx(argv[i]);
      if(param_ct <=0) usage_exit("positive transition count required");
      continue;
    }
    // option: trans. dens.
    if((option=="-n") || (option=="--loop-count")) {
      i++; if(i>=argc) usage_exit();
      param_n= ToIdx(argv[i]);
      if(param_ce <=0) usage_exit("positive loop count");
      continue;
    }
    // option: seed
    if((option=="-s") || (option=="--seed0")) {
      param_s=1;
      continue;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage_exit();
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage_exit("unknown option "+ option);
      continue;
    }
    // filename
    usage_exit("unknown command "+ option);
    continue;
  }


  // seed random generator
  if(param_s==0) {
    faudes_systime_t now;
    faudes_gettimeofday(&now);
    faudes::ran_init(now.tv_sec);
  } else {
    faudes::ran_init(123456789);
  }

  // record last diagnostic output
  faudes_systime_t lastprint;
  faudes_gettimeofday(&lastprint);

  // accumulate amount of the runtime
  faudes_mstime_t overall_duration = 0;
 
  // accumulate state set reduction
  long int overall_reduction = 0;

  // loop the test operation to evaluate the timing
  for(int i=0; i<param_n; ++i) {

    // generate test case
    Generator grand;
    RandomGenerator(param_cs,param_ce,param_ct, grand);

    // start clock
    faudes_systime_t start;
    faudes_gettimeofday(&start);
  
    //test function
    std::map<faudes::Idx, faudes::Idx> pmap;
    faudes::Generator gbisim;
    calcBisimulation(grand,pmap, gbisim);

    // stop clock and accumulate duration
    faudes_systime_t stop;
    faudes_gettimeofday(&stop);
    faudes_mstime_t delta;
    faudes_diffsystime(stop, start, &delta);
    overall_duration += delta;

    // accumulate state set reduction
    long int reduction = grand.Size()-gbisim.Size();
    overall_reduction +=  reduction; 


    // report within loop every two seconds
    faudes_mstime_t deltaprint;
    faudes_diffsystime(stop, lastprint, &deltaprint);
    if((deltaprint>2000) && param_q==0) {
      std::cout << "perfloop: state set reduction #" << reduction << " amounts to " << 
	((long int)(10.0 * 100.0 * reduction/grand.Size())) / 10.0 << "%" << std::endl;
      std::cout << "perfloop: time elapse " <<  ((delta + 50) / 100) / 10.0 << "sec." << std::endl;
    }


  }


  // report average duration per loop
  std::cout << "perfloop: average state set reduction " << 
    ((long int)(10.0 * 100.0 * overall_reduction / param_cs / param_n)) / 10.0 << "%" << std::endl;
  long int average_duration = ((double) overall_duration) / (param_n*100.0) + 0.5; // 0.1sec
  std::cout << "perfloop: average duration per loop " << ((double) average_duration) / 10.0 << "sec." << std::endl;



  return 0;
}//end main


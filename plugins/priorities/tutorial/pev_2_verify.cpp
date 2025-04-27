// @file pev_3_verify.cpp verification with std synchronous marking

// 1) generate models via "pev_cbs_setup.lua"
// 2) run this executable


#include "libfaudes.h"

using namespace faudes;



// main
int main(int argc, char* argv[]) {

    // read number of conveyor belts from command line
    // (defaults to 5, needs to match models prepared by pev_cbs_setup.lua)
    int count=0;
    if(argc==2) count=atoi(argv[1]);
    if(count==0) count=5;
    std::cout<<"configured for "<<count<<" conveyor belts" << std::endl;

    // prepare result
    ConsoleOut::G()->Verb(1);
    GeneratorVector gvoi;
    gvoi.Clear();
    EventPriorities prios;

    // read models
    gvoi.Append(Generator("tmp_pev_cbs_src.gen"));
    for(int i = 1;i<=count;i++){
        gvoi.Append(Generator("tmp_pev_cbs_"+ToStringInteger(i)+"_cl.gen"));
    }
    gvoi.Append(Generator("tmp_pev_cbs_snk.gen"));

    // read priorities
    prios.Read("tmp_pev_cbs_prios.alph");

    // do compositional verification
    std::cout<<"starting compositional verification" << std::endl;
    auto start = std::clock();
    bool isnc = IsPNonblocking(gvoi,prios);
    std::cout<<"duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    if(isnc)
      std::cout<<"p-nonconflicting test passed"<<std::endl;
    else
      std::cout<<"p-nonconflicting test failed"<<std::endl;

    // do monolitic verification
    if(count<6) {
      std::cout<<"starting monolitic verification" << std::endl;
      start = std::clock();
      Generator gall;
      gall.StateNamesEnabled(false);
      ParallelNB(gvoi,gall);
      ShapePriorities(gall,prios);
      isnc = IsNonblocking(gall);
      std::cout<<"duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
      if(isnc)
        std::cout<<"p-nonconflicting test passed"<<std::endl;
      else
        std::cout<<"p-nonconflicting test failed"<<std::endl;
    }

    return 0;


}

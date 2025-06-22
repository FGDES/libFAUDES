// @file pev_3_verfair.cpp verification with fairness constraints  

// 1) generate models via "pev_cbs_setup.lua"
// 2) run this executable


#include "libfaudes.h"

using namespace faudes;


int main(int argc, char* argv[]) {


    // read number of conveyor belts from command line
    // (defaults to 5, needs to match models prepared by pev_cbs_setup.lua)
    int count=0;
    if(argc==2) count=atoi(argv[1]);
    if(count==0) count=5;
    std::cout<<"configured for "<<count<<" conveyor belts" << std::endl;
    
    // prepare results
    ConsoleOut::G()->Verb(1);
    FairGeneratorVector gvoi;
    gvoi.Clear();
    EventPriorities prios;
    FairnessConstraints faircons;
    EventSet fair;
    
    // read models
    gvoi.Append(FairGenerator("tmp_pev_cbs_src.gen"));
    for(int i = 1;i<=count;i++){
        gvoi.Append(FairGenerator("tmp_pev_cbs_"+ToStringInteger(i)+"_cl.gen"));
    }
    gvoi.Append(FairGenerator("tmp_pev_cbs_snk.gen"));

    // construct fairness constraints 
    for (int i = 0; i<=count+1; i++){
        faircons.Clear();
        fair.Clear();
        fair.Insert("ar_"+ToStringInteger(i));
        faircons.Append(fair);
        gvoi.At(i).Fairness(faircons);
    }

    // read priorities
    prios.Read("tmp_pev_cbs_prios.alph");

    // do compositional verification
    std::cout<<"starting compositional verification" << std::endl;
    auto start = std::clock();
    bool isnc = IsPFNonblocking(gvoi,prios);
    std::cout<<"duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    if(isnc)
      std::cout<<"p-nonconflicting test passed"<<std::endl;
    else
      std::cout<<"p-nonconflicting test failed"<<std::endl;

    // record test case
    FAUDES_TEST_DUMP("prio comp ver",isnc);

    // done
    return 0;

}

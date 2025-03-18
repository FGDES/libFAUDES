// @file pev_3_verfair.cpp verification with fairness constraints  

// 1) generate models via "pev_cbs_setup.lua"
// 2) run this executable


#include "libfaudes.h"

using namespace faudes;


int main(int argc, char* argv[]) {


    ConsoleOut::G()->Verb(1);
    GeneratorVector gvoi;
    gvoi.Clear();
    EventPriorities prios;
    std::vector<FairnessConstraints> fairvec;
    FairnessConstraints faircons;
    EventSet fair;
    
    // number of conveyor belts
    const int count = 8;

    // read models
    gvoi.Append(Generator("data/pev_cbs_source.gen"));
    for(int i = 1;i<=count;i++){
        gvoi.Append(Generator("data/pev_cbs_"+ToStringInteger(i)+"_cl.gen"));
    }
    gvoi.Append(Generator("data/pev_cbs_sink.gen"));

    // read priorities
    prios.Read("data/pev_cbs_prios.alph");

    // report
    prios.Write();
    
    // construct fairness constraints 
    for (int i = 0; i<=count+1; i++){
        faircons.Clear();
        fair.Clear();
        fair.Insert("ar_"+ToStringInteger(i));
        faircons.Append(fair);
        fairvec.push_back(faircons);
    }

    auto start = std::clock();
    bool isnc = IsPFNonblocking(gvoi,prios,fairvec);
    std::cout<<"Duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    std::cout<<"Is P-Nonconflicting? "<< isnc<<std::endl;
    return 0;

}

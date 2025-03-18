// @file pev_3_verify.cpp verification with std synchronous marking

// 1) generate models via "pev_cbs_setup.lua"
// 2) run this executable

// NOTE: not functional as of 2025 TM

#include "libfaudes.h"

using namespace faudes;



// main
int main(int argc, char* argv[]) {

    ConsoleOut::G()->Verb(1);
    GeneratorVector gvoi;
    gvoi.Clear();
    EventPriorities prios;
    
    // number of conveyor belts (needs to match cbs_setup.lua)
    const int count = 8;

    // read models
    gvoi.Append(Generator("data/pev_cbs_source.gen"));
    for(int i = 1;i<=count;i++){
        gvoi.Append(Generator("data/pev_cbs_"+ToStringInteger(i)+"_cl.gen"));
    }
    gvoi.Append(Generator("data/pev_cbs_sink.gen"));

    // read priorities
    prios.Read("data/pev_cbs_prios.alph");

    // do compositional verification
    auto start = std::clock();
    bool isnc = IsPNonblocking(gvoi,prios);
    std::cout<<"Duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    std::cout<<"Is P-Nonconflicting? "<< isnc<<std::endl;

    // do monolytic verification
    /*
    start = std::clock();
    Generator gall;
    Parallel(gvoi,gall);
    Shape(gall,prios);
    isnc = IsNonblocking(gall);
    std::cout<<"Duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    std::cout<<"Is P-Nonconflicting? "<< isnc<<std::endl;
    */

    return 0;


}

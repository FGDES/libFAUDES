// @file pev_4_versbd.cpp verification of Yiheng's SBD example  



#include "libfaudes.h"

using namespace faudes;


int main(int argc, char* argv[]) {


    // prepare results
    ConsoleOut::G()->Verb(1);
    FairGeneratorVector gvoi;
    EventPriorities prios;
    
    // read priorities
    prios.Read("data/pev_4_prios.alph");

    // read models
    gvoi.Clear();
    gvoi.Append(FairGenerator("data/pev_4_sbd_m12.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_p2.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_take_l2.gen"));
    gvoi.Append(FairGenerator("data/pev_4_one_wpon_cb.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_send2.gen"));
    gvoi.Append(FairGenerator("data/pev_4_g_rbpm_coupl.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_m22.gen")); 
    
    // do compositional verification
    std::cout<<"starting compositional verification" << std::endl;
    auto start = std::clock();
    bool isnc = IsPFNonblocking(gvoi,prios);
    std::cout<<"duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    if(isnc)
      std::cout<<"p-nonconflicting test passed"<<std::endl;
    else
      std::cout<<"p-nonconflicting test failed [testcase ERROR]"<<std::endl;

    // record test case
    FAUDES_TEST_DUMP("prio comp ver",isnc);

    // read models (without "wpon")
    gvoi.Clear();
    gvoi.Append(FairGenerator("data/pev_4_sbd_m12.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_p2.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_take_l2.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_send2.gen"));
    gvoi.Append(FairGenerator("data/pev_4_g_rbpm_coupl.gen"));
    gvoi.Append(FairGenerator("data/pev_4_sbd_m22.gen")); 

    // do compositional verification
    std::cout<<"starting compositional verification" << std::endl;
    start = std::clock();
    bool isnnc = IsPFNonblocking(gvoi,prios);
    std::cout<<"duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    if(isnnc)
      std::cout<<"p-nonconflicting test passed [testcase ERROR]"<<std::endl;
    else
      std::cout<<"p-nonconflicting test failed"<<std::endl;

    // record test case
    FAUDES_TEST_DUMP("prio comp ver",isnnc);    // done
    return 0;

}

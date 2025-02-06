
#include "libfaudes.h"

using namespace faudes;

void ParallelAll(const GeneratorVector& rGvec, Generator& result){
    result = rGvec.At(0);
    Idx git = 1;
    for(;git!=rGvec.Size();git++){
        Parallel(result,rGvec.At(git),result);
    }
}


// parse commandline and invoke per task wrapper
int main(int argc, char* argv[]) {


    ConsoleOut::G()->Verb(1);
    GeneratorVector gvoi;
    gvoi.Clear();
    EventPriorities prios;
    std::vector<Fairness> fairvec;
    Fairness faircons;
    EventSet fair;
    
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

    // programatic priorities
    /*
    prios.Clear();
    Idx id = 0;
    for(;id!=gvoi.Size();id++){
        Generator* cgen = &gvoi.At(id);
        EventSet::Iterator eit = cgen->AlphabetBegin();
        for(;eit!=cgen->AlphabetEnd();eit++){
            std::string name = cgen->EventName(*eit);
            if (name[0]=='s'){ // its a send event sd_*
	      prios.InsPriority(name,0);
            }
            else if(name[0]=='o'){ // its an on/off event
  	        prios.InsPriority(name,1);
            }
            else{
  	        prios.InsPriority(name,2);
            }
        }
    }
    */

    // report
    prios.Write();
    
    // construct fairness  
    for (int i = 0; i<=count+1; i++){
        faircons.clear();
        fair.Clear();
        fair.Insert("ar_"+ToStringInteger(i));
        faircons.insert(fair);
        fairvec.push_back(faircons);
    }

    auto start = std::clock();
    bool isnc = IsPNonblocking(gvoi,prios,&fairvec);
    std::cout<<"Duration in seconds: "<<ToStringFloat((std::clock()-start)/(double) CLOCKS_PER_SEC)<<std::endl;
    std::cout<<"Is P-Nonconflicting? "<< isnc<<std::endl;
    return 0;


}

/** @file hyb_6_robotex.cpp  

Tutorial, hybrid systems plugin. 
This tutorial sets up the patrol robot example and synthesises 
a simple controller. It is used in a WODES and in a JDEDS 
submission.

@ingroup Tutorials 

@include hyb_6_robotex.cpp

*/

#include "libfaudes.h"
#include "hyb_compute.h"

// make the faudes namespace available to our program
using namespace faudes;



/* generate plant model: robot on a grid */
/* option: use relative y-symbols (as opposed to absolute) */
// #define RELY
void robot(int szi, int szj, int mesh, int overlap, int disturbance, LinearHybridAutomaton& harobot, EventSet& uevents) {
  // have input events (comment in/out to configure)
  uevents.Clear();
  uevents.Insert("u_north");
  uevents.Insert("u_east");
  uevents.Insert("u_south");
  uevents.Insert("u_west");
  //uevents.Insert("u_northeast");
  //uevents.Insert("u_southeast");
  //uevents.Insert("u_southwest");
  //uevents.Insert("u_northwest");
  harobot.InsEvents(uevents);
  // support matrix A for 2 dim box
  Matrix A42;
  A42.Zero(4,2);
  A42.At(0,0,-1);
  A42.At(1,0, 1);
  A42.At(2,1,-1);
  A42.At(3,1, 1);
  // generate bloated invariants per cell
  std::vector< Polyhedron > Inv;
  Inv.resize(szi*szj);
  for(int i=0; i< szi; ++i) {
    for(int j=0; j< szj; ++j) {
      std::stringstream name;
      name << "Inv_" << j+1 << "_" << i+1;
      Vector Bij;
      Bij.Zero(4);
      Bij.At(0, -1* (mesh*j - 2*overlap));
      Bij.At(1,     (mesh*(j+1) + 2*overlap));
      Bij.At(2, -1* (mesh*i - 2*overlap));
      Bij.At(3,     (mesh*(i+1) + 2*overlap));
      Polyhedron Invij;
      Invij.AB(A42,Bij);
      Invij.Name(name.str());
      Inv[i*szj+j]=Invij;
    }
  }
  // generate the all-invariant aka continuous state set
  Vector BAll;
  BAll.Zero(4);
  BAll.At(0, -1* (mesh*0 - 2*overlap));
  BAll.At(1,     (mesh*szj + 2*overlap));
  BAll.At(2, -1* (mesh*0 - 2*overlap));
  BAll.At(3,     (mesh*szi + 2*overlap));
  Polyhedron InvAll;
  InvAll.AB(A42,BAll);
  // generate right hand sides
  std::vector< Polyhedron > Rate;
  std::vector< std::string > Direction;
  EventSet::Iterator uit=uevents.Begin();
  EventSet::Iterator uit_end=uevents.End();
  for(;uit!=uit_end;++uit) {
    Vector F;
    F.Zero(2);
    if(uevents.SymbolicName(*uit)=="u_north") {
      Direction.push_back("n");
      F.At(0,0);
      F.At(1,mesh);
    }  
    if(uevents.SymbolicName(*uit)=="u_northeast") {
      Direction.push_back("ne");
      F.At(0,mesh);
      F.At(1,mesh);
    }
    if(uevents.SymbolicName(*uit)=="u_east") {
      Direction.push_back("e");
      F.At(0,mesh);
      F.At(1,0);    
    }
    if(uevents.SymbolicName(*uit)=="u_southeast") {
      Direction.push_back("se");
      F.At(0,mesh);
      F.At(1,-mesh);
    }
    if(uevents.SymbolicName(*uit)=="u_south") {
      Direction.push_back("s");
      F.At(0,0);
      F.At(1,-mesh);    
    }
    if(uevents.SymbolicName(*uit)=="u_southwest") {
      Direction.push_back("sw");
      F.At(0,-mesh);
      F.At(1,-mesh);    
    }
    if(uevents.SymbolicName(*uit)=="u_west") {
      Direction.push_back("w");
      F.At(0,-mesh);
      F.At(1,0);
    }
    if(uevents.SymbolicName(*uit)=="u_northwest") {
      Direction.push_back("nw");
      F.At(0,-mesh);
      F.At(1,mesh);    
    }
    Vector Bd;
    Bd.Zero(4);
    Bd.At(0, -1* (F.At(0) - disturbance));
    Bd.At(1,     (F.At(0) + disturbance));
    Bd.At(2, -1* (F.At(1) - disturbance));
    Bd.At(3,     (F.At(1) + disturbance));
    Polyhedron Rated;
    Rated.Name(Direction.back());
    Rated.AB(A42,Bd);
    Rate.push_back(Rated);
  }
  // generate the non-rate 
  Vector BNone;
  BNone.Zero(4);
  BNone.At(0, -1* 1 );
  BNone.At(1,      -1 );
  BNone.At(2, -1* (1));
  BNone.At(3,      -1);
  Polyhedron RateNone;
  RateNone.AB(A42,BNone);
  // set continuous state set and initial constraint
  harobot.StateSpace(InvAll);
  // set locations q(j,i,d)
  for(int j=0; j< szj; ++j) {
    for(int i=0; i< szi; ++i) {
      for(int d=0; d<Direction.size(); ++d) {
	std::stringstream name;
	name << "q_" << j+1 << "_" << i+1 << "_" << Direction.at(d);
	Idx q = harobot.InsState(name.str());
	// rate by d
	harobot.Rate(q,Rate.at(d));
	// invariant by (j,i)
	harobot.Invariant(q,Inv.at(i*szj + j));       
      }
    }
  }
  // set locations q(j,i,?)
  for(int j=0; j< szj; ++j) {
    for(int i=0; i< szi; ++i) {
      std::stringstream name;
      name << "q_" << j+1 << "_" << i+1 << "_?";
      Idx q = harobot.InsState(name.str());
      // no-rate
      harobot.Rate(q,RateNone);
      // invariant by (j,i)
      harobot.Invariant(q,Inv.at(i*szj + j));
      // record initialstate (right upper)
      /*
      if((j+1==szj) && (i+1==szi)) {
        harobot.InsInitState(q);
        harobot.InitialConstraint(q,Inv.at(i*szj + j));
      }
      */
    }
  }  
  // set transitions from q(j,i,?)
  for(int j=0; j< szj; ++j) {
    for(int i=0; i< szi; ++i) {
      std::stringstream name1;
      name1 << "q_" << j+1 << "_" << i+1 << "_?";
      Idx q1 = harobot.StateIndex(name1.str());
      Idx q2,ev;
      std::stringstream name2;
      // north
      name2 << "q_" << j+1 << "_" << i+1 << "_n";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_north");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
      // northeast
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_ne";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_northeast");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
      // east
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_e";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_east");
      if(ev) harobot.SetTransition(q1,ev,q2);      
      // southeast
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_s";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_southeast");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
      // south
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_s";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_south");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
      // southwest
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_sw";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_southwest");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
      // west
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_w";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_west");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
      // northwest
      name2.str("");
      name2 << "q_" << j+1 << "_" << i+1 << "_nw";
      q2 = harobot.StateIndex(name2.str());
      ev=harobot.EventIndex("u_northwest");
      if(harobot.ExistsEvent(ev)) harobot.SetTransition(q1,ev,q2);      
    }
  }
#ifndef RELY  
  // have y-events
  for(int j=0; j< szj; ++j) {
    for(int i=0; i< szi; ++i) {
      std::stringstream nameev;
      if((j+1==szj) && (i+1 ==szi))
	nameev << "y_A";	
      else if((j+1==1) && (i+1 ==1))
	nameev << "y_B";	
      else 
        nameev << "y_" << j+1 << "_" << i+1;
      harobot.InsEvent(nameev.str());
    }
  }
#else
  // have y-events
  harobot.InsEvent("y_n");
  harobot.InsEvent("y_e");
  harobot.InsEvent("y_s");
  harobot.InsEvent("y_w");
  harobot.InsEvent("y_A");
  harobot.InsEvent("y_B");
#endif  
  // set transitions from q(j,i,!)
  for(int j=0; j< szj; ++j) {
    for(int i=0; i< szi; ++i) {
      for(int d=0; d<Direction.size(); ++d) {
	std::stringstream name1;
	name1 << "q_" << j+1 << "_" << i+1 << "_" << Direction.at(d);
        Idx q1 = harobot.StateIndex(name1.str());
        Idx q2,ev;
        std::stringstream name2;
        std::stringstream nameev;
        Vector Bgrd;
        Polyhedron Grd;
	// north neighbour
	name2.str("");
	name2 << "q_" << j+1 << "_" << i+1+1 << "_?";
	q2 = harobot.StateIndex(name2.str());
	nameev.str("");
#ifndef RELY  
	nameev << "y_" << j+1 << "_" << i+1+1;
#else
	nameev << "y_n";
#endif	
	if((j+1==1) && (i+1+1 ==1)) {
	  nameev.str("");
  	  nameev << "y_B";	
	}
	if((j+1==szj) && (i+1+1==szi)) {
	  nameev.str("");
  	  nameev << "y_A";	
	}
	ev = harobot.EventIndex(nameev.str());
	if((q2!=0) && (ev!=0)) {
	  // transition
	  harobot.SetTransition(q1,ev,q2);
	  // north guard
          Bgrd.Zero(4);
          Bgrd.At(0, -1* (mesh*j - 2*overlap));
          Bgrd.At(1,     (mesh*(j+1) + 2*overlap));
          Bgrd.At(2, -1* (mesh*(i+1) + 1*overlap));
          Bgrd.At(3,     (mesh*(i+1) + 2*overlap));
          Grd.AB(A42,Bgrd);
	  harobot.Guard(Transition(q1,ev,q2),Grd);
	}
 	// east neighbour
	name2.str("");
	name2 << "q_" << j+1+1 << "_" << i+1 << "_?";
	q2 = harobot.StateIndex(name2.str());
	nameev.str("");
#ifndef RELY  
	nameev << "y_" << j+1+1 << "_" << i+1;
#else
	nameev << "y_e";
#endif	
	if((j+1+1==1) && (i+1==1)) {
	  nameev.str("");
  	  nameev << "y_B";	
	}
	if((j+1+1==szj) && (i+1==szi)) {
	  nameev.str("");
  	  nameev << "y_A";	
	}
	ev = harobot.EventIndex(nameev.str());
	if((q2!=0) && (ev!=0)) {
	  // transition
	  harobot.SetTransition(q1,ev,q2);
	  // east guard
          Bgrd.Zero(4);
          Bgrd.At(0, -1* (mesh*(j+1) + 1*overlap));
          Bgrd.At(1,     (mesh*(j+1) + 2*overlap));
          Bgrd.At(2, -1* (mesh*(i) - 2*overlap));
          Bgrd.At(3,     (mesh*(i+1) + 2*overlap));
          Grd.AB(A42,Bgrd);
	  harobot.Guard(Transition(q1,ev,q2),Grd);
	}
 	// south neighbour
	name2.str("");
	name2 << "q_" << j+1 << "_" << i+1-1 << "_?";
	q2 = harobot.StateIndex(name2.str());
	nameev.str("");
#ifndef RELY  
	nameev << "y_" << j+1 << "_" << i+1-1;
#else
	nameev << "y_s";
#endif	
	if((j+1==1) && (i+1-1 ==1)) {
	  nameev.str("");
  	  nameev << "y_B";	
	}
	if((j+1==szj) && (i+1-1==szi)) {
	  nameev.str("");
  	  nameev << "y_A";	
	}
	ev = harobot.EventIndex(nameev.str());
	if((q2!=0) && (ev!=0)) {
	  // transition
	  harobot.SetTransition(q1,ev,q2);
	  // south guard
          Bgrd.Zero(4);
          Bgrd.At(0, -1* (mesh*j - 2*overlap));
          Bgrd.At(1,     (mesh*(j+1) + 2*overlap));
          Bgrd.At(2, -1* (mesh*(i) - 2*overlap));
          Bgrd.At(3,     (mesh*(i) - 1*overlap));
          Grd.AB(A42,Bgrd);
	  harobot.Guard(Transition(q1,ev,q2),Grd);
	}
 	// west neighbour
	name2.str("");
	name2 << "q_" << j+1-1 << "_" << i+1 << "_?";
	q2 = harobot.StateIndex(name2.str());
	nameev.str("");
#ifndef RELY  
	nameev << "y_" << j+1-1 << "_" << i+1;
#else
	nameev << "y_w";
#endif	
	if((j+1-1==1) && (i+1 ==1)) {
	  nameev.str("");
  	  nameev << "y_B";	
	}
	if((j+1-1==szj) && (i+1==szi)) {
	  nameev.str("");
  	  nameev << "y_A";	
	}
	ev = harobot.EventIndex(nameev.str());
	if((q2!=0) && (ev!=0)) {
	  // transition
	  harobot.SetTransition(q1,ev,q2);
	  // west guard
          Bgrd.Zero(4);
          Bgrd.At(0, -1* (mesh*(j) - 2*overlap));
          Bgrd.At(1,     (mesh*(j) - 1*overlap));
          Bgrd.At(2, -1* (mesh*(i) - 2*overlap));
          Bgrd.At(3,     (mesh*(i+1) + 2*overlap));
          Grd.AB(A42,Bgrd);
	  harobot.Guard(Transition(q1,ev,q2),Grd);
	}
      }
    }
  }
  // optional: unconstraint version
  StateSet::Iterator sit= harobot.StatesBegin();
  StateSet::Iterator sit_end= harobot.StatesEnd();
  for(;sit!=sit_end;++sit) {
    harobot.InsInitState(*sit);
    harobot.InitialConstraint(*sit,harobot.Invariant(*sit));
  }
}


/* generate plant model: time invariant marine vehicle within a rectangualr area */
void marine(int width, int height, int overlap, int velocity, int disturbance, LinearHybridAutomaton& hamarine, EventSet& uevents) {
  // have input events (comment in/out to configure)
  uevents.Clear();
  uevents.Insert("u_northeast");
  uevents.Insert("u_southeast");
  uevents.Insert("u_southwest");
  uevents.Insert("u_northwest");
  hamarine.InsEvents(uevents);
  // support matrix A for 2 dim box
  Matrix A42;
  A42.Zero(4,2);
  A42.At(0,0,-1);
  A42.At(1,0, 1);
  A42.At(2,1,-1);
  A42.At(3,1, 1);
  // generate the all-invariant aka continuous state set
  Vector BAll;
  BAll.Zero(4);
  BAll.At(0, -1* 0     );
  BAll.At(1,     width );
  BAll.At(2, -1* 0     );
  BAll.At(3,     height);
  Polyhedron InvAll;
  InvAll.AB(A42,BAll);
  // generate right hand sides
  std::vector< Polyhedron > Rate;
  std::vector< std::string > Direction;
  EventSet::Iterator uit=uevents.Begin();
  EventSet::Iterator uit_end=uevents.End();
  for(;uit!=uit_end;++uit) {
    Vector F;
    F.Zero(2);
    if(uevents.SymbolicName(*uit)=="u_north") {
      Direction.push_back("n");
      F.At(0,0);
      F.At(1,velocity);
    }  
    if(uevents.SymbolicName(*uit)=="u_northeast") {
      Direction.push_back("ne");
      F.At(0,velocity);
      F.At(1,velocity);
    }
    if(uevents.SymbolicName(*uit)=="u_east") {
      Direction.push_back("e");
      F.At(0,velocity);
      F.At(1,0);    
    }
    if(uevents.SymbolicName(*uit)=="u_southeast") {
      Direction.push_back("se");
      F.At(0,velocity);
      F.At(1,-velocity);
    }
    if(uevents.SymbolicName(*uit)=="u_south") {
      Direction.push_back("s");
      F.At(0,0);
      F.At(1,-velocity);    
    }
    if(uevents.SymbolicName(*uit)=="u_southwest") {
      Direction.push_back("sw");
      F.At(0,-velocity);
      F.At(1,-velocity);    
    }
    if(uevents.SymbolicName(*uit)=="u_west") {
      Direction.push_back("w");
      F.At(0,-velocity);
      F.At(1,0);
    }
    if(uevents.SymbolicName(*uit)=="u_northwest") {
      Direction.push_back("nw");
      F.At(0,-velocity);
      F.At(1,velocity);    
    }
    Vector Bd;
    Bd.Zero(4);
    Bd.At(0, -1* (F.At(0) - disturbance));
    Bd.At(1,     (F.At(0) + disturbance));
    Bd.At(2, -1* (F.At(1) - disturbance));
    Bd.At(3,     (F.At(1) + disturbance));
    Polyhedron Rated;
    Rated.Name(Direction.back());
    Rated.AB(A42,Bd);
    Rate.push_back(Rated);
  }
  // generate the non-rate 
  Vector BNone;
  BNone.Zero(4);
  BNone.At(0, -1* 1 );
  BNone.At(1,      -1 );
  BNone.At(2, -1* (1));
  BNone.At(3,      -1);
  Polyhedron RateNone;
  RateNone.AB(A42,BNone);
  // set continuous state set and initial constraint
  hamarine.StateSpace(InvAll);
  // set locations q(d)
  for(int d=0; d<Direction.size(); ++d) {
    std::stringstream name;
    name << "q_" << Direction.at(d);
    Idx q = hamarine.InsState(name.str());
    // rate by d
    hamarine.Rate(q,Rate.at(d));
    // invariant by (j,i)
    hamarine.Invariant(q,InvAll);       
  }
  // set location q(?)
  std::string nameqq="q_?";
  Idx qq = hamarine.InsInitState(nameqq);
  hamarine.Rate(qq,RateNone);
  hamarine.Invariant(qq,InvAll);
  // set transitions from q(?)
  uit=uevents.Begin();
  uit_end=uevents.End();
  for(;uit!=uit_end;++uit) {
    Idx ev=*uit;
    Idx qn;
    std::string nameqn;
    if(uevents.SymbolicName(*uit)=="u_northeast") 
      nameqn="q_ne";
    if(uevents.SymbolicName(*uit)=="u_northwest") 
      nameqn="q_nw";
    if(uevents.SymbolicName(*uit)=="u_southwest") 
      nameqn="q_sw";
    if(uevents.SymbolicName(*uit)=="u_southeast") 
      nameqn="q_se";
    if(nameqn=="") 
      std::cout << "Setting up model: INTERNAL ERRROR A\n"; 
    qn=hamarine.StateIndex(nameqn);
    hamarine.SetTransition(qq,*uit,qn);
  }  
  // have y-events
  hamarine.InsEvent("y_n");
  hamarine.InsEvent("y_e");
  hamarine.InsEvent("y_s");
  hamarine.InsEvent("y_w");
  // set transitions from q(!)
  for(int d=0; d<Direction.size(); ++d) {
    std::string namen = "q_" + Direction.at(d);
    Idx qn=hamarine.StateIndex(namen);
    Vector Bgrd;
    Polyhedron Grd;
    Idx ev;
    std::string nameev;
    // north boundary
    if((Direction.at(d)=="ne") || (Direction.at(d)=="nw")) {
      nameev = "y_n";
      ev = hamarine.EventIndex(nameev);
      hamarine.SetTransition(qn,ev,qq);
      // north guard
      Bgrd.Zero(4);
      Bgrd.At(0, -1* (0               ));
      Bgrd.At(1,     (width           ));
      Bgrd.At(2, -1* (height - overlap));
      Bgrd.At(3,     (height          ));
      Grd.AB(A42,Bgrd);
      hamarine.Guard(Transition(qn,ev,qq),Grd);
    }
    // south boundary
    if((Direction.at(d)=="se") || (Direction.at(d)=="sw")) {
      nameev = "y_s";
      ev = hamarine.EventIndex(nameev);
      hamarine.SetTransition(qn,ev,qq);
      // south guard
      Bgrd.Zero(4);
      Bgrd.At(0, -1* (0               ));
      Bgrd.At(1,     (width           ));
      Bgrd.At(2, -1* (0               ));
      Bgrd.At(3,     (overlap         ));
      Grd.AB(A42,Bgrd);
      hamarine.Guard(Transition(qn,ev,qq),Grd);
    }  
    // west boundary
    if((Direction.at(d)=="nw") || (Direction.at(d)=="sw")) {
      nameev = "y_w";
      ev = hamarine.EventIndex(nameev);
      hamarine.SetTransition(qn,ev,qq);
      // west guard
      Bgrd.Zero(4);
      Bgrd.At(0, -1* (0               ));
      Bgrd.At(1,     (overlap         ));
      Bgrd.At(2, -1* (0               ));
      Bgrd.At(3,     (height          ));
      Grd.AB(A42,Bgrd);
      hamarine.Guard(Transition(qn,ev,qq),Grd);
    }  
    // east boundary
    if((Direction.at(d)=="ne") || (Direction.at(d)=="se")) {
      nameev = "y_e"; 
      ev = hamarine.EventIndex(nameev);
      hamarine.SetTransition(qn,ev,qq);
      // east guard
      Bgrd.Zero(4);
      Bgrd.At(0, -1* (width-overlap   ));
      Bgrd.At(1,     (width           ));
      Bgrd.At(2, -1* (0               ));
      Bgrd.At(3,     (height          ));
      Grd.AB(A42,Bgrd);
      hamarine.Guard(Transition(qn,ev,qq),Grd);
    }  
  }
  // optional: still unconstraint, but must start with u-event
  hamarine.InitialConstraint(qq,InvAll);
}

/* excute interactively */
int execute(LinearHybridAutomaton& plant, HybridStateSet& cstates) {
  
    // compute reachable states per successor event
    std::map< Idx, HybridStateSet* > sstatespe;
    LhaReach(plant,cstates,sstatespe);

    // show current state    
    std::cout << "################################\n";
    std::cout << "# current state set \n";
    cstates.DWrite(plant);
    std::cout << "################################\n";
    
    // show mode transitions    
    std::cout << "################################\n";
    std::cout << "# mode transitions \n";
    IndexSet::Iterator qit=cstates.LocationsBegin();
    IndexSet::Iterator qit_end=cstates.LocationsEnd();
    for(;qit!=qit_end;++qit){
      TransSet::Iterator tit=plant.TransRelBegin(*qit);
      TransSet::Iterator tit_end=plant.TransRelEnd(*qit);
      for(;tit!=tit_end;++tit)
	std::cout << plant.TStr(*tit) << "\n";
    }
    std::cout << "################################\n";
    

    // show next events    
    std::cout << "################################\n";
    std::cout << "# successor events \n";
    std::map< Idx, HybridStateSet* >::iterator sit=sstatespe.begin();
    std::map< Idx, HybridStateSet* >::iterator sit_end=sstatespe.end();
    EventSet enabled;
    for(;sit!=sit_end;++sit) {
      enabled.Insert(sit->first);
      std::cout << plant.EStr(sit->first) << " ";
      //std::cout << "\n# successor states for " << plant.EStr(sit->first) << "\n";
      //sit->second->DWrite(plant);
    }
    std::cout << "\n################################\n";

    // interactive loop until accept
    while(true) {
    
      // choose event
      Idx ev;
      while(true) {
        std::cout << "choose event (or 'x' for exit): ";
        std::string choice;
        std::cin>>choice;
        if(choice=="x") return 0;
        ev=ToIdx(choice);
        if(enabled.Exists(ev)) break;
        ev=plant.EventIndex(choice);
        if(enabled.Exists(ev)) break;
      }
      sit=sstatespe.find(ev);

      // dump successor states
      std::cout << "################################\n";
      std::cout << "# successor states for " << plant.EStr(sit->first) << "\n";
      sit->second->DWrite(plant);
      std::cout << "################################\n";

      // accept choice
      std::cout << "accept ('n' for no, other for yes): ";
      std::string choice;
      std::cin>>choice;
      if(choice!="n") break;

    } // end interactive

    // update states
    cstates.Assign(*sit->second);

    // report event
    return sit->first;
}


/** Run the tutorial: time variant robot, target control */
int robot2017() {

  /** generate plant */
  LinearHybridAutomaton lioha;
  EventSet uevents;
  robot(3 /* rows */, 5 /*cols*/ , 10 /* mesh */, 1 /* overlap */, 1 /* disturbance */, lioha, uevents);
  
  // report to console
  /*
  std::cout << "################################\n";
  std::cout << "# linear hybrid automaton: \n";
  lioha.Write();
  std::cout << "################################\n";
  std::cout << "# Valid() returns " << lioha.Valid() << "\n";
  std::cout << "################################\n";
  */
  
  // get initial state
  HybridStateSet istates;
  StateSet::Iterator qit=lioha.InitStatesBegin();
  StateSet::Iterator qit_end=lioha.InitStatesEnd();
  for(;qit!=qit_end;++qit){
    Polyhedron* poly = new Polyhedron(lioha.InitialConstraint(*qit));
    PolyIntersection(lioha.StateSpace(),*poly);
    PolyFinalise(*poly);
    istates.Insert(*qit,poly);
  }

  /*
  std::cout << "################################\n";
  std::cout << "# dumping initial states\n";
  istates.DWrite(lioha);    
  std::cout << "################################\n";
  */
  
  // run interactively for manual inspection
  HybridStateSet cstates;
  cstates.Assign(istates);
  while(true) {
    Idx ev=execute(lioha,cstates);
    if(ev==0) break;
  }


  // prepare abstraction
  int theL=2;
  LhaCompatibleStates* comp;
  Experiment* exp;
  
  // do l-complete abstraction from scratch (time variant)
  std::cout << "################################\n";
  std::cout << "# compute l-complete approximation, time variant \n";
  std::cout << "################################\n";
  comp = new LhaCompatibleStates(lioha);
  comp->InitialiseConstraint();
  exp = new Experiment(lioha.Alphabet());
  exp->InitialStates(comp);
  LbdAbstraction tvabs;
  tvabs.Experiment(exp);
  tvabs.RefineUniformly(theL);
 
  // Do l-complete abstraction from scratch (time invariant)
  std::cout << "################################\n";
  std::cout << "# compute l-complete approximation, time invariant \n";
  std::cout << "################################\n";
  comp = new LhaCompatibleStates(lioha);
  comp->InitialiseFull();
  exp = new Experiment(lioha.Alphabet());
  exp->InitialStates(comp);
  LbdAbstraction tivabs;
  tivabs.Experiment(exp);
  tivabs.RefineUniformly(theL);
 
  // loop refine
  while(true) {

  // compose both for finest result
  std::cout << "################################\n";
  std::cout << "# compose abstraction \n";
  std::cout << "################################\n";
  tvabs.Experiment().SWrite();
  tivabs.Experiment().SWrite();
  tvabs.TvAbstraction().SWrite();
  tivabs.TivAbstraction().SWrite();
  Generator abst;
  abst.StateNamesEnabled(false);
  ProductCompositionMap cmap_tv_tiv;
  aProduct(tvabs.TvAbstraction(),tivabs.TivAbstraction(),cmap_tv_tiv,abst);
  //abst=tivabs.TivAbstraction();
  MarkAllStates(abst);

  
  // report statistics
  abst.SWrite();
  //abst.GraphWrite("tmp_lv4.png");

  std::cout << "################################\n";
  std::cout << "# target control synthesis \n";
  std::cout << "# ctrl evs " << uevents.ToString() << "\n";
  std::cout << "################################\n";
  
  // target specification by y
  Generator spec;
  spec.InjectAlphabet(lioha.Alphabet());
  Idx s0=spec.InsInitState();
  Idx st=spec.InsMarkedState();
  spec.SetTransition(s0,spec.EventIndex("y_target"),st);
  spec.SetTransition(st,spec.EventIndex("y_target"),st);
  EventSet sfloop=lioha.Alphabet();
  sfloop.Erase("y_target");
  //sfloop.Erase("u_east");
  SelfLoop(spec,sfloop);
 
  /* 
  // target specification by xtarget
  Generator spec=abst;
  spec.ClearMarkedStates();
  StateSet::Iterator csit=abst.StatesBegin();
  StateSet::Iterator csit_end=abst.StatesEnd();
  for(;csit!=csit_end;++csit) {
    Idx qabstv = cmap_tv_tiv.Arg1State(*csit);
    bool ok_tv=true;
    const LhaCompatibleStates* pHstates = dynamic_cast<const LhaCompatibleStates*>(tvabs.Experiment().States(qabstv));
    if(!pHstates) 
      throw Exception("hyb_6_robotex(..)", "incompatible cstates type A", 90);
    IndexSet::Iterator lit = pHstates->States()->LocationsBegin();
    IndexSet::Iterator lit_end = pHstates->States()->LocationsEnd();
    for(;lit!=lit_end;++lit) {
      HybridStateSet::Iterator sit = pHstates->States()->StatesBegin(*lit);
      HybridStateSet::Iterator sit_end = pHstates->States()->StatesEnd(*lit);
      for(;sit!=sit_end;++sit) 
        if(!PolyInclusion(**sit,xtarget)) { ok_tv=false;  break;}
    }
    Idx qabstiv = cmap_tv_tiv.Arg2State(*csit);
    bool ok_tiv=true;
    pHstates = dynamic_cast<const LhaCompatibleStates*>(tivabs.Experiment().States(qabstiv));
    if(!pHstates) 
      throw Exception("hyb_6_robotex(..)", "incompatible cstates type B", 90);
    lit = pHstates->States()->LocationsBegin();
    lit_end = pHstates->States()->LocationsEnd();
    for(;lit!=lit_end;++lit) {
      HybridStateSet::Iterator sit = pHstates->States()->StatesBegin(*lit);
      HybridStateSet::Iterator sit_end = pHstates->States()->StatesEnd(*lit);
      for(;sit!=sit_end;++sit) 
        if(!PolyInclusion(**sit,xtarget)) { ok_tiv=false;  break;}
    }
    if(ok_tiv || ok_tv) {
      spec.InsMarkedState(*csit);
    }
  }
  std::cout << "# found #" << spec.MarkedStates().Size() << " good states\n";  
  */
  
  // target control: reach marked state
  Generator loop;
  ProductCompositionMap cmap_abst_spec;
  aProduct(abst,spec,cmap_abst_spec,loop);
  StateSet good=loop.MarkedStates();
  while(true) {
    int gsz=good.Size();
    std::cout << "target control: #" << gsz << " / #" << loop.Size() << "\n";
    StateSet::Iterator sit=loop.StatesBegin();
    StateSet::Iterator sit_end=loop.StatesEnd();
    for(;sit!=sit_end;++sit) {
      if(good.Exists(*sit)) continue;
      bool ok=false;
      TransSet::Iterator tit=loop.TransRelBegin(*sit);
      TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
      for(;tit!=tit_end;++tit) {
        if(good.Exists(tit->X2)) {ok=true; continue;}
	if(uevents.Exists(tit->Ev)) continue;
	ok=false;
	break;
      }
      if(ok) good.Insert(*sit);
    }
    if(good.Size()==gsz) break;
  }
  std::cout << "target control: #" << good.Size() << " / #" << loop.Size() << "\n";
  bool success=good.Exists(loop.InitState());
  if(success) 
    std::cout << "target control: success (init state #" << loop.InitState() << " found to be good)\n";
  else
    std::cout << "target control: FAIL\n";
  if(success) {
    return 0;
  }

  // diagnose failure
  StateSet tvleaves;
  StateSet tivleaves;
  StateSet::Iterator sit=loop.StatesBegin();
  StateSet::Iterator sit_end=loop.StatesEnd();
  for(;sit!=sit_end;++sit) {
    // ok done
    if(good.Exists(*sit)) continue;
    // sort states
    Idx qabs = cmap_abst_spec.Arg1State(*sit);
    Idx qtvabs = cmap_tv_tiv.Arg1State(qabs);
    Idx qtivabs = cmap_tv_tiv.Arg2State(qabs);
    // allways refine
    if(tivabs.Experiment().IsLeave(qtivabs)) tivleaves.Insert(qtivabs);
    if(tvabs.Experiment().IsLeave(qtvabs)) tvleaves.Insert(qtvabs); 
    TransSet::Iterator tit=loop.TransRelBegin(*sit);
    TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit) {
      if(good.Exists(tit->X2)) break;
      TransSet::Iterator tit2=loop.TransRelBegin(tit->X2);
      TransSet::Iterator tit2_end=loop.TransRelEnd(tit->X2);
      for(;tit2!=tit2_end;++tit2) {
        if(good.Exists(tit2->X2)) break;
      }
      if(tit2!=tit2_end) break;
    }
    if(tit==tit_end) continue;
    // refine on on chance for success 
    if(tivabs.Experiment().IsLeave(qtivabs)) tivleaves.Insert(qtivabs);
    if(tvabs.Experiment().IsLeave(qtvabs)) tvleaves.Insert(qtvabs); 
  }
  std::cout << "################################\n";
  std::cout << "# diagnosis\n";
  std::cout << "# tv-leaves to refine #" << tvleaves.Size() << "/" << tvabs.Experiment().Leaves().Size() << "\n";
  std::cout << "# tiv-leaves to refine #" << tivleaves.Size() << "/" << tivabs.Experiment().Leaves().Size() << "\n";
  std::cout << "################################\n";


  // do refine
  std::cout << "################################\n";
  std::cout << "# refine\n";
  std::cout << "################################\n";
  sit=tvleaves.Begin();
  sit_end=tvleaves.End();
  for(;sit!=sit_end;++sit) {
    tvabs.RefineAt(*sit);
  }  
  sit=tivleaves.Begin();
  sit_end=tivleaves.End();
  for(;sit!=sit_end;++sit) {
    tivabs.RefineAt(*sit);
  }

  // loop refine/synthesis
  }
  

  // done
  return 0;
}


/** Run the tutorial: time invariant robot */
int robot2018ti() {

  /** generate plant */
  LinearHybridAutomaton lioha;
  EventSet uevents;
  robot(3 /* rows */, 5 /*cols*/ , 10 /* mesh */, 1 /* overlap */, 1 /* disturbance */, lioha, uevents);
  
  // report to console
  /*
  std::cout << "################################\n";
  std::cout << "# linear hybrid automaton: \n";
  lioha.Write();
  std::cout << "################################\n";
  std::cout << "# Valid() returns " << lioha.Valid() << "\n";
  std::cout << "################################\n";
  */
  
  // get initial state
  HybridStateSet istates;
  StateSet::Iterator qit=lioha.InitStatesBegin();
  StateSet::Iterator qit_end=lioha.InitStatesEnd();
  for(;qit!=qit_end;++qit){
    Polyhedron* poly = new Polyhedron(lioha.InitialConstraint(*qit));
    PolyIntersection(lioha.StateSpace(),*poly);
    PolyFinalise(*poly);
    istates.Insert(*qit,poly);
  }

  /*
  std::cout << "################################\n";
  std::cout << "# dumping initial states\n";
  istates.DWrite(lioha);    
  std::cout << "################################\n";
  */
  
  // run interactively for manual inspection
  HybridStateSet cstates;
  cstates.Assign(istates);
  while(true) {
    Idx ev=execute(lioha,cstates);
    if(ev==0) break;
  }


  // prepare abstraction
  int theL=2;
  LhaCompatibleStates* comp;
  Experiment* exp;
  
  // Do l-complete abstraction from scratch (time invariant)
  std::cout << "################################\n";
  std::cout << "# compute l-complete approximation, time invariant \n";
  std::cout << "################################\n";
  comp = new LhaCompatibleStates(lioha);
  comp->InitialiseFull();
  exp = new Experiment(lioha.Alphabet());
  exp->InitialStates(comp);
  LbdAbstraction tivabs;
  tivabs.Experiment(exp);
  tivabs.RefineUniformly(theL);
 
  // loop refine
  while(true) {

  // copy abstraction
  Generator abst=tivabs.TivAbstraction();
  abst.StateNamesEnabled(false);
  MarkAllStates(abst);
  abst.Name("overall abstraction");

  // report statistics
  tivabs.Experiment().SWrite();
  abst.Write("tmp_abs2.gen");

  std::cout << "################################\n";
  std::cout << "# cycle control synthesis \n";
  std::cout << "# ctrl evs " << uevents.ToString() << "\n";
  std::cout << "################################\n";
  
  // A/B target specification by y
  Generator spec;
  spec.InjectAlphabet(lioha.Alphabet());
  Idx sga=spec.InsInitState();
  Idx sgb=spec.InsState();
  Idx ssa=spec.InsMarkedState();
  Idx ssb=spec.InsMarkedState();
  spec.SetTransition(sga,spec.EventIndex("y_A"),ssa);
  spec.SetTransition(sga,spec.EventIndex("y_B"),sga);
  spec.SetTransition(ssa,spec.EventIndex("y_A"),sgb);
  spec.SetTransition(ssa,spec.EventIndex("y_B"),ssb);
  spec.SetTransition(sgb,spec.EventIndex("y_B"),ssb);
  spec.SetTransition(sgb,spec.EventIndex("y_A"),sgb);
  spec.SetTransition(ssb,spec.EventIndex("y_B"),sga);
  spec.SetTransition(ssb,spec.EventIndex("y_A"),ssa);
  EventSet other=lioha.Alphabet();
  other.Erase("y_A");
  other.Erase("y_B");
  EventSet::Iterator eit=other.Begin();
  for(;eit!=other.End();++eit) {
    spec.SetTransition(sga,*eit,sga);
    spec.SetTransition(ssa,*eit,sgb);
    spec.SetTransition(sgb,*eit,sgb);
    spec.SetTransition(ssb,*eit,sga);
  }  
  spec.GraphWrite("tmp_spec.png");

  // target control: reach marked state inf often
  Generator loop;
  ProductCompositionMap cmap_abst_spec;
  aProduct(abst,spec,cmap_abst_spec,loop);
  StateSet constraint = loop.States();
  StateSet winning;

  while(true) {
    StateSet target = loop.MarkedStates();
    target.RestrictSet(constraint);
    int csz=constraint.Size();
    int tsz=target.Size();
    std::cout << "target control: constraint #" << csz << " target #" << tsz << "\n";
    winning.Clear();

  while(true) {
    int wsz=winning.Size();
    std::cout << "target control: #" << wsz << " / #" << loop.Size() << "\n";
    StateSet ctrlreach;
    StateSet::Iterator sit=loop.StatesBegin();
    StateSet::Iterator sit_end=loop.StatesEnd();
    for(;sit!=sit_end;++sit) {
      if(winning.Exists(*sit)) continue;
      bool ok=false;
      TransSet::Iterator tit=loop.TransRelBegin(*sit);
      TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
      for(;tit!=tit_end;++tit) {
        if(winning.Exists(tit->X2)) {ok=true; continue;}
        if(target.Exists(tit->X2)) {ok=true; continue;}
	if(uevents.Exists(tit->Ev)) continue;
	ok=false;
	break;
      }
      if(ok) ctrlreach.Insert(*sit);
    }
    if(ctrlreach <= winning) break;
    winning.InsertSet(ctrlreach);
  }

  constraint.RestrictSet(winning);
  if(csz==constraint.Size()) break;
  }
  
  std::cout << "target control: #" << winning.Size() << " / #" << loop.Size() << "\n";
  bool success=winning.Exists(loop.InitState());
  if(success) 
    std::cout << "target control: success (init state #" << loop.InitState() << " found to be winning)\n";
  else
    std::cout << "target control: FAIL\n";
  if(success) {
    return 0;
  }

  // diagnose failure
  StateSet tvleaves;
  StateSet tivleaves;
  StateSet::Iterator sit=loop.StatesBegin();
  StateSet::Iterator sit_end=loop.StatesEnd();
  for(;sit!=sit_end;++sit) {
    // ok done
    if(winning.Exists(*sit)) continue;
    // sort states
    Idx qtivabs = cmap_abst_spec.Arg1State(*sit);
    // allways refine
    if(tivabs.Experiment().IsLeave(qtivabs)) tivleaves.Insert(qtivabs);
    // is there a chance to win?
    TransSet::Iterator tit=loop.TransRelBegin(*sit);
    TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit) {
      if(winning.Exists(tit->X2)) break;
      TransSet::Iterator tit2=loop.TransRelBegin(tit->X2);
      TransSet::Iterator tit2_end=loop.TransRelEnd(tit->X2);
      for(;tit2!=tit2_end;++tit2) {
        if(winning.Exists(tit2->X2)) break;
      }
      if(tit2!=tit2_end) break;
    }
    if(tit==tit_end) continue;
    // refine on a chance for success 
    if(tivabs.Experiment().IsLeave(qtivabs)) tivleaves.Insert(qtivabs);
  }
  std::cout << "################################\n";
  std::cout << "# diagnosis\n";
  std::cout << "# tiv-leaves to refine #" << tivleaves.Size() << "/" << tivabs.Experiment().Leaves().Size() << "\n";
  std::cout << "################################\n";


  // do refine
  std::cout << "################################\n";
  std::cout << "# refine\n";
  std::cout << "################################\n";
  sit=tivleaves.Begin();
  sit_end=tivleaves.End();
  for(;sit!=sit_end;++sit) {
    tivabs.RefineAt(*sit);
  }

  // loop refine/synthesis
  }
  

  // done
  return 0;
}


/** Run the tutorial: time invariant marine */
int marine2018() {

  /** generate plant */
  LinearHybridAutomaton lioha;
  EventSet uevents;
  marine(30 /* width */, 10 /*height*/ , 1 /*overlap*/, 10 /*velo*/, 1 /* disturbance */, lioha, uevents);
  EventSet yevents = lioha.Alphabet();
  yevents.EraseSet(uevents);
  
  // report to console
  std::cout << "################################\n";
  std::cout << "# linear hybrid automaton: \n";
  lioha.Write();
  std::cout << "################################\n";
  std::cout << "# Valid() returns " << lioha.Valid() << "\n";
  std::cout << "################################\n";

  // get initial state
  HybridStateSet istates;
  StateSet::Iterator qit=lioha.InitStatesBegin();
  StateSet::Iterator qit_end=lioha.InitStatesEnd();
  for(;qit!=qit_end;++qit){
    Polyhedron* poly = new Polyhedron(lioha.InitialConstraint(*qit));
    PolyIntersection(lioha.StateSpace(),*poly);
    PolyFinalise(*poly);
    istates.Insert(*qit,poly);
  }

  // run interactively for manual inspection
  HybridStateSet cstates;
  cstates.Assign(istates);
  while(true) {
    Idx ev=execute(lioha,cstates);
    if(ev==0) break;
  }

  // prepare abstraction
  int theL=2;
  LhaCompatibleStates* comp;
  Experiment* exp;
  
  // Do l-complete abstraction from scratch (time invariant)
  std::cout << "################################\n";
  std::cout << "# compute l-complete approximation, time invariant \n";
  std::cout << "################################\n";
  comp = new LhaCompatibleStates(lioha);
  comp->InitialiseConstraint(); // still time invariant but start with u-event
  exp = new Experiment(lioha.Alphabet());
  exp->InitialStates(comp);
  LbdAbstraction tivabs;
  tivabs.Experiment(exp);
  tivabs.RefineUniformly(theL);
  //tivabs.Experiment().DWrite();
  
 
  // loop refine
  while(true) {

  // copy abstraction
  Generator abst=tivabs.TivAbstraction();
  abst.StateNamesEnabled(false);
  MarkAllStates(abst);
  abst.Name("overall abstraction");

  // report statistics
  tivabs.Experiment().SWrite();
  abst.Write("tmp_abs2.gen");

  std::cout << "################################\n";
  std::cout << "# cycle control synthesis \n";
  std::cout << "# ctrl evs " << uevents.ToString() << "\n";
  std::cout << "################################\n";

   
  // target specification by y (full automata)
  Generator spec;
  spec.InjectAlphabet(lioha.Alphabet());
  Idx s0=spec.InsInitState();
  Idx st=spec.InsMarkedState();
  spec.SetTransition(s0,spec.EventIndex("y_w"),st);
  spec.SetTransition(st,spec.EventIndex("y_w"),st);
  EventSet sfloop=lioha.Alphabet();
  sfloop.Erase("y_w");
  SelfLoop(spec,sfloop);
  spec.GraphWrite("tmp_spec.png");

  // hack : enforce to start with u
  /*
  Generator guy;
  guy.InjectAlphabet(lioha.Alphabet());
  Idx qu=guy.InsInitState();
  Idx qy=guy.InsState();
  MarkAllStates(guy);
  EventSet::Iterator eit=uevents.Begin();
  EventSet::Iterator eit_end=uevents.End();
  for(;eit!=eit_end;++eit) 
    guy.SetTransition(qu,*eit,qy); 
  eit=yevents.Begin();
  eit_end=yevents.End();
  for(;eit!=eit_end;++eit) 
    guy.SetTransition(qy,*eit,qu);
  Parallel(spec,guy,spec);
  */

  // guide: do not use repeated inputs
  EventSet::Iterator eit=uevents.Begin();
  EventSet::Iterator eit_end=uevents.End();
  Generator guu;
  guu.InjectAlphabet(uevents);
  for(;eit!=eit_end;++eit) 
    guu.InsState(*eit);
  Idx q0 = guu.InsInitState();
  eit=uevents.Begin();
  eit_end=uevents.End();
   for(;eit!=eit_end;++eit) {
    guu.SetTransition(q0,*eit,*eit);
    EventSet::Iterator eit2=uevents.Begin();
    EventSet::Iterator eit2_end=uevents.End();
    for(;eit2!=eit2_end;++eit2) 
      if(*eit2!=*eit)
        guu.SetTransition(*eit,*eit2,*eit2);
  }
  MarkAllStates(guu);
  Parallel(spec,guu,spec);
      

  // patrol spec
  /*
  Generator spec;
  spec.InjectAlphabet(lioha.Alphabet());
  Idx sga=spec.InsInitState();
  Idx sgb=spec.InsState();
  Idx ssa=spec.InsMarkedState();
  Idx ssb=spec.InsMarkedState();
  spec.SetTransition(sga,spec.EventIndex("y_A"),ssa);
  spec.SetTransition(sga,spec.EventIndex("y_B"),sga);
  spec.SetTransition(ssa,spec.EventIndex("y_A"),sgb);
  spec.SetTransition(ssa,spec.EventIndex("y_B"),ssb);
  spec.SetTransition(sgb,spec.EventIndex("y_B"),ssb);
  spec.SetTransition(sgb,spec.EventIndex("y_A"),sgb);
  spec.SetTransition(ssb,spec.EventIndex("y_B"),sga);
  spec.SetTransition(ssb,spec.EventIndex("y_A"),ssa);
  EventSet other=lioha.Alphabet();
  other.Erase("y_A");
  other.Erase("y_B");
  EventSet::Iterator eit=other.Begin();
  for(;eit!=other.End();++eit) {
    spec.SetTransition(sga,*eit,sga);
    spec.SetTransition(ssa,*eit,sgb);
    spec.SetTransition(sgb,*eit,sgb);
    spec.SetTransition(ssb,*eit,sga);
  }  
  spec.GraphWrite("tmp_spec.png");
  */

  // target control: reach marked state inf often
  Generator loop;
  ProductCompositionMap cmap_abst_spec;
  aProduct(abst,spec,cmap_abst_spec,loop);
  StateSet constraint = loop.States();
  StateSet winning;
  StateSet target;

  while(true) {
    target = loop.MarkedStates();
    target.RestrictSet(constraint);
    int csz=constraint.Size();
    int tsz=target.Size();
    std::cout << "target control: constraint #" << csz << " target #" << tsz << "\n";
    winning.Clear();

  while(true) {
    int wsz=winning.Size();
    std::cout << "target control: #" << wsz << " / #" << loop.Size() << "\n";
    StateSet ctrlreach;
    StateSet::Iterator sit=loop.StatesBegin();
    StateSet::Iterator sit_end=loop.StatesEnd();
    for(;sit!=sit_end;++sit) {
      if(winning.Exists(*sit)) continue;
      bool ok=false;
      TransSet::Iterator tit=loop.TransRelBegin(*sit);
      TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
      for(;tit!=tit_end;++tit) {
        if(winning.Exists(tit->X2)) {ok=true; continue;}
        if(target.Exists(tit->X2)) {ok=true; continue;}
	if(uevents.Exists(tit->Ev)) continue;
	ok=false;
	break;
      }
      if(ok) ctrlreach.Insert(*sit);
    }
    winning.InsertSet(ctrlreach);
    if(winning.Size()==wsz) break;
  }

  constraint.RestrictSet(winning);
  if(csz==constraint.Size()) break;
  }
  
  std::cout << "target control: #" << winning.Size() << " / #" << loop.Size() << "\n";
  bool success=winning.Exists(loop.InitState());
  if(success) 
    std::cout << "target control: success (init state #" << loop.InitState() << " found to be winning)\n";
  else
    std::cout << "target control: FAIL\n";
  if(success) {
    std::cout << "target control: preparing ctrl\n";
    StateSet::Iterator sit=loop.StatesBegin();
    StateSet::Iterator sit_end=loop.StatesEnd();
    for(;sit!=sit_end;++sit) {
      if(!winning.Exists(*sit)) continue;
      TransSet tremove;
      TransSet::Iterator tit=loop.TransRelBegin(*sit);
      TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
      for(;tit!=tit_end;++tit) {
	// fixme: this is not quite correct
        if(winning.Exists(tit->X2)) {continue;}
        if(target.Exists(tit->X2))  {continue;}
	if(uevents.Exists(tit->Ev)) { tremove.Insert(*tit); continue; }
	std::cout << "synthesis: INTERNAL ERROR C";
	return 0;
      }
      tit=tremove.Begin();
      tit_end=tremove.End();
      for(;tit!=tit_end;++tit) 
        loop.ClrTransition(*tit);
    }  
    loop.Write("tmp_sup.gen");
    return 0;
  }

  // diagnose failure
  StateSet tivleaves;
  StateSet tivleaves_goodchance;
  StateSet::Iterator sit=loop.StatesBegin();
  StateSet::Iterator sit_end=loop.StatesEnd();
  for(;sit!=sit_end;++sit) {
    // ok done
    if(winning.Exists(*sit)) continue;
    // sort states
    Idx qtivabs = cmap_abst_spec.Arg1State(*sit);
    // allways refine
    if(tivabs.Experiment().IsLeave(qtivabs)) tivleaves.Insert(qtivabs);
    // is there a chance to win?
    TransSet::Iterator tit=loop.TransRelBegin(*sit);
    TransSet::Iterator tit_end=loop.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit) {
      if(winning.Exists(tit->X2)) break;
      TransSet::Iterator tit2=loop.TransRelBegin(tit->X2);
      TransSet::Iterator tit2_end=loop.TransRelEnd(tit->X2);
      for(;tit2!=tit2_end;++tit2) {
        if(winning.Exists(tit2->X2)) break;
      }
      if(tit2!=tit2_end) break;
    }
    if(tit==tit_end) continue;
    if(tivabs.Experiment().IsLeave(qtivabs)) tivleaves_goodchance.Insert(qtivabs);
  }
  std::cout << "################################\n";
  std::cout << "# diagnosis\n";
  std::cout << "# tiv-leaves to refine #" << tivleaves.Size() << "/" << tivabs.Experiment().Leaves().Size() << "\n";
  std::cout << "# candidates with better chance #" << tivleaves_goodchance.Size() << "/" << tivabs.Experiment().Leaves().Size() << "\n";
  std::cout << "################################\n";

  // do refine
  std::cout << "################################\n";
  std::cout << "# refine\n";
  std::cout << "################################\n";
  sit=tivleaves_goodchance.Begin();
  sit_end=tivleaves_goodchance.End();
  int xsz=tivabs.Experiment().Size();
  for(;sit!=sit_end;++sit) {
    tivabs.RefineAt(*sit);
  }
  if(xsz==tivabs.Experiment().Size())
  {
    sit=tivleaves.Begin();
    sit_end=tivleaves.End();
    for(;sit!=sit_end;++sit) {
      tivabs.RefineAt(*sit);
    }
  }

  // loop refine/synthesis
  }
  

  // done
  return 0;
}



// choose version
int main() {
  //return robot2017();
  //return robot2018ti();
  return marine2018();
}  

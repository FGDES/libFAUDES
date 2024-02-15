// obsolete --- moved to corefaudes cfl_bisimulation* --- file kept for reference 

/** @file op_bisimulation.cpp 

Methods to compute bisimulations on dynamic systems (represented
by a finite automaton). 
The relevant algorithms are described in
J.-C. Fernandez, “An implementation of an efficient algorithm for
bisimulation equivalence,” Science of Computer Programming, vol. 13,
pp. 219-236, 1990.
The class bisimulation supports these methods.
*/
   
/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <op_bisimulation.h>

using namespace std;

namespace faudes {
// calcBisimulation(rGenOrig, rMapStateToPartition, rGenPart, rNewPartitions)
void calcBisimulation(Generator& rGenOrig, map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart, vector<Idx>& rNewPartitions)
{
    OP_DF("calcBisimulation(" << rGenOrig.Name() << ", rMapStateToPartition, " << rGenPart.Name() << ", rNewPartitions)");
    // Construct an instance of the Bisimulation class from rGenOrig
    Bisimulation bisim = Bisimulation(rGenOrig);
    // method to compute the bisimulation on rGenOrig
    bisim.partition(rMapStateToPartition, rGenPart, rNewPartitions);
    #ifdef OP_DF_PLOT
        cout << "The result of the partition refinement is:" << endl;
        bisim.writeW();
        bisim.writeRo();
    #endif
    OP_DF("calcBisimulation: leaving function");
}

// calcBisimulation(rGenOrig, rMapStateToPartition, rNewPartitions)
void calcBisimulation(Generator& rGenOrig, map<Idx,Idx>& rMapStateToPartition, vector<Idx>& rNewPartitions)
{
    OP_DF("calcBisimulation(" << rGenOrig.Name() << ", rMapStateToPartition, rNewPartitions)");
    // Construct an instance of the Bisimulation class from rGenOrig
    Bisimulation bisim = Bisimulation(rGenOrig);
    // method to compute the bisimulation on rGenOrig
    bisim.partition(rMapStateToPartition, rNewPartitions);
    #ifdef FAUDES_DEBUG_FUNCTION
        cout << "The result of the partition refinement is:" << endl;
        bisim.writeW();
        bisim.writeRo();
    #endif
    OP_DF("calcBisimulation: leaving function");    
}

// Constructor Bisimulation(g)
Bisimulation::Bisimulation(Generator& g)
{
    OP_DF("Bisimulation::Bisimulation(" << g.Name() << ")");
    gen = &g;
    gen->TransRel(tset_evx2x1);
    index=1;

    // create universal partition holding the complete state space
    Partition universalPartition;
    universalPartition.index=index;
    universalPartition.pFather=NULL;
    universalPartition.pFirstChild=NULL;
    universalPartition.pSecondChild=NULL;
    universalPartition.pBrother=NULL;
    universalPartition.states = gen->States();
    universalPartition.numberOfStates = universalPartition.states.Size();

    // add universal partition (index 1) as root to W-tree
    W[index]=universalPartition; 

    // add universal Partition to ro and roDividers (all equivalence classes are potential dividers)
    ro.push_back(&W[index]);
    roDividers.insert(&W[index]);

    index++;
    
    OP_DF("Bisimulation::Bisimulation: leaving function");
}

// Bisimulation::partitionSplitter(B)
void Bisimulation::partitionSplitter(Partition& B)
{
    OP_DF("Bisimulation::partitionSplitter(B)");
    OP_DF("Bisimulation::partitionSplitter: index of current coset is " << B.index);

    // it is ensured by the calling function partition that the passed coset B has a father coset B' and
    // that ro is stable with respect to B'. Therefore, the info-maps for the father coset B' already exist.

    // pointer to father coset
    Partition* pFather = B.pFather;

    // Choose the coset with fewer states among B and its brother coset and let pSmallerPart point to it
    Partition* pSmallerPart;
    Partition* pLargerPart;

    if(B.numberOfStates <= B.pBrother->numberOfStates)
    {
        pSmallerPart = &B;
        pLargerPart = B.pBrother;
    }
    else
    {
        pSmallerPart = B.pBrother;
        pLargerPart = &B;
    }

    OP_DF("Bisimulation::partitionSplitter: the chosen (smaller) coset for segmentation has the index: " << pSmallerPart->index);
    OP_DF("Bisimulation::partitionSplitter: the larger coset has the index: " << pLargerPart->index << "\n");

    // helpers

    // Iterators for loop over events
    EventSet::Iterator eIt;
    EventSet::Iterator eItBegin=gen->AlphabetBegin(); 
    EventSet::Iterator eItEnd=gen->AlphabetEnd();

    // Iterators for loop over states
    StateSet::Iterator sIt;
    StateSet::Iterator sItBegin;
    StateSet::Iterator sItEnd;

    // Iterators over Transitions ordered by EvX2X1
    TransSetEvX2X1::Iterator tIt;
    TransSetEvX2X1::Iterator tItBegin;
    TransSetEvX2X1::Iterator tItEnd;

    // Iterator over ro
    vector<Partition*>::iterator roIt;
    
    // Stack of iterators over ro
    stack<vector<Partition*>::iterator> toDo;

     // start computation
     //loop over all events
    for (eIt=eItBegin; eIt != eItEnd; ++eIt)
    {
	// the current event only has to be processed if there are transitions with *eIt to states in the 
	// father coset, i.e., there is an entry for *eIt in the infoMap of pFather
	if(pFather->infoMap.find(*eIt) != pFather->infoMap.end() ){
		OP_DF("Bisimulation::partitionSplitter: partitioning for event " << gen->EventName(*eIt));
		// initialize info-map for current event of larger child coset with 
		// a copy of the info-map of the current event of the parent coset 
		pLargerPart->infoMap[*eIt]= pFather->infoMap.find(*eIt)->second;
		// initialize info-map for current event of smaller child coset with an empty info-map
		pSmallerPart->infoMap[*eIt]=map<Idx,Idx>();
		// compute the info maps for current event for smaller and larger child cosets
		computeInfoMaps(*(pSmallerPart), pSmallerPart, pLargerPart, eIt);
		// if one of the infomaps for the event *eIt is empty, tbe element is erased
		map<Idx, map<Idx, Idx> >::iterator imIt = pSmallerPart->infoMap.find(*eIt);
		if(imIt == pSmallerPart->infoMap.end() )
			pSmallerPart->infoMap.erase(imIt);
		imIt = pLargerPart->infoMap.find(*eIt);
		if(imIt ==  pLargerPart->infoMap.end() )
			 pLargerPart->infoMap.erase(imIt);
		// compute cosets in ro that are completely contained in T_{eIt}^{-1}{B}
		OP_DF("Bisimulation::partitionSplitter: computing predecessor cosets of parent coset with index " << pFather->index);
		Idx tempState;
		// iteration over all cosets in ro
		for(roIt=ro.begin(); roIt!=ro.end(); roIt++)
		{
			// choose an arbitrary state from the current coset
			tempState = *((*roIt)->states.Begin());
			OP_DF("Bisimulation::partitionSplitter: checking candidate coset with index " << (*roIt)->index << " and its state " << gen->StateName(tempState) << " [" << tempState << "]");
			// check if tempState has an eIt Transition to a state in the parent coset. If yes, 
			// dividing pFather might cause a division of the current coset
			if(stateLeadsToPartition(tempState, *pFather, eIt)) 
			{
				toDo.push(roIt);
				OP_DF("Bisimulation::partitionSplitter: coset with index " << (*roIt)->index << " is predecessor for parent coset with index " << (*pFather).index << " and event = " << gen->EventName(*eIt) << " and was pushed on toDo-stack");
			}
		}
		// split all cosets on the toDo-stack using the info-maps. This process is described in Equation (2) of 
		// J.-C. Fernandez, “An implementation of an efficient algorithm for bisimulation equivalence,” Science of Computer Programming, vol. 13, pp. 219-236, 1990.
		// helpers
		Idx index1,index2,index3;
		vector<Idx> removeSet;
		vector<Partition*> addSet;
		
		// iteration over all cosets on todo-stack
		while(!toDo.empty())
		{
			roIt=toDo.top();
			toDo.pop();
			OP_DF("Bisimulation::partitionSplitter: current coset taken from todo-stack has index " << (*roIt)->index);
			
			bool nonNull[3] = {0,0,0};
			
			// create three empty cosets and add them to W-tree
			Partition X1,X2,X3;
			index1=index;
			W[index1]=X1;
			W[index1].index=index1;
			index++;
			index2=index;
			W[index2]=X2;
			W[index2].index=index2;
			index++;
			index3=index;
			W[index3]=X3;
			W[index3].index=index3;
			index++;
			
			sItBegin = (*roIt)->states.Begin();
			sItEnd = (*roIt)->states.End();
			
			// pointer to info map for current event of partent coset 
			map<Idx, map<Idx, Idx> >::const_iterator itMapFather = pFather->infoMap.find(*eIt);
			// pointer to info map for current event of smaller coset
			map<Idx, map<Idx, Idx> >::const_iterator itMapSmallerPart = pSmallerPart->infoMap.find(*eIt);
		
			// iteration over all states of current candidate coset
			// apply rules for splitting cosets into subsets using info-maps 
			for(sIt=sItBegin; sIt!=sItEnd; sIt++)
			{
				if(itMapSmallerPart == pSmallerPart->infoMap.end() ){
					W[index2].states.Insert(*sIt);
					nonNull[1]=true;
				}
				else if(itMapSmallerPart->second.find(*sIt)==itMapSmallerPart->second.end())
				{
					W[index2].states.Insert(*sIt);
					nonNull[1]=true;
				}
				else
				{  
				if(itMapFather->second.find(*sIt)->second==itMapSmallerPart->second.find(*sIt)->second)
				{                
					W[index1].states.Insert(*sIt);                
					nonNull[0]=true;
				}
				if(itMapFather->second.find(*sIt)->second > itMapSmallerPart->second.find(*sIt)->second)
				{                   
					W[index3].states.Insert(*sIt);                
					nonNull[2]=true;
				}
				}
			}
			
			// update W-tree, ro and roDividers. Different cases are distinguished depending on which of the sets are empty and non-empty
			switch(nonNull[0]+nonNull[1]*2+nonNull[2]*4) 
			{
			//case 0: not possible
			case 1: //only X1 is not empty, no changes in W-tree and ro
				index=index-3;
				OP_DF("Bisimulation::partitionSplitter: coset " << (*roIt)->index << " has not been split ");
				break;
		
			case 2: //only X2 is not empty, no changes in W-tree and ro
				index=index-3;
				OP_DF("Bisimulation::partitionSplitter: coset " << (*roIt)->index << " has not been split ");
				break;
		
			case 4: //only X3 is not empty, no changes in W-tree and ro
				index=index-3;
				OP_DF("partition " << (*roIt)->index << " has not been split ");
				break;
		
			case 3: //X1 and X2 are not empty
				OP_DF("Bisimulation::partitionSplitter: coset " << (*roIt)->index << " has been split into cosets X1 and X2");
				
				// erase the empty set X3 from W-tree
				W.erase(W.find(index3));
				
				// set cross references
				W[index2].pBrother=&W[index1];
				W[index2].pFather=*roIt;
				W[index2].pFirstChild=NULL;
				W[index2].pSecondChild=NULL;
				W[index2].numberOfStates=W[index2].states.Size();
				W[index1].pBrother=&W[index2];
				W[index1].pFather=*roIt;
				W[index1].pFirstChild=NULL;
				W[index1].pSecondChild=NULL;
				W[index1].numberOfStates=W[index1].states.Size();
				(*roIt)->pFirstChild=&W[index1];
				(*roIt)->pSecondChild=&W[index2];
		
				// states of split coset are no longer needed as they are now stored in X1 and X2 
				(*roIt)->states.Clear();
				
				// if split coset was in roDividers, then delete it from roDividers
				if(roDividers.find(*roIt)!=roDividers.end())
				roDividers.erase(roDividers.find(*roIt));
				
				// info-map of the split coset's parent coset can be deleted if it is no longer needed 
				if((*roIt)->pFather!=NULL && (*roIt)->pFather != pFather)
				{
				if((*roIt)->pFather->pFirstChild->pFirstChild!=NULL && (*roIt)->pFather->pSecondChild->pSecondChild!=NULL)
				{
					(*roIt)->pFather->infoMap.clear();
				}
				}
		
				// remember which manipulations have to be made to ro and roDividers
				removeSet.push_back((*roIt)->index);
				addSet.push_back(&W[index1]);
				addSet.push_back(&W[index2]);
				roDividers.insert(&W[index1]);
				roDividers.insert(&W[index2]);
		
				break;
			case 5: //X1 and X3 are not empty
				OP_DF("Bisimulation::partitionSplitter: coset " << (*roIt)->index << " has been split into cosets X1 and X3");
				
				// computation analogical to case 3
				
				W.erase(W.find(index2));
				W[index3].pBrother=&W[index1];
				W[index3].pFather=*roIt;
				W[index3].pFirstChild=NULL;
				W[index3].pSecondChild=NULL;
				W[index3].numberOfStates=W[index3].states.Size();
				W[index1].pBrother=&W[index3];
				W[index1].pFather=*roIt;
				W[index1].pFirstChild=NULL;
				W[index1].pSecondChild=NULL;
				W[index1].numberOfStates=W[index1].states.Size();
				(*roIt)->pFirstChild=&W[index1];
				(*roIt)->pSecondChild=&W[index3];
		
				(*roIt)->states.Clear();
				if(roDividers.find(*roIt)!=roDividers.end())
				roDividers.erase(roDividers.find(*roIt));
				if((*roIt)->pFather!=NULL && (*roIt)->pFather != pFather)
				{
				if((*roIt)->pFather->pFirstChild->pFirstChild!=NULL && (*roIt)->pFather->pSecondChild->pSecondChild!=NULL)
					(*roIt)->pFather->infoMap.clear();
				}
		
				removeSet.push_back((*roIt)->index);
				addSet.push_back(&W[index1]);
				addSet.push_back(&W[index3]);
				roDividers.insert(&W[index1]);
				roDividers.insert(&W[index3]);
				break;
				
			case 6: //X2 and X3 are not empty
				OP_DF("Bisimulation::partitionSplitter: coset " << (*roIt)->index << " has been split into cosets X2 and X3");
		
				// computation analogical to case 3
		
				W.erase(W.find(index1));
				W[index3].pBrother=&W[index2];
				W[index3].pFather=*roIt;
				W[index3].pFirstChild=NULL;
				W[index3].pSecondChild=NULL;
				W[index3].numberOfStates=W[index3].states.Size();
				W[index2].pBrother=&W[index3];
				W[index2].pFather=*roIt;
				W[index2].pFirstChild=NULL;
				W[index2].pSecondChild=NULL;
				W[index2].numberOfStates=W[index2].states.Size();
				(*roIt)->pFirstChild=&W[index2];
				(*roIt)->pSecondChild=&W[index3];
				
		
				(*roIt)->states.Clear();
				if(roDividers.find(*roIt)!=roDividers.end())
				roDividers.erase(roDividers.find(*roIt));
				if((*roIt)->pFather!=NULL && (*roIt)->pFather != pFather)
				{
				if((*roIt)->pFather->pFirstChild->pFirstChild!=NULL && (*roIt)->pFather->pSecondChild->pSecondChild!=NULL)
					(*roIt)->pFather->infoMap.clear();
				}
				
				removeSet.push_back((*roIt)->index);
				addSet.push_back(&W[index2]);
				addSet.push_back(&W[index3]);
				roDividers.insert(&W[index2]);
				roDividers.insert(&W[index3]);
				
				break;
				
			case 7: //X1 and X2 and X3 are not empty
			
				OP_DF("Bisimulation::partitionSplitter: coset " << (*roIt)->index << " has been split into cosets X1, X2 and X3");
				
				// computation analogical to case 3
				
				Partition X23;
				W[index]=X23;
				W[index].index=index;
				(*roIt)->pFirstChild=&W[index1];
				(*roIt)->pSecondChild=&W[index];
				W[index1].pFather=*roIt;
				W[index1].pBrother=&W[index];
				W[index1].pFirstChild=NULL;
				W[index1].pSecondChild=NULL;
				W[index1].numberOfStates=W[index1].states.Size();
				W[index].pBrother=&W[index1];
				W[index].pFather=*roIt;
				W[index].pFirstChild=&W[index2];
				W[index].pSecondChild=&W[index3];
				W[index2].pBrother=&W[index3];
				W[index2].pFather=&W[index];
				W[index2].pFirstChild=NULL;
				W[index2].pSecondChild=NULL;
				W[index2].numberOfStates=W[index2].states.Size();
				W[index3].pBrother=&W[index2];
				W[index3].pFather=&W[index];
				W[index3].pFirstChild=NULL;
				W[index3].pSecondChild=NULL;
				W[index3].numberOfStates=W[index3].states.Size();
				W[index].numberOfStates=W[index2].numberOfStates+W[index3].numberOfStates;
		
				(*roIt)->states.Clear();
		
				if(roDividers.find(*roIt)!=roDividers.end())
				roDividers.erase(roDividers.find(*roIt));
				if((*roIt)->pFather!=NULL && (*roIt)->pFather != pFather)
				{
				if((*roIt)->pFather->pFirstChild->pFirstChild!=NULL && (*roIt)->pFather->pSecondChild->pSecondChild!=NULL)
					(*roIt)->pFather->infoMap.clear();
				}
		
				removeSet.push_back((*roIt)->index);
				addSet.push_back(&W[index1]);
				addSet.push_back(&W[index2]);
				addSet.push_back(&W[index3]);
				roDividers.insert(&W[index1]);
				roDividers.insert(&W[index2]);
				roDividers.insert(&W[index3]);
				index++;
		
				break;
			} //end switch-case
	
		} // end iteration over todo-stack
	
		// delete split partitions from Ro
		OP_DF("Bisimulation::partitionSplitter: deleting split cosets from ro");
		vector<Idx>::iterator remIt;
		for(remIt=removeSet.begin(); remIt != removeSet.end(); ++remIt)
		{
			vector<Partition*>::iterator delRoIt = ro.begin();
			vector<Partition*>::iterator delRoItEnd = ro.end();
			for(; delRoIt != delRoItEnd; ++delRoIt)
			{
				if((*delRoIt)->index == *remIt)
				{
				ro.erase(delRoIt);
				OP_DF("Bisimulation::partitionSplitter: coset "<< (*delRoIt)->index << " deleted from ro");
				break;
				}
			}
		}
		removeSet.clear();
	
		// insert the newly created partitions to ro
		OP_DF("Bisimulation::partitionSplitter: inserting the new cosets into ro");
		vector<Partition*>::iterator addIt;
		for(addIt=addSet.begin(); addIt != addSet.end(); ++addIt)
		{
		ro.push_back(*addIt);
		OP_DF("Bisimulation::partitionSplitter: coset " << (*addIt)->index << " inserted into ro");
		}
		addSet.clear();
	}
    } //end loop over all events

    // infoMap of parent coset is now lo longer needed and can be deleted (the information has been transferred to the children cosets)
    pFather->infoMap.clear();

    // ro is now stable with respect to the larger and smaller child cosets
    pSmallerPart->nonSplitting=true;
    pLargerPart->nonSplitting=true;
    
    // delete the stateSets of the smaller and larger child cosets if no longer needed
    if(pSmallerPart->pFirstChild != NULL)
        pSmallerPart->states.Clear();
    if(pLargerPart->pFirstChild != NULL)
        pLargerPart->states.Clear();
    
    // if smaller and larger partitions were in roDividers, then they now have to be deleted from this vector
    if(roDividers.find(pSmallerPart) != roDividers.end())
            roDividers.erase(pSmallerPart);
    if(roDividers.find(pLargerPart) != roDividers.end())
            roDividers.erase(pLargerPart);

    OP_DF("Bisimulation::partitionSplitter: leaving function");
}

// Bisimulation::computeInfoMaps(node, pSmallerPart, pLargerPart, eIt)
void Bisimulation::computeInfoMaps(Partition& node, Partition* pSmallerPart, Partition* pLargerPart, EventSet::Iterator eIt)
{
	OP_DF("Bisimulation::computeInfoMaps(" << node.index << "," << pSmallerPart->index << "," << pLargerPart->index << "," << gen->EventName(*eIt) << ")");
	
	//helpers
	Idx tempState;
	// iterators over stateSets
	StateSet::Iterator sIt;
	StateSet::Iterator sItBegin;
	StateSet::Iterator sItEnd;
	// iterators over transitions ordered by EvX2X1
	TransSetEvX2X1::Iterator tIt;
	TransSetEvX2X1::Iterator tItBegin;
	TransSetEvX2X1::Iterator tItEnd;
	// loop over states of current stateSet of coset node
	sItBegin=node.states.Begin();
	sItEnd=node.states.End();
	map<Idx, map<Idx,Idx> >::iterator imItLa, imItSm;
	imItLa = pLargerPart->infoMap.find(*eIt);
	imItSm = pSmallerPart->infoMap.find(*eIt);
	for(sIt=sItBegin; sIt!=sItEnd; sIt++)
	{
		tItBegin=tset_evx2x1.BeginByEvX2(*eIt,*sIt);
		tItEnd=tset_evx2x1.EndByEvX2(*eIt,*sIt);
		// loop over transitions of current state ordered by EvX2 to find backward reachable states. Note that all states investigated are states of the smaller coset.
		for(tIt=tItBegin; tIt != tItEnd; ++tIt)
		{
			tempState=tIt->X1;
			// insert a newly found state in the infomap. Then, the number of transition is set to one
			if(imItSm->second.find(tempState) == imItSm->second.end())
			{
				imItSm->second[tempState]=1;
			}
			// The number of transitions for the found state is incremented
			else
			{
				imItSm->second[tempState]++;
			}
			imItLa->second[tempState]--;
			if(imItLa->second[tempState]==0)
				imItLa->second.erase(tempState);
		}
	}
	// recursively call this function for the child cosets of node if the current coset is not a leaf of the W-tree
	if(node.pFirstChild!=NULL){
		computeInfoMaps(*(node.pFirstChild), pSmallerPart, pLargerPart, eIt);
	}
	if(node.pSecondChild!=NULL){
		computeInfoMaps(*(node.pSecondChild),pSmallerPart, pLargerPart, eIt);
	}
        
    OP_DF("Bisimulation::computeInfoMaps: leaving function");
}

// Bisimulation::stateLeadsToPartition(state,  node, eIt)
bool Bisimulation::stateLeadsToPartition(Idx state, Partition& node, EventSet::Iterator eIt)
{
    OP_DF("Bisimulation::stateLeadsToPartition(" << state << "," << node.index << "," << gen->EventName(*eIt) << ")");

	bool found=false;
	// there is a direct transition from state to a state in the coset node
	if(node.states.Exists(gen->TransRelBegin(state, *eIt)->X2) && gen->TransRelBegin(state, *eIt)->X1 == state && gen->TransRelBegin(state, *eIt)->Ev == *eIt)
	{
		found = true;
	}
	// the stateset of node is stored in its children. Then, try to find a transition to any of the children cosets of node
	if(found == false && node.pFirstChild!=NULL)
	{
		found=stateLeadsToPartition(state,*node.pFirstChild,eIt);
	}
	
	if(found == false && node.pSecondChild!=NULL)
	{
		found=stateLeadsToPartition(state,*node.pSecondChild,eIt); 
	}
	
	OP_DF("Bisimulation::stateLeadsToPartition: leaving function");
	return found;
}

// Bisimulation::partitionClass(B)
void Bisimulation::partitionClass(Partition& B)
{
    OP_DF("Bisimulation::partitionClass(" << B.index << ")");
    OP_DF("Bisimulation::partitionClass: index of passed coset is " << B.index);

    // helpers
    EventSet::Iterator eIt;
    EventSet::Iterator eItBegin = gen->AlphabetBegin(); 
    EventSet::Iterator eItEnd   = gen->AlphabetEnd();


    vector<Partition*>::iterator roIt;

    vector<Partition*> addSet;
    vector<Partition*>::iterator addIt;
    vector<Idx> removeSet;
    vector<Idx>::iterator remIt;

    // temporary StateSet for computing events T_{eIt}^{-1}(B) for current event eIt
    StateSet tb;

    FD_DV("Bisimulation::partitionClass: loop over events " << gen->Alphabet().ToString());

    // Loop over all events
    for (eIt=eItBegin; eIt!=eItEnd; ++eIt)
    {
        OP_DF("Bisimulation::partitionClass: partitioning for event " << gen->EventName(*eIt));
        // initialize info-map for current event of current coset node with an empty info-map
        tb.Clear();
        // compute info map for current event eIt and coset B and store predecessor states in T_{eIt}^{-1}(B) in tb
        computeInfoMap(B, B, eIt, tb);
        // compute the cosets in ro that are split into the subsets intersec and diff by evaluating the backward reachability from states in B
        for(roIt = ro.begin(); roIt != ro.end(); ++roIt)
        {
             OP_DF("Bisimulation::partitionClass: candidate coset to be split has index " << (*roIt)->index);

            // compute intersection of tb with current candidate coset
            Partition intersection;
            W[index]=intersection;
            W[index].states=(*roIt)->states;
            W[index].states.SetIntersection(tb);
            // check if the coset must be split
            if(!W[index].states.Empty() && W[index].states.Size()!=(*roIt)->states.Size() )
            {
                // the coset will be split into two new cosets 
                OP_DF("Bisimulation::partitionClass: current coset with index " << (*roIt)->index << " will be split");
                
                // if it was in roDividers, delete it form that vector
                if(roDividers.find(*roIt)!=roDividers.end())
                {
                    OP_DF("Bisimulation::partitionClass: candidate was in roDividers and will be deleted from it");
                    roDividers.erase(roDividers.find(*roIt));
                }
                
                // remember that the split coset needs to be deleted from ro
                removeSet.push_back((*roIt)->index);

                // set cross references and properties of the intersection-coset
                W[index].index=index;
                W[index].pBrother=NULL;
                W[index].pFirstChild=NULL;
                W[index].pSecondChild=NULL;
                W[index].pFather=*roIt;
                W[index].numberOfStates=W[index].states.Size();
                (*roIt)->pFirstChild=&W[index];

                // remember that the new coset has to be added to ro and roDividers
                addSet.push_back(&W[index]);
                roDividers.insert(&W[index]);                
                OP_DF("Bisimulation::partitionClass: the coset with index " << (&W[index])->index << " has been added to addSet and to roDividers");

                index++;
                
                // compute difference-coset and set cross references and properties
                Partition diff;
                W[index]=diff;
                W[index].states = ((*roIt)->states) - W[index-1].states;
                W[index].index=index;
                W[index].pBrother=&W[index-1];
                W[index-1].pBrother=&W[index];
                W[index].pFirstChild=NULL;
                W[index].pSecondChild=NULL;
                W[index].pFather=*roIt;
                W[index].numberOfStates=W[index].states.Size();
                (*roIt)->pSecondChild=&W[index];
                
                // remember that the new coset has to be added to ro and roDividers
                addSet.push_back(&W[index]);
                roDividers.insert(&W[index]);
                OP_DF("Bisimulation::partitionClass: the coset with index " << (&W[index])->index << " has been added to addSet and to roDividers");
                
                index++;
                OP_DF("Bisimulation::partitionClass: the candidate coset has been split");

                // delete info-map of parent coset of split coset if no longer needed
                if((*roIt)->pFather!=NULL)
                {
                    OP_DF("Bisimulation::partitionClass: split coset has parent coset with index " << (*roIt)->pFather->index);
                    if((*roIt)->pFather->pFirstChild->pFirstChild!=NULL && (*roIt)->pFather->pSecondChild->pSecondChild!=NULL)
                    {
                        (*roIt)->pFather->infoMap.clear();
                        OP_DF("Bisimulation::partitionClass: info map of parent coset deleted");
                    }
                }
                
                // delete stateSet of split coset as the states are now stored in the child cosets
                (*roIt)->states.Clear();
                OP_DF("Bisimulation::partitionClass: states of split coset " << (*roIt)->index << " have been deleted");
            }
            else
            {
                // candidate is not split
                W.erase(index); 
            }
        }

        // delete split partitions from ro
        OP_DF("Bisimulation::partitionClass: deleting split cosets from ro");
        for(remIt=removeSet.begin(); remIt!=removeSet.end();remIt++)
        {
            vector<Partition*>::iterator delRoIt = ro.begin();
            vector<Partition*>::iterator delRoItEnd = ro.end();
            for(; delRoIt!=delRoItEnd; ++delRoIt)
            {
                if((*delRoIt)->index==*remIt)
                {
                    ro.erase(delRoIt);
                    OP_DF("Bisimulation::partitionClass: coset " << (*delRoIt)->index << " was deleted from ro");
                    break;
                }
            }
        }
        removeSet.clear();
 
        // insert the new cosets into ro
        OP_DF("Bisimulation::partitionClass: inserting the new cosets into ro");
        for(addIt=addSet.begin();addIt!=addSet.end();addIt++)
        {
            ro.push_back(*addIt);
            OP_DF("Bisimulation::partitionClass: coset with index " << (*addIt)->index << " was inserted into ro"); 
        }
        addSet.clear();


    } //end loop over events

    // ro is now stable with respect to coset B
    B.nonSplitting = true;
    
    // delete stateSet of coset B if no longer needed
    if(B.pFirstChild != NULL)
        B.states.Clear();
        
    // delete coset B from roDividers
    roDividers.erase(&B);
    
    OP_DF("Bisimulation::partitionClass: leaving function");
}

// Bisimulation::computeInfoMap(B, Bstates, eIt, tb)
void Bisimulation::computeInfoMap(Partition& B, Partition& Bstates, EventSet::Iterator eIt, StateSet& tb)
{
	OP_DF("Bisimulation::computeInfoMap(" << B.index << "," << Bstates.index << "," << gen->EventName(*eIt) << ", Stateset&)");
	OP_DF("Bisimulation::computeInfoMap: consider stateSet of coset " << Bstates.index);
	
	// helpers
	Idx temp;
	
	// iterators over StateSets
	StateSet::Iterator sIt;
	StateSet::Iterator sItBegin = Bstates.states.Begin();
	StateSet::Iterator sItEnd  = Bstates.states.End();
	
	// iterators over TransSets ordered by EvX2X1
	TransSetEvX2X1::Iterator tIt;
	TransSetEvX2X1::Iterator tItBegin;
	TransSetEvX2X1::Iterator tItEnd;
	map<Idx, map<Idx, Idx> >::iterator imIt;
	// loop over all states of current partition B
	for(sIt=sItBegin; sIt != sItEnd; ++sIt)
	{
	tItBegin=tset_evx2x1.BeginByEvX2(*eIt,*sIt);
	tItEnd=tset_evx2x1.EndByEvX2(*eIt,*sIt);
	
		// loop over all transitions of current state ordered
		for(tIt=tItBegin; tIt != tItEnd; ++tIt)
		{
			imIt = B.infoMap.find(*eIt);
			// if the infoMap does not contaion the event *eIt, yet, an entry is inserted
			if(imIt == B.infoMap.end() )
				imIt = B.infoMap.insert(B.infoMap.begin(), make_pair(*eIt, map<Idx,Idx>() ) );
			
			temp=tIt->X1;
			tb.Insert(temp);
		
			// increase number of occurrences for found predecessor state and current event eIt in info-map of coset B
			if(imIt->second.find(temp)==imIt->second.end())
				imIt->second[temp]=1;
			else
				imIt->second[temp]++;
		}
	}
	// recursively call this function if coset Bstates has child cosets that contain the state informaton of B
	if(Bstates.pFirstChild!=NULL)
	{
	computeInfoMap(B, *(Bstates.pFirstChild), eIt, tb);
	}
	if(Bstates.pSecondChild!=NULL)
	{
	computeInfoMap(B, *(Bstates.pSecondChild), eIt, tb);
	}
	OP_DF("Bisimulation::computeInfoMap: leaving function");
}

// Bisimulation::partition(rMapStateToPartition, rGenPart, rNewPartitions)
void Bisimulation::partition(std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart, std::vector<Idx>& rNewPartitions)
{
    OP_DF("Bisimulation::partition(rMapStateToPartition," << rGenPart.Name() << ", rNewPartitions)");
    
    //helpers
    set<Partition*>::iterator roDivIt;
    set<Partition*>::iterator roDivItEnd=roDividers.end();
    Partition* pFather;
    bool done=false;

    // choose first entry of roDividers to start the partition refinement
    roDivIt=roDividers.begin();
    
    // repeat refinement steps performed by the functions partitionClass and partitionSplitter 
    // as long as there are elements in roDividers
    while(done == false)
    {
        // iterate over roDividers and try to find a coset that has a parent coset B' such that ro is stable w.r.t. B'
        // (note that the order in which the dividers are chosen is in principle arbitrary)
        while(roDivIt != roDivItEnd && !roDividers.empty())
        {
            pFather=(*roDivIt)->pFather;
            if(pFather != NULL) 
            {
                if(pFather->nonSplitting == true) 
                {
                    partitionSplitter(**roDivIt);
                    
                    // roDividers might have been manipulated by the function partitionSplitter. Recompute iterators.
                    roDivItEnd=roDividers.end();
                    roDivIt=roDividers.begin();
                    continue;
                }
            }
            roDivIt++;
        }
        // no element B' could be found such that ro is stable w.r.t. it, but roDividers is not empty. In that case,
        // the first element of roDividers is chosen to serve as a "class" in the bisimulation algorithm. 
        if(!roDividers.empty())
        {
            roDivIt=roDividers.begin();
            partitionClass(**roDivIt);
            
            // roDividers might have been manipulated by the function partitionClass. Recompute iterators.
            roDivItEnd=roDividers.end();
            roDivIt=roDividers.begin();
        }
        if(roDividers.empty())
            done=true;
    }

    // The following part of the algorithm computes the quotient automaton rGenPart resulting from the 
    // equivalence relation determined above. 
    OP_DF("Bisimulation::partition: entering function: building generator from equivalence classes");
    
    Idx newstate;
    // loop over all elements of ro and create a new state for every coset in ro (ro contains the set of equivalence classes)
    vector<Partition*>::const_iterator cRoIt    = ro.begin();
    vector<Partition*>::const_iterator cRoItEnd = ro.end();
    StateSet::Iterator cSIt;
    StateSet::Iterator cSItEnd;
    for (; cRoIt != cRoItEnd; ++cRoIt) 
    {
        std::ostringstream ostr;
        newstate = rGenPart.InsState();
        

        OP_DF("Bisimulation::partition: new state " << newstate << " for coset " 
	      << (*cRoIt)->index << " :"<< (*cRoIt)->states.ToString());

        // store the index of the equivalence class (new state) in the vector rNewPartitions
        rNewPartitions.push_back(newstate);
        
        cSIt=(*cRoIt)->states.Begin();
        cSItEnd=(*cRoIt)->states.End();
        
        // loop over all states of current coset
        for (; cSIt != cSItEnd; ++cSIt) 
        {
            // map every state of the original generator to its equivalence class (= state in rGenPart) 
            // by creating an entry in the map rMapStateToPartition 
            rMapStateToPartition[*cSIt] = newstate;
            
            // set state names for resulting generator
            if (rGenPart.StateNamesEnabled()) 
            {
                if (gen->StateName(*cSIt) == "") 
                {
                    ostr << ToStringInteger(*cSIt) << ",";
                }
                else 
                {
                    ostr << gen->StateName(*cSIt) << ",";
                }
            }

            // set init states
            if (gen->ExistsInitState(*cSIt)) 
            {
                rGenPart.SetInitState(newstate);
            }
            // set marked states
            if (gen->ExistsMarkedState(*cSIt)) 
            {
                rGenPart.SetMarkedState(newstate);
            }
        }
        
        if (rGenPart.StateNamesEnabled()) 
        {
            std::string statename = ostr.str();
            if(statename.length()>=1) statename.erase(statename.length()-1);
            statename = "{" + statename + "}";
            rGenPart.StateName(newstate, statename); 
            OP_DF("Bisimulation::partition: new state " << statename);
        }
    }
    // create transition relation
    // iterate over all transitions of the original generator
    TransSet::Iterator tIt = gen->TransRelBegin();
    TransSet::Iterator tItEnd = gen->TransRelEnd();
    for (; tIt != tItEnd; ++tIt) 
    {
        rGenPart.InsEvent(tIt->Ev); // ??
        rGenPart.SetTransition(rMapStateToPartition[tIt->X1], tIt->Ev, rMapStateToPartition[tIt->X2]);
        OP_DF("Bisimulation::partition: adding transition: X1=" << rGenPart.TStr(*tIt) );
    }
    OP_DF("Bisimulation::partition: leaving function");
}

// Bisimulation::partition(rMapStateToPartition, rNewPartitions)
void Bisimulation::partition(std::map<Idx,Idx>& rMapStateToPartition, std::vector<Idx>& rNewPartitions)
{
    OP_DF("Bisimulation::partition(rMapStateToPartition, rNewPartitions)");
    
    // this function is equivalent to the other function Bisimulation:partition, but no generator is built from the result
    set<Partition*>::iterator roDivIt;
    set<Partition*>::iterator roDivItEnd=roDividers.end();
    Partition* pFather;
    bool done=false;
    // choose first entry of roDividers to start the partition refinement
    roDivIt=roDividers.begin();
    // repeat refinement steps performed by the functions partitionClass and partitionSplitter 
    // as long as there are elements in roDividers    
    while(done==false)
    {
        // iterate over roDividers and try to find a coset that has a parent coset B' such that ro is stable w.r.t. B'
        // (note that the order in which the dividers are chosen is in principle arbitrary)
        while(roDivIt!=roDivItEnd && !roDividers.empty())
        {
            pFather=(*roDivIt)->pFather;
            if(pFather!=NULL)
            {
                if(pFather->nonSplitting == true) 
                {
                    partitionSplitter(**roDivIt);
                    // roDividers might have been manipulated by the function partitionSplitter. Recompute iterators.
                    roDivItEnd=roDividers.end();
                    roDivIt=roDividers.begin();
                    continue;
                }
            }
            roDivIt++;
        }
        // no element B' could be found such that ro is stable w.r.t. it, but roDividers is not empty. In that case,
        // the first element of roDividers is chosen to serve as a "class" in the bisimulation algorithm. 
        if(!roDividers.empty())
        {
            roDivIt=roDividers.begin();
            partitionClass(**roDivIt);
            // roDividers might have been manipulated by the function partitionClass. Recompute iterators.
            roDivItEnd=roDividers.end();
            roDivIt=roDividers.begin();
        }
        if(roDividers.empty())
            done=true;
    }


    // create an index for every coset in ro an write it to vector rNewPartitions
    OP_DF("Bisimulation::partition: writing vector rMapStateToPartition");
    
    Idx newstate;
    // loop over all elements of ro
    vector<Partition*>::const_iterator cRoIt = ro.begin();
    vector<Partition*>::const_iterator cRoItEnd = ro.end();
    StateSet::Iterator cSIt;
    StateSet::Iterator cSItEnd;
    for (; cRoIt != cRoItEnd; ++cRoIt) 
    {
        Generator dummy;
        newstate = dummy.InsState();
        rNewPartitions.push_back(newstate);
        cSIt=(*cRoIt)->states.Begin();
        cSItEnd=(*cRoIt)->states.End();
        for ( ; cSIt!=cSItEnd; ++ cSIt) 
        {
            // map every state in the current coset to the index of this coset by creating an entry in the map rMapStateToPartition
            rMapStateToPartition[*cSIt] = newstate;
        }
    }
    OP_DF("Bisimulation::partition: leaving function");
}

// Bisimulation::writeW()
void Bisimulation::writeW(void)
{
    OP_DF("Bisimulation:writeW()");
    cout << "Plotting the W-tree:" << endl;
    writeNode(W.find(1)->second);
}

// Bisimulation::writeNode(node)
void Bisimulation::writeNode(Partition& node)
{
        OP_DF("Bisimulation::writeNode(" << node.index << ")");
        cout << "Coset with index " << node.index << " has the following states:" << endl;
        node.states.Write();
        if(node.pFather!=NULL)
            cout << "Parent coset has index: " << node.pFather->index << endl;
        else
            cout << "Coset is the root of the tree" << endl;
        if(node.pBrother!=NULL)
            cout << "Coset has brother coset with index: " << node.pBrother->index << endl;
        else
            cout << "Coset has no brother" << endl;
        if(node.pFirstChild!=NULL && node.pSecondChild!=NULL)
            cout << "Child cosets have indices : " << node.pFirstChild->index << " and " << node.pSecondChild->index << endl;
        else
            cout << "Coset has no children" << endl;
        cout << "ro is stable with respect to this coset (1=yes; 0=no): " << node.nonSplitting << endl;
        cout << "Info-map of coset: " << endl;
        EventSet::Iterator eIt;
        for(eIt=gen->AlphabetBegin();eIt != gen->AlphabetEnd(); ++eIt)
            node.writeInfoMap(*eIt);

        if(node.pFirstChild!=NULL)
            writeNode(*(node.pFirstChild));
        if(node.pSecondChild!=NULL)
            writeNode(*(node.pSecondChild));
}

// Bisimulation::writeRo()
void Bisimulation::writeRo(void)
{
    OP_DF("Bisimulation::writeRo()");
    cout << "The Cosets with the following indices are in ro: " << endl;
    vector<Partition*>::iterator roIt;
    vector<Partition*>::iterator roItBegin =ro.begin();
    vector<Partition*>::iterator roItEnd = ro.end();
    for(roIt=roItBegin; roIt!=roItEnd; roIt++)
        cout << (*roIt)->index << endl;
    cout << endl;
}

}

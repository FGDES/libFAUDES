/** @file con_cctrim.cpp more efficient Trim() operation */

/*
 *
 * Copyright (C) 2011  Tomas Masopust
 *
 * Implementation of a bit more efficient Trim() function
 * based on graph algorithms
 *
 */

#include "con_cctrim.h"
#include <vector>
#include <string>
#include <queue>

using namespace std;

namespace faudes {

//  The graph adjecency list representation
class adjlist {
	private:
		unsigned int root;
		StateSet finalStates;
		unsigned int numberOfNodes;
     		struct node {
        	  	unsigned int state;
	          	node *link;
			node *blink;
		};
		vector<node*> nodeArray;
		vector<bool> examined;
	public:
		adjlist(const Generator& gen);
		unsigned int getRoot();
		unsigned int getNumberOfNodes();
		void Display();
		void SetExamined(bool b);
		StateSet Accessible();
		StateSet CoAccessible();
		void ReverseAndRoot();
		bool IsAccessible();
		~adjlist();
};

// Constructor
adjlist::adjlist(const Generator& gen) : 
  root(gen.InitState()), 
  finalStates(gen.MarkedStates()),
  numberOfNodes(gen.States().MaxIndex()), 
  nodeArray(gen.States().MaxIndex()),
  examined(gen.States().MaxIndex()) {
	// Better to use gen.Size() instead of gen.States().MaxIndex(), but there is a problem with numbered states!
	// Or add some additional table with refering my indexes to the original indexes
	// The array of nodes and examine flags
	for (unsigned int i=0; i < numberOfNodes; i++) {  
		nodeArray[i] = NULL;
		examined[i]  = false;
	}
	// List of adjecent nodes to each node
	TransSet::Iterator eit;
	for (eit = gen.TransRelBegin(); eit != gen.TransRelEnd(); ++eit) {
		if (eit->X1 != eit->X2) {  // we can ignore loops
			node* n = new node;
			n->state = eit->X2;
			n->link = nodeArray[eit->X1-1];
			n->blink = NULL;
			if (nodeArray[eit->X1-1] == NULL) {
				nodeArray[eit->X1-1] = n;
			} else {
				nodeArray[eit->X1-1]->blink = n;
				nodeArray[eit->X1-1] = n;
			}
		}
	}
	// remove multiedges
	vector<unsigned int> mult (numberOfNodes);
	for (unsigned int i=0; i < numberOfNodes; i++) {  
		mult[i] = 0;
	}
	for (unsigned int i=0; i < nodeArray.size(); i++) {
		node* q=nodeArray[i]; 
		while (q != NULL) {
			if (mult[q->state-1] != i+1) {
				mult[q->state-1] = i+1;
				q=q->link;
			} else {
				if (q->blink != NULL) {
					node* p = q->blink;
					p->link = q->link;
					if (p->link != NULL) {
						p->link->blink = p;
					}
					delete q;
					q = p->link;
				} else {
					nodeArray[i] = q->link;
					node* p = q;
					delete p;
					if (nodeArray[i] != NULL) {
						nodeArray[i]->blink = NULL;
					} 
					q = nodeArray[i];
				}
			}
		}
	}
}

unsigned int adjlist::getRoot() {
	return root;
}

unsigned int adjlist::getNumberOfNodes() {
	return numberOfNodes;
}

void adjlist::Display() {
	cout << " ===== Displaying the graph ===== " << endl;
	for (unsigned int i=0; i < nodeArray.size(); i++) {  
		node *q;
		cout << "Neighbours of " << i+1 << ": ";
		for(q=nodeArray[i]; q != NULL; q=q->link) {
			cout << q->state << ", ";
		}
		cout << endl;
	}
}

// Sets all examined flags of nodes to a value b
void adjlist::SetExamined(bool b) {
	for (unsigned int i=0; i < nodeArray.size(); i++) {
		examined[i] = false;
	}
}

// Returns the set of accessible states
StateSet adjlist::Accessible() {
	this->SetExamined(false);
	StateSet AccessibleSet;
	queue<Idx> que;
	que.push(root);
	examined[root-1]=true;
	Idx index;
	while (!que.empty()) {
		index = que.front();
		que.pop();
		AccessibleSet.Insert(index);
		node* q=nodeArray[index-1];
		while (q != NULL) {
			if (!examined[q->state-1]) {
				que.push(q->state);
				examined[q->state-1] = true;
			}
			q=q->link;
		}
	}
	return AccessibleSet;
}

// Returns the set of coaccessible states
StateSet adjlist::CoAccessible() {
	// Reverse the graph -- adds new root node
	this->ReverseAndRoot();
	// Check if the reversed rooted graph is accessible
	return this->Accessible();
}

// Reverses the graph and creates a new unique root
void adjlist::ReverseAndRoot() {
	vector<node*> v(numberOfNodes+1);
	for (unsigned int i=0; i < v.size(); i++) {
		v[i] = NULL;
	}
	for (unsigned int i=0; i < nodeArray.size(); i++) {  
		for(node* q=nodeArray[i]; q != NULL; q=q->link) {
			node* sNode = new node;
			sNode->state = i+1;
			sNode->link = v[q->state-1];
			v[q->state-1] = sNode;
		}
	}
	nodeArray.resize(numberOfNodes+1);
	nodeArray = v;
	// new root node linked with all original final states
	StateSet::Iterator sit;
	for (sit = finalStates.Begin(); sit != finalStates.End(); ++sit) {
		node* n = new node;
		n->state = *sit;
		n->link = nodeArray[numberOfNodes]; 
		nodeArray[numberOfNodes] = n;
	}
	root = numberOfNodes+1;
}

bool adjlist::IsAccessible() {
	StateSet acc = this->Accessible();
	if (this->getNumberOfNodes() == acc.Size()) {
		return true;
	}
	return false;
}

// Destructor
adjlist::~adjlist() {
	for (unsigned int i=0; i < nodeArray.size(); i++) {  
		node* p = nodeArray[i];
		node* q;
		while (p != NULL) {
			q = p->link;
			delete p;
			p = q;
		}
	}
} 
// end of the adjacency list


// ccTrim() function
bool ccTrim(const Generator& gen, Generator& trimGen) {
	// This means that states are number
	if (gen.Size() != gen.States().MaxIndex()) {
		cerr    << "=============================================================================\n" 
			<< "You probably use numbers for state names, and some state-numbers are missing.\n" 
			<< "Rename states for the algorithm to work more efficiently.\n"
		 	<< "=============================================================================" << endl;
	}

	if (!IsDeterministic(gen)) {
		cerr << "The generator is not deterministic." << endl;
		return false;
	}
	
	if (gen.InitStatesSize() != 1) {
		cerr << "The generator does not have only one initial state." << endl;
		return false;
	}

	trimGen = gen;
	// Create the graph representation of the generator gen
	adjlist graph (trimGen);
	// Set of accessible states
	StateSet acc = graph.Accessible();
	// Set of unaccessible states
	StateSet unacc = trimGen.States() - acc;
	// Set of coaccessible states
	StateSet coacc = graph.CoAccessible();
	// Set of uncoaccessible states
	StateSet uncoacc = trimGen.States() - coacc;
	// Set of states that are not accessible or coaccessible
	StateSet un = unacc + uncoacc;
	trimGen.DelStates(un);
	return un.Size()==0;
}

} //end namespace

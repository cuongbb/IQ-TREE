#ifndef TERRACE_COUNT
#define TERRACE_COUNT

#include "node.h"
//#include "splitgraph.h"
#include "pda/split.h"
#include <iostream>
#include <sstream>
#include "pda/hashsplitset.h"
#include "pda/splitset.h"
#include "constrainttree.h"
#include "mtree.h"
#include "mexttree.h"

class Terraces : public MExtTree
{
	public:
	Node* newNode(int node_id, const char* node_name) {
    	return new Node(node_id, node_name);
	}

	Node* newNode(int node_id, int node_name) {
    	return new Node(node_id, node_name);
	}


	void initializeTree(Node *node=NULL, Node* dad=NULL)
	{
    	if (!node) {
        	node = root;
        	nodeNum = leafNum;
        	branchNum = 0;
	    }
    	if (!node->isLeaf())
    	{
        	node->id = nodeNum;
        	nodeNum++;
	        //node->name = node->id;

    	}
	    //for (int i = 0; i < node->neighbors.size(); i++)
    	//if (node->neighbors[i]->node != dad)
    	FOR_NEIGHBOR_IT(node, dad, it) {
        	(*it)->id = branchNum;
        	(*it)->node->findNeighbor(node)->id = branchNum;
        	branchNum++;
    	    initializeTree((*it)->node, node);
    	}
	}

	
	void generateTrees(int size, bool binary)
	{
		if (size < 3)
			outError(ERR_FEW_TAXA);
		int i,j;

		// list of left- and right-end of branches
		NodeVector leftend, rightend, myleaves;
		Node *node;
		double len;
		

		root = newNode(0,"0");
		// create initial tree with 2 leaves
		node = newNode(1, "1");
		len = 1;								//setting the length as 1 as we just need the abscense matrix
		root->addNeighbor(node, len);
		node->addNeighbor(root, len);

		leftend.push_back(root);
		rightend.push_back(node);

		myleaves.push_back(root);
		myleaves.push_back(node);

		// additionally add a leaf
		for (i = 2; i < size; i++)
		{
			int index;
			for(j=1;j<=2*i-3;j++)
			{
				index = j;
				//cout << "step " << i << " left = " << leftend[index]->id << " right = " << rightend[index]->id << endl;
	
				// add an internal node
				Node *newnode = newNode(i+2*j,"NULL");
				// reconnect the left end
				node = leftend[index];
				for (NeighborVec::iterator it = node->neighbors.begin(); it != node->neighbors.end(); it++) 
					if ((*it)->node == rightend[index]) {
						(*it)->node = newnode;
						(*it)->length = len;
						newnode->addNeighbor(node, len);
						//cout << "  left " << leftend[index]->id << " " << newnode->id << endl;
						break;
					}
				// reconnect the right end
				node = rightend[index];
				for (NeighborVec::iterator it = node->neighbors.begin(); it != node->neighbors.end(); it++) 
					if ((*it)->node == leftend[index]) {
						(*it)->node = newnode;
						(*it)->length = len;
						newnode->addNeighbor(node, len);
						//cout << "  right " << rightend[index]->id  << " " << newnode->id  << endl;
						break;
					}

				// add a new leaf
				Node *newleaf = newNode(i, i);
				newnode->addNeighbor(newleaf, len);
				newleaf->addNeighbor(newnode, len);

				// update the leftend and rightend list
				leftend.push_back(newnode);
				rightend.push_back(rightend[index]);

				leftend.push_back(newnode);
				rightend.push_back(newleaf);

				rightend[index] = newnode;

				myleaves.push_back(newleaf);

			}
		}

		// indexing the leaves
		setLeavesName(myleaves);

		leafNum = size;
		nodeNum = leafNum;
		initializeTree();
	}	

};

int terraces_count_main(){};
int terraces_count_main3(){
{
	int a[10][10],s,g;
	int nos[10],sum=0;
	int i,j;
	Terraces T;
	cin>>s>>g;
								//for now g=2, just assuming but not ideally
								//should go with a use driven approach
	for(i=0;i<s;i++)
	{	for(j=0;j<g;j++)
		{	cin>>a[i][j];
		}
	}

	//for calculating the number of species for a given tree
	for(j=0;j<g;j++)
	{	sum=0;
		for(i=0;i<s;i++)
		{	if(a[i][j]==1)
				sum++;
		}
		nos[j]=sum;
	}
	cout<<"Finally, IT RAN";	//It didn't
	T.generateTrees(s,TRUE);
	return 0;
};

int terraces_count_main2();


#endif

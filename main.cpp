#include <iostream>
#include <stack>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <math.h>
#include <queue>
using namespace std;
struct Leaf;
struct Block;
struct Connection{
    Leaf * l1;
    Leaf * l2;
    int toyType;
    int ID;
    Leaf * parent;
    Leaf * child;
    Connection(Leaf * l1, Leaf * l2, int streetID, int toyType){
        this->l1 = l1;
        this->l2 = l2;
        this->ID = streetID;
        this->toyType = toyType;
    }
    void setParent(Leaf * parent){//if true - the l1 else the l2
        if(l1 == parent || l2 == parent) {
            this->parent = parent;
            this->child = l1 == parent ? l2 : l1;
        }
        else
            cout<<"ERROR";
    }
};
struct Leaf{
    Connection * parentPath;
    vector<Connection *> connections;
    int id;
    int eulerTourID;
    int levelInTree;
    int firstOccurrenceInEuler = -1;
    int lastOccurrenceInEuler;

    Block * block;
    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};
struct Block{
    int ID; //the number of the block starting from top
    int blockSize;

    int levelInTreeOfLowestFloor;
    int levelInTreeOfHighestFloor;

   // set<Leaf *> lowestFloor; // looking from the top
   //  set<Leaf *> highestFloor; // looking from the bottom

    map<Leaf *, map<int,int>> highestFloorMemoization; //the memoization and the leaf that is entered
    Block(int id, int s){
        this->ID = id;
        this->blockSize = s;
        levelInTreeOfLowestFloor = (blockSize) * ID;
        levelInTreeOfHighestFloor = levelInTreeOfLowestFloor + (blockSize - 1);
    }
};
bool belongsToSubTree(Leaf * root, Leaf * node){
    return (root->firstOccurrenceInEuler <= node->firstOccurrenceInEuler && root->lastOccurrenceInEuler >= node->firstOccurrenceInEuler);
}
vector<string> split(string str, char divider){
    vector<string> result;

    string currWord = "";
    for(int i = 0; i < str.size(); i ++){
        currWord+=str[i];
        if(str[i] == divider || str.size()-1 == i){
            result.push_back(currWord);
            currWord = "";
        }
    }
    return result;
}
void propagateParent(Leaf * root){
    stack<Leaf *> queue;
    queue.push(root);
    root->levelInTree = 0;
    while (!queue.empty()){
        Leaf * subject = queue.top();
        if(subject != root) {
            int height = subject->parentPath->parent->levelInTree + 1;
            subject->levelInTree = height;
        }
        queue.pop();
        for(int i = 0 ; i < subject->connections.size(); i ++){
            Connection * c = subject->connections.at(i);
            if(c != subject->parentPath) {
                Leaf * another = !(c->l1 == subject) ? c->l1 : c->l2;
                c->setParent(subject);
                another->setParentPath(c);
                queue.push(another);
            }
        }
    }
}

void blockPropagation(Leaf * node, vector<Block *> &blocks){
    int levelInTree = node->levelInTree;
    int blockHeight = blocks.at(0)->blockSize;
    int blockID = levelInTree / blockHeight;

    Block * block = blocks[blockID];

    node->block = block;


    if(levelInTree == block->levelInTreeOfHighestFloor)
        block->highestFloorMemoization[node];

    //block->floors[levelInBlock].push_back(node);

    for(int i = 0; i < node->connections.size(); i ++){
        if(node->connections.at(i) != node->parentPath){
            blockPropagation(node->connections.at(i)->child, blocks);
        }
    }
}

int main() {
    string line;
    getline(cin, line);
    vector<string> args = split(line, ' ');
    int townsCount = stoi(args[0]);
    int kindsOfToys = stoi(args[1]);
    int requests = stoi(args[2]);
    vector <Leaf * > towns;
    vector <Connection * > streets;
    for(int i = 0 ; i < townsCount; i ++){
        Leaf * town = new Leaf();
        town->id = i;
        towns.push_back(town);
    }
    for(int i = 0 ; i < townsCount - 1; i ++){
        getline(cin, line);
        args = split(line, ' ');
        Leaf * town1 = towns[stoi(args[0]) - 1];
        Leaf * town2 = towns[stoi(args[1]) - 1];
        int toyType = stoi(args[2]) - 1;
        Connection * conn = new Connection(town1, town2, i, toyType);
        town1->connections.push_back(conn);
        town2->connections.push_back(conn);
        streets.push_back(conn);
    }

    Leaf * rootTown = towns[0];
    rootTown->setParentPath(nullptr);
    propagateParent(rootTown);

    int height = 0;
    for(int i = 0; i < townsCount; i ++){
        Leaf * town = towns.at(i);
        if(town->levelInTree > height)
            height = town->levelInTree;
    }

    int blockSize = sqrt(height);
    int blocksCount = (height / blockSize) + (bool)(height % blockSize);
    vector<Block *> blocks;
    for(int i = 0; i < blocksCount; i ++){
        blocks.push_back(new Block(i, blockSize));
    }
    //cout<<blocksCount<<" bS = "<<blockSize<<endl;
    blockPropagation(rootTown, blocks);




    return 0;
}
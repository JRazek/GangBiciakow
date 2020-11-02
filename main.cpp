#include <iostream>
#include <stack>
#include <unordered_map>
#include <map>
#include <bits/stdc++.h>
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

    Block * block;
    Leaf * entranceLeafInBlock;

    int levelInBlock;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};
struct Block{
    int ID; //the number of the block starting from top
    int blockSize;

    int levelInTreeOfLowestFloor;
    int levelInTreeOfHighestFloor;

    unordered_map<Leaf *, pair<Leaf *, unordered_map<int,int>>> tpWay; // the entrance(the highest level) and escape from the block

    map<Leaf *, map<int,int>> highestFloorMemoization; //the memoization and the leaf that is entered
    Block(int id, int s){
        this->ID = id;
        this->blockSize = s;
        levelInTreeOfLowestFloor = (blockSize) * ID;
        levelInTreeOfHighestFloor = levelInTreeOfLowestFloor + (blockSize - 1);
    }
};
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

void blockPropagation(Leaf * root, vector<Block *> &blocks){

    stack<Leaf*> queue;
    unordered_map<int,int> memoization;

    queue.push(root);

    Leaf * lastEntranceToBlock;

    while(!queue.empty()){
        Leaf * node = queue.top();
        queue.pop();

        int levelInTree = node->levelInTree;
        int blockHeight = blocks.at(0)->blockSize;
        int blockID = levelInTree / blockHeight;
        int heightInBlock = levelInTree % blockHeight;

        node->levelInBlock = heightInBlock;

        if(heightInBlock == 0) {
            lastEntranceToBlock = node;
            memoization.clear();
        }

        Block * block = blocks[blockID];

        node->block = block;
        if(node->levelInBlock == block->blockSize - 1){//if the last node in block
            block->tpWay[node] = pair<Leaf *, unordered_map<int,int>>(lastEntranceToBlock, memoization);
        }
        for(int i = 0; i < node->connections.size(); i ++){
            if(node->connections.at(i) != node->parentPath){
                Leaf * child = node->connections.at(i)->child;
                queue.push(child);
            }
        }
    }
}

unordered_map<int, int> countTheWayToTheRoot(Leaf * node){
    unordered_map<int, int> toys;
    while(node->parentPath != nullptr){
        if(node->levelInBlock == node->block->blockSize - 1 && node->block->blockSize > 1){
            node = node->block->tpWay[node].first;
            if(unordered_map<int, int>::iterator it = node->block->tpWay[node].second.begin(); it != node->block->tpWay[node].second.end()){
                int toyType = it->first;
                int count = it->second;
                unordered_map<int, int>::iterator it2 = toys.find(toyType);
                if(it2 != toys.end()){
                    it2->second += count;
                }else{
                    toys[toyType] = count;
                }
            }
            continue;
        }
        unordered_map<int, int>::iterator it = toys.find(node->parentPath->toyType);
        if(it != toys.end()) {
            it->second ++;
        }
        else{
            toys[node->parentPath->toyType] = 1;
        }
        node = node->parentPath->parent;
    }
    return toys;
}

void updateNodeParentPath(Leaf * node){
    Block * block = node->block;
    node->levelInBlock;
}

int main() {
    time_t start, end;
    time(&start);
    ios_base::sync_with_stdio(false);

    string line;
    getline(cin, line);
    vector<string> args = split(line, ' ');
    int townsCount = stoi(args[0]);
    int kindsOfToys = stoi(args[1]);
    int requestsCount = stoi(args[2]);
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
    int blocksCount = (height / blockSize) + 1;
    vector<Block *> blocks;
    for(int i = 0; i < blocksCount; i ++){
        blocks.push_back(new Block(i, blockSize));
    }
    blockPropagation(rootTown, blocks);

    for(int i = 0; i < requestsCount; i ++){
        getline(cin, line);
        args = split(line, ' ');
        char request = args[0][0];
        if(request == 'Z'){
            Leaf * targetTown = towns.at(stoi(args[1]) - 1);
            cout<<countTheWayToTheRoot(targetTown).size()<<"\n";
        }
        else if(request == 'B'){
            Connection * targetStreet = streets.at(stoi(args[1]) - 1);
            int newToy = stoi(args[2]) - 1;
        }
    }


    time(&end);
    cout << "execution time = " << double(end - start);
    return 0;
}
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
struct Path;
struct BlockContainer;
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

    vector<Path *> blockPaths;
    Block * block;

    int levelInBlock;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};

struct Block{
    int ID; //the number of the block starting from top

    int levelInTreeOfLowestFloor;
    int levelInTreeOfHighestFloor;
    BlockContainer * blockContainer;

    Block(BlockContainer * bC, int id){
        this->blockContainer = bC;
        this->ID = id;
    }
};
struct Path{
    Block * block;
    vector<Leaf *> path;//from lower to higher
    Leaf * entry;   //the highest node
    Leaf * escape; // the lowest node
    unordered_map<int,int> lazyCounter;
    Path(Block * b, Leaf * entry){
        this->block = b;
        this->entry = entry;
    }
};
unordered_map<int,int> merge(unordered_map<int,int> m1, unordered_map<int,int> m2){
    for(unordered_map<int,int>::iterator it = m2.begin(); it != m2.end(); ++it){
        unordered_map<int,int>::iterator it2 = m1.find(it2->first);
        if(it2 == m1.end())
            m1[it2->first] = 0;
        m1[it2->first] ++;
    }
    return m1;
}


struct BlockContainer{
    vector<Block *> blocks;

    int blockSize;
    BlockContainer(int s){
        blockSize = s;
    }
    void blockPropagation(Leaf * root){
        stack<Leaf *> queue;
        queue.push(root);
        while (!queue.empty()){
            Leaf * node = queue.top();
            queue.pop();
            vector<Path *> paths = findPaths(node);
            for(int i = 0; i < paths.size(); i ++){
                Path * p = paths[i];
                p->escape = p->path[p->path.size() - 1];
                for(int j = 0 ; j < p->entry->connections.size(); j ++){
                    if(p->entry->connections[j] != p->entry->parentPath){
                        queue.push(p->entry->connections[j]->child);
                    }
                }
            }
        }
    };
    vector<Path *> findPaths(Leaf * node){
        int blockID = node->levelInTree / blockSize;
        int heightInBlock = node->levelInTree % blockSize;
        node->block = blocks[blockID];
        node->levelInBlock = heightInBlock;

        vector<Path *> paths;
        if(node->levelInBlock == blockSize - 1){
            Path * p = new Path(node->block, node);
            p->lazyCounter[node->parentPath->toyType] = 1;
            return {p};
        }
        for(int i = 0 ; i < node->connections.size(); i ++){
            Connection * c = node->connections.at(i);
            if(c != node->parentPath) {
                vector<Path *> tmp = findPaths(node->connections[i]->child);
                paths.insert(paths.end(), tmp.begin(), tmp.end());
            }
        }
        for(int i = 0; i < paths.size(); i ++){
            paths[i]->path.push_back(node);
            if(node->parentPath != nullptr){
                if(paths[i]->lazyCounter.find(node->parentPath->toyType) == paths[i]->lazyCounter.end()) {
                    paths[i]->lazyCounter[node->parentPath->toyType] = 1;
                }
                else {
                    paths[i]->lazyCounter[node->parentPath->toyType]++;
                }
            }
        }

        node->blockPaths = paths;

        return paths;
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
unordered_map<int, int> countTheWayToTheRoot(Leaf * node){
    while (node->parentPath != nullptr){
        if(node->levelInBlock == node->block->blockContainer->blockSize - 1){
            cout<<"here";
        }
        node = node->parentPath->parent;
    }
}

void updateNodeParentPath(Connection * conn, int newToy){
    int oldToy = conn->toyType;
    Leaf * node = conn->child;
    conn->toyType = newToy;
    if(oldToy != newToy) {
        for (int i = 0; i < node->blockPaths.size(); i++) {
            Path * p = node->blockPaths[i];
            p->lazyCounter[oldToy] --;
            if(p->lazyCounter[oldToy] == 0) {
                p->lazyCounter.erase(oldToy);
            }
            if(p->lazyCounter.find(newToy) == p->lazyCounter.end())
                p->lazyCounter[newToy] = 1;
            else
                p->lazyCounter[newToy] ++;
        }
    }
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
    BlockContainer * blockContainer = new BlockContainer(blockSize);
    for(int i = 0; i < blocksCount; i ++){
        blockContainer->blocks.push_back(new Block(blockContainer, i));
    }
    blockContainer->blockPropagation(rootTown);
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
            updateNodeParentPath(targetStreet, newToy);
        }
    }

    cout << towns[1000]->blockPaths.size() << "\n";

    time(&end);
    cout << "execution time = " << double(end - start);
    return 0;
}
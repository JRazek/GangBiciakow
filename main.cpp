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
struct HeavyPath;
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
    int levelInTree;

    HeavyPath * heavyPath;
    int idInHeavyPath = -1; // if our node is in heavy path - this will be the id of it counting from the top(lowest number floors)

    int subTreeSize = 0;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};
unordered_map<int,int> merge(unordered_map<int,int> m1, unordered_map<int,int> m2){
    unordered_map<int, int> smaller = m1.size() < m2.size() ? move(m1) : move(m2);
    unordered_map<int, int> bigger = m1.size() < m2.size() ? move(m2) : move(m1);
    unordered_map<int,int> result = bigger;

    for(unordered_map<int, int>::iterator it = smaller.begin(); it != smaller.end(); ++it){
        if(result.find(it->first) == result.end()){
            result[it->first] = it->second;
        }else{
            result[it->first] += it->second;
        }
    }
    return result;
}
struct HeavyPath{
    vector<Leaf *> path;
    /**
     * first vector stands for floor num(counting from the bottom), second for nodes in each floor.
     * the map is the memoization on each segment of the toys
     */
    struct BinaryNode{
        BinaryNode * parent;

        BinaryNode * left;
        BinaryNode * right;

        int rangeMin;
        int rangeMax;

        unordered_map<int,int> value;
        BinaryNode(BinaryNode * l, BinaryNode * r){
            this->left = l;
            this->right = r;
            if(l != nullptr && r != nullptr){
                this->rangeMin = l->rangeMin;
                this->rangeMax = r->rangeMax;
            }
        }
        BinaryNode(int range){
            this->rangeMin = range;
            this->rangeMax = range;
        }
    };
    vector< vector< BinaryNode*> > floors;
    void preprocessSegmentTree(){
        float logVal = log2(path.size());
        int height = logVal + (bool) logVal + 1;
        int firstFloorLength = pow(2, height - 1);
        for(int i = 0; i < height; i ++){
            floors.push_back(vector<BinaryNode *>());
            if(i == 0){
                for(int j = 0; j < firstFloorLength; j ++ ){
                    BinaryNode * leaf = new BinaryNode(j);
                    if(path[j] != nullptr){
                        leaf->value[path[j]->parentPath->toyType] = 1;
                    }
                    floors[0].push_back(leaf);
                }
            }
            else{
                int currFloorLength = firstFloorLength / pow(2, i);
                for(int j = 0; j < currFloorLength; j ++ ){
                    BinaryNode * leftChild = floors[i - 1][2 * j];
                    BinaryNode * rightChild = floors[i - 1][2 * j + 1];
                    BinaryNode * parent = new BinaryNode(leftChild, rightChild);
                    parent->value = merge(leftChild->value, rightChild->value);
                    floors[i].push_back(parent);
                }
            }
        }
    }
    unordered_map<int,int> segmentQuery(int min, int max){

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
int countSubTreeSizes(Leaf * node){
    for(int i = 0 ; i < node->connections.size(); i ++){
        Connection * c = node->connections.at(i);
        if(c != node->parentPath) {
            node->subTreeSize += countSubTreeSizes(node->connections[i]->child);
        }
    }
    node->subTreeSize += 1;
    return node->subTreeSize;
}

void findHeavyPaths(Leaf * node, vector<HeavyPath *> &heavyPaths){
    int edgeValue = node->subTreeSize / 2;
    for(int i = 0 ; i < node->connections.size(); i ++){
        Connection * c = node->connections.at(i);
        if(c != node->parentPath) {
            if(c->child->subTreeSize > edgeValue){
                if(node->parentPath != nullptr) {
                    if (node->heavyPath == nullptr) {
                        HeavyPath *p = new HeavyPath();
                        p->path.push_back(node);
                        p->path.push_back(c->child);

                        node->heavyPath = p;
                        c->child->heavyPath = p;

                        node->idInHeavyPath = 0;
                        c->child->idInHeavyPath = 1;

                        heavyPaths.push_back(p);
                    } else {
                        HeavyPath *p = node->heavyPath;
                        p->path.push_back(c->child);
                        c->child->heavyPath = p;
                        c->child->idInHeavyPath = p->path.size() - 1;
                    }
                }
            }
            findHeavyPaths(c->child, heavyPaths);
        }
    }
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

}

void updateNodeParentPath(Connection * conn, int newToy){

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

    for(int i = 0; i < requestsCount; i ++){
        getline(cin, line);
        args = split(line, ' ');
        char request = args[0][0];
        if(request == 'Z'){
            Leaf * targetTown = towns.at(stoi(args[1]) - 1);
          //  cout<<countTheWayToTheRoot(targetTown).size()<<"\n";
        }
        else if(request == 'B'){
            Connection * targetStreet = streets.at(stoi(args[1]) - 1);
            int newToy = stoi(args[2]) - 1;
            updateNodeParentPath(targetStreet, newToy);
        }
    }
    countSubTreeSizes(rootTown);
    vector<HeavyPath *> heavyPaths;
    findHeavyPaths(rootTown, heavyPaths);
    for(auto h : heavyPaths){
        h->preprocessSegmentTree();
    }
    time(&end);
    cout << "execution time = " << double(end - start);
    return 0;
}
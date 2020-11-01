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
struct ChangeQueryInterval;
int min(vector<int> values){
    int min = values[0];
    for(int i = 0; i < values.size(); i++){
        if(min < values[i])
            min = values[i];
    }
    return min;
}
struct SparseTableMin{
    vector<int> values;
    vector<vector<int>*> sparseTable;
    SparseTableMin(vector<int> values){
        this->values = values;
        int columns = values.size();
        int rows = log2(values.size());
        for(int i = 0; i < rows; i ++){
            sparseTable.push_back(new vector<int>());
            int segmentSize = pow(2, i);
            for(int j = 0; j < columns - segmentSize + 1; j ++){
                if(i == 0){
                    sparseTable[0]->push_back(values[j]);
                } else{
                    sparseTable[i]->push_back(getMinimum(j, j + segmentSize - 1));
                }
            }
        }
    }
    int getMinimum(int from, int to){
        int length = to - from + 1;
        int rowNum = (int)(log2(length)) - 1;
        int fixSize = pow(2, rowNum);
        vector<int> * row = sparseTable[rowNum];
        int firstMin = row->at(from);
        int secondMin = row->at(to - fixSize + 1);
        int min = firstMin > secondMin ? secondMin : firstMin;
        return min;
    }
};

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

    //All children AND parent. Must omit parent by check. after propagating the root
    vector<Connection *> connections;
    int id;
    int eulerTourID;
    int levelInTree;
    int firstOccurrenceInEuler = -1;
    int lastOccurrenceInEuler;

    queue<ChangeQueryInterval *> intervalsAccessed;

    queue<ChangeQueryInterval *> intervalsChanged;

    unordered_map<int, int> memoization;
    int lastSavedMemo = -1;
    unordered_set<int> queriesNumsToMemo;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};

bool belongsToSubTree(Leaf * root, Leaf * node){
    return (root->firstOccurrenceInEuler <= node->firstOccurrenceInEuler && root->lastOccurrenceInEuler >= node->firstOccurrenceInEuler);
}

struct ChangeQueryInterval{
    int num;
    Leaf * changedRootNode;
    set<Leaf *> accessedNodesDuringQuery;
    int toyType;
    set<Leaf *> directlyImpacted; //only in subtree and only during that query accessed
    void findDirectlyImpacted(){
        set<Leaf*>::iterator it = accessedNodesDuringQuery.begin();
        while (it != accessedNodesDuringQuery.end()){
            Leaf * node = *it;
            if(belongsToSubTree(changedRootNode, node))
                directlyImpacted.insert(node);
            it++;
        }
    }
    ChangeQueryInterval(Leaf * root, int num, int toyType){
        this->changedRootNode = root;
        this->num = num;
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
    while (!queue.empty()){
        Leaf * subject = queue.top();
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

void eulerTourIndexing(Leaf * node, int * index, vector<Leaf *> &tourOrder, int level = 0){
    node->eulerTourID = *index;
    node->levelInTree = level;
    *index += 1;
    tourOrder.push_back(node);
    for(int i = 0; i < node->connections.size(); i ++){
        if(node->connections.at(i) != node->parentPath){
            eulerTourIndexing(node->connections.at(i)->child, index, tourOrder, ++level);
            tourOrder.push_back(node);
        }
    }
}

void countPathToTheRoot(Leaf * node, int queryPerformed){
    //this will write to node
    stack<Leaf *> queue;
    Leaf * currentNode = node;
    while (currentNode->parentPath != nullptr){
        queue.push(currentNode);
        currentNode = currentNode->parentPath->parent;
        //what if found?
    }
    unordered_map<int, int> totalOccurrences;
    while (!queue.empty()){
        Leaf * subject = queue.top();
        queue.pop();
        Connection * street = subject->parentPath;
        if( totalOccurrences.find(street->toyType) == totalOccurrences.end())
            totalOccurrences[street->toyType] = 0;
        totalOccurrences[street->toyType] ++;
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

    int changeQueryNum = 0;//if 0 than it means that there's no change.
    map<int, ChangeQueryInterval *> changeQueryIntervals;//number and query itself
    changeQueryIntervals[0] =  new ChangeQueryInterval(nullptr, changeQueryNum, 0);


    for(int i = 0 ; i < requests; i ++){
        getline(cin, line);
        vector<string> args = split(line, ' ');
        bool typeOfQuery = args[0][0] == 'Z' ? true : false;//first argument and the first char in the string
        if(typeOfQuery){
            Leaf * targetTown = towns.at(stoi(args[1]) - 1);

        }
        else if(!typeOfQuery){
            Connection * street = streets.at(stoi(args[1]) - 1);
            int newToy = stoi(args[2]) - 1;
            Leaf * targetTown = street->child;
            if(street->toyType != newToy) {
                changeQueryNum++;
            }
        }
    }
    vector<Leaf *> tourOrder;
    vector<int> eulerTourIDs;
    int * tmp = new int(0);
    eulerTourIndexing(rootTown, tmp,tourOrder);

    for(int i = 0; i < tourOrder.size(); i ++){
        Leaf * town = tourOrder.at(i);
        if(town->firstOccurrenceInEuler == -1)
            town->firstOccurrenceInEuler = i;
        town->lastOccurrenceInEuler = i;
        eulerTourIDs.push_back(town->eulerTourID);
    }

    SparseTableMin sparseTableMin(eulerTourIDs);
    cout<<sparseTableMin.getMinimum(700, 701);
   // SparseTableMin sparseTableMin({5,1,4,7,6,3,0,2});
    return 0;
}
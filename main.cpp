#include <iostream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>

using namespace std;
struct Leaf;

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
    vector<Connection *> connections;//all children AND parent. Must omit parent by check. after propagating the root
    int id;
    int eulerTourID;

    bool queriesPerformed = false;//defines if we perform queries. Used in Euler's tour to index subNodes

    unordered_map<int, int> memoizationTable;

    unordered_set<Leaf *> directMarkedChildren;
    unordered_set<Leaf *> subTreeAllMarkedChildren;


    unordered_set<Leaf *> changeQueriesBelonging;
    unordered_set<int> changeQueries;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
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

void eulerTourIndexing(Leaf * node, int * index, Leaf * closestQueriedParent){
    node->eulerTourID = *index;
    if(node->queriesPerformed){
        if(closestQueriedParent != nullptr)
            closestQueriedParent->directMarkedChildren.insert(node);

        closestQueriedParent = node;
        //adding closest marked children
    }
    *index += 1;
    for(int i = 0; i < node->connections.size(); i ++){
        if(node->connections.at(i) != node->parentPath){
            eulerTourIndexing(node->connections.at(i)->child, index, closestQueriedParent);
        }
    }
}

vector<Leaf *> getChangeList(Leaf * changeRoot, int changeQuery){
    vector<Leaf *> subTreeNodes;

    vector<Leaf *> affectedNodes;

    stack<Leaf *> queue;
    queue.push(changeRoot);
    while (!queue.empty()) {
        Leaf *subject = queue.top();
        queue.pop();
        if(!subject->subTreeAllMarkedChildren.empty()){
            subTreeNodes.insert(subTreeNodes.end(), subject->subTreeAllMarkedChildren.begin(), subject->subTreeAllMarkedChildren.end());
            //rekurencyjne zapisywanie do dalszych, jezeli change query jest performed
            continue;
        }
        subTreeNodes.push_back(subject);
        for (auto node : subject->directMarkedChildren) {
            queue.push(node);
        }
    }
    for(auto node : subTreeNodes){
        changeRoot->subTreeAllMarkedChildren.insert(node);
        bool isPresent = node->changeQueries.find(changeQuery) != node->changeQueries.end();
        if(isPresent){
            affectedNodes.push_back(node);
        }
    }
    return affectedNodes;
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
    //saving the queries for preProcessing of the tree
    vector<tuple<bool, int, int>> queries;
    int changeQuery = 0;
    map<int, unordered_set<Leaf *>> accessedDuringQuery; //not sure if they are actually the ones that are impacted
    for(int i = 0 ; i < requests; i ++){
        getline(cin, line);
        vector<string> args = split(line, ' ');
        bool typeOfQuery = args[0][0] == 'Z' ? true : false;//first argument and the first char in the string
        if(typeOfQuery){
            int targetTownID = stoi(args[1]) - 1;
            Leaf * targetTown = towns[targetTownID];
            targetTown->queriesPerformed = true;
            queries.push_back(make_tuple(typeOfQuery, targetTownID, 0));
            targetTown->changeQueries.insert(changeQuery);
            if(accessedDuringQuery.find(changeQuery) == accessedDuringQuery.end()){
                accessedDuringQuery[changeQuery] = unordered_set<Leaf *>();
            }
            accessedDuringQuery[changeQuery].insert(targetTown);
        }
        else if(!typeOfQuery){
            int newToy = stoi(args[2]) - 1;
            int targetStreetID = stoi(args[1]) - 1;
            streets.at(targetStreetID)->child->changeQueries.insert(changeQuery);
            queries.push_back(make_tuple(typeOfQuery, targetStreetID, newToy));
            streets.at(targetStreetID)->child->queriesPerformed = true;
            changeQuery ++;
        }
    }
    vector<Leaf *> tourOrder;
    int * tmp = new int(0);
    eulerTourIndexing(rootTown, tmp, rootTown);
    cout<<*tmp;
    delete(tmp);

    unordered_map<int, vector<Leaf *>> impactedByCertainQuery;

    int currQuery = 0;
    for(int i = 0; i < queries.size(); i ++){
        bool requestType = get<0>(queries.at(i));
        if(!requestType){
            Connection * targetStreet = streets[get<1>(queries.at(i))];
            int newToy = get<2>(queries.at(i));
            Leaf * changeRoot = targetStreet->child;//only below the street are affected
            impactedByCertainQuery[currQuery] = getChangeList(changeRoot, currQuery);
            ////you must fucking test it or the fucking yeti will fuck u during sleep   i m   d e a d
        }
    }

/*
    for(int i = 0; i < queries.size(); i ++){
        bool requestType = get<0>(queries.at(i));
        if(requestType){
            Leaf * targetTown = towns[get<1>(queries.at(i))];
            //count and mark others
        }
        else if(!requestType){
            Connection * targetStreet = streets[get<1>(queries.at(i))];
            int newToy = get<2>(queries.at(i));
            int oldToy = targetStreet->toyType;
            //update
        }
    }*/
    return 0;
}
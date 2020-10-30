#include <iostream>
#include <stack>
#include <map>
#include <vector>
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
    vector<Connection *> connections;//all children after propagating the root
    int id;
    int eulerTourID;
    map<int, int> memoizationTable;
    int typesOfToys;
    bool markedToSave = false;
    vector<Leaf *> firstDegreeMarkedChildren;
    Leaf(int typesOfToys){
        this->typesOfToys = typesOfToys;
    }
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

void countOccurrencesFromTown(Leaf * targetTown){
    map<int,int> totalOccurrencesOfToys;

    stack<Connection *> queue;
    Connection * currParentPath = targetTown->parentPath;
    queue.push(currParentPath);
    while(currParentPath->parent->parentPath != nullptr){
        currParentPath = currParentPath->parent->parentPath;
        queue.push(currParentPath);
        if(currParentPath->parent->memoizationTable.size() > 0){
            totalOccurrencesOfToys = currParentPath->parent->memoizationTable;
            break;
        }
    }
    while (!queue.empty()){
        Connection * consideredConnection = queue.top();
        queue.pop();
        int toyType = consideredConnection->toyType;
        if(totalOccurrencesOfToys.find(toyType) == totalOccurrencesOfToys.end()){
            totalOccurrencesOfToys[toyType] = 0;
        }
        totalOccurrencesOfToys[toyType] ++;
        consideredConnection->child->memoizationTable = totalOccurrencesOfToys;
    }
}

void updateSubTree(Connection * rootConnection, int oldToyType, int newToyType){
    stack<Connection *> queue;
    rootConnection->toyType = newToyType;
    queue.push(rootConnection);
    while (!queue.empty()){
        Connection * subject = queue.top();
        queue.pop();
        if(subject->child->memoizationTable.size() > 0){
            subject->child->memoizationTable[oldToyType] --;
            if(subject->child->memoizationTable[oldToyType] == 0){
                subject->child->memoizationTable.erase(oldToyType);
            }
            subject->child->memoizationTable[newToyType] ++;
            for(int i = 0; i < subject->child->connections.size(); i ++){
                if(subject->child->connections.at(i) != subject->child->parentPath){
                    queue.push(subject->child->connections.at(i));
                }
            }
        }
    }
}
void eulerTourIndexing(Leaf * root){
    stack<Leaf *> queue;
    queue.push(root);
    int index = 0;
    while (!queue.empty()){
        Leaf * subject = queue.top();
        
    }
}
//problem is accessing m*z times each toy, even if equal to 0.
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
        Leaf * town = new Leaf(kindsOfToys);
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


    //saving the queries and marking them with eulers
    vector<tuple<bool, int, int>> queries;
    for(int i = 0 ; i < requests; i ++){
        getline(cin, line);
        vector<string> args = split(line, ' ');
        bool typeOfQuery = args[0][0] == 'Z' ? true : false;//first argument and the first char in the string
        if(typeOfQuery){
            int targetTownID = stoi(args[1]) - 1;
            Leaf * targetTown = towns[targetTownID];
            targetTown->markedToSave = true;
            queries.push_back(make_tuple(typeOfQuery, targetTownID, 0));

        }
        if(!typeOfQuery){
            int newToy = stoi(args[2]) - 1;
            int targetStreetID = stoi(args[1]) - 1;
            if(!streets.at(targetStreetID)->toyType == newToy){
                queries.push_back(make_tuple(typeOfQuery, targetStreetID, newToy));
            }
        }
    }

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
    }
    return 0;
}
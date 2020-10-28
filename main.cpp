#include <iostream>
#include <stack>
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
    int * memoizationTable = nullptr;//each index stands for amount of toys of specific type
    int typesOfToys;
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
    int * totalOccurrencesOfToys = new int[targetTown->typesOfToys];
    stack<Connection *> queue;
    Connection * currParentPath = targetTown->parentPath;

    for(int i = 0; i < targetTown->typesOfToys; i ++){
        totalOccurrencesOfToys[i] = 0;
    }
    //making the path O(n)
    while(currParentPath != nullptr){
        queue.push(currParentPath);
        if(currParentPath->parent->memoizationTable != nullptr){
            totalOccurrencesOfToys = currParentPath->parent->memoizationTable;
            break;
            //to test
        }
        currParentPath = currParentPath->parent->parentPath;
    }


    while (!queue.empty()){
        Connection * currConn = queue.top();
        totalOccurrencesOfToys[currConn->toyType] ++;
        currConn->child->memoizationTable = totalOccurrencesOfToys;
        queue.pop();
    }
}

void updateSubTree(Leaf * root, int oldToyType, int newToyType){
    stack<Leaf *> queue;
    root->parentPath->toyType = newToyType;
    queue.push(root);
    while (!queue.empty()){
        Leaf * subject = queue.top();
        queue.pop();
        if(subject->memoizationTable != nullptr) {
            subject->memoizationTable[oldToyType]--;
            subject->memoizationTable[newToyType]++;
            for (int i = 0; i < subject->connections.size(); i++) {
                Connection *c = subject->connections.at(i);
                if (c != subject->parentPath) {
                    queue.push(c->child);
                }
            }
        }
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

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
    for(int i = 0; i < requests; i ++){
        getline(cin, line);
        vector<string> args = split(line, ' ');
        char requestType = args[0][0];//first argument and the first char in the string
        if(requestType == 'Z'){
            Leaf * targetTown = towns[stoi(args[1]) - 1];
            countOccurrencesFromTown(targetTown);
            int different = 0;
            for(int j = 0; j < kindsOfToys; j ++){
                if(targetTown->memoizationTable[j] > 0)
                    different ++;
            }
            cout<<different<<"\n";
        }
        else if(requestType == 'B'){
            Connection * targetStreet = streets[stoi(args[1]) - 1];
            int newToy = stoi(args[2]) - 1;
            int oldToy = targetStreet->toyType;
            if(oldToy != newToy){
                updateSubTree(targetStreet->child, oldToy, newToy);
            }
        }
    }
    return 0;
}
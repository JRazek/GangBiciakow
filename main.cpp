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
    Connection(Leaf * l1, Leaf * l2, int streetID){
        this->l1 = l1;
        this->l2 = l2;
        this->ID = streetID;
    }
    void setParent(bool first){//if true - the l1 else the l2
        parent = first ? l1 : l2;
    }
};
struct Leaf{
    Connection * parentPath;
    vector<Connection *> connections;//all children after propagating the root
    int id;
    vector<int> memoizationTable;//each index stands for amount of toys of specific type
    void setParent(Connection * parentPath){
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
                bool first = !(c->l1 == subject) ? 1 : 0;
                Leaf *another = first ? c->l1 : c->l2;
                c->setParent(first);
                another->setParent(c);
                queue.push(another);
            }
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
        Connection * conn = new Connection(town1, town2, i);
        town1->connections.push_back(conn);
        town2->connections.push_back(conn);
    }
    Leaf * rootTown = towns[0];
    propagateParent(rootTown);
    for(int i = 0; i < requests; i ++){
        getline(cin, line);
        vector<string> args = split(line, ' ');
        char requestType = args[0][0];//first argument and the first char in the string
        if(requestType == 'Z'){
            Leaf * targetTown = towns[stoi(args[1])];

        }
        else if(requestType == 'B'){

        }
    }
    return 0;
}
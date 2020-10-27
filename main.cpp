#include <iostream>

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
        l1 = l1;
        l2 = l2;
        ID = streetID;
    }
    void setParent(bool first){//if true - the l1 else the l2
        parent = first ? l1 : l2;
    }
};
struct Leaf{
    Connection * parentPath;
    vector<Connection *> childPaths;
    int id;
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
        Leaf * town1 = towns[stoi(args[0])];
        Leaf * town2 = towns[stoi(args[1])];
        int toyType = stoi(args[2]);
        Connection * conn = new Connection(town1, town2, i);
    }
    return 0;
}
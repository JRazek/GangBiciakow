#include <iostream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <deque>
#include <map>
#include <vector>
#include <math.h>

using namespace std;
struct Leaf;
struct Range{
    int min;
    int max;
    Range(int min, int max){
        this->min = min;
        this->max = max;
    }
    static Range * commonPart(const Range * r1, const Range * r2){
        int min = r1->min > r2->min ? r1->min : r2->min;
        int max = r1->max < r2->max ? r1->max : r2->max;
        return new Range(min, max);
    }
};
struct Update{
    const int timeStamp;//unique num for each update. id of it.

    const int lowIndexUpdated;

    const int highIndexUpdated;

    const int prevValue; // the same value for both of indexes

    const int nextValue; // the same value for both of indexes
    Update(int timeStamp, int lowIndexUpdated, int highIndexUpdated, int prevVal, int nextValue) :
    timeStamp(timeStamp), lowIndexUpdated(lowIndexUpdated), highIndexUpdated(highIndexUpdated), prevValue(prevVal), nextValue(nextValue){}
};
struct Query{
    const int low;
    const int high;
    const int timeStamp;

    int answer = -1;//undefined for the time until finding the answer

    Query(int low, int high, int timeStamp) : low(low), high(high), timeStamp(timeStamp){}

    struct Comparator {
        int blockSize;
        Comparator(int blockSize){
            this->blockSize = blockSize;
        }
        bool operator()(const Query * q1, const Query * q2){
            return compareQuery(q1, q2, blockSize);
        }
    private :
        bool compareQuery(const Query * q1, const Query * q2, const int blockSize) {
            //no need to check the first as all are the same size.
            if (q1->timeStamp != q2->timeStamp) {
                return q1->timeStamp < q2->timeStamp;
            }
            int q1HighBlock = q1->high / blockSize;
            int q2HighBlock = q2->high / blockSize;
            return q1HighBlock < q2HighBlock;
        }
    };
};
struct Edge{
    Leaf * l1;
    Leaf * l2;
    int ID;
    Leaf * parent;
    Leaf * child;


    int lDFS;
    int hDFS;

    int toyType;
    Edge(Leaf * l1, Leaf * l2, int streetID, int toyType){
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
    Edge * parentPath;
    vector<Edge *> edges;
    int id;
    int levelInTree;


    void setParentPath(Edge * parentPath){
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
    root->levelInTree = 0;
    while (!queue.empty()){
        Leaf * subject = queue.top();
        if(subject != root) {
            int height = subject->parentPath->parent->levelInTree + 1;
            subject->levelInTree = height;
        }
        queue.pop();
        for(int i = 0 ; i < subject->edges.size(); i ++){
            Edge * c = subject->edges.at(i);
            if(c != subject->parentPath) {
                Leaf * another = !(c->l1 == subject) ? c->l1 : c->l2;
                c->setParent(subject);
                another->setParentPath(c);
                queue.push(another);
            }
        }
    }
}

vector<Edge *> dfsOrder(Leaf * root){
    stack<pair<Leaf *, bool>> stack;
    stack.push(make_pair(root, false));

    vector<Edge *> order;

    int index = 0;
    while (!stack.empty()){
        Leaf * node = stack.top().first;
        bool comeBack = stack.top().second;
        stack.pop();
        if(!comeBack){//in case if we are the first time visiting
            stack.push(make_pair(node, true));

            if(node->parentPath != nullptr) {
                order.push_back(node->parentPath);
                node->parentPath->lDFS = index;
            }
            for(int i = 0; i < node->edges.size(); i ++){
                Edge * e = node->edges[i];
                if(e != node->parentPath){
                    stack.push(make_pair(e->child, false));
                }
            }
        }
        else if(node->parentPath != nullptr){
            order.push_back(node->parentPath);
            node->parentPath->lDFS = index;
        }
        if(node->parentPath != nullptr){
            index++;
        }
    }
    return order;
}
void sortQueries(vector<Query *> &queries){
    sort(queries.begin(), queries.end(), Query::Comparator(sqrt(queries.size())));
}

void performUpdates(vector<Edge *> &dfsOrdered, const vector<Update *> &updates, const int currentTimeStamp, const int designatedTimeStamp){
    //updates vector is sorted;
    if(currentTimeStamp < designatedTimeStamp){
        int updateToPerformNum = currentTimeStamp;
        while(updateToPerformNum < designatedTimeStamp){
            Update * update = updates[updateToPerformNum];
            Edge * updatedStreet = dfsOrdered[update->lowIndexUpdated];
            //dont have to perform for both as it is the same object
            updatedStreet->toyType = update->nextValue;
            updateToPerformNum ++;
        }
    }
    if(currentTimeStamp > designatedTimeStamp){
        int updateToUndoNum = currentTimeStamp - 1;
        while (updateToUndoNum >= designatedTimeStamp){
            Update * update = updates[updateToUndoNum];
            Edge * updatedStreet = dfsOrdered[update->lowIndexUpdated];
            //dont have to perform for both as it is the same object
            updatedStreet->toyType = update->prevValue;
            updateToUndoNum --;
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
    vector <Edge * > streets;
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
        int toy = stoi(args[2]) - 1;
        Edge * edge = new Edge(town1, town2, i, toy);
        town1->edges.push_back(edge);
        town2->edges.push_back(edge);
        streets.push_back(edge);
    }

    Leaf * rootTown = towns[0];
    rootTown->setParentPath(nullptr);
    propagateParent(rootTown);

    vector<Edge *> dfsOrdered = dfsOrder(rootTown);



    vector<Query *> queries;
    vector<Update *> updates;
    {
        int timeStamp = 0;
        for (int i = 0; i < requestsCount; i++) {
            getline(cin, line);
            args = split(line, ' ');
            char request = args[0][0];
            if (request == 'Z') {
                Leaf * targetTown = towns.at(stoi(args[1]) - 1);
                queries.push_back(new Query(0, targetTown->parentPath->lDFS, timeStamp));
            } else if (request == 'B') {
                timeStamp += 1;
                Edge * targetStreet = streets[stoi(args[1]) - 1];
                int newToyType = stoi(args[2]) - 1;
                int oldToyType = targetStreet->toyType;
                updates.push_back(new Update(timeStamp, targetStreet->lDFS, targetStreet->hDFS, oldToyType, newToyType));
            }
        }
    }
    //performUpdates(dfsOrdered, updates, 0, 1);
    //performUpdates(dfsOrdered, updates, 1, 0);
    for(auto q : queries){
        if(q == nullptr){
            cout<<"here!";
        }
    }
    sortQueries(queries);
    Range * mos = new Range(0,0);//there is minimum 1 query
    unordered_map<Edge *, bool> usedEdges;//if false - once used, if true - twice used

    unordered_map<int, int> toyOccurrence;//first for toyType, second for frequency

    // mos is at 0,0 so adding the starting values
    usedEdges[dfsOrdered[0]] = false;
    toyOccurrence[dfsOrdered[0]->toyType] = 1;

    int currentTimeStamp = 0;

    for(int i = 0; i < queries.size(); i ++){
        Query * query = queries[i];
        while(mos->min != query->low){
            if(mos->min < query->low){
                mos->min ++;
            }
            if(mos->min > query->low){
                mos->min --;
            }
            //never in this loop.
            cout<<"error";
        }
        while(mos->max != query->high){
            if(mos->max < query->high){
                mos->max ++;
                Edge * affectedEdge = dfsOrdered[mos->max];//adding to chain
                if(usedEdges.find(affectedEdge) != usedEdges.end()){
                    usedEdges[affectedEdge] = true;
                    toyOccurrence[affectedEdge->toyType] --;
                    if(toyOccurrence[affectedEdge->toyType] == 0){
                        toyOccurrence.erase(affectedEdge->toyType);
                    }
                }else{
                    usedEdges[affectedEdge] = false;
                    if(toyOccurrence.find(affectedEdge->toyType) == toyOccurrence.end()){
                        toyOccurrence[affectedEdge->toyType] = 0;
                    }
                    toyOccurrence[affectedEdge->toyType] ++;
                }
            }
            if(mos->max > query->high){
                Edge * affectedEdge = dfsOrdered[mos->max];//removing it from chain
                if(usedEdges[affectedEdge] == true){
                    usedEdges[affectedEdge] = false;
                    if(toyOccurrence.find(affectedEdge->toyType) == toyOccurrence.end()){
                        toyOccurrence[affectedEdge->toyType] = 0;
                    }
                    toyOccurrence[affectedEdge->toyType] ++;
                }else{
                    usedEdges.erase(affectedEdge);
                    toyOccurrence[affectedEdge->toyType] --;
                    if(toyOccurrence[affectedEdge->toyType] == 0){
                        toyOccurrence.erase(affectedEdge->toyType);
                    }
                }
                mos->max --;
            }
        }
        performUpdates(dfsOrdered, updates, currentTimeStamp, query->timeStamp);
        currentTimeStamp = query->timeStamp;
    }

    for(auto n : towns){
        delete n;
    }
    for(auto c : streets){
        delete c;
    }
    for(auto q : queries){
        delete q;
    }
    for(auto u : updates){
        delete u;
    }
    time(&end);
    cout << "execution time = " << double(end - start);
    return 0;
}
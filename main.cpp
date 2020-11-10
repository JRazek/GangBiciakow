#include <iostream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <math.h>

using namespace std;
struct Leaf;
struct Toy;
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
    ~SparseTableMin(){
        for(auto v : sparseTable){
            delete v;
        }
    }
};
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
struct Connection{
    Leaf * l1;
    Leaf * l2;
    Toy * toy;
    int ID;
    Leaf * parent;
    Leaf * child;
    Connection(Leaf * l1, Leaf * l2, int streetID, Toy * toy){
        this->l1 = l1;
        this->l2 = l2;
        this->ID = streetID;
        this->toy = toy;
    }
    void setParent(Leaf * parent){//if true - the l1 else the l2
        if(l1 == parent || l2 == parent) {
            this->parent = parent;
            this->child = l1 == parent ? l2 : l1;
        }
        else
            cout<<"ERROR";
    }
    ~Connection(){
        delete toy;
    }
};
struct Leaf{
    Connection * parentPath;
    vector<Connection *> connections;
    int id;
    int levelInTree;
    int eulerTourID;


    int subTreeSize = 0;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};
struct Toy{
    int toyType;
    int nextOccurrenceInEuler = 2147483645;
    int prevOccurrenceInEuler = -1; //isnt very needed and could be done without it but it doesnt matter and doesnt have to be updated.

    int positiveOccurrenceInEuler;
    int negativeOccurrenceInEuler;
    Toy(int toyType){
        this->toyType = toyType;
    }
};
vector<Toy *> merge(const vector<Toy *> &v1, const vector<Toy *> &v2){
    int i = 0, j = 0, k = 0;
    bool iOverload = false, jOverload = false;

    vector<Toy *> merged;
    if(v1.empty()){merged = v2; return merged;}
    if(v2.empty()){merged = v1; return merged;}
    while(k != (v1.size() + v2.size())){
        int firstValue = 2147483647;
        int secondValue = 2147483647;
        if(i >= v1.size())
            iOverload = true;
        if(j >= v2.size())
            jOverload = true;

        if(!iOverload)
            firstValue = v1[i]->nextOccurrenceInEuler;
        if(!jOverload)
            secondValue = v2[j]->nextOccurrenceInEuler;

        if(firstValue < secondValue){
            merged.push_back(v1[i]);
            i++;
        }else{
            merged.push_back(v2[j]);
            j++;
        }
        k++;
    }
    return merged;
}
void eulerTourIndexing(Leaf * node, int &index, vector<pair<Connection *, bool>> &tourStreetOrder, int level = 0){
    node->eulerTourID = index;
    node->levelInTree = level;
    index += 1;
    ///todo stack overflow change to iterations instead!
    if(node->parentPath != nullptr) {
        tourStreetOrder.push_back(make_pair(node->parentPath, true));
        node->parentPath->toy->positiveOccurrenceInEuler = level;
    }
    for(int i = 0; i < node->connections.size(); i ++){
        if(node->connections.at(i) != node->parentPath){
            Connection * c = node->connections.at(i);
            eulerTourIndexing(c->child, index, tourStreetOrder, ++level);
        }
    }
    if(node->parentPath != nullptr) {
        tourStreetOrder.push_back(make_pair(node->parentPath, false));
        node->parentPath->toy->negativeOccurrenceInEuler = level;
    }

}

struct SegmentTree{
    struct BinaryNode{
        int id;
        Range * range;

        unordered_set<Toy *> negativeSet;//first stand for toyType second for quantity. The important thing is that it is not negative in map!

        vector<Toy *> toys; //sorted by the elements next occurrence

        BinaryNode(int id, Range * r){
            this->id = id;
            this->range = r;
        }
        ~BinaryNode(){
            delete range;
        }
    };
    vector< BinaryNode * > nodes;
    vector<pair<Connection *, bool>> eulerTour;

    int firstFloorSize;
    int height;
    SegmentTree(vector<pair<Connection *, bool>> eulerTour){
        this->eulerTour = eulerTour;//if not enough space jus add &
        float logVal = log2(eulerTour.size());
        this->firstFloorSize = pow(2, (int)logVal + (bool)((int)logVal != logVal));
        this->height = log2(firstFloorSize) + 1;
        this->eulerTour = eulerTour;
        buildTree();
        preprocess();
        cout<<"";
    }

    void buildTree(){
        int nodesCount = pow(2, height) - 1;
        for(int i = 0; i < nodesCount; i ++){
            nodes.push_back(new BinaryNode(i, new Range(0,0)));
        }
        for(int i = 0 ; i < firstFloorSize; i ++){
            BinaryNode * n = nodes[i +  pow(2, height - 1) - 1];
            n->range->min = i;
            n->range->max = i;
        }
        for(int i = nodesCount - 1 - firstFloorSize; i >= 0; i --){
            BinaryNode * n = nodes[i];
            n->range->min = getChild(i, true)->range->min;
            n->range->max = getChild(i, false)->range->max;
        }
        for(int i = 0 ; i < firstFloorSize; i ++){
            BinaryNode * n = nodes[i +  pow(2, height - 1) - 1];
            n->range->min = i;
            n->range->max = i;
        }
        for(int i = nodesCount - 1 - firstFloorSize; i >= 0; i --){
            BinaryNode * n = nodes[i];
            n->range->min = getChild(i, true)->range->min;
            n->range->max = getChild(i, false)->range->max;
        }
    }
    void preprocess(){
        for(int i = 0; i < eulerTour.size(); i++){
            int nodeID = i + pow(2, height - 1) - 1;
            BinaryNode * n = nodes[nodeID];
            Connection * correspondingStreet = eulerTour[i].first;
            Toy * toy = correspondingStreet->toy;
            bool positive = eulerTour[i].second;
            if(positive){
                n->toys.push_back(toy);
            }else{
                n->negativeSet.insert(toy);
            }
        }
        for(int i = 0; i < nodes.size() - firstFloorSize; i ++){
            int nodeID = nodes.size() - firstFloorSize - 1 - i;
            BinaryNode * n = nodes[nodeID];
            BinaryNode * leftChild = getChild(nodeID, true);
            BinaryNode * rightChild = getChild(nodeID, false);
            n->negativeSet.insert(leftChild->negativeSet.begin(), leftChild->negativeSet.end());
            n->negativeSet.insert(rightChild->negativeSet.begin(), rightChild->negativeSet.end());
            vector<Toy *> unDeletedToys = merge(leftChild->toys, rightChild->toys);

            vector<Toy *> finalToys;
            for(int i = 0 ; i < unDeletedToys.size(); i ++){
                Toy * t = unDeletedToys[i];
                if(n->negativeSet.find(t) == n->negativeSet.end()){
                    finalToys.push_back(t);
                }else{
                    n->negativeSet.erase(t);
                }
            }
            n->toys = finalToys;
        }
    }

    BinaryNode * getChild(int binaryNodeID, bool left){
        return nodes[(binaryNodeID) * 2 + !left + 1];
    }
    BinaryNode * getParent(int binaryNodeID){
        return nodes[(binaryNodeID - 1) / 2];
    }
    vector<BinaryNode *> rangeQuery(const Range * range, int nodeID = 0){
        if(range == nullptr){
            cout<<"error";
            return vector<BinaryNode *>();
        }
        BinaryNode * parent = nodes[nodeID];

        if(range->min == parent->range->min && range->max == parent->range->max){
            return {parent};
        }

        BinaryNode * leftChild = getChild(nodeID, true);
        BinaryNode * rightChild = getChild(nodeID, false);



        Range * commonLeft = Range::commonPart(range, leftChild->range);
        Range * commonRight = Range::commonPart(range, rightChild->range);
        vector<BinaryNode *> result;
        if(commonLeft->min <= commonLeft->max){
            vector<BinaryNode *> resLeft = rangeQuery(commonLeft, leftChild->id);
            result.insert(result.begin(), resLeft.begin(), resLeft.end());
        }
        if(commonRight->min <= commonRight->max){
            vector<BinaryNode *> resRight = rangeQuery(commonRight, rightChild->id);
            result.insert(result.begin(), resRight.begin(), resRight.end());
        }
        delete commonRight;
        delete commonLeft;
        return result;
    }

    int getUniqueElementsCount(int maxRoad){
        const Range * range = new Range(0, maxRoad);
        vector<BinaryNode *> nodes = rangeQuery(range);
        int uniqueSum = 0;
        for(int i = 0; i < nodes.size(); i ++){
            BinaryNode * n = nodes[i];
            const int smallerThan = range->max;
            if(n->toys.size() > 0) {
                int low = 0;
                int high = n->toys.size() - 1;
                int mid;
                if(n->toys[0]->nextOccurrenceInEuler > smallerThan){
                    uniqueSum += n->toys.size();
                }
                else if(n->toys[n->toys.size() - 1]->nextOccurrenceInEuler <= smallerThan){
                    uniqueSum += 0;
                   cout<<"";
                }
                else {
                    while (true) {
                        ///todo fix the binary search
                        mid = (low + high) / 2;
                        if (low == high) {
                            uniqueSum += low - n->toys.size() + 1;//change!
                            break;
                        }
                        if (n->toys[mid]->nextOccurrenceInEuler < smallerThan) {
                            low = mid + 1;
                        }
                        if (n->toys[mid]->nextOccurrenceInEuler > smallerThan) {
                            high = mid - 1;
                        }
                    }
                }
            }
        }
        delete range;
        return uniqueSum;
    }
    ~SegmentTree(){
        for(auto n : nodes){
            delete n;
        }
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
        Toy * toy = new Toy( stoi(args[2]) - 1);
        Connection * conn = new Connection(town1, town2, i, toy);
        town1->connections.push_back(conn);
        town2->connections.push_back(conn);
        streets.push_back(conn);
    }

    Leaf * rootTown = towns[0];
    rootTown->setParentPath(nullptr);
    propagateParent(rootTown);

    vector<Leaf *> tourTownOrder;
    vector<pair<Connection *, bool>> tourStreetOrder;

    int tmp = 0;
    eulerTourIndexing(rootTown, tmp, tourStreetOrder);
    {//assigning the next occurence of a toy in a euler tour. Omitting the negative ones.
        unordered_map<int, pair<Toy *, int>> tmpMap;//key - toytype, <Toy itself, occurrence itself>
        for(int  i = 0; i < tourStreetOrder.size(); i ++){
            if(tourStreetOrder[i].second){
                Connection * street = tourStreetOrder[i].first;
                Toy * toy = street->toy;
                if(tmpMap.find(toy->toyType) != tmpMap.end()){
                    toy->prevOccurrenceInEuler = tmpMap[toy->toyType].second;
                    tmpMap[toy->toyType].first->nextOccurrenceInEuler = i;
                }
                tmpMap[toy->toyType] = make_pair(toy, i);
            }
        }
    }


    SegmentTree segmentTree(tourStreetOrder);

    cout<<"\n";
    cout<<"\n";

    for(int i = 0; i < requestsCount; i ++){
        getline(cin, line);
        args = split(line, ' ');
        char request = args[0][0];
        if(request == 'Z'){
            Leaf * targetTown = towns.at(stoi(args[1]) - 1);

        }
        else if(request == 'B'){
            Connection * targetStreet = streets.at(stoi(args[1]) - 1);
            int newToy = stoi(args[2]) - 1;


        }
    }

    cout<<segmentTree.getUniqueElementsCount(4)<<"\n";

    for(Leaf * n : towns){
        delete n;
    }
    for(Connection * c : streets){
        delete c;
    }
    time(&end);
    cout << "execution time = " << double(end - start);
    return 0;
}
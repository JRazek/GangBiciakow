#include <iostream>
#include <stack>
#include <unordered_map>
#include <map>
#include <vector>
#include <math.h>

using namespace std;
struct Leaf;
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
    static vector<Range *> toDelete;
    Range(int min, int max){
        this->min = min;
        this->max = max;
    }
    static Range * commonPart(Range * r1, Range * r2){
        int min = r1->min > r2->min ? r1->min : r2->min;
        int max = r1->max < r2->max ? r1->max : r2->max;
        return new Range(min, max);
    }
};
struct Connection{
    Leaf * l1;
    Leaf * l2;
    int toyType;
    int ID;
    Leaf * parent;
    Leaf * child;

    int positiveNumberInEulerTour;
    int negativeNumberInEulerTour;
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
    int eulerTourID;

    int firstOccurrenceInEuler = -1;
    int lastOccurrenceInEuler;

    int subTreeSize = 0;

    void setParentPath(Connection * parentPath){
        this->parentPath = parentPath;
    }
};
unordered_map <int,int> addValues(unordered_map<int,int> &m1, unordered_map<int,int> &m2){
    bool firstSmaller = m1.size() < m2.size() ? true : false;
    unordered_map <int,int> bigger = firstSmaller ? m1 : m2;

    unordered_map <int,int> * smaller =  firstSmaller ? &m2 : &m1;

    for(unordered_map <int,int>::iterator it = smaller->begin(); it != smaller->end(); ++it){
        int toyType = it->first;
        int quantity = it->second;
        if(bigger.find(toyType) == bigger.end()){
            bigger[toyType] = quantity;
        }else{
            bigger[toyType] += quantity;
        }
    }
   // free(smaller);
    return bigger;
}
void eulerTourIndexing(Leaf * node, int &index, vector<Leaf *> &tourTownOrder,  int level = 0){
    node->eulerTourID = index;
    node->levelInTree = level;
    index += 1;
    tourTownOrder.push_back(node);
    for(int i = 0; i < node->connections.size(); i ++){
        if(node->connections.at(i) != node->parentPath){
            Connection * c = node->connections.at(i);
            eulerTourIndexing(c->child, index, tourTownOrder, ++level);
            tourTownOrder.push_back(node);
        }
    }

}

struct SegmentTree{
    struct BinaryNode{
        int id;
        Range * range;

        unordered_map<int,int> values;

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
            bool positive = eulerTour[i].second;
            n->values[correspondingStreet->toyType] = positive;
        }
        for(int i = 0; i < nodes.size()  - firstFloorSize; i ++){
            BinaryNode * node = nodes[i];
            BinaryNode * leftChild = getChild(node->id, true);
            BinaryNode * rightChild = getChild(node->id, false);
            node->values = addValues(leftChild->values, rightChild->values);
        }
    }

    BinaryNode * getChild(int binaryNodeID, bool left){
        return nodes[(binaryNodeID) * 2 + !left + 1];
    }
    BinaryNode * getParent(int binaryNodeID){
        return nodes[(binaryNodeID - 1) / 2];
    }
    vector<BinaryNode *> rangeQuery(Range * range, int nodeID = 0){
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
    ~SegmentTree(){
        for(auto n : nodes){
            delete n;
        }
    }
    void updateLeaf(int leafNum, int oldToyType, int newToyType, bool positive){
        BinaryNode * node = nodes[leafNum + pow(2, height - 1) - 1];
        //different behaviour to leaf!!!
        while (true) {
            node->values[oldToyType] += !positive;
            if(node->values[oldToyType] == 0){
                node->values.erase(oldToyType);
            }
            node->values[newToyType] = 1;
            if(node == getParent(node->id))
                break;
            node = getParent(node->id);
        }
    }
    int differentFromLeafToRoot(int leafNum){
        Range * r = new Range(0, leafNum);
        vector<BinaryNode *> nodes = rangeQuery(r);
        delete r;
        unordered_map<int,int> res;
        for(auto n : nodes){
            res = addValues(res, n->values);
        }
        return res.size();
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
        int toyType = stoi(args[2]) - 1;
        Connection * conn = new Connection(town1, town2, i, toyType);
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

    eulerTourIndexing(rootTown, tmp, tourTownOrder);

    for(int i = 0; i < tourTownOrder.size(); i++){
        Leaf * node = tourTownOrder[i];


        if(node->parentPath != nullptr && node->parentPath->parent == tourTownOrder[i - 1]){
            Connection * c = node->parentPath;
            tourStreetOrder.push_back(make_pair(c, true));
            c->positiveNumberInEulerTour = i;
        }else if(i > 0 && tourTownOrder[i - 1]->parentPath != nullptr && tourTownOrder[i - 1]->parentPath->parent == node){
            Connection * c = tourTownOrder[i - 1]->parentPath;
            tourStreetOrder.push_back(make_pair(c, false));
            c->negativeNumberInEulerTour = i;
        }else{
            cout<<"";
        }


        if(node->firstOccurrenceInEuler == -1){
            node->firstOccurrenceInEuler = i;
        }
        node->lastOccurrenceInEuler = i;
    }


    SegmentTree segmentTree(tourStreetOrder);


    cout<<"";

    for(int i = 0; i < requestsCount; i ++){
        getline(cin, line);
        args = split(line, ' ');
        char request = args[0][0];
        if(request == 'Z'){
            Leaf * targetTown = towns.at(stoi(args[1]) - 1);
            cout<<segmentTree.differentFromLeafToRoot(targetTown->parentPath->positiveNumberInEulerTour)<<"\n";
        }
        else if(request == 'B'){
            Connection * targetStreet = streets.at(stoi(args[1]) - 1);
            int newToy = stoi(args[2]) - 1;
            if(targetStreet->toyType != newToy){
                int oldToyType = targetStreet->toyType;
                targetStreet->toyType = newToy;
                segmentTree.updateLeaf(targetStreet->positiveNumberInEulerTour, oldToyType, newToy, true);
                segmentTree.updateLeaf(targetStreet->positiveNumberInEulerTour, oldToyType, newToy, false);
            }
        }
    }


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
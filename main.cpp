#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

class TreeNode {
    public:
        int type; // 0 = h1, 1 = h2, 2 = h3, 3 = h4, 4 = h5, 5 = h6, 6 = p, 7 = text, -1 = root
        string body;
        vector<TreeNode*> children;
        TreeNode *parent = nullptr;
        TreeNode(int newtype) {
            type = newtype;
        }

        void addChild(TreeNode *child) {
            children.push_back(child);
        }
};

int classify(string line, string *body){
    //firstly, im looking for paragraphs and headers. so if i read the first 6 characters of the line, 
    //and count how many #s there are, this will tell me. of course, i need to check if there are 6 characters
    //in the line. if not, i will read the whole line
    int i = 6;
    if (line.length() < 6) {
        i = line.length();
    }
    
    int j = 0;
    for (j; j <= i; j++) {
        if (line[j] != '#') {
            break;
        }
    }

    switch (j) {
    case 0:
        return 6;
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    case 5:
        return 4;
    case 6:
        return 5;
    default:
        return 7;
    }
}

void printTree(TreeNode *root) {
    cout << root->type << endl;
    for (TreeNode *child : root->children){
        printTree(child);
        cout << "| ";
    } 
    cout << endl;
}

int main() {
    //first we need input from the user - im not bothered to build CLI so just gonna use simple text IO
    cout << "Input the target markdown file \n";
    string input;
    cin >> input;

    ifstream mdFile;
    mdFile.open(input);
    if (mdFile.is_open()){
        cerr << "There was an error opening this file. Check if this file exists at this location";
        return 1;
    }

    //now we need to parse the file and build the tree. I am going to build the node tree here, and then use
    //a recursive helper function to build the rest - will come later, there will be nothing nested rn
    TreeNode treeRoot(-1);
    TreeNode *cursor = &treeRoot;

    string buf;
    string body;
    while (getline(mdFile, buf)) {
        int newtype = classify(buf, &body);
        TreeNode tmp(newtype);
        cursor->addChild(&tmp);
    }

    //from here we need to make a new file and populate it with html, traversing the tree recurisvely
    //first i want to build a tree printing function for debugging purposes, you best believe im gonna need it
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

//make a map to store the catalogue of conversions both for building the file and the tree
map<int, string> catalogOut;
map<string, int> catalogTree;

class TreeNode {
    public:
        int type; // 0 = h1, 1 = h2, 2 = h3, 3 = h4, 4 = h5, 5 = h6, 6 = p, -1 = root
        vector<string> body;
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
    string command;
    if (line.length() < 6) {
        command = line;
    } else {
        command = line.erase(6, line.length()-6);
    }    
    command.erase(remove_if(command.begin(), command.end(), [](char c) { return c != '#'; }), command.end());
    return catalogTree[command];
}

void printTree(TreeNode *root) {
    cout << root->type << endl;
    for (TreeNode *child : root->children){
        printTree(child);
        cout << "| ";
    } 
    cout << endl;
}

void buildFile(TreeNode *root, string build){
    for (TreeNode *child : root->children){
        buildFile(child, build);
    }
}

int main() {
    //populate the catalogs
    catalogOut[0] = "<h1>";
    catalogOut[1] = "<h2>";
    catalogOut[2] = "<h3>";
    catalogOut[3] = "<h4>";
    catalogOut[4] = "<h5>";
    catalogOut[5] = "<h6>";
    catalogOut[6] = "<p>";

    catalogTree["#"] = 0;
    catalogTree["##"] = 1;
    catalogTree["###"] = 2;
    catalogTree["####"] = 3;
    catalogTree["#####"] = 4;
    catalogTree["######"] = 5;
    catalogTree[""] = 6;

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

    //remove file if it already exists from a previous run
    remove("htmlOut.html");
    ofstream htmlOut("htmlOut.html");
    if (!htmlOut.is_open()) {
        cerr << "Something went wrong opening the output file" << endl;
    }

}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

//make a map to store the catalogue of conversions both for building the file and the tree
map<int, string> catalogOut1;
map<int, string> catalogOut2;
map<string, int> catalogTree;

class TreeNode {
    public:
        int type; // 0 = h1, 1 = h2, 2 = h3, 3 = h4, 4 = h5, 5 = h6, 6 = p, -1 = root
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
    string command;
    if (line.length() <= 6) {
        command = line;
    } else {
        string linecopy = line;
        command = linecopy.erase(6, linecopy.length()-6);
    }    
    command.erase(remove_if(command.begin(), command.end(), [](char c) { return c != '#'; }), command.end());
    if (command != "") {
        size_t pos = line.find(command);
        line.erase(pos, command.length());
    }
    *body = line;
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

void buildFile(TreeNode *root, ofstream &file){
    file << catalogOut1[root->type] << endl;
    for (TreeNode *child : root->children){
        buildFile(child, file);
    }
    file << root->body << "\n" << catalogOut2[root->type] << endl;
}

int main() {
    //populate the catalogs
    catalogOut1[0] = "<h1>";
    catalogOut1[1] = "<h2>";
    catalogOut1[2] = "<h3>";
    catalogOut1[3] = "<h4>";
    catalogOut1[4] = "<h5>";
    catalogOut1[5] = "<h6>";
    catalogOut1[6] = "<p>";

    catalogOut2[0] = "</h1>";
    catalogOut2[1] = "</h2>";
    catalogOut2[2] = "</h3>";
    catalogOut2[3] = "</h4>";
    catalogOut2[4] = "</h5>";
    catalogOut2[5] = "</h6>";
    catalogOut2[6] = "</p>";

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
    if (!mdFile.is_open()){
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
        TreeNode *tmp = new TreeNode(newtype);
        tmp->body = body;
        cursor->addChild(tmp);
    }

    //from here we need to make a new file and populate it with html, traversing the tree recurisvely
    //first i want to build a tree printing function for debugging purposes, you best believe im gonna need it

    //remove file if it already exists from a previous run
    remove("htmlOut.html");
    ofstream htmlOut("htmlOut.html");
    if (!htmlOut.is_open()) {
        cerr << "Something went wrong opening the output file" << endl;
    }
    buildFile(&treeRoot, htmlOut);
}
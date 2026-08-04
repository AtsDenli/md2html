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

vector<string> split_string(const string& s, const string& delim) {
    vector<string> parts;
    size_t start = 0;
    size_t search_from = 0;
    size_t pos;
    bool doubles_up = (delim == "*" || delim == "_");
    while ((pos = s.find(delim, search_from)) != string::npos) {
        if (doubles_up && pos + 1 < s.size() && s[pos + 1] == delim[0]) {
            //"**" or "__" found — the current delim shouldnt split here
            search_from = pos + 2;
            continue;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + delim.length();
        search_from = start;
    }
    parts.push_back(s.substr(start));
    return parts;
}

int countSubstr(const string& s, const string& sub) {
    if (sub.empty()) return 0;
    int count = 0;
    size_t pos = 0;
    while ((pos = s.find(sub, pos)) != string::npos) {
        ++count;
        pos += sub.length(); // move past this match (non-overlapping)
    }
    return count;
}

int buildNode(string line, TreeNode *node){
    string command;
    if (line[0] == '*' || line[0] == '_') {
        if (line[1] == line[0]) {
            command = to_string(line[0]) + to_string(line[1]);
            line.erase(0,2);
            line.erase(line.length()-2, 2);
        } else {
            command = to_string(line[0]);
            line.erase(0,1);
            line.erase(line.length()-1, 1);
        }
    } else {
        if (line.length() <= 6) {
            command = line;
        } else {
            string linecopy = line;
            command = linecopy.erase(6, linecopy.length()-6);
        }    
        command.erase(remove_if(command.begin(), command.end(), [](char c) { return c != '#' ; }), command.end());
        if (command != "") {
            size_t pos = line.find(command);
            line.erase(pos, command.length());
        }
    }
    string rawBody = line;
    node->type = catalogTree[command];

    string bodyPart = "";
    for (int i = 0; i < rawBody.length(); i++) {
        if ((rawBody[i] != '*' || rawBody[i] != '_') || ((rawBody[i] == '*'  || rawBody[i] == '_') && rawBody[i+1] == ' ')) {// no valid delimiter
            bodyPart += rawBody[i];
        } else { //valid delimiter
            node->body.push_back(bodyPart);
            bodyPart = "";
            TreeNode *child = new TreeNode(-1);
            child->parent = node;
            string delim = to_string(rawBody[i]);
            if (rawBody[i] == rawBody[i+1]) {
                delim += to_string(rawBody[i+1]);
            } 
            string childBody = split_string(rawBody, delim)[node->children.size() + node->body.size()];

        }
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

void buildFile(TreeNode *root, ofstream &file){
    file << catalogOut1[root->type] << endl;
    for (TreeNode *child : root->children){
        buildFile(child, file);
    }
    file << "\t" << root->body << "\n" << catalogOut2[root->type] << endl;
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
    catalogOut1[7] = "<strong>";
    catalogOut1[8] = "<em>";

    catalogOut2[0] = "</h1>";
    catalogOut2[1] = "</h2>";
    catalogOut2[2] = "</h3>";
    catalogOut2[3] = "</h4>";
    catalogOut2[4] = "</h5>";
    catalogOut2[5] = "</h6>";
    catalogOut2[6] = "</p>";
    catalogOut2[7] = "</strong>";
    catalogOut2[8] = "</em>";

    catalogTree["#"] = 0;
    catalogTree["##"] = 1;
    catalogTree["###"] = 2;
    catalogTree["####"] = 3;
    catalogTree["#####"] = 4;
    catalogTree["######"] = 5;
    catalogTree[""] = 6;
    catalogTree["**"] = 7; //bold
    catalogTree["__"] = 7; //also bold
    catalogTree["*"] = 8; //italics
    catalogTree["_"] = 8; //also italics

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

    TreeNode treeRoot(-1);
    string buf;
    string body;
    while (getline(mdFile, buf)) {
        TreeNode *tmp = new TreeNode(-1);
        tmp -> parent = &treeRoot;
        buildNode(buf, tmp);
    }

    //remove file if it already exists from a previous run
    remove("htmlOut.html");
    ofstream htmlOut("htmlOut.html");
    if (!htmlOut.is_open()) {
        cerr << "Something went wrong opening the output file" << endl;
    }
    buildFile(&treeRoot, htmlOut);
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
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

        TreeNode(int newtype) {
            type = newtype;
        }

        void addChild(TreeNode *child) {
            children.push_back(child);
        }
};

void buildNode(string line, TreeNode *node, bool linkAllow, ifstream *mdFile);

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

bool isInteger(const string& s) {
    // String must not be empty, and all characters must be digits
    return !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { 
        return isdigit(c); 
    });
}

string escHTML(string line) {
    string escapedLine = "";
    char toReplace[] = {'&', '<', '>'};
    string replaceWith[] = {"&amp;", "&lt;", "&gt;"};
    for (int i = 0; i < line.length(); i++) {
        bool isHTML = false;
        for (int j = 0; j < 3; j++) {
            if (toReplace[j] == line[i]) {
                isHTML = true;
                escapedLine += replaceWith[j];
                break;
            }
        }
        if (!isHTML) {
            escapedLine += line[i];
        }
    }
    return escapedLine;
}

bool parseLinkImg(string line, TreeNode *node) {
    //at this point, all we know is that the line starts with a [, need to actually check its a link first
    bool noWhitespace = false;
    string linkText = "";
    int i;
    if (line[0] == '!') {
        i = 2;
    } else {
        i = 1;
    }

    int openCount = 1;
    int closeCount = 0;
    for (i; i < line.length(); i++) {
        if (line[i] == ']'){
            closeCount++;
        } else if (line[i] == '[') {
            openCount++;
        }
        if (i != line.length()-1) {
            if (line[i] == ']' && line[i+1] == '(' && openCount == closeCount) {
                noWhitespace = true;
                break;
            } else if (line[i] == ']' && line[i+1] == ' ' && closeCount == openCount) {
                return false;
            }
        }
        linkText += line[i];
    }
    if (!(noWhitespace && line[line.length()-1] == ')')) {
        //not a valid link
        return false;
    }
    //create node to add to tree
    TreeNode *child = new TreeNode(-1);

    //parse the link text
    buildNode(linkText, child, false, nullptr);
    node->addChild(child);
    string destiTitle = split_string(line, "(")[1];
    destiTitle.erase(destiTitle.length()-1, 1);

    vector<string> splits = split_string(destiTitle, "\"");
    node->body.push_back(splits[0]);
    if (splits.size() > 1) {
        node->body.push_back(splits[1]);
    }
    return true;
}

void buildNode(string line, TreeNode *node, bool linkAllow, ifstream *mdFile){
    if (line.empty()) {
        node->type = catalogTree[""];
        return;
    }
    bool inlinesSuppressed = false;
    bool isLinkImg = false;
    string command;
    string ordListNum = split_string(line, ".")[0];
    if (line == "****") {
        command = line;
        line = "";
    } else if (line[0] == '[' && linkAllow) {
        isLinkImg = parseLinkImg(line, node);
        if (isLinkImg) {
            node->type = 11;
        }
    } else if (line[0] == '!' && line[1] == '[' && linkAllow) {
        isLinkImg = parseLinkImg(line, node);
        if(isLinkImg) {
            node->type = 12;
        }
    } else if (line[0] == '`') {
        inlinesSuppressed = true;
        int backTickCount = 0;
        for (int n = 0; n < line.length(); n++) {
            if (line[n] == '`') {
                backTickCount++;
            } else {
                break;
            }
        }
        int backTickCheck = 0;
        if (line[line.length()-1] == '`'){
            for (int m = line.length()-1; m >=0; m--) {
                if (line[m] == '`') {
                    backTickCheck++;
                } else {
                    break;
                }
            }
        } 

        if (backTickCheck == backTickCount) {
            command = '`';
            line.erase(0, backTickCount);
            line.erase(line.length()-backTickCount, backTickCount);
        } else {
            //not the same number of bacticks before and after - its a paragraph instead
            command = "";
            inlinesSuppressed = false;
        }
        
    } else if ((line[0] == '*' || line[0] == '+' || line[0] == '-') && line[1] == ' ') {//unordered list
        node->type = 14;
        string newLine = line;
        streampos lastPos = mdFile->tellg();
        do {
            if (newLine == "") {
                continue;
            } else if ((newLine[0] == '*' || newLine[0] == '+' || newLine[0] == '-') && newLine[1] == ' ') {
                TreeNode* child = new TreeNode(16);
                newLine.erase(0,2);
                buildNode(newLine, child, linkAllow, mdFile);
                node->addChild(child);
            } else {
                mdFile->seekg(lastPos);
                break;
            }
            lastPos = mdFile->tellg();
        } while (getline(*mdFile, newLine));
        return;
    } else if (isInteger(ordListNum) && line[ordListNum.length()] == '.' && line[ordListNum.length()+1] == ' ') {//ordered list
        node->type = 15;
        string newLine = line;
        streampos lastPos = mdFile->tellg();
        do {
            if (newLine == "") {
                continue;
            } else if (isInteger(ordListNum) && newLine[ordListNum.length()] == '.' && newLine[ordListNum.length()+1] == ' ') {
                TreeNode* child = new TreeNode(16);
                newLine.erase(0, ordListNum.length()+2);
                buildNode(newLine, child, linkAllow, mdFile);
                node->addChild(child);
            } else {
                mdFile->seekg(lastPos);
                break;
            }
            lastPos = mdFile->tellg();
        } while (getline(*mdFile, newLine));
        return;
    } else if (line[0] == '*' || line[0] == '_') {
        if (line[1] == line[0]) {
            command = string(1, line[0]) + string(1, line[1]);
            line.erase(0,2);
            line.erase(line.length()-2, 2);
        } else {
            command = string(1, line[0]);
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

    if (!isLinkImg) {
        if (command == "" && !inlinesSuppressed && (count(line.begin(), line.begin()+3, '`') == 3) || (count(line.begin(), line.begin()+3, '~') == 3)) {
            char delimChar = line[0];
            string delim = "";
            for (int i = 0; i < line.length(); i++) {
                if (line[i] != delimChar) { 
                    break;
                }
                delim += line[i];
            }
            line.erase(0,delim.length());
            size_t pos = 0;
            string multiLine = line;
            streampos orgPos = mdFile->tellg();
            bool found = false;
            string newLine = line;
            while (true) {
                pos = newLine.find(delim);
                if (pos != string::npos) {
                    found = true;
                    break;
                } else if (mdFile->eof()) {
                    break;
                } else {
                    string tmpLine;
                    getline(*mdFile, newLine);
                    multiLine += "\n";
                    multiLine += newLine;
                }
            }
            node->type = 13;
            if (!found) {
                mdFile->clear(); 
                mdFile->seekg(orgPos);
            } else {
                multiLine.erase(multiLine.end() - delim.length(), multiLine.end());
                node->body.push_back(multiLine);
            }
            inlinesSuppressed = true;
            line = delim + line;
            return;
        }

        string rawBody = escHTML(line);
        if (node->type == -1) {
            node->type = catalogTree[command];
        }

        if (inlinesSuppressed && node->type != 13) {
            //if this is an inline code, inlines are supressed and not formatted.
            //first lets remove any leading/trailing spaces
            if (rawBody[0] == ' ' && rawBody[rawBody.length()-1] == ' '){
                rawBody.erase(0,1);
                rawBody.erase(rawBody.length()-1,1);
            }
            node->body.push_back(rawBody);
            return;
        }

        string bodyPart = "";
        for (int i = 0; i < rawBody.length(); i++) {
            bool isDoubled = ((rawBody[i] == '*'  || rawBody[i] == '_') && rawBody[i+1] == rawBody[i]);
            if (((rawBody[i] == '*'  || rawBody[i] == '_') && rawBody[i+1] != ' ' && !isDoubled) || (isDoubled && rawBody[i+2] != ' ')) { //valid delimiter
                string delim = string(1, rawBody[i]);
                if (rawBody[i] == rawBody[i+1] && i <= rawBody.length()) {
                    delim += string(1, rawBody[i+1]);
                } 

                if (rawBody.find(delim, i+1) != string::npos){ //making sure this isnt the last delimiter in the line without a pair - edge case
                    node->body.push_back(bodyPart);
                    bodyPart = "";
                    TreeNode *child = new TreeNode(-1);

                    size_t start = i + delim.length();
                    size_t end = rawBody.find(delim, start);

                    string childBody;
                    if (end != string::npos){
                        childBody = delim + rawBody.substr(start, end - start) + delim;
                    } else {
                        childBody = delim;
                    }

                    node->addChild(child);
                    buildNode(childBody, child, true, mdFile);
                    i += childBody.length() - 1;
                } else {
                    bodyPart += rawBody[i];
                    if (i == rawBody.length()-1) {
                    node->body.push_back(bodyPart);
                    }
                }
            } else {
                bodyPart += rawBody[i];
                if (i == rawBody.length()-1) {
                    node->body.push_back(bodyPart);
                }
            }
        }
    }
}

void buildFile(TreeNode *root, ofstream &file, bool isImg){
    if (root->type == 11) {
        file << catalogOut1[root->type][0] << catalogOut1[root->type][1] << "  href=\"" << root->body[0] << "\" ";
        if (root->body.size() > 1) {
            file << "title=\"" << root->body[1] << "\"";
        }
        file << ">" << endl << "\t";
        buildFile(root->children[0], file, false);
        file << catalogOut2[root->type] << endl;
    } else if (root->type == 12) {
        file << catalogOut1[root->type] << " src=\"" << root->body[0] << "\" alt=\"";
        buildFile(root->children[0], file, true);
        file << "\"";
        if (root->body.size() > 1) {
            file << " title=\"" << root->body[1] << "\"";
        }
        file << catalogOut2[root->type] << endl;
    } else if (root->type == 14 || root->type == 15) {
        file << catalogOut1[root->type] << "\n\t";
        for (TreeNode* child : root->children) {
            buildFile(child, file, false);
        }
        file << catalogOut1[root->type] << "\n";
    } else {
        if (!isImg) {
            file << catalogOut1[root->type] << endl << "\t";
        }
        int i = 0;
        for (string bodyPart : root->body){
            file << bodyPart << endl;
            if (i < root->children.size()) {
                buildFile(root->children[i], file, false);
            }
            i++;
        }
        if (!isImg) {
            file << catalogOut2[root->type] << endl;
        }
    }
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
    catalogOut1[9] = "<hr>";
    catalogOut1[10] = "<code>";
    catalogOut1[11] = "<a>";
    catalogOut1[12] = "<img ";
    catalogOut1[13] = "<pre><code>";
    catalogOut1[14] = "<ul>";
    catalogOut1[15] = "<ol>";
    catalogOut1[16] = "<li>";

    catalogOut2[0] = "</h1>";
    catalogOut2[1] = "</h2>";
    catalogOut2[2] = "</h3>";
    catalogOut2[3] = "</h4>";
    catalogOut2[4] = "</h5>";
    catalogOut2[5] = "</h6>";
    catalogOut2[6] = "</p>";
    catalogOut2[7] = "</strong>";
    catalogOut2[8] = "</em>";
    catalogOut2[9] = "";
    catalogOut2[10] = "</code>";
    catalogOut2[11] = "</a>";
    catalogOut2[12] = "/>";
    catalogOut2[13] = "</code></pre>";
    catalogOut2[14] = "</ul>";
    catalogOut2[15] = "</ol>";
    catalogOut2[16] = "</li>";

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
    catalogTree["****"] = 9; //horizontal rule
    catalogTree["`"] = 10; //inline code

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
        buildNode(buf, tmp, true, &mdFile);
        treeRoot.addChild(tmp);
    }

    //remove file if it already exists from a previous run
    remove("htmlOut.html");
    ofstream htmlOut("htmlOut.html");
    if (!htmlOut.is_open()) {
        cerr << "Something went wrong opening the output file" << endl;
    }

    for (TreeNode *child : treeRoot.children) {
        buildFile(child, htmlOut, false);
    }
}
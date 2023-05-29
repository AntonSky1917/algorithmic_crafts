#include <iostream>
#include <string>
#include <vector>

using namespace std;

const int ALPHABET_SIZE = 26;

struct TrieNode {
    TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
};

TrieNode* getNewNode() {
    TrieNode* node = new TrieNode;
    node->isEndOfWord = false;
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        node->children[i] = nullptr;
    }
    return node;
}

void insert(TrieNode* root, const string& word) {
    TrieNode* current = root;
    for (char c : word) {
        int index = c - 'a';
        if (current->children[index] == nullptr) {
            current->children[index] = getNewNode();
        }
        current = current->children[index];
    }
    current->isEndOfWord = true;
}

void autoCompleteUtil(TrieNode* node, const string& prefix, vector<string>& suggestions) {
    if (node->isEndOfWord) {
        suggestions.push_back(prefix);
    }
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (node->children[i] != nullptr) {
            char c = 'a' + i;
            autoCompleteUtil(node->children[i], prefix + c, suggestions);
        }
    }
}

vector<string> autoComplete(TrieNode* root, const string& prefix) {
    TrieNode* current = root;
    for (char c : prefix) {
        int index = c - 'a';
        if (current->children[index] == nullptr) {
            return {};
        }
        current = current->children[index];
    }
    vector<string> suggestions;
    autoCompleteUtil(current, prefix, suggestions);
    return suggestions;
}

string getCommonPrefix(const vector<string>& words) {
    if (words.empty()) {
        return "";
    }

    string prefix = words[0];
    for (int i = 1; i < words.size(); ++i) {
        const string& word = words[i];
        int j = 0;
        while (j < prefix.length() && j < word.length() && prefix[j] == word[j]) {
            ++j;
        }
        prefix = prefix.substr(0, j);
    }

    return prefix;
}

int main() {
    TrieNode* root = getNewNode();

    // Построение префиксного дерева на основе словаря
    insert(root, "apple");
    insert(root, "app");
    insert(root, "application");
    insert(root, "papa");
    insert(root, "mydadtookmybanana");
    insert(root, "mygrandmotherwrotethiscodeforme");

    string prefix;
    cout << "Enter a prefix (or 'exit' to quit): ";

    while (true) {
        cin >> prefix;

        if (prefix == "exit") {
            break;
        }

        vector<string> suggestions = autoComplete(root, prefix);

        if (suggestions.empty()) {
            cout << "No suggestions found." << endl;
        } else {
            string commonPrefix = getCommonPrefix(suggestions);
            if (commonPrefix.length() > prefix.length()) {
                cout << "Autocomplete suggestions:" << endl;
                for (const string& word : suggestions) {
                    cout << word << endl;
                }
                cout << "Common prefix: " << commonPrefix << endl;
            } else {
                cout << "No autocomplete suggestions found." << endl;
            }
        }

        cout << "Enter a prefix (or 'exit' to quit): ";
    }

    return 0;
}

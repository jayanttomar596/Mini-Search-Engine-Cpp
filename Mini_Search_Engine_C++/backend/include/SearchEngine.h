#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Trie.h"

using namespace std;

struct Posting {
    int frequency = 0;
    vector<int> positions;
    vector<long long> offsets;
};

struct SearchResult {
    string document;
    int frequency;
    vector<int> positions;
    vector<long long> offsets;
};

class SearchEngine {
public:
    void addDocument(const string& path);
    void buildIndex();
    vector<SearchResult> searchAPI(const string& word);
    vector<string> autocompleteAPI(const string& prefix);

private:
    vector<string> documents;
    unordered_map<string, unordered_map<int, Posting>> invertedIndex;
    Trie trie;
};

#endif

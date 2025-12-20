#include "SearchEngine.h"
#include <fstream>
#include <cctype>

static string normalize(const string& word) {
    string clean;
    for (char c : word)
        if (isalpha(c))
            clean += tolower(c);
    return clean;
}

void SearchEngine::addDocument(const string& path) {
    documents.push_back(path);
}

void SearchEngine::buildIndex() {
    for (int docID = 0; docID < documents.size(); docID++) {
        ifstream file(documents[docID]);
        if (!file) continue;

        string word;
        int position = 0;
        long long offset = 0;

        while (file >> word) {
            string clean = normalize(word);
            if (clean.empty()) continue;

            // Inverted Index
            auto& posting = invertedIndex[clean][docID];
            posting.frequency++;
            posting.positions.push_back(position);
            posting.offsets.push_back(offset);

            // Trie for autocomplete
            trie.insert(clean);

            offset = file.tellg();
            position++;
        }
    }
}

vector<SearchResult> SearchEngine::searchAPI(const string& word) {
    vector<SearchResult> results;
    string q = normalize(word);

    if (invertedIndex.find(q) == invertedIndex.end())
        return results;

    for (auto& [docID, posting] : invertedIndex[q]) {
        SearchResult res;
        res.document = documents[docID];
        res.frequency = posting.frequency;
        res.positions = posting.positions;
        res.offsets = posting.offsets;
        results.push_back(res);
    }

    return results;
}

vector<string> SearchEngine::autocompleteAPI(const string& prefix) {
    return trie.autocomplete(normalize(prefix));
}

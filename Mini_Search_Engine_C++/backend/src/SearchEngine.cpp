#include "SearchEngine.h"
#include <fstream>
#include <sstream>
#include <cctype>

static string normalize(const string& word) {
    string clean;
    for (char c : word)
        if (isalpha(static_cast<unsigned char>(c)))
            clean += tolower(c);
    return clean;
}

// Existing function
void SearchEngine::addDocument(const string& path) {
    documents.push_back(path);
}

// NEW: Add document by content
void SearchEngine::addDocumentContent(const string& name, const string& content) {

    if (usingSample) {
        clearIndex();
    }

    documents.push_back(name);
    int docID = documents.size() - 1;

    documentContents[docID] = content;
    indexDocument(docID, content);
}



void SearchEngine::buildIndex() {
    for (int docID = 0; docID < documents.size(); docID++) {
        ifstream file(documents[docID]);
        if (!file) continue;

        stringstream buffer;
        buffer << file.rdbuf();
        indexDocument(docID, buffer.str());
    }
}

// NEW: Core indexing logic separated
void SearchEngine::indexDocument(int docID, const string& content) {
    stringstream ss(content);
    string word;
    int position = 0;
    long long offset = 0;

    while (ss >> word) {
        string clean = normalize(word);
        if (clean.empty()) continue;

        auto& posting = invertedIndex[clean][docID];
        posting.frequency++;
        posting.positions.push_back(position);
        posting.offsets.push_back(offset);

        trie.insert(clean);

        offset += word.length() + 1;
        position++;
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

        //  Generate snippet from memory
        // 🔥 Generate snippet from memory
        if (!posting.offsets.empty()) {
            long long start = posting.offsets[0];
            string& content = documentContents[docID];

            int begin = max(0LL, start - 40);
            int end = min((long long)content.size(), start + 100);

            res.snippet = content.substr(begin, end - begin);
        }


        results.push_back(res);
    }

    sort(results.begin(), results.end(),
         [](const SearchResult& a, const SearchResult& b) {
             return a.frequency > b.frequency;
         });

    return results;
}


vector<string> SearchEngine::autocompleteAPI(const string& prefix) {
    return trie.autocomplete(normalize(prefix));
}


void SearchEngine::clearIndex() {
    documents.clear();
    invertedIndex.clear();
    documentContents.clear();
    trie = Trie();
    usingSample = false;
}



void SearchEngine::loadSampleDataset() {

    if (usingSample) return;

    clearIndex();

    vector<string> sampleDocs = {
        "../documents/doc1.txt",
        "../documents/doc2.txt",
        "../documents/doc3.txt"
    };

    for (auto& path : sampleDocs) {
        ifstream file(path);
        if (!file) continue;

        stringstream buffer;
        buffer << file.rdbuf();

        addDocumentContent(path, buffer.str());
    }

    usingSample = true;
}

#include "SearchEngine.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cmath>

using namespace std;

// ---------------- NORMALIZE ----------------
static string normalize(const string& word) {
    string clean;
    for (char c : word)
        if (isalpha(static_cast<unsigned char>(c)))
            clean += tolower(c);
    return clean;
}

// ---------------- SPLIT QUERY ----------------
static vector<string> splitQuery(const string& query) {
    vector<string> tokens;
    string word;
    stringstream ss(query);

    while (ss >> word) {
        word = normalize(word);
        if (!word.empty())
            tokens.push_back(word);
    }

    return tokens;
}

// ---------------- PHRASE MATCH ----------------
static int countPhraseOccurrences(
    const vector<int>& pos1,
    const vector<int>& pos2,
    vector<int>& phrasePositions
) {
    int i = 0, j = 0, count = 0;

    while (i < pos1.size() && j < pos2.size()) {

        if (pos2[j] == pos1[i] + 1) {
            count++;
            phrasePositions.push_back(pos1[i]);
            i++; j++;
        }
        else if (pos2[j] > pos1[i] + 1)
            i++;
        else
            j++;
    }

    return count;
}



static int countProximityMatches(
    const vector<int>& pos1,
    const vector<int>& pos2,
    int k,
    vector<int>& proxPositions
) {

    int i = 0, j = 0, count = 0;

    while (i < pos1.size() && j < pos2.size()) {

        int gap = abs(pos2[j] - pos1[i]) - 1;

        if (gap <= k && gap >= 0) {
            count++;
            proxPositions.push_back(pos1[i]);
            i++; j++;
        }
        else if (pos1[i] < pos2[j])
            i++;
        else
            j++;
    }

    return count;
}




// ---------------- ADD DOCUMENT PATH ----------------
void SearchEngine::addDocument(const string& path) {
    documents.push_back(path);
}


// ---------------- ADD DOCUMENT CONTENT ----------------
void SearchEngine::addDocumentContent(const string& name, const string& content) {

    if (usingSample)
        clearIndex();

    documents.push_back(name);
    int docID = documents.size() - 1;

    documentContents[docID] = content;
    indexDocument(docID, content);
}

// ---------------- BUILD INDEX ----------------
void SearchEngine::buildIndex() {

    for (int docID = 0; docID < documents.size(); docID++) {

        ifstream file(documents[docID]);
        if (!file) continue;

        stringstream buffer;
        buffer << file.rdbuf();

        documentContents[docID] = buffer.str();
        indexDocument(docID, buffer.str());
    }
}

// ---------------- INDEX DOCUMENT ----------------
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


    documentLength[docID] = position;

    double total = 0;
    for(auto &p : documentLength)
        total += p.second;

    avgDocLength = total / documentLength.size();

}






double computeBM25(
    int tf,
    int df,
    int docLen,
    int N,
    double avgdl
){
    double k1 = 1.5;
    double b  = 0.75;

    double idf = log(1 + ((N - df + 0.5) / (df + 0.5)));
    

    double norm = (double)docLen / (double)avgdl;


    double num = tf * (k1 + 1.0);
    double den = tf + k1 *
        (1.0 - b + b * norm);

    return idf * (num / den);
}






// ======================= SEARCH API =======================
vector<SearchResult> SearchEngine::searchAPI(const string& query) {

    vector<SearchResult> results;
    vector<string> terms = splitQuery(query);

    if (terms.empty()) return results;

    unordered_map<int, int> docPresence;

    for (const string& term : terms) {

        if (invertedIndex.find(term) == invertedIndex.end())
            return {};

        for (auto& [docID, posting] : invertedIndex[term]) {
            docPresence[docID]++;
        }
    }

    vector<int> candidateDocs;

    for (auto& [docID, count] : docPresence) {
        if (count == terms.size())   // ✔ must contain all terms
            candidateDocs.push_back(docID);
    }


    // -------- RESULT GENERATION --------
    for (int docID : candidateDocs){

        SearchResult res;
        res.document = documents[docID];

        string& content = documentContents[docID];




        double bm25Score = 0.0;
        int N = documents.size();

        for(const string& term : terms){

            if(invertedIndex[term].find(docID) == invertedIndex[term].end())
                continue;

            auto& posting = invertedIndex[term].at(docID);
            int tf = posting.frequency;

            int df = invertedIndex[term].size();
            int docLen = documentLength[docID];

            bm25Score += computeBM25(
                tf,
                df,
                docLen,
                N,
                avgDocLength
            );
        }

        res.score = bm25Score;



        if(terms.size() >= 2){

            auto& p1 = invertedIndex[terms[0]].at(docID);
            auto& p2 = invertedIndex[terms[1]].at(docID);

            vector<int> phrasePositions;

            int phraseFreq = countPhraseOccurrences(
                p1.positions,
                p2.positions,
                phrasePositions
            );

            if(phraseFreq > 0){
                res.score += 1.5 * phraseFreq;   // phrase boost
            }
        }



        if(terms.size() >= 2){

            auto& p1 = invertedIndex[terms[0]].at(docID);
            auto& p2 = invertedIndex[terms[1]].at(docID);

            vector<int> proxPositions;

            int proxFreq = countProximityMatches(
                p1.positions,
                p2.positions,
                3,
                proxPositions
            );

            if(proxFreq > 0){
                res.score += 0.75 * proxFreq;   // proximity boost
            }
        }




        // ================= SINGLE WORD =================
        if (terms.size() == 1) {

            auto& posting = invertedIndex[terms[0]][docID];
            res.frequency = posting.frequency;

            if (!posting.positions.empty()) {

                int idx = 0;
                long long offset = posting.offsets[idx];

                int start = max(0LL, offset - 60);
                int end = min((long long)content.size(), offset + 100);

                res.snippet = content.substr(start, end - start);
            }

            results.push_back(res);
            continue;
        }

        auto& posting = invertedIndex[terms[0]][docID];

        res.frequency = posting.frequency;

        if (!posting.positions.empty()) {

            int idx = 0;
            long long offset = posting.offsets[idx];

            int start = max(0LL, offset - 60);
            int end = min((long long)content.size(), offset + 100);

            res.snippet = content.substr(start, end - start);
        }


        results.push_back(res);
    }

    sort(results.begin(), results.end(),
    [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });


    return results;
}

// ---------------- AUTOCOMPLETE ----------------
vector<string> SearchEngine::autocompleteAPI(const string& prefix) {
    return trie.autocomplete(normalize(prefix));
}

// ---------------- CLEAR INDEX ----------------
void SearchEngine::clearIndex() {
    documents.clear();
    invertedIndex.clear();
    documentContents.clear();
    trie = Trie();
    usingSample = false;
}

// ---------------- LOAD SAMPLE ----------------
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

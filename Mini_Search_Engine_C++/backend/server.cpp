#include "httplib.h"
#include "SearchEngine.h"
#include <iostream>

using namespace std;

string toJson(const vector<SearchResult>& results) {
    string json = "{ \"results\": [";

    for (int i = 0; i < results.size(); i++) {
        auto& r = results[i];
        json += "{";
        json += "\"document\":\"" + r.document + "\",";
        json += "\"frequency\":" + to_string(r.frequency) + ",";

        json += "\"positions\":[";
        for (int j = 0; j < r.positions.size(); j++) {
            json += to_string(r.positions[j]);
            if (j + 1 < r.positions.size()) json += ",";
        }
        json += "],";

        json += "\"offsets\":[";
        for (int j = 0; j < r.offsets.size(); j++) {
            json += to_string(r.offsets[j]);
            if (j + 1 < r.offsets.size()) json += ",";
        }
        json += "]";

        json += "}";
        if (i + 1 < results.size()) json += ",";
    }

    json += "] }";
    return json;
}

int main() {
    SearchEngine engine;

    engine.addDocument("../documents/doc1.txt");
    engine.addDocument("../documents/doc2.txt");
    engine.addDocument("../documents/doc3.txt"); 

    engine.buildIndex();   //  INDEX BUILT ONCE

    httplib::Server server;

    // Exact search
    server.Get("/search", [&](const httplib::Request& req,
                              httplib::Response& res) {
        auto q = req.get_param_value("q");
        auto results = engine.searchAPI(q);
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(toJson(results), "application/json");
    });

    // Autocomplete
    server.Get("/autocomplete", [&](const httplib::Request& req,
                                    httplib::Response& res) {
        auto prefix = req.get_param_value("prefix");
        auto words = engine.autocompleteAPI(prefix);

        string json = "{ \"suggestions\": [";
        for (int i = 0; i < words.size(); i++) {
            json += "\"" + words[i] + "\"";
            if (i + 1 < words.size()) json += ",";
        }
        json += "] }";

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(json, "application/json");
    });

    cout << "Fast Search Engine running at http://localhost:8080\n";
    server.listen("localhost", 8080);
}

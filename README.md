# Mini-Search-Engine-Cpp
Fast Search Engine in C++ using Inverted Index for efficient exact search and Trie for autocomplete. Documents are indexed once to enable fast queries without rescanning files. Includes a web frontend that displays document matches, frequency, positions, and offsets.



# Fast Search Engine in C++ 🚀  
### Inverted Index + Trie Based Mini Search Engine with Web Frontend

This project implements a **fast and efficient text search engine** in **C++**, inspired by the core ideas used in real-world search engines.  
It uses **Inverted Indexing** for fast exact word search and a **Trie data structure** for prefix-based autocomplete, combined with a **web-based frontend** for interactive searching.

---

## 📌 Project Overview

Traditional text search scans documents line by line, which becomes slow as data grows.  
This project solves that problem by separating the system into two phases:

1. **Indexing Phase (one-time, slow)**
2. **Querying Phase (fast, repeated)**

All documents are indexed once, and subsequent searches are performed in near constant time without rescanning files.

---

## ✨ Key Features

- 🔍 **Fast Exact Search**
  - Uses **Inverted Index** (word → documents mapping)
  - No document scanning during queries
- ⚡ **Autocomplete Suggestions**
  - Implemented using **Trie**
  - Supports prefix-based search suggestions
- 📊 **Rich Search Metadata**
  - Word frequency
  - Word positions
  - Byte offsets
- 🌐 **Web-Based Frontend**
  - Built using HTML, CSS, and JavaScript
  - Communicates with backend via HTTP APIs
- 🧠 **Clean Architecture**
  - Clear separation between indexing and querying
  - Backend logic independent of frontend UI

---


## 🏗️ System Architecture

User (Browser)
|
| HTTP Requests
v
Frontend (HTML / CSS / JS)
|
| REST API
v
C++ Backend Server
|
v
Search Engine Core
├── Inverted Index (Fast Exact Search)
└── Trie (Autocomplete)





## 📁 Project Structure

Mini_Search_Engine_C++/
│
├── backend/
│ ├── include/
│ │ ├── SearchEngine.h
│ │ ├── Trie.h
│ │ └── httplib.h
│ │
│ ├── src/
│ │ ├── SearchEngine.cpp
│ │ └── Trie.cpp
│ │
│ └── server.cpp
│
├── frontend/
│ ├── index.html
│ ├── script.js
│ └── style.css
│
└── documents/
├── doc1.txt
├── doc2.txt
└── doc3.txt






---

## ⚙️ How It Works

### 🔹 Indexing Phase
- All documents are read once
- Each word is:
  - Normalized (lowercase, punctuation removed)
  - Stored in an **Inverted Index** with:
    - Document ID
    - Frequency
    - Positions
    - Byte offsets
  - Inserted into a **Trie** for autocomplete

### 🔹 Querying Phase
- Exact search:
  - Direct lookup in the inverted index
- Autocomplete:
  - Prefix lookup in the trie
- No file I/O during search queries

---

## 🚀 Getting Started

### 🔧 Backend Setup

```bash
cd backend
g++ -std=c++20 server.cpp src/SearchEngine.cpp src/Trie.cpp -I include -o server
./server



🌐 Frontend Setup
cd frontend
python3 -m http.server 5500


Open in browser:

http://localhost:5500



🧪 Example Queries
Exact Search
search
engine
inverted

Autocomplete
se → search, searching, search engine
in → inverted, indexing, information

⏱️ Time Complexity
Operation	Complexity
Indexing	O(N) (one-time)
Search	O(1) + results
Autocomplete	O(prefix length + matches)
🧠 Concepts & Technologies Used

Inverted Index

Trie Data Structure

Hash Maps

File Handling in C++

HTTP Server (cpp-httplib)

REST API Design

Frontend–Backend Integration

📌 Future Enhancements

Phrase search ("search engine")

Ranking (TF-IDF)

Snippet generation using offsets

Fuzzy search (typo tolerance)

Persistent disk-based index





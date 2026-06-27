from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from langchain_ollama import OllamaLLM, OllamaEmbeddings # Added OllamaEmbeddings
from langchain_core.prompts import PromptTemplate
import requests
import uvicorn
from fastapi.middleware.cors import CORSMiddleware
import numpy as np
from collections import deque

app = FastAPI(title="RAG Orchestration API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# 1. Setup Models
# Use a dedicated embedding model for vector math
embedder = OllamaEmbeddings(model="nomic-embed-text", base_url="http://localhost:11434")
llm = OllamaLLM(model="llama3", base_url="http://localhost:11434")

MAX_CACHE_SIZE = 100
semantic_cache = deque(maxlen=MAX_CACHE_SIZE)
SIMILARITY_THRESHOLD = 0.95

class ChatRequest(BaseModel):
    question: str

@app.post("/chat")
def rag_chat(request: ChatRequest):
    # A. SEMANTIC CACHE CHECK
    question_vector = np.array(embedder.embed_query(request.question))
    
    for entry in semantic_cache:
        similarity = np.dot(question_vector, entry["vector"]) / (
            np.linalg.norm(question_vector) * np.linalg.norm(entry["vector"])
        )
        if similarity > SIMILARITY_THRESHOLD:
            return {"answer": entry["answer"], "cached": True}

    # B. RAG LOGIC
    try:
        cpp_response = requests.get(f"http://localhost:8080/search?q={request.question}&page=1&limit=5")
        cpp_response.raise_for_status()
        search_data = cpp_response.json()

        snippets = [res.get("snippet", "") for res in search_data.get("results", [])]
        context = "\n---\n".join(snippets) if snippets else "No relevant context found."

        template = "Context: {context}\n\nQuestion: {question}\n\nAnswer:"
        prompt = PromptTemplate(template=template, input_variables=["context", "question"])
        
        answer = llm.invoke(prompt.format(context=context, question=request.question))

        # Save to cache
        semantic_cache.append({"vector": question_vector, "answer": answer})

        return {"answer": answer, "cached": False}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=3000)
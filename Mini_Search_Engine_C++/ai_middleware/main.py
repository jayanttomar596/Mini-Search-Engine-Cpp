from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from langchain_ollama import OllamaLLM  # UPDATED IMPORT
from langchain_core.prompts import PromptTemplate
import requests
import uvicorn
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(title="RAG Orchestration API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allows all origins
    allow_methods=["*"],  # Allows all methods (POST, GET, etc.)
    allow_headers=["*"],  # Allows all headers
)

# Setup local Ollama LLM using the new updated class
llm = OllamaLLM(model="llama3", base_url="http://localhost:11434")

class ChatRequest(BaseModel):
    question: str

@app.post("/chat")
def rag_chat(request: ChatRequest):
    try:
        # 1. RETRIEVAL: Query the C++ High-Performance Engine
        cpp_response = requests.get(f"http://localhost:8080/search?q={request.question}&page=1&limit=5")
        cpp_response.raise_for_status()
        search_data = cpp_response.json()

        # 2. EXTRACT CONTEXT: Pull the Top-K snippets
        snippets = []
        if "results" in search_data:
            for res in search_data["results"]:
                snippets.append(res.get("snippet", ""))
        
        context = "\n---\n".join(snippets) if snippets else "No relevant context found in the database."

        # 3. AUGMENTATION: Build the LangChain Prompt
        template = """
        You are an intelligent assistant. Use ONLY the following context retrieved from our C++ database to answer the question.
        If the context does not contain the answer, say "I do not have enough information in the database to answer that."

        Context:
        {context}

        Question: {question}
        
        Answer:
        """
        prompt = PromptTemplate(template=template, input_variables=["context", "question"])
        formatted_prompt = prompt.format(context=context, question=request.question)

        # 4. GENERATION: Stream to Ollama
        answer = llm.invoke(formatted_prompt)

        return {"answer": answer, "context_used": len(snippets)}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=3000)


    
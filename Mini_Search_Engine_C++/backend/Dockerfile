# Build Stage
FROM gcc:latest AS builder
RUN apt-get update && apt-get install -y cmake poppler-utils
COPY . /app
WORKDIR /app
RUN g++ -O3 server.cpp src/SearchEngine.cpp src/Trie.cpp -o engine -lpthread

# Run Stage
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libstdc++6 poppler-utils
WORKDIR /app
COPY --from=builder /app/engine .
# Create necessary dirs
RUN mkdir -p ../runtime_corpus ../database
EXPOSE 8080
CMD ["./engine"]
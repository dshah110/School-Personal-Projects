# Context Retriever

This is a context retriever that uses OpenSearch and LangChain to retrieve context from a given query. This was a hackathon project I and some friends and colleagues worked on. 

What this does is it that given a repository you want to analyze (public or private if provided auth token), it will download the repository, split it into chunks, and embed it into OpenSearch using Qodo's embedding model. 

Then, given a query, it will perform an embedding of the query to retrieve the most relevant context chunks from the opensearch database. 

This was a project that was done in around a day so there is MUCH to improve and clean up. Currently it splits code based on file type (go, yaml, sh, js, c, cpp) and splits the code into chunks of 1000 tokens with an overlap of 200 tokens. In the future it may be possible to split the code based on function or class definitions.



## Setup

1. Install dependencies:
```bash
pip install -r requirements.txt
```

2. Set environment variables:
```bash
export OPENSEARCH_URL="https://search-test-domain-cytn3qyhwkkjflb7fpfg47djwi.us-east-1.es.amazonaws.com"
export QODO_MODEL_NAME="Qodo/Qodo-Embed-1-1.5B"
export NUM_GPUS="4"
export BULK_SIZE="10000"
```


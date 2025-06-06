# VibeLang Examples

This directory contains sample `.vibe` programs that can be compiled with `vibec`.

## Running the Joke Example

1. Ensure `OPENAI_API_KEY` is set in your environment. Example:
   ```bash
   export OPENAI_API_KEY=YOUR_OPENAI_API_KEY
   ```
   Replace `YOUR_OPENAI_API_KEY` with the actual key provided by OpenAI.

2. Compile the example and run:
   ```bash
   vibec joke.vibe
   # joke.c contains the generated function
   gcc -o joke_app joke_app.c joke.c -lvibelang
   ./joke_app <topic>
   ```
   The program will print a short joke about the given topic.

# Pokemon Query Program

This program is designed to explore the properties of Pokemon by providing a client-server system. The primary purpose of this project is to allow users to look up information about different Pokemon, even when they are in remote locations. The program consists of two main components:

1. **Pokemon Property Server (PPS)**
   - The administrator initiates the PPS program.
   - The PPS prompts the administrator to enter the name of the file containing Pokemon descriptions.
   - PPS starts listening on a predefined IP address and port (localhost:7001).
   
2. **Pokemon Query Client (PQC)**
   - Gamers can start the PQC and connect to the PPS.
   - PQC offers a menu with options to search for Pokemon by type, save query results, and exit the program.

## Project Files
The source files for this project include:
- `server.c`
- `client.c`
- `utils.c`
- `thread.c`
- `utils.h`
- `thread.h`
- `Makefile`
- `design_doc.pdf`
- `pokemon.csv`

## Instructions for Compiling and Running
1. **Compile the code**: Use the provided Makefile with the command `make <file>` (replace `<file>` with `server`, `client`, or `all` as needed).
2. **Run the file**: Use the command `./<name>` (replace `<name>` with either `server` or `client`). This executes the compiled machine code generated from the previous step.

## Compilation and Execution Commands
Here are some examples of how to compile and run the program:
- Compile server and client, then clean: 
    ```
    make server && make client && make clean
    ```
- Compile all files, then clean: 
    ```
    make all && make clean
    ```
- Compile all files, then clean: 
    ```
    make && make clean
    ```

## Execution Commands
After compilation, you can run the program using either of these commands:
- Run client: 
    ```
    ./client
    ```
- Run server: 
    ```
    ./server
    ```
- Run client with Valgrind for memory leak checks: 
    ```
    valgrind --leak-check=full ./client
    ```
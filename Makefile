# Compiler settings
CC=gcc
CFLAGS=-g -Wall -std=gnu11 -Werror

# Program names and sources
PARSER=parser
PARSER_SRC=parser.c

SEARCH=search
SEARCH_SRC=search.c

# Default target to build both programs
all: $(PARSER) $(SEARCH)

# Rule for compiling the parser program
$(PARSER): $(PARSER_SRC) card.h
	$(CC) $(CFLAGS) -o $(PARSER) $(PARSER_SRC)

# Rule for compiling the search program
$(SEARCH): $(SEARCH_SRC)
	$(CC) $(CFLAGS) -o $(SEARCH) $(SEARCH_SRC)

# Rule to run both programs
run: $(PARSER) $(SEARCH)
	./$(PARSER)
	./$(SEARCH)

# Clean up compiled files
clean:
	rm -f $(PARSER) $(SEARCH)

.PHONY: all run clean

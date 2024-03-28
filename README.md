# Advanced Pokémon Card Search System Documentation

## Overview
This documentation provides an overview of the Advanced Pokémon Card Search System, a program designed to manage and search through a database of Pokémon cards and their associated abilities. The system is developed in C, utilizing advanced programming concepts and C standard libraries to achieve efficient data parsing, storage, and retrieval.

![Pidgeot](https://github.com/LinkdJulioFlores/Pokemon-Card-Search/assets/64749260/b811b44f-1df2-43fb-814e-14b1fdc56c90)
![Pikachu](https://github.com/LinkdJulioFlores/Pokemon-Card-Search/assets/64749260/37106ee4-5b69-4682-9e7a-4c1465cc7106)

## Features
- **CSV Parsing**: The system can parse CSV files containing Pokémon card and ability data, creating structured representations in memory.
- **Binary File I/O**: Data is stored in binary format for efficient access and retrieval, with separate files for Pokémon cards, abilities, and indexes.
- **Dynamic Data Structures**: Utilizes dynamic arrays of pointers to structures (`ABILITY_T**`, `POKEMON_T**`) for scalable data management.
- **Advanced Sorting and Searching**: Implements custom sorting functions for abilities and Pokémon, and provides a search mechanism using indexes.
- **Memory Management**: Employs dynamic memory allocation and deallocation to manage resources effectively, ensuring stability and performance.

## Key Functions and Concepts
- `parse_ability()`, `parse_pokemon()`: Functions to parse individual entries from CSV files and populate corresponding structures.
- `write_ability()`, `write_pokemons()`, `write_index()`: Functions to write parsed data into binary files, ensuring proper format and structure.
- `sort_abilities()`, `sort_pokemon()`, `sort_index()`: Sorting functions used for organizing data to facilitate efficient searching.
- `search_comp()`: A comparison function used during search operations to match user queries with available data.
- **Dynamic Memory Allocation**: The program makes extensive use of `malloc()`, `realloc()`, and `free()` to manage memory for dynamic data structures.
- **File Stream Functions**: Utilizes `fopen()`, `fwrite()`, `fread()`, and `fclose()` for reading and writing binary files.
- **String Manipulation**: Employs `strdup()`, `strsep()`, and `strcmp()` for parsing and comparing strings.
- **Type Conversions**: Uses `atoi()` and `atof()` for converting string data to integer and floating-point values, respectively.

## Compilation and Execution
The program is compiled using GNU11 standards with strict error and warning flags (`-std=gnu11`, `-Werror`, `-Wall`) to ensure code quality. It is divided into two main parts: the parser, which reads and processes CSV files, and the search application, which allows users to query the processed data.

## Usage
- **Parser**: Processes CSV files and generates binary files for Pokémon, abilities, and indexes.
- **Search Application**: Reads binary files and allows users to search for Pokémon cards by name, displaying relevant information and associated abilities.

## Data Structure and File Format
- **Binary Files**: The system uses a structured binary format for storing Pokémon and ability data, optimizing space and access speed.
- **Indexes**: Utilizes an index mechanism to quickly locate Pokémon data in binary files, improving search performance.

## Dependencies
- GNU C Library (`#define _GNU_SOURCE`)
- Standard C Libraries: `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<stdint.h>`, `<search.h>`
- Custom Header: `"card.h"` for structure definitions and constants.

This system showcases advanced programming techniques in C, including file handling, dynamic memory management, and data structure manipulation, providing a robust and efficient solution for managing and searching Pokémon card data.

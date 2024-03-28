#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <stdint.h>
#include "card.h"
// :cd %:h set wdir

// you will need to implement these two functions:
ABILITY_T* parse_ability(char * buf);
POKEMON_T* parse_pokemon(char * buf);

void format(char *);
// making these global will make things easier
ABILITY_T** abilities;
size_t total_abilities;
POKEMON_T** pokemons;
size_t total_pokemons;

// this is used if/when sorting the abilities by name
int sort_abilities(const void *a, const void *b) {
    // qsort uses this
    const ABILITY_T *A = *(const ABILITY_T **)a;
    const ABILITY_T *B = *(const ABILITY_T **)b;
    return strcmp(A->name, B->name);
}

// this is used when sorting the pokemon by name
int sort_pokemon(const void *a, const void *b) {
    const POKEMON_T *A = *(const POKEMON_T **)a;
    const POKEMON_T *B = *(const POKEMON_T **)b;
	return strcmp(A->name, B->name);
}

// Sort the indexes
int sort_index(const void *a, const void *b)
{
    const INDEX_T *A = *(const INDEX_T **)a;
    const INDEX_T *B = *(const INDEX_T **)b;
    return strcmp(A->name, B->name);
}

// this is used when comparing the string of the ability in the
// pokemon buf in parse_pokemon to the abilities array entries
int search_comp(const void *a, const void *b) {
    const char *A = (const char *)a;
    const ABILITY_T *B = *(const ABILITY_T **)b;
	return strcmp(A, B->name);
}

/*--------------Write Abilities-----------------*/
void write_ability(FILE* fd_abilities_binary, int i)
{
    ABILITY_T* ability = abilities[i];

    // Write offset based on file pointer
    ability->offset = ftell(fd_abilities_binary);

    uint64_t nameLength = strlen(ability->name);
    fwrite(&nameLength, sizeof(nameLength), 1, fd_abilities_binary);
    fwrite(ability->name, sizeof(char), nameLength, fd_abilities_binary);

    uint64_t descLength = strlen(ability->desc);
    fwrite(&descLength, sizeof(descLength), 1, fd_abilities_binary);
    fwrite(ability->desc, sizeof(char), descLength, fd_abilities_binary);
}

/*--------------Write Pokemons-----------------*/
void write_pokemons(FILE* fd_pokemons_binary, INDEX_T* index, int i)
{
    POKEMON_T* pokemon = pokemons[i];

    // Store the offset
    index->offset = ftell(fd_pokemons_binary);

    // Store the name onto the index
    index->name = pokemon->name;

    // 3 * 4 Bytes
    fwrite(&pokemon->id, sizeof(pokemon->id), 1, fd_pokemons_binary);

    uint32_t type = (uint32_t)pokemon->type;
    uint32_t subtype = (uint32_t)pokemon->subtype;
    fwrite(&type, sizeof(type), 1, fd_pokemons_binary);
    fwrite(&subtype, sizeof(subtype), 1, fd_pokemons_binary);

    // 3 * 8 Bytes
    uint64_t abilityOneOffset = pokemon->ability_one ? pokemon->ability_one->offset : INT64_MAX;
    uint64_t abilityTwoOffset = pokemon->ability_two ? pokemon->ability_two->offset : INT64_MAX;
    uint64_t abilityThreeOffset = pokemon->ability_three ? pokemon->ability_three->offset : INT64_MAX;
    fwrite(&abilityOneOffset, sizeof(abilityOneOffset), 1, fd_pokemons_binary);
    fwrite(&abilityTwoOffset, sizeof(abilityTwoOffset), 1, fd_pokemons_binary);
    fwrite(&abilityThreeOffset, sizeof(abilityThreeOffset), 1, fd_pokemons_binary);

    // 9 & 4 Bytes
    fwrite(&pokemon->base_experience, sizeof(pokemon->base_experience), 1, fd_pokemons_binary);

    fwrite(&pokemon->weight, sizeof(pokemon->weight), 1, fd_pokemons_binary);
    fwrite(&pokemon->height, sizeof(pokemon->height), 1, fd_pokemons_binary);

    fwrite(&pokemon->hp, sizeof(pokemon->hp), 1, fd_pokemons_binary);
    fwrite(&pokemon->attack, sizeof(pokemon->attack), 1, fd_pokemons_binary);
    fwrite(&pokemon->defense, sizeof(pokemon->defense), 1, fd_pokemons_binary);
    fwrite(&pokemon->special_attack, sizeof(pokemon->special_attack), 1, fd_pokemons_binary);
    fwrite(&pokemon->special_defense, sizeof(pokemon->special_defense), 1, fd_pokemons_binary);
    fwrite(&pokemon->speed, sizeof(pokemon->speed), 1, fd_pokemons_binary);

    // 1 * 8 bytes
    fwrite(&pokemon->offset, sizeof(pokemon->offset), 1, fd_pokemons_binary);
}

void write_index(FILE* fd_index_binary, INDEX_T *index)
{
    // Write the name
    uint64_t nameLength = strlen(index->name);
    fwrite(&nameLength, sizeof(nameLength), 1, fd_index_binary);
    fwrite(index->name, sizeof(char), nameLength, fd_index_binary);
    fwrite(&index->offset, sizeof(off_t), 1, fd_index_binary);
}

// The goal of parser is just to output to binary
// so that ./search can work
int main(int argc, char **argv) 
{
    // A1 Has comments that better explain this program
    // Not going to comment as much to make it easier on me when going through
    // I already know what this program accomplishes
    size_t numberOfAbilities = 0; // Size of buffer
    FILE *fd_abilities = fopen("abilities.csv", "r");
    char *line = NULL;

    getline(&line, &numberOfAbilities, fd_abilities);

    while(getline(&line, &numberOfAbilities, fd_abilities) != -1)
    {
        ABILITY_T *temp = parse_ability(line);
        if(temp == NULL) continue;

        abilities = realloc(abilities, (total_abilities + 1) * sizeof(ABILITY_T *));
        abilities[total_abilities] = temp;

        total_abilities++;

        free(line);
        line = NULL;
    }
    free(line);
    
    // DONE: Make abilities a binary file
    FILE *fd_abilities_binary = fopen("abilities.bin", "wb");
    if(fd_abilities_binary == NULL)
    {
        perror("Failed to open: abilities.bin\n");
        return -10;
    }

    // Write abilities data to abilities.bin
    for(int i = 0; i < total_abilities; i++)
    {
        write_ability(fd_abilities_binary, i);
    }
    fclose(fd_abilities_binary);
    // End
    
    size_t numberOfPokemon = 0;
    FILE *fd_pokemon = fopen(argv[1], "r");

    // Get the format line out of the way
    getline(&line, &numberOfPokemon, fd_pokemon);
    free(line);
    line = NULL;

    while(getline(&line, &numberOfPokemon, fd_pokemon) != -1)
    {
        POKEMON_T *temp = parse_pokemon(line);
        if(!temp)
            continue;
        
        pokemons = realloc(pokemons, (total_pokemons + 1) * sizeof(POKEMON_T *));
        pokemons[total_pokemons] = temp;

        total_pokemons++;
        free(line);
        line = NULL;
    }
    free(line);

    // Put the pokemons into a binary
    FILE *fd_pokemons_binary = fopen("pokemon.bin", "wb");
    if(fd_pokemons_binary == NULL)
    {
        perror("Failed to open: pokemon.bin\n");
        return -12;
    }

    INDEX_T **index = malloc(sizeof(INDEX_T *) * total_pokemons);
    for(int i=0; i<total_pokemons; i++)
    {
        index[i] = malloc(sizeof(INDEX_T));
        write_pokemons(fd_pokemons_binary, index[i], i);
    }
    
    fclose(fd_pokemons_binary);

    qsort(index, total_pokemons, sizeof(INDEX_T *), sort_index);

    // Put the indexes into index.bin
    FILE *fd_index_binary = fopen("indexes.bin", "wb");
    // Write the first line
    fwrite(&total_pokemons, sizeof(uint64_t), 1, fd_index_binary);
    for(int i=0; i<total_pokemons; i++)
    {
        write_index(fd_index_binary, index[i]);
    }
    fclose(fd_index_binary);

    // ---------------------------------
    for(int i=0; i<total_pokemons; i++)
    {
        //printf("%s ", index[i]->name);
        //printf("%lu\n", index[i]->offset);
    }

    // DONE cleanup the rest of resources used in program
    for(int i=0; i<total_abilities; i++)
    {
        free(abilities[i]->name);
        free(abilities[i]->desc);
        free(abilities[i]);
    }
    free(abilities);

    for(int i=0; i<total_pokemons; i++)
    {
        free(pokemons[i]->name);
        free(pokemons[i]);
    }
    free(pokemons);

    for(int i=0; i<total_pokemons; i++)
        free(index[i]);
    free(index);

    fclose(fd_abilities); 
    fclose(fd_pokemon);

    return 0;
}

// this function has to allocate an ABILITY_T* for the ability
// then call strsep to parse out the name and description
// from the buffer text, and return
ABILITY_T* parse_ability(char * buf) {
    ABILITY_T *ability = malloc(sizeof(ABILITY_T));
    // Without strdup it won't work. Why?
    // because token only returns a pointer to a location
    // in ability line. We need to copy the string from then on.
    // replace . with null terminator
    ability->name = strdup(strsep(&buf, ","));
    ability->desc = strdup(strsep(&buf, "\n"));

    format(ability->name);
    // strrchr returns a pointer to the last ocurrance of delimeter
    // Found in stack overflow
    char *period = strrchr(ability->desc, '.');
    if (period && *(period + 1) == '\0'){
        *period = '\0';
    }

	return ability;
}

void format(char *str) {
    // Goal: capitalize first character and replace - with space
    // We are garanteed first lowercase and all - need to be replaced with space
    // We are garanteed that after a - there will be a character
    // It's good practice to be smarter about this but for this assignment
    //  it should be okay
    if (str == NULL) return;

    char *p = str;
    int isFirstTime = 1;
    while(*p != '\0')    
    {
        if(isFirstTime)
        {
            *p -= 32;
            isFirstTime = 0;
        }
        if(*p == '-')
        {
            *p = 32;
            p++;
            *p -= 32;
            p--;
        }
        p++;
    }
    // Since strdup includes a \0 it's not needed but it's 
    // easy and good practice to include it just in case
    *p = '\0';
}

// this function has a lot going on it it
// once you have the parsed to the id, if it is missing return null as 
// this means it is an empty line. if it is there, then you can continue.
// allocate a POKEMON_T*, and start filling out the data in it.
// for the abilities, once the name is read search for it in the abilities array.
// the weight and height fields have to be converted to the right unit
// of measurement, and finally into a proper float.
// weight/10 height/10
POKEMON_T* parse_pokemon(char * buf) {
    POKEMON_T *pokemon = malloc(sizeof(POKEMON_T));
    char *token = NULL;

    // Name
    // If checks that token exists but since we are garanteed
    // a name in the file it's most likely not needed but oh well
    token = strdup(strsep(&buf, ","));
    if(!token && *token == '\0')
        pokemon->name = "None";
    format(token);
    pokemon->name = token;

    // unsigned id 
    // If there is no id then get out of here
    // and free variables
    token = strsep(&buf, ",");
    if(token == NULL || *token == '\0'){
        free(pokemon->name);
        free(pokemon);
        return NULL;
    }
    pokemon->id = atoi(token);


    // Type type
    // Checks the type with i as the glue and assigns casts it 
    // to the right enum
    Type myType = NONE;
    token = strdup(strsep(&buf, ","));
    for(int i=0; i<sizeof(type_str) / sizeof(char *); i++)
    {
        // Some types are not uppercase and it needs to be
        if(token[0] >= 'a' && token[0] <= 'z')
            token[0] = token[0] - 32;

        if(strcmp(token, type_str[i]) == 0)
        {
            myType = (Type)i;
            break;
        }
    }
    free(token);
    pokemon->type = myType;

    // Type subtype
    // same as before
    myType = NONE;
    token = strdup(strsep(&buf, ","));
    for(int i=0; i<sizeof(type_str) / sizeof(char *); i++)
    {
        if(token[0] >= 'a' && token[0] <= 'z')
            token[0] = token[0] - 32;

        if(strcmp(token, type_str[i]) == 0)
        {
            myType = (Type)i;
            break;
        }
    }
    free(token);
    pokemon->subtype = myType;

    // ABILITY_T ability_one
    // All abilities are formatted so to match it against key with 
    // bsearch we need to also format token as well
    token = strdup(strsep(&buf, ","));
    format(token);
    if(token && *token != '\0')
    {
        ABILITY_T **key = (ABILITY_T **)lfind(token, 
                      abilities, 
                      &total_abilities, 
                      sizeof(ABILITY_T *), 
                      search_comp);
        pokemon->ability_one = (key != NULL) ? *key : NULL;
    } else {
        pokemon->ability_one = NULL;
    }
    free(token);

    // ABILITY_T *ability_two
    // same as before
    token = strdup(strsep(&buf, ","));
    format(token);
    if(token != NULL && *token != '\0')
    {
        ABILITY_T **key = (ABILITY_T **)lfind(token, 
                      abilities, 
                      &total_abilities, 
                      sizeof(ABILITY_T *), 
                      search_comp);
        pokemon->ability_two = (key != NULL) ? *key : NULL;
    } else {
        pokemon->ability_two = NULL;
    }
    free(token);

    // ABILITY_T *ability_three
    // same as before
    token = strdup(strsep(&buf, ","));
    format(token);
    if(token != NULL && *token != '\0')
    {
        ABILITY_T **key = (ABILITY_T **)lfind(token, 
                      abilities, 
                      &total_abilities, 
                      sizeof(ABILITY_T *), 
                      search_comp);
        pokemon->ability_three = (key != NULL) ? *key : NULL;
        // Is this necessary?
    } else 
        pokemon->ability_three = NULL;
    free(token);
        
    // unsigned base_experience
    // Guards against empty strings and casts it to the right type
    token = strsep(&buf, ",");
    pokemon->base_experience = (*token != '\0') ? (unsigned)atoi(token) : 0;


    // Guards against empty strings and casts it to the right type
    // float weight/10
    token = strsep(&buf, ",");
    pokemon->weight = (*token != '\0') ? atof(token)/10 : 0.0f;
    
    // Guards against empty strings and casts it to the right type
    // float height/10
    token = strsep(&buf, ",");
    pokemon->height = (*token != '\0') ? atof(token)/10 : 0.0f;

    // Guards against empty strings and casts it to the right type
    // unsigned hp
    token = strsep(&buf, ",");
    pokemon->hp = (*token != '\0') ? (unsigned)atoi(token) : 0;


    // Guards against empty strings and casts it to the right type
    // unsigned attack
    token = strsep(&buf, ",");
    pokemon->attack = (*token != '\0') ? (unsigned)atoi(token) : 0;

    // Guards against empty strings and casts it to the right type
    // unsigned defense
    token = strsep(&buf, ",");
    pokemon->defense = (*token != '\0') ? (unsigned)atoi(token) : 0;

    // Guards against empty strings and casts it to the right type
    // unsigned special_attack
    token = strsep(&buf, ",");
    pokemon->special_attack = (*token != '\0') ? (unsigned)atoi(token) : 0;

    // Guards against empty strings and casts it to the right type
    // unsigned special_defense 
    // to be fair all the previous ones should look like this
    token = strsep(&buf, ",");
    if(token!=NULL)
        pokemon->special_defense = (*token != '\0') ? (unsigned)atoi(token) : 0;
    else
        pokemon->special_defense = 0;

    // Guards against empty strings and casts it to the right type
    // unsigned speed
    token = strsep(&buf, ",");
    if(token!=NULL)
        pokemon->speed = (*token != '\0') ? (unsigned)atoi(token) : 0;
    else
        pokemon->speed = 0;


    // off_t offset
    // We are garanteed an offset value
    token = strsep(&buf, ".");
    int parsedValue = atoi(token);
    pokemon->offset = (*token != '\0') ? (off_t)parsedValue : 0;

	return pokemon;
}

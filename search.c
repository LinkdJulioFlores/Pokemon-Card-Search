#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "card.h"

void pretty_print(POKEMON_T*, FILE*);
void free_pokemon(POKEMON_T *);
void find_ability_n(ABILITY_T*);
void handle_terminal_input(char*);
void handle_file_input(char*);
void format(char*);

POKEMON_T *bundle_pokemon(off_t offset, const char *name);

off_t find_pokemon_offset(const char *name, FILE *fd_indexes_bin, int total_pokemon);


// Goal we will read from 3 binaries that ./parser will create
// Then we will search that binary using the index (sort index)
//
// Currently the program is inefficient since it only holds one
// POKEMON_T instead of loading all of the data from the 3 .bin files
// and creating a POKEMON_T **pokemon.
int main(int argc, char **argv)
{    
    if(argc != 2)
    {
        perror("Usage: ./parser (0 || 1)");
        return -1;
    }
    if(*argv[1] != '0' && *argv[1] != '1') 
    {
        printf("usage: ./parser ->(0 || 1)<-\n");
        return -2;
    }

    if(isatty(0))
        handle_terminal_input(argv[1]);
    else 
        handle_file_input(argv[1]);

    return 0;
}

void handle_file_input(char *show_ascii)
{
    FILE *fd_indexes_bin = fopen("indexes.bin", "rb");

    // Read total_pokemon from indexes.bin
    size_t total_pokemon = 0;
    fread(&total_pokemon, sizeof(total_pokemon), 1, fd_indexes_bin);

    //FILE *fd_input_txt = fopen("input.txt", "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Iterate over each line in input.txt
    while ((read = getline(&line, &len, stdin)) != -1) {
        if (*line == '!') {
            printf(">> !\n");
            break;
        }

        // For error display
        char *temp = line;

        // For function manipulation
        char *token = strdup(strsep(&temp,"\n"));

        printf(">> %s\n", token);

        fseek(fd_indexes_bin, sizeof(total_pokemon), SEEK_SET);

        format(token);

        off_t pokemon_offset = find_pokemon_offset(token, fd_indexes_bin, total_pokemon);
        if (pokemon_offset == -1) {
            printf("%s not found!\n", line);
        } else {
            POKEMON_T *pokemon = bundle_pokemon(pokemon_offset, token);
            if (pokemon) {
                FILE *ascii = fopen("ascii.bin", "rb");
                pretty_print(pokemon, (*show_ascii == '1') ? ascii : NULL);
                fclose(ascii);
                free_pokemon(pokemon);
            }
        }
        free(token);
    }
    free(line);

    fclose(fd_indexes_bin);
}

// Safety checks everywhere to handle all 
// frees
void free_pokemon(POKEMON_T *pokemon)
{
    if(pokemon == NULL) return;
    if(pokemon->name) free(pokemon->name);

    if(pokemon->ability_one)
    {
        if(pokemon->ability_one->name)
            free(pokemon->ability_one->name);
        if(pokemon->ability_one->desc)
            free(pokemon->ability_one->desc);

        free(pokemon->ability_one);
    }

    if(pokemon->ability_two)
    {
        if(pokemon->ability_two->name)
            free(pokemon->ability_two->name);
        if(pokemon->ability_two->name)
            free(pokemon->ability_two->desc);

        free(pokemon->ability_two);
    }

    if(pokemon->ability_three)
    {
        if(pokemon->ability_three->name)
            free(pokemon->ability_three->name);
        if(pokemon->ability_three->name)
            free(pokemon->ability_three->desc);

        free(pokemon->ability_three);
    }

    free(pokemon);
}

// Function is used to format the input by the user such input is used
// to match against the pokemon name in indexes.bin
void format(char *str) {
    if (str == NULL || str[0] == '\0' || str[0] == '!') return;

    if (*str >= 'a' && *str <= 'z') {
        *str -= 32;
    }

    char *p = str + 1;
    while(*p != '\0')    
    {
        if(*p >= 'A' && *p <= 'Z')
            *p += 32;

        p++;
    }
}

void handle_terminal_input(char *show_ascii)
{
    size_t total_pokemon = 0;
    size_t bufsize = 0;
    FILE *fd_indexes_bin = fopen("indexes.bin", "rb");
    fread(&total_pokemon, sizeof(total_pokemon), 1, fd_indexes_bin);
    size_t position = ftell(fd_indexes_bin);

    char *ans = NULL;
    do {
        fseek(fd_indexes_bin, position, SEEK_SET);
        printf(">> ");
        ssize_t len = getline(&ans, &bufsize, stdin);

        if(ans[len-1] == '\n')
        {
            ans[len - 1] = '\0';
            len -= 1;
        }

        if(ans[0] != '!')
        {
            char *temp = strdup(ans);
            format(ans);

            off_t pokemon_offset = find_pokemon_offset(ans, fd_indexes_bin, total_pokemon);

            if(pokemon_offset == -1)
            {
                printf("%s not found!\n", temp);
                free(temp);
                continue;
            } else {
                POKEMON_T *pokemon = bundle_pokemon(pokemon_offset, ans);
                if(pokemon)
                {
                    FILE *ascii = fopen("ascii.bin", "rb");
                    pretty_print(pokemon, (*show_ascii == '1') ? ascii : NULL);
                    fclose(ascii);
                    free_pokemon(pokemon);
                    free(temp);
                }
            }
        }
    }while (ans[0] != '!');
    free(ans);
    fclose(fd_indexes_bin);
}

// Just need to seek the index and use them to put it into
// pokemon for user display
off_t find_pokemon_offset(const char *name, FILE *fd_indexes_bin, int total_pokemon) {
    uint64_t len = 0;
    off_t offset = 0;
    char *temp_name = NULL;

    while(fread(&len, sizeof(len), 1, fd_indexes_bin) == 1)
    {
        char *new_temp_name = realloc(temp_name, len + 1);

        fread(new_temp_name, 1, len, fd_indexes_bin);
        if(!new_temp_name)
        {
            free(new_temp_name);
            printf("Nothing in temp name\n");
            return -3;
        }
        temp_name = new_temp_name;

        temp_name[len] = '\0';
        fread(&offset, sizeof(off_t), 1, fd_indexes_bin);

        // If we found the pokemon then we can use the offset 
        // to look into pokemon.bin from indexes.bin
        if(strcmp(temp_name, name) == 0)
        {
            free(temp_name);
            return offset;
        }
    }

    // We didn't find the name in indexes.bin
    free(temp_name);
    return -1;
}


void find_ability_n(ABILITY_T *ability)
{
    FILE *fd_ability_bin = fopen("abilities.bin", "rb");
    if(!fd_ability_bin)
    {
        perror("Failed to open abilities.bin\n");
        exit(EXIT_FAILURE);
    }

    off_t my_offset = ability->offset;

    // Set the offset
    if(fseek(fd_ability_bin, my_offset, SEEK_SET) != 0)
    {
        perror("Failed seek: ABILITY\n");
        fclose(fd_ability_bin);
        exit(EXIT_FAILURE);
    }

    // Search abilities
    
    // Find name length
    uint64_t nameLength = 0;
    if (fread(&nameLength, sizeof(nameLength), 1, fd_ability_bin) != 1)
    {
        perror("Failed to read ability name LENGTH\n");
        fclose(fd_ability_bin);
        exit(EXIT_FAILURE);
    }
    
    // Set the name using name length
    ability->name = malloc(nameLength + 1);
    if(fread(ability->name, sizeof(char), nameLength, fd_ability_bin) != nameLength)
    {
        perror("Failed to find ability NAME\n");
    }
    ability->name[nameLength] = '\0';

    // Find desc
    uint64_t descLen = 0;
    fread(&descLen, sizeof(descLen), 1, fd_ability_bin);

    ability->desc = malloc(descLen + 1);
    if(fread(ability->desc, sizeof(char), descLen, fd_ability_bin) != descLen)
    {
        perror("Failed to find ability DESC\n");
    }
    ability->desc[descLen] = '\0';


    fclose(fd_ability_bin); 
}


POKEMON_T *bundle_pokemon(off_t offset, const char *name)
{
    POKEMON_T *pokemon = malloc(sizeof(POKEMON_T));
    if(pokemon == NULL)
        return NULL;

    pokemon->name = strdup(name);
    if(pokemon->name == NULL)
    {
        perror("Error assignment of pokemon: NAME\n");
        free(pokemon->name);
        free(pokemon);
        return NULL;
    }

    FILE *fd_pokemon_bin = fopen("pokemon.bin", "rb");


    // Seek to the given offset
    if(fseek(fd_pokemon_bin, offset, SEEK_SET) != 0)
    {
        perror("Failed seek\n");
        fclose(fd_pokemon_bin);
        free(pokemon->name);
        free(pokemon);
        return NULL;
    }

    // Unpack id
    if(fread(&pokemon->id, sizeof(pokemon->id), 1, fd_pokemon_bin) !=1)
    {
        perror("Error assignment of pokemon: ID\n");
        return NULL;
    }
    
    // Type
    if(fread(&pokemon->type, sizeof(pokemon->type), 1, fd_pokemon_bin) !=1)
    {
        perror("Error assignment of pokemon: TYPE\n");
        return NULL;
    }

    // Subtype
    if(fread(&pokemon->subtype, sizeof(pokemon->subtype), 1, fd_pokemon_bin) !=1)
    {
        perror("Error assignment of pokemon: SUBTYPE\n");
        return NULL;
    }

    // A_1 Offset
    pokemon->ability_one = malloc(sizeof(ABILITY_T));
    if(fread(&pokemon->ability_one->offset, sizeof(pokemon->ability_one->offset), 1, fd_pokemon_bin) !=1)
    {
        free(pokemon->ability_one);
        perror("Error assignment of pokemon: ABILITY_1->OFFSET\n");
        return NULL;
    }
    if(pokemon->ability_one->offset == INT64_MAX)
    {
        free(pokemon->ability_one);
        pokemon->ability_one = NULL;
    }
    else
        find_ability_n(pokemon->ability_one);

    // A_2 offset
    pokemon->ability_two = malloc(sizeof(ABILITY_T));
    if(fread(&pokemon->ability_two->offset, sizeof(pokemon->ability_two->offset), 1, fd_pokemon_bin) !=1)
    {
        free(pokemon->ability_two);
        perror("Error assignment of pokemon: ABILITY_2->OFFSET\n");
        return NULL;
    }
    if(pokemon->ability_two->offset == INT64_MAX)
    {
        free(pokemon->ability_two);
        pokemon->ability_two = NULL;
    }
    else
        find_ability_n(pokemon->ability_two);

    // A_3 offset
    pokemon->ability_three = malloc(sizeof(ABILITY_T));
    if(fread(&pokemon->ability_three->offset, sizeof(pokemon->ability_three->offset), 1, fd_pokemon_bin) !=1)
    {
        free(pokemon->ability_three);
        perror("Error assignment of pokemon: ABILITY_3->OFFSET\n");
        return NULL;
    }
    if(pokemon->ability_three->offset == INT64_MAX)
    {
        free(pokemon->ability_three);
        pokemon->ability_three = NULL;
    }
    else
        find_ability_n(pokemon->ability_three);

    // base_experience
    if(fread(&pokemon->base_experience, sizeof(pokemon->base_experience), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: BASE EXPERIENCE\n");
        return NULL;
    }

    // weight
    if(fread(&pokemon->weight, sizeof(pokemon->weight), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: WEIGHT\n");
        return NULL;
    }

    // Height
    if(fread(&pokemon->height, sizeof(pokemon->height), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: HEIGHT\n");
        return NULL;
    }
    // HP
    if(fread(&pokemon->hp, sizeof(pokemon->hp), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: HP\n");
        return NULL;
    }
    // ATTACK
    if(fread(&pokemon->attack, sizeof(pokemon->attack), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: ATTACK\n");
        return NULL;
    }
    // DEFENSE
    if(fread(&pokemon->defense, sizeof(pokemon->defense), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: DEFENSE\n");
        return NULL;
    }
    // SPECIAL ATTACK
    if(fread(&pokemon->special_attack, sizeof(pokemon->special_attack), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: special_attack\n");
        return NULL;
    }
    // SPECIAL DEFENSE
    if(fread(&pokemon->special_defense, sizeof(pokemon->special_defense), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: SPECIAL DEFENSE\n");
        return NULL;
    }
    // SPEED
    if(fread(&pokemon->speed, sizeof(pokemon->speed), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: SPEED\n");
        return NULL;
    }
    // OFFSET
    if(fread(&pokemon->offset, sizeof(pokemon->offset), 1, fd_pokemon_bin) != 1)
    {
        perror("Error assignment of pokemon: HEIGHT\n");
        return NULL;
    }


    fclose(fd_pokemon_bin);
    return pokemon;
}

void pretty_print(POKEMON_T* pokemon, FILE* fp_ascii) {
    // PICTURE ////////////
    // if fp_ascii is null, that means its not supposed to display the art
    if(fp_ascii) {
        size_t data_len = 0;
        fseek(fp_ascii, pokemon->offset, SEEK_SET);
        fread(&data_len, sizeof(data_len), 1, fp_ascii);
        char *data_buf = malloc(data_len + 1);
        fread(data_buf, 1, data_len, fp_ascii);
        data_buf[data_len] = 0;
        printf("%s\n", data_buf);
        free(data_buf);
    }

    // ID, NAME & TYPE ////////////
    // print name & pokedex #
    printf("#%.03d ", pokemon->id);
    printf("%s\n", pokemon->name);
    // print type (and subtype if applicable)
    printf("\033[1m%s\033[0m", type_str[pokemon->type]);
    if (pokemon->subtype != NONE) {
        printf(" / \033[1m%s\033[0m\n", type_str[pokemon->subtype]);
    } else {
        printf("\n");
    }

    // ABILITIES ////////////
    // print abilities in a nice list
    printf("\n\033[1mAbilities:\033[0m");
    printf("\n1. %s\n", pokemon->ability_one->name);
    printf("   %s\n", pokemon->ability_one->desc);

    if(pokemon->ability_two){
        // sometimes the second ability is hidden
        if(pokemon->ability_three == NULL) {
            printf("\033[3m");
        }
        printf("2. %s", pokemon->ability_two->name);
        if(pokemon->ability_three == NULL) {
            printf(" (hidden)");
        }
        printf("\n   %s", pokemon->ability_two->desc);

        printf("\033[0m\n");
    }
    // third ability if it exists is guaranteed to be hidden. make italic
    if(pokemon->ability_three) {
        printf("\033[3m");
        printf("3. %s (hidden)\n", pokemon->ability_three->name);
        printf("   %s", pokemon->ability_three->desc);
        printf("\033[0m\n");
    }

    // remaining stat block
    printf("\nHP: %d\t\t| %.2fkg\n", pokemon->hp, pokemon->weight);
    if (pokemon->attack > 99) {
        printf("Atk: %d\t| %.2fm\n", pokemon->attack, pokemon->height);
    } else {
        printf("Atk: %d\t\t| %.2fm\n", pokemon->attack, pokemon->height);
    }
    if (pokemon->defense > 99) {
        printf("Def: %d\t|\n", pokemon->defense);
    } else {
        printf("Def: %d\t\t|\n", pokemon->defense);
    }
    printf("Sp. Atk: %d\t|\n", pokemon->special_attack);
    printf("Sp. Def: %d\t|\n", pokemon->special_defense);
    printf("Speed: %d\t| XP: %d\n\n", pokemon->speed, pokemon->base_experience);
}

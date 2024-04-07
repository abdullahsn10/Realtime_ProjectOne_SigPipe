#ifndef __FUNCTIONS__
#define __FUNCTIONS__

#include "header.h"
#include "constants.h"

void read_settings_file(char *filename); /* reading from file and fill arguments*/
void ask_user_tofill_settings();
void display_settings_and_init(); /* display all settings to the user and tell that the game will start*/

// reading from a settings file
void read_settings_file(char *filename)
{
    char line[255];
    char label[50];

    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Failed to open the file\n");
        exit(-2); // return failure status code
    }

    // seperator of the file
    char separator[] = "=";

    // read the file line by line until reaching null
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // split line
        char *str = strtok(line, separator);
        strncpy(label, str, sizeof(label));
        str = strtok(NULL, separator);

        // assign arguments according to the label
        if (strcmp(label, "NUMBER_OF_TEAMS") == 0)
        {
            NUMBER_OF_TEAMS = atoi(str);
        }
        else if (strcmp(label, "NUMBER_OF_ROUNDS") == 0)
        {
            NUMBER_OF_ROUNDS = atoi(str);
        }
        else if (strcmp(label, "NUMBER_OF_PLAYERS") == 0)
        {
            NUMBER_OF_PLAYERS = atoi(str);
        }
        else if (strcmp(label, "ROUND_DURATION") == 0)
        {
            ROUND_DURATION = atoi(str);
        }
    }

    fclose(file);
}

void ask_user_tofill_settings()
{
    int game_rduration, num_of_rounds;
    printf("--------------BEACH BALL GAME--------------\n");
    printf("[SETTINGS]:\n");
    while (1)
    {
        printf(">> PLEASE ENTER THE GAME ROUND DURATION (IN SEC): "); /* Get game duration*/
        scanf("%d", &game_rduration);
        if (!(game_rduration > 0))
        { /* Validation */
            printf("\n>> !!ERROR!!: Please Enter a Valid Game Duration\n");
            continue;
        }
        printf(">> PLEASE ENTER NUMBER OF ROUNDS: "); /* Get Number of Rounds*/
        scanf("%d", &num_of_rounds);
        if (!(num_of_rounds > 0))
        { /* Validation */
            printf("\n>> !!ERROR!!: Please Enter a Valid Number of Rounds\n");
            continue;
        }

        break;
    }
    // fill the settings
    ROUND_DURATION = game_rduration;
    NUMBER_OF_ROUNDS = num_of_rounds;

    printf("\n------------------------------------\n");
}

void display_settings_and_init()
{
    printf("\n[GAME SETTINGS]\nEVERYTHING IS OK, THE GAME SETTINGS ARE:\n");
    printf("GAME DURATION=[%d Sec]\t\tNUMBER OF ROUNDS=[%d]\n", ROUND_DURATION, NUMBER_OF_ROUNDS);
    printf("NUMBER OF TEAMS=[%d]\t\tNUMBER OF PLAYERS PER TEAM=[%d]\n", NUMBER_OF_TEAMS, NUMBER_OF_PLAYERS);
    printf("\n----------- THE GAME WILL START NOW :-) -----------\n");
}

#endif
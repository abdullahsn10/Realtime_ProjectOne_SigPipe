#include "header.h"
#include "functions.h"
#include "constants.h"
#include "gamefuncs.h"

// globals
int *teams;
char *fifo_fdes = "/tmp/fifi";

// game parameters
int current_round = 1;
int is_round_end = 0;

// teams score
int team_one_score = 0;
int team_two_score = 0;

// teams balls
int team_one_balls = 0;
int team_two_balls = 0;

// communction with opengl
int n, privatefifo, publicfifo;
struct message msg;

int main()
{
    // assign values to GAME SETTINGS
    read_settings_file("settings.txt");
    ask_user_tofill_settings();
    display_settings_and_init();

    // create the FIFO
    remove(fifo_fdes);
    if (mkfifo(fifo_fdes, __S_IFIFO | 0777) == -1)
    {
        perror("FIFO: Can't create the fifo\n");
        exit(50);
    }

    /* Open the public FIFO for writing */
    if ((publicfifo = open(PUBLIC, O_WRONLY)) == -1)
    {
        printf("Public error");
        perror(PUBLIC);
        exit(2);
    }
    // create teams array of pids
    teams = malloc(sizeof(int) * NUMBER_OF_TEAMS);
    pid_t pid; // temp var

    // create teams for the game
    for (int i = 0; i < NUMBER_OF_TEAMS; i++)
    {
        if ((pid = fork()) == -1)
        {
            perror("FORK: No Enough Memory\n");
            exit(5);
        }
        else if (pid == 0)
        {
            // child process (team)

            char team_num[20], num_of_players[20];
            sprintf(team_num, "%d", (i + 1));
            sprintf(num_of_players, "%d", NUMBER_OF_PLAYERS);
            execlp("./team", "team", team_num, num_of_players, NULL);

            // if no error, the team process will not execute this
            perror("EXEC: Can't Execute Team Functionality\n");
            exit(6);
        }
        else
        {
            // parent process (Owner of the game)

            teams[i] = pid; /* Save team pid */
            printf("[GAME NOTIFICATION]:\nTEAM NO.[%d] CREATED SUCCESSFULLY\n", (i + 1));
        }
    }

    sleep(10); // wait until all children be created

    // Starting the game from the parent application
    printf("-------------------STARTING THE GAME--------------------------\n\n");

    // game signals (sensitive)
    if (sigset(SIGALRM, end_round) == SIG_ERR)
    {
        perror("Sigset can not set SIGALRM");
        exit(SIGINT);
    }

    if (sigset(SIGUSR1, send_ball) == SIG_ERR)
    {
        perror("SIGNAL: can not set SIGUSR1\n");
        exit(SIGUSR1);
    }

    if (sigset(SIGUSR2, send_ball) == SIG_ERR)
    {
        perror("SIGNAL: can not set SIGUSR1\n");
        exit(SIGUSR1);
    }

    // ---------------------------------------------------------
    while (1)
    {
        // START ROUND

        is_round_end = 0;
        alarm(ROUND_DURATION + 3); // alarm to end the round
        printf("=============== ROUND [%d] Started ===============\n", current_round);

        // send the starting ball for each team
        for (int i = 0; i < NUMBER_OF_TEAMS; i++)
        {
            kill(teams[i], SIGINT); // send a ball for each team
        }

        // wait until the round end
        while (!is_round_end)
        {
            pause();
        }

        // END ROUND

        // Notify OpenGL that the round is end, TODO
        // Send to opengl scores, TODO
        sprintf(msg.cmd_line, "F,%d,%d,%d", current_round, team_one_score, team_two_score);
        write(publicfifo, (char *)&msg, sizeof(msg));

        if (current_round > NUMBER_OF_ROUNDS)
        {
            // END GAME (exit)
            break;
        }
    }

    // sensitive to teams

    sleep(20);
    return (0);
}

void end_round(int sig)
{

    // notify OpenGL that the round is end TODO:

    // Set the parent listening on the fifo (Reading)
    char msg_recv[BUFSIZ];

    // open the FIFO file
    int fifo;
    if ((fifo = open(fifo_fdes, O_RDONLY | O_NONBLOCK)) == -1)
    {
        perror("FILE: Can't Open the FIFO file\n");
        exit(10);
    }

    // Send end round signal to the teams to get info
    for (int i = 0; i < NUMBER_OF_TEAMS; i++)
    {
        kill(teams[i], SIGUSR2);
    }

    sleep(10); // wait

    // wait msg from each team
    while (read(fifo, msg_recv, sizeof(msg_recv)) > 0)
    {
        printf("Recv:\t%s\n", msg_recv);
        set_team_info(msg_recv);
    }

    // calc round's score function
    calc_score();

    // reset everything
    team_one_balls = 0;
    team_two_balls = 0;

    // increment number of rounds and set the round end flag
    current_round += 1;
    is_round_end = 1;
}

void send_ball(int team_sig)
{
    if (team_sig == SIGUSR1)
    {
        // send ball to team 1
        kill(teams[0], SIGUSR1);
    }
    else
    {
        // send ball to team 2
        kill(teams[1], SIGUSR1);
    }
}

void set_team_info(char *team_info)
{
    char *token;
    token = strtok(team_info, ",");
    if (atoi(token) == 1)
    {
        token = strtok(NULL, ",");
        team_one_balls = atoi(token);
    }
    else
    {
        token = strtok(NULL, ",");
        team_two_balls = atoi(token);
    }
}

void calc_score()
{
    if (team_one_balls > team_two_balls)
    {
        team_two_score += 1;
    }
    else if (team_one_balls < team_two_balls)
    {
        team_one_score += 1;
    }
    // Send to opengl scores, TODO
}

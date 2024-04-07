#include "header.h"
#include "constants.h"
#include "team.h"
#include "gamefuncs.h"
#include "player.h"

// globals
int is_game_running = 0;
int enable = 0;
// team and player object
struct TEAM our_team;
struct PLAYER me;

// fifo information
char PUBLIC_FIFO[30] = "/tmp/fifi";
char *fifo_fdes = "/tmp/fifi";

pid_t next_pid;
int *pidplayer; // array of pids for all players
pid_t next_pid; // next player (brother)

// OpenGL information
int n, privatefifo, publicfifo;
static char buffer[_PC_PIPE_BUF];
struct message msg;
char team_energies[100];
char player_energies[100];

// --------------------------------------------------------------------------
int main(int argc, char *argv[])
{

    // check arguments
    if (argc != 3)
    {
        perror("USAGE: Not Enough Arguments for Team Process\n");
        exit(7);
    }

    // get arguments
    our_team.teamnumber = atoi(argv[1]);
    our_team.number_of_players = atoi(argv[2]);
    our_team.teamlead_energy = get_energy_level(getpid());
    our_team.num_of_balls = 0; // initial value
    sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // important values
    pid_t pid_lead = getpid();
    pidplayer = malloc(sizeof(int) * our_team.number_of_players);
    printf("[TEAM NOTIFICATION]\nHello, I am the Team Lead [%d] of Team No.[%d] with Energy = %d\n\n", pid_lead, our_team.teamnumber, our_team.teamlead_energy);
    fflush(stdout);

    // Open the public FIFO for writing
    if ((publicfifo = open(PUBLIC, O_WRONLY)) == -1)
    {
        printf("Public error");
        perror(PUBLIC);
        exit(2);
    }

    // file settings for each team ( to store players pids )
    char filename[MAX_FILENAME_LENGTH];
    sprintf(filename, "teamplayer%d.txt", our_team.teamnumber);

    // Open the file
    file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }
    // -------------------------------- CREATE PLAYERS ------------------------------
    pid_t pid; // temp var

    // Create Players
    for (int i = 0; i < our_team.number_of_players; i++)
    {
        if ((pid = fork()) == -1)
        {
            perror("FORK: Can't create player\n");
            exit(99);
        }
        else if (pid == 0)
        {
            // child process - player

            // fill attributes
            me.teamnum = our_team.teamnumber;
            me.number = i + 1;
            me.energy = get_energy_level(getpid());

            // send player energies to opengl
            sprintf(msg.cmd_line, "x,%d,%d,%d", me.number, me.teamnum, me.energy);
            write(publicfifo, (char *)&msg, sizeof(msg));
            break; // break the child
        }
        else
        {
            // parent process - team (team lead)

            pidplayer[i] = pid; // save child pid

            // send team_leader energies to opengl
            sprintf(msg.cmd_line, "y,%d,%d", our_team.teamnumber, our_team.teamlead_energy);
            write(publicfifo, (char *)&msg, sizeof(msg));
        }
    }

    //-------------------- PARENT (TEAM) WORK------------------------------
    if (pid_lead == getpid())
    {
        // sensitive to signals from parent application (main.c) and children (players)
        if (sigset(SIGUSR1, recv_ball) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGUSR1\n");
            exit(30);
        }

        if (sigset(SIGUSR2, send_info) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGUSR1\n");
            exit(35);
        }
        if (sigset(SIGBUS, recv_ball_from_last_player) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGUSR1\n");
            exit(35);
        }
        if (sigset(SIGINT, recv_starting_ball) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGINT\n");
            exit(35);
        }

        // PARENT PROCESS - TEAM LEAD

        // save pid for all players in a shared file
        for (int i = 0; i < our_team.number_of_players; i++)
            fprintf(file, "%d\n", pidplayer[i]); // save pid of the child in the file
        fclose(file);

        while (1)
        {
            while (!is_round_end)
            {
                if (our_team.num_of_balls > 0)
                {
                    if (!is_game_running)
                    {
                        // Send to openGL that teamlead recv a ball
                        sprintf(msg.cmd_line, "t,%d,%d,r", our_team.teamnumber, our_team.teamlead_energy);
                        write(publicfifo, (char *)&msg, sizeof(msg));
                        wait_until_send_ball_team_leader(our_team.teamlead_energy);

                        enable = 0;
                        // Send ball to the 1st player
                        kill(pidplayer[0], SIGUSR2);
                        // printf("Ball Sent From Team Lead <%d> ===> ", our_team.teamnumber);
                        // fflush(stdout);

                        // send message to opengl about the teamlead that he sent the ball
                        sprintf(msg.cmd_line, "t,%d,%d,s", our_team.teamnumber, our_team.teamlead_energy);
                        write(publicfifo, (char *)&msg, sizeof(msg));
                    }
                    /*
                    WAIT SIGNALS
                    4 cases:
                    -  end round   (DONE)
                    -  ball back from player 5
                    -  recv ball from main.c when balls = 0
                    -  recv ball from another team
                    */

                    pause();

                    sleep(2); // wait until everything is ready
                }
            }
        }
    }

    // ------------------- CHILD (PLAYER) WORK -------------------------------------
    else
    {
        // CHILD PROCESS - PLAYER PROCESS

        sleep(3); // wait team lead until writing all pids in the file

        char buffer[100];   // store contents of the file
        pid_t family_id[5]; // pids for all brothers
        int i = 0;

        file = fopen(filename, "r");
        if (file == NULL)
        {
            perror("FILE ERROR: Can't open the file\n");
            exit(55);
        }
        // read the file
        while (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            // Convert string to integer
            family_id[i] = atoi(buffer);
            i++;
        }
        fclose(file);

        // get the next pid (next player (my brother) to send the ball)
        if (me.number != 5)
        {
            next_pid = family_id[me.number];
        }
        else
        {
            next_pid = getppid();
        }

        // sensitive to signals from the parent (team)
        if (sigset(SIGUSR2, play_with_ball) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGUSR2\n");
            exit(95);
        }

        if (sigset(SIGUSR1, round_begin) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGUSR1\n");
            exit(95);
        }

        if (sigset(SIGBUS, round_end) == SIG_ERR)
        {
            perror("SIGNAL: Can't sigset to SIGBUS\n");
            exit(95);
        }

        while (1)
        {
            pause();
        }
    }

    return (0);
}

// -------------------------------------------------------
// this function to recv the starting ball from the parent
void recv_starting_ball(int start_ball_sig)
{
    // send round starting signal for each player
    // GET READY ALL PLAYERS :D !!!!!!!!
    for (int i = 0; i < our_team.number_of_players; i++)
    {
        kill(pidplayer[i], SIGUSR1);
    }

    // set global flags
    is_game_running = 0;
    is_round_end = 0;
    our_team.num_of_balls += 1;
    sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // wait untill all players be Ready
    sleep(3);

    printf("[TEAM %d]: A Starting Ball Received\t[Total Balls = %d]\n", our_team.teamnumber, our_team.num_of_balls);
    fflush(stdout);
}

// -------------------------------------------------------

void recv_ball(int ball_sig)
{
    // Problem Here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // two simultaneously signals
    if (our_team.num_of_balls > 0)
    {
        is_game_running = 1;
    }
    if (enable)
    {
        is_game_running = 0;
    }
    our_team.num_of_balls += 1;
    printf("[TEAM %d]: A Ball Received\t[Total Balls = %d]\n", our_team.teamnumber, our_team.num_of_balls);
    fflush(stdout);
    sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // wait_until_send_ball(our_team.teamlead_energy);
}

// -------------------------------------------------------
void send_info(int end_rd_sig)
{
    // TODO:

    char msg_sent[BUFSIZ];
    sprintf(msg_sent, "%d,%d", our_team.teamnumber, our_team.num_of_balls);

    int out_fd = open(fifo_fdes, O_WRONLY);
    if (out_fd == -1)
    {
        perror("open error");
        exit(0);
    }

    if (write(out_fd, msg_sent, BUFSIZ) == -1)
    {
        perror("sent error");
        exit(0);
    }
    // for loop to sent sgin for each player round ended
    for (int i = 0; i < our_team.number_of_players; i++)
    {
        kill(pidplayer[i], SIGBUS);
    }

    is_round_end = 1;
    our_team.num_of_balls = 0;
    sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // send sign to players round is end
}

// -------------------------------------------------------

void play_with_ball(int send_ball_sig)
{

    // recv ball from previous player
    printf("Ball Received with Player <%d> Team[%d]\n", me.number, me.teamnum);
    fflush(stdout);
    sprintf(msg.cmd_line, "p,%d,%d,%d,r", me.number, me.teamnum, me.energy);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // sleep according to energy of the player
    wait_until_send_ball(me.energy);

    // send ball to the next player
    printf("Ball Sent From Player <%d> Team[%d] ===> ", me.number, me.teamnum);
    fflush(stdout);
    sprintf(msg.cmd_line, "p,%d,%d,%d,s", me.number, me.teamnum, me.energy);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // test if the round not end, then send the ball to my brother
    if (!is_round_end)
    {

        if (next_pid != getppid())
        {
            kill(next_pid, SIGUSR2);
        }
        else
        {
            kill(next_pid, SIGBUS);
            // send sign to parent
            // printf("round  ended\n");
            // fflush(stdout);
        }
    }
}

// -------------------------------------------------------

void recv_ball_from_last_player(int from_player_five_sig)
{

    // printf("Ball Back From Player 5 <team %d>\n", our_team.teamnumber);

    // Send to Open GL Recv ball from player 5
    sprintf(msg.cmd_line, "t,%d,%d,r", our_team.teamnumber, our_team.teamlead_energy);
    write(publicfifo, (char *)&msg, sizeof(msg));

    wait_until_send_ball_team_leader(our_team.teamlead_energy);
    is_game_running = 0;
    enable = 1;
    // send the ball to another team
    if (our_team.teamnumber == 1)
    {

        // send ball to team 2
        kill(getppid(), SIGUSR2);
        sprintf(msg.cmd_line, "t,%d,%d,s", our_team.teamnumber, our_team.teamlead_energy);
        write(publicfifo, (char *)&msg, sizeof(msg));

        // decrease number of balls
        our_team.num_of_balls -= 1;
        sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
        write(publicfifo, (char *)&msg, sizeof(msg));

        // check if our balls are zero, then request a new one
        if (our_team.num_of_balls == 0)
        {

            kill(getppid(), SIGUSR1);
        }
    }
    else
    {
        // send ball to team 1
        kill(getppid(), SIGUSR1);
        sprintf(msg.cmd_line, "t,%d,%d,s", our_team.teamnumber, our_team.teamlead_energy);
        write(publicfifo, (char *)&msg, sizeof(msg));

        // decrease number of balls
        our_team.num_of_balls -= 1;
        sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
        write(publicfifo, (char *)&msg, sizeof(msg));

        // check if our balls are zero, then request a new one
        if (our_team.num_of_balls == 0)
        {
            kill(getppid(), SIGUSR2);
        }
    }

    sprintf(msg.cmd_line, "b,%d,%d", our_team.teamnumber, our_team.num_of_balls);
    write(publicfifo, (char *)&msg, sizeof(msg));
    sleep(2);
}

// -------------------------------------------------------

void wait_until_send_ball_team_leader(int energy)
{
    float time_to_sleep;

    // add tome for lost ball
    if (our_team.teamlead_energy > 95)
    {
        time_to_sleep = 1; // ideal waiting time
    }
    else
    {
        time_to_sleep = (100.0 / our_team.teamlead_energy) * 3;
    }
    our_team.teamlead_energy -= 3;
    // sleep

    sleep(time_to_sleep);
}

// ----------------------------------------------------
void wait_until_send_ball(int energy)
{
    float time_to_sleep;
    float drop_probability;
    // calculate the drop probabilty
    srand(getpid());
    // relation
    // drop X energy
    drop_probability = (rand() % 95);

    if (drop_probability > energy)
    {
        // Send to OpenGL that the player lost the ball
        sprintf(msg.cmd_line, "l,%d,%d,%d,s", me.number, me.teamnum, me.energy);
        write(publicfifo, (char *)&msg, sizeof(msg));
        sleep(3);
    }

    // Send to OpenGL to recollect the ball
    sprintf(msg.cmd_line, "p,%d,%d,%d,r", me.number, me.teamnum, me.energy);
    write(publicfifo, (char *)&msg, sizeof(msg));

    // calculate the sleep time according to energy
    if (me.energy > 95)
    {
        time_to_sleep = 1; // ideal waiting time
    }
    else
    {
        time_to_sleep = (100.0 / energy) * 3;
    }

    me.energy -= 5;

    sleep(time_to_sleep);
}

// -------------------------------------------------------

// -------------------------------------------------------

void round_begin(int rd_begin_sig)
{
    is_round_end = 0;
}

// -------------------------------------------------------
void round_end(int rd_end_sig)
{
    is_round_end = 1;
}
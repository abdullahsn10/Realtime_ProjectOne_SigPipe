#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include "player.h"
#include "team.h"
#include "header.h"
#include "constants.h"
// Global variables for the energy levels
int team1Energy = 100;
int team2Energy = 100;

// Global variable for the timer
int elapsedTime = 0;
int timerInterval = 100; // Timer interval in milliseconds

float team_leaders[2][3] = {1.0, 0, 0}; // initially RED
float players[10][3] = {1.0, 0, 0};     // initially RED
int team_leaders_energy[2];
int players_energy[10];
char *fifo_fdes = "/tmp/fifi1";
int n, done, dummyfifo, publicfifo, privatefifo;
// FILE *fin;
static char buffer[_PC_PIPE_BUF];
int breakL = 1;

int round_time;
int round_number;
int team_A_Balls;
int team_B_Balls;

int team_A_Score = 0;
int team_B_Score = 0;

char round_time_String[100];
char round_number_String[100];
char team_A_Balls_String[100];
char team_B_Balls_String[100];
char end_round_String[100];

void init()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);

    for (int i = 0; i < 10; i++)
    {
        players[i][0] = 1.0;
        players[i][1] = 0.0;
        players[i][2] = 0.0;
    }
    for (int i = 0; i < 2; i++)
    {
        team_leaders[i][0] = 1.0;
        team_leaders[i][1] = 0.0;
        team_leaders[i][2] = 0.0;
    }
    for (int i = 0; i < 2; i++)
    {
        team_leaders_energy[i] = 0;
    }
    for (int i = 0; i < 10; i++)
    {
        players_energy[i] = 0;
    }

    int pid;

    if ((pid = fork()) == -1)
    {
        perror("error in fork\n");
        exit(-1);
    }
    else if (pid == 0)
    {
        execlp("./main", "main", "20", "2", NULL);
        perror("Error in exec\n");
        exit(-1);
    }
    else
    {
        FILE *pf;
        char command[20];
        char data[512];
        sprintf(command, "rm /tmp/PUBLIC");
        pf = popen(command, "r");
        fgets(data, 512, pf);
        printf("first\n");
        if ((mknod(PUBLIC, __S_IFIFO | 0666, 0)) == -1)
        {
            perror("Error");
            exit(-1);
        }
        printf("second\n");
        if ((publicfifo = open(PUBLIC, O_RDONLY)) == -1)
        {

            perror(PUBLIC);
            exit(1);
        }

        printf("third\n");

        /*else if( firstLetter == 'e'){
            break;
        }*/
    }
}

void drawCircle(float x, float y, float radius, int energy, int type, int number)
{
    // Draw container
    glBegin(GL_QUADS);
    glVertex2f(x - radius * 1.5, y - radius * 1.5);
    glVertex2f(x - radius * 1.5, y + radius * 1.5);
    glVertex2f(x + radius * 1.5, y + radius * 1.5);
    glVertex2f(x + radius * 1.5, y - radius * 1.5);
    glEnd();

    // Draw filled container based on energy level
    glColor3f(0.5, 0.5, 0.5); // Grey color

    glBegin(GL_QUADS);
    double firstEq = radius * 1.5 * energy / 100;

    glVertex2f(x - radius * 1.5, y - radius * 1.5);
    glVertex2f(x - radius * 1.5, y + radius * 1.5 * energy / 100);
    glVertex2f(x + radius * 1.5, y + radius * 1.5 * energy / 100);
    glVertex2f(x + radius * 1.5, y - radius * 1.5);
    glEnd();

    // Draw player circle
    // glColor3f(0.0 , 0.0, 0.0); // Orange color
    if (type == 0)
    {
        // glColor3f(1.0 , 0.0 , 0.0); // red color
        if (number == 1)
        {
            glColor3fv(team_leaders[0]);
        }
        else if (number == 2)
        {
            glColor3fv(team_leaders[1]);
        }
    }
    else
    {
        //        glColor3f(1.0 , 1.0 , 0.0); // yellow Color

        glColor3fv(players[number]);
    }
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++)
    {
        glVertex2f(x + (radius * cos(i * 3.14159 / 180)), y + (radius * sin(i * 3.14159 / 180)));
    }
    glEnd();

    // Draw energy value
    glColor3f(0.0, 0.0, 1.0); // blue color
    char energyStr[10];
    sprintf(energyStr, "%d", energy);
    glRasterPos2f(x - 10, y - 5);
    for (int i = 0; energyStr[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, energyStr[i]);
    }

    // Draw guide squares and text
    // Red square and text
    glColor3f(1.0, 0.0, 0.0); // Red color
    glBegin(GL_QUADS);
    glVertex2f(50, 50);
    glVertex2f(70, 50);
    glVertex2f(70, 70);
    glVertex2f(50, 70);
    glEnd();
    glColor3f(0.0, 0.0, 0.0); // Black color
    glRasterPos2f(80, 60);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char *)"Player without ball");

    // Green square and text
    glColor3f(0.0, 1.0, 0.0); // Green color
    glBegin(GL_QUADS);
    glVertex2f(240, 50);
    glVertex2f(260, 50);
    glVertex2f(260, 70);
    glVertex2f(240, 70);
    glEnd();
    glColor3f(0.0, 0.0, 0.0); // Black color
    glRasterPos2f(270, 60);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char *)"Player with ball");

    // Yellow square and text
    glColor3f(1.0, 1.0, 0.0); // Orange color
    glBegin(GL_QUADS);
    glVertex2f(420, 50);
    glVertex2f(440, 50);
    glVertex2f(440, 70);
    glVertex2f(420, 70);
    glEnd();
    glColor3f(0.0, 0.0, 0.0); // Black color
    glRasterPos2f(460, 60);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char *)"Player miss the ball");
    glFlush();
}

void drawLine(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void display()
{
    struct message msg;
    int count = 0;

    char teamA_balls[80];
    char teamB_balls[80];
    float sleep_time;
    int end_sleep = 1;

    if (end_sleep == 1)
    {
        while (read(publicfifo, (char *)&msg, sizeof(msg)) > 0)
        {
            char message[100];
            strcpy(message, msg.cmd_line);
            printf("%s\n", msg.cmd_line);
            char firstLetter = msg.cmd_line[0];

            char *allString;
            allString = strtok(msg.cmd_line, ",");
            char letter = allString[0];

            if (firstLetter == 'x')
            {
                // player(number, team , energy)
                allString = strtok(NULL, ",");

                int player_number = atoi(allString);
                allString = strtok(NULL, ",");

                int team_number = atoi(allString);
                allString = strtok(NULL, ",");
                int player_energy = atoi(allString);

                if (team_number == 1)
                {
                    players_energy[player_number - 1] = player_energy;
                }
                else if (team_number == 2)
                {
                    players_energy[player_number + 5 - 1] = player_energy;
                    if (player_number == 5)
                    {
                        break;
                    }
                }

                count++;
            }
            else if (firstLetter == 'y')
            {

                // team lead(team num , energy)
                allString = strtok(NULL, ",");

                int team_number = atoi(allString);

                allString = strtok(NULL, ",");
                int team_energy = atoi(allString);
                team_leaders_energy[team_number - 1] = team_energy;

                count++;
                printf("My ValueTeam: %d,%d\n", team_number, team_energy);
            }
            else if (firstLetter == 't')
            {
                int number_of_balls;
                int team_number;

                allString = strtok(NULL, ",");
                // team number

                team_number = atoi(allString);

                // team energy
                allString = strtok(NULL, ",");
                int team_energy = atoi(allString);

                allString = strtok(NULL, ",");
                char state = allString[0];

                team_leaders_energy[team_number - 1] = team_energy;
                if (state == 's')
                {

                    team_leaders[team_number - 1][0] = 1.0;
                    team_leaders[team_number - 1][1] = 0.0;
                    team_leaders[team_number - 1][2] = 0.0;
                }
                else if (state == 'r')
                {
                    printf("team rec\n");
                    team_leaders[team_number - 1][0] = 0.0;
                    team_leaders[team_number - 1][1] = 1.0;
                    team_leaders[team_number - 1][2] = 0.0;
                    /*
                     if (team_energy > 95)
                          {
                              sleep_time = 1; // ideal waiting time
                          }
                          else
                          {
                              sleep_time = (100.0 / team_energy) * 3;
                          }
                          end_sleep= 0;

                          sleep(sleep_time);

                          end_sleep = 1;
                          */
                }
                else
                {
                    printf("EROOR---------------------------------------n\n");
                }

                /*

               team_leaders[team_number-1][0] = 0.0;
               team_leaders[team_number-1][1] = 1.0;
               team_leaders[team_number-1][2] = 0.0;

               if(team_number == 1){


               }
               else{


                   players[5][0] =  0.0;
                   players[5][1] =  1.0;
                   players[5][2] = 0.0;
               }
               sleep(2);

               team_leaders[team_number-1][0] = 1.0;
               team_leaders[team_number-1][1] = 0.0;
               team_leaders[team_number-1][2] = 0.0;
           */
            }

            else if (firstLetter == 'p')
            {

                allString = strtok(message, ",");

                int player_number, team_number, energy_number;

                allString = strtok(NULL, ",");

                player_number = atoi(allString);

                allString = strtok(NULL, ",");

                team_number = atoi(allString);

                allString = strtok(NULL, ",");

                energy_number = atoi(allString);

                allString = strtok(NULL, ",");

                char state = allString[0];

                printf("MyPLayerValues: %d,%d,%d,%c\n", player_number, team_number, energy_number, state);
                /*
                if(state != 's' && state != 'r'){
                    state = msg.cmd_line[10];
                }
                */

                if (team_number == 1)
                {
                    players_energy[player_number - 1] = energy_number;
                    if (state == 's')
                    {
                        /*
                         if (energy_number > 95)
                        {
                            sleep_time = 1; // ideal waiting time
                        }
                        else
                        {
                            sleep_time = (100.0 / energy_number) * 3;
                        }
                        end_sleep = 0;
                        sleep(sleep_time);
                        end_sleep = 1;
                        */
                        players[player_number - 1][0] = 1.0;
                        players[player_number - 1][1] = 0.0;
                        players[player_number - 1][2] = 0.0;
                    }
                    else if (state == 'r')
                    {

                        players[player_number - 1][0] = 0.0;
                        players[player_number - 1][1] = 1.0;
                        players[player_number - 1][2] = 0.0;
                    }
                }

                else if (team_number == 2)
                {
                    players_energy[player_number + 5 - 1] = energy_number;
                    if (state == 's')
                    {
                        players[player_number + 5 - 1][0] = 1.0;
                        players[player_number + 5 - 1][1] = 0.0;
                        players[player_number + 5 - 1][2] = 0.0;
                    }
                    else if (state == 'r')
                    {
                        players[player_number + 5 - 1][0] = 0.0;
                        players[player_number + 5 - 1][1] = 1.0;
                        players[player_number + 5 - 1][2] = 0.0;
                        /*
                         if (energy_number > 95)
                            {
                                sleep_time = 1; // ideal waiting time
                            }
                            else
                            {
                                sleep_time = (100.0 / energy_number) * 3;
                            }
                            end_sleep = 0;

                            sleep(sleep_time);

                            end_sleep = 1;*/
                    }
                }
                printf("End P\n");
            }
            else if (firstLetter == 'l')
            {
                allString = strtok(message, ",");

                int player_number, team_number, energy_number;

                allString = strtok(NULL, ",");

                player_number = atoi(allString);

                allString = strtok(NULL, ",");

                team_number = atoi(allString);

                allString = strtok(NULL, ",");

                energy_number = atoi(allString);

                allString = strtok(NULL, ",");

                char state = allString[0];

                if (team_number == 1)
                {
                    players_energy[player_number - 1] = energy_number;
                    players[player_number - 1][0] = 1.0;
                    players[player_number - 1][1] = 1.0;
                    players[player_number - 1][2] = 0.0;
                }

                else if (team_number == 2)
                {
                    players_energy[player_number + 5 - 1] = energy_number;
                    players[player_number + 5 - 1][0] = 1.0;
                    players[player_number + 5 - 1][1] = 1.0;
                    players[player_number + 5 - 1][2] = 0.0;
                }
                /*
                end_sleep = 0;
                sleep(2);
                end_sleep = 1;
                */
            }
            else if (firstLetter == 'k')
            {

                int team_number;

                allString = strtok(NULL, ",");
                // team number

                team_number = atoi(allString);

                // team energy
                allString = strtok(NULL, ",");
                int team_energy = atoi(allString);

                team_leaders[team_number - 1][0] = 1.0;
                team_leaders[team_number - 1][1] = 1.0;
                team_leaders[team_number - 1][2] = 0.0;
            }
            else if (firstLetter == 'R')
            {
                /*
                allString = strtok(NULL , ",");
                round_number = atoi(allString);

                allString = strtok(NULL , ",");
                int balls1=  atoi(allString);
                allString = strtok(NULL , ",");
                int balls2= atoi(allString);

                team_A_Balls = balls1;
                team_B_Balls = balls2;
                */
                allString = strtok(NULL, ",");
                round_number = atoi(allString);
            }
            else if (firstLetter == 'b')
            {
                allString = strtok(NULL, ",");
                int team_number = atoi(allString);
                allString = strtok(NULL, ",");
                int number_of_balls;
                number_of_balls = atoi(allString);
                if (team_number == 1)
                {
                    team_A_Balls = number_of_balls;
                }
                else if (team_number == 2)
                {
                    team_B_Balls = number_of_balls;
                }
            }
            else if (firstLetter == 'F')
            {
                allString = strtok(NULL, ",");
                round_number = atoi(allString);
                allString = strtok(NULL, ",");
                team_A_Score = atoi(allString);
                allString = strtok(NULL, ",");
                team_B_Score = atoi(allString);

                sprintf(end_round_String, "Round %d Finished", round_number);
            }
            else
            {
                break;
            }
            break;
        }
        for (int i = 0; i < 2; i++)
        {
            if (team_leaders_energy[i] == 0)
            {
                breakL = 0;
            }
        }

        for (int i = 0; i < 10; i++)
        {
            if (players_energy[i] == 0)
            {
                breakL = 0;
            }
        }
        if (breakL == 1)
        {
            // break;
        }
        else
        {
            breakL = 1;
        }
    }

    if (elapsedTime == 10)
    {
        get_ball_leader(0);
    }
    if (elapsedTime == 20)
    {
        get_ball_leader(1);
    }
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw team 1 leader

    drawCircle(200, 500, 20, team_leaders_energy[0], 0, 1);

    // Draw team 1 players
    for (int i = 0; i < 5; i++)
    {

        drawCircle(150 + i * 50, 400, 15, players_energy[i], 1, i);
    }

    // Draw team 2 leader

    drawCircle(600, 500, 20, team_leaders_energy[1], 0, 2);

    // Draw team 2 players
    for (int i = 0; i < 5; i++)
    {

        drawCircle(550 + i * 50, 400, 15, players_energy[i + 5], 1, i + 5);
    }

    // Draw line between teams
    glColor3f(0.0, 0.0, 0.0); // Black color
    drawLine(400, 600, 400, 300);

    // Draw timer
    char timerStr[10];
    sprintf(timerStr, "Time: %d", elapsedTime);
    glColor3f(0.0, 0.0, 0.0); // Black color
    glRasterPos2f(700, 550);
    for (int i = 0; timerStr[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, timerStr[i]);
    }

    sprintf(round_number_String, "Round: %d", round_number);
    glColor3f(0.0, 0.0, 0.0); // Black color
    glRasterPos2f(390, 290);
    for (int i = 0; round_number_String[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, round_number_String[i]);
    }

    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2f(150, 300);
    sprintf(team_A_Balls_String, "#ofTeamA_Balls:%d", team_A_Balls);
    for (int i = 0; team_A_Balls_String[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, team_A_Balls_String[i]);
    }

    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2f(550, 300);
    sprintf(team_B_Balls_String, "#ofTeamB_Balls:%d", team_B_Balls);
    for (int i = 0; team_B_Balls_String[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, team_B_Balls_String[i]);
    }

    if (team_A_Score != 0 || team_B_Score != 0)
    {
        glColor3f(0.0, 0.0, 0.0);
        glRasterPos2f(550, 200);
        char team_B_Score_String[100];
        sprintf(team_B_Score_String, "Team_BScore:%d", team_B_Score);
        for (int i = 0; team_B_Balls_String[i] != '\0'; i++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, team_B_Score_String[i]);
        }

        glColor3f(0.0, 0.0, 0.0);
        glRasterPos2f(150, 200);
        char team_A_Score_String[100];
        sprintf(team_A_Score_String, "Team_AScore:%d", team_A_Score);
        for (int i = 0; team_A_Score_String[i] != '\0'; i++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, team_A_Score_String[i]);
        }
    }

    glFlush();
}

void update(int value)
{
    // Update energy levels
    team1Energy -= 5;
    team2Energy -= 5;

    // Update elapsed time
    elapsedTime += 1;

    /*
     if(elapsedTime> 4){
    team_leaders[0][0] = 1.0;
    team_leaders[0][1] = 1.0;
    team_leaders[0][2] = 1.0;
    }
    */
    struct PLAYER p;
    struct TEAM t;

    // Redraw the display
    glutPostRedisplay();

    // Set up the next timer
    // glutTimerFunc(timerInterval, update, 0);
}

void get_ball_leader(int team)
{

    team_leaders[team][0] = 0.0;
    team_leaders[team][1] = 1.0;
    team_leaders[team][2] = 0.0;

    // Redraw the display
    glutPostRedisplay();
}
void send_info(int end_rd_sig)
{
    // TODO:

    char msg_sent[BUFSIZ];
    // sprintf(msg_sent, "%d,%d", our_team.teamnumber, our_team.num_of_balls);

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
}
void readFromFifo()
{
    /*FILE *fifo;
    char buffer[BUFSIZ];

    fifo = fopen(fifo_fdes, "r");
    if (!fifo) {
        perror("Error opening FIFO for reading");
        exit(EXIT_FAILURE);
    }
*/

    /*
      char msg_recv[BUFSIZ];
        // open the FIFO file
        int f;
 if ((f = open(fifo_fdes, O_RDONLY)) == -1) {
        perror("ERROR OPEN");
        exit(EXIT_FAILURE);
    }

    // wait msg from each team
    if(read(f, msg_recv , sizeof(msg_recv) > 0)){
         printf("Recv:\t%s\n",msg_recv);
    }
    */

    /*
   while(read(f, msg_recv, sizeof(msg_recv)) > 0){
      printf("Recv:\t%s\n",msg_recv);
   }
   */
    /*
    struct message msg;
     while(read(publicfifo, (char*) &msg ,sizeof(msg) ) >0){
          printf("myMessage:%s\n" , msg.cmd_line);
          char first_lettter;
          first_lettter = msg.cmd_line[0];
          if(first_lettter == 't'){


               int team_number;
              char number[5] = ""; // Initialize the string
              number[0] = msg.cmd_line[1]; // Assign the character directly from the string
              number[1] = '\0'; // Terminate the string with null character
              printf("n:%s\n", number);
              team_number = atoi(number);
              char energy_num[10];
              energy_num[0] = msg.cmd_line[4];
              energy_num[1] = msg.cmd_line[5];
              energy_num[2] = '\0';
              int team_energy = atoi(number);

              char state = msg.cmd_line[6];
              if(state != 's' && state != 'r'){
                  state = msg.cmd_line[7];
              }

              printf("before\n");


              team_leaders_energy[team_number-1] = team_energy;


                   if(state == 's'){

                      team_leaders[team_number-1][0] = 1.0;
                      team_leaders[team_number-1][1] = 0.0;
                      team_leaders[team_number-1][2] = 0.0;

                  }
                  else if(state == 'r'){
                      printf("team rec\n");
                        team_leaders[team_number-1][0] = 0.0;
                        team_leaders[team_number-1][1] = 1.0;
                        team_leaders[team_number-1][2] = 0.0;
                         sleep(2);
                  }
                  else{
                      printf("EROOR---------------------------------------n\n");

                  }
              printf("n:%d\n" , team_number);
               printf("Thisis\n");
               /*

              team_leaders[team_number-1][0] = 0.0;
              team_leaders[team_number-1][1] = 1.0;
              team_leaders[team_number-1][2] = 0.0;

              if(team_number == 1){


              }
              else{


                  players[5][0] =  0.0;
                  players[5][1] =  1.0;
                  players[5][2] = 0.0;
              }
              sleep(2);

              team_leaders[team_number-1][0] = 1.0;
              team_leaders[team_number-1][1] = 0.0;
              team_leaders[team_number-1][2] = 0.0;



          }

          else if(first_lettter == 'p'){
              int player_number , team_number , energy_number;
              char play_num[5] = "";
              char team_num[5] = "";
              char energy_num[5] = "";
              char state ;
              play_num[0] = msg.cmd_line[1];
              play_num[1] = '\0';

              team_num[0] = msg.cmd_line[4];
              team_num[1] = '\0';

              energy_num[0] = msg.cmd_line[7];
              energy_num[1] = msg.cmd_line[8];
              energy_num[2] = '\0';

              player_number = atoi(play_num);
              team_number = atoi(team_num);
              energy_number = atoi(energy_num);

              state = msg.cmd_line[9];
              if(state != 's' && state != 'r'){
                  state = msg.cmd_line[10];
              }


              if(team_number == 1){
                  players_energy[player_number-1] = energy_number;
                  if(state == 's'){
                  players[player_number-1][0] =  1.0;
                  players[player_number-1][1] =  0.0;
                  players[player_number-1][2] = 0.0;

                  }
                  else if(state == 'r'){

                  players[player_number][0] =  0.0;
                  players[player_number][1] =  1.0;
                  players[player_number][2] = 0.0;
                   sleep(2);
                  }


              }

              else{
                   players_energy[player_number+5-1] = energy_number;
                  if(state == 's'){
                       players[player_number+5-1][0] =  1.0;
                  players[player_number+5-1][1] =  0.0;
                  players[player_number+5-1][2] = 0.0;

                  }
                  else if(state == 'r'){
                      players[player_number+5][0] =  0.0;
                  players[player_number+5][1] =  1.0;
                  players[player_number+5][2] = 0.0;
                   sleep(2);
                  }






              }
               break;

          }
          */
    /*
    char firstLetter = msg.cmd_line[0];
    printf("%s\n", msg.cmd_line);
    if(firstLetter == 'x'){
        //player(number, team , energy)
        char team_num[10],p_num[10] , p_energy[10];

        team_num[0] = msg.cmd_line[3];
        team_num[1] = '\0';
        int team_number = atoi(team_num);
        p_num[0] = msg.cmd_line[1];
        p_num[1] = '\0';
        int player_number = atoi(p_num);
        p_energy[0] = msg.cmd_line[5];
        p_energy[1] = msg.cmd_line[6];
        p_energy[2] = '\0';
        int player_energy = atoi(p_energy);


        if(team_number == 1){
            players_energy[player_number- 1] = player_energy;
        }
        else if (team_number == 2){
            players_energy[player_number+5-1] = player_energy;
            if(player_number == 5){
                break;
            }
        }

     //count ++;
     //continue;

    }else if(firstLetter == 'y'){

        //team lead(team num , energy)
        char team_num[10], t_energy[10];
        team_num[0] = msg.cmd_line[1];
        team_num[1] = '\0';
        int team_number = atoi(team_num);

        t_energy[0] = msg.cmd_line[5];
        t_energy[1] = msg.cmd_line[6];
        t_energy[2] = '\0';
        int team_energy = atoi(t_energy);
        team_leaders_energy[team_number-1] = team_energy;

        //count++;
        //continue;
    }*/
    // Redraw the display
    /*
   }
   */
    glutPostRedisplay();
    // Set up the next timer
    glutTimerFunc(100, readFromFifo, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Beach Ball Game");
    init();

    glutDisplayFunc(display);

    // Set up the timer
    glutTimerFunc(timerInterval, readFromFifo, 0);

    glutMainLoop();
    return 0;
}

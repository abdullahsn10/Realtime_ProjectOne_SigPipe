#ifndef __TEAM_H_
#define __TEAM_H_

#include "header.h"
#define MAX_FILENAME_LENGTH 100
FILE *file;

int is_round_end = 0; // ending round flag

struct TEAM
{
    int teamnumber;
    int number_of_players;
    int teamlead_energy;
    int num_of_balls;
} team;

void recv_ball(int ball_sig);
void send_info(int end_rd_sig);
void play_with_ball(int send_ball_sig);
void recv_ball_from_last_player(int from_player_five_sig);
void wait_until_send_ball(int energy);
void recv_starting_ball(int start_ball_sig);
void round_begin(int rd_begin_sig);
void round_end(int rd_end_sig);

#endif
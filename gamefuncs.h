#ifndef __GAMEFUNCS_H_
#define __GAMEFUNCS_H_
/* Prototype for Beach Ball Game functions */
#include "header.h"
#include "constants.h"

void end_round(int sig);
void send_ball(int team_sig);



int get_energy_level(int pid){
    srand(pid);

    // return energy level from MIN to MAX(excluded)
    return MIN_LEVEL_ENERGY + (rand() % (MAX_LEVEL_ENERGY - MIN_LEVEL_ENERGY + 1));
}








#endif
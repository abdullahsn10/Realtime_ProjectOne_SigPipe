// TEAM TODO:
// Team Process will stuck here
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

    while (1)
    {
        if (!is_round_end)
        {
            // START ROUND
            if (our_team.num_of_balls > 0)
            {
                int j = 0;
                // SEND MSG FOR CATCHING BALL TO EACH PLAYER
                for (; j < our_team.number_of_players; j++)
                {

                    sprintf(message_id, "%d", pidplayer[j]); // player id
                    if (write(f_des[1], message_id, BUFSIZ) == -1)
                    {
                        perror("WRITE ERROR: Can't write on the pipe\n");
                        exit(85);
                    }
                    else
                    {
                        if (j == 0)
                            printf("Ball Sent From <Team Lead %d> =====> ", our_team.teamnumber);
                        else
                            printf("Ball Sent From <Player %d> Team[%d] =====> ", (j), our_team.teamnumber);
                    }

                    // wait until the player send to another player
                    // blocking (read)
                    if (read(f_des[0], message_two, BUFSIZ) != -1)
                    {
                        if (atoi(message_two) == atoi(message_id))
                        {
                            printf("Ball with <Player %d> Team[%d]\n", (j + 1), our_team.teamnumber);
                        }
                    }
                    else
                    {
                        perror("READ ERROR: Can't read pipe\n");
                    }
                }
                printf("Ball Sent From <Player %d> Team[%d] =====> <Team Lead>\n", j, our_team.teamnumber);
                our_team.num_of_balls -= 1;
                printf("Ball Sent From <Team Lead %d> =====> Another team\n", our_team.teamnumber);
            }

            // END ROUND
        }
        pause();
    }
}
// PLAYER TODO:
else
{
    // Player Process
    while (1)
    {
        // read msg recv from the team lead
        if (read(f_des[0], message_id, BUFSIZ) != -1)
        {
            // check if its turn to catch the ball
            if (atoi(message_id) == getpid())
            {
                // catch the ball

                // TODO: Wait with energy

                // notify teamlead that I sent the ball to next player
                if (write(f_des[1], message_id, BUFSIZ) != -1)
                {
                }
                else
                {
                    printf("write ERROR\n");
                }
            }
        }
        else
        {
            perror("READ ERROR: Player Can't Read Fifo\n");
        }
    }
}
return (0);
}
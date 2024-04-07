while (1)
{
    // START ROUND FROM TEAM
    // conditions: num of balls, ending round
    if (our_team.num_of_balls > 0)
    {
        is_round_end = 0;
        // Send ball to the 1st player
        if (is_ball_recvd_while_playing == 0)
        {
            kill(pidplayer[0], SIGUSR2);
            printf("Ball Sent From Team Lead <%d> ===> ", our_team.teamnumber);
            fflush(stdout);
        }
        /*
        WAIT SIGNALS
        4 cases:
        - x end round
        - x ball back from player 5
        - X recv ball from main.c when balls = 0
        - X recv ball from another team
        */
        pause();

        // send the ball to the other team and request ball if no ball with me
        if (!enable && !is_round_end)
        {
            our_team.num_of_balls -= 1;
            // falg to avoid sent kill ;
            if (our_team.teamnumber == 1)
            {
                kill(getppid(), SIGUSR2);
                if (our_team.num_of_balls == 0)
                {
                    kill(getppid(), SIGUSR1);
                    // slepp 1s
                }
            }
            else
            {
                kill(getppid(), SIGUSR1);
                if (our_team.num_of_balls == 0)
                {
                    kill(getppid(), SIGUSR2);
                }
            }
        }
    }
}
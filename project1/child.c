// RealTime Project 1 ... Signals and Pipes
// Hana Kafri 1190084 . Sec.1
// Amany Khdair 1190728. Sec.2
// Maryam Altawil 1192099. Sec.1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define NUM_TEAMS 2
#define NUM_PLAYERS_PER_TEAM 2
#define NUM_PLAYERS (NUM_TEAMS * NUM_PLAYERS_PER_TEAM)
#define PLAYERS 5

#define BUF_SIZE 64
#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

volatile sig_atomic_t should_stop = 0;

void handle_signal(int signal)
{
    should_stop = 1;
}

int main(int argc, char *argv[])
{

    if (argc >= 3)
    {
        int pipe[NUM_PLAYERS][2];
        int childN = (int)strtol(argv[1], (char **)NULL, 10);
        int pid = (int)strtol(argv[2], (char **)NULL, 10);
        int child_pipes[PLAYERS][2];
        int co_processor_pipe[2];

        child_pipes[childN][PIPE_READ_END] = (int)strtol(argv[3], (char **)NULL, 10);
        child_pipes[childN][PIPE_WRITE_END] = (int)strtol(argv[4], (char **)NULL, 10);

        if (childN < 4)
        { // first 4 children (0-3)

            FILE *range, *randv;

            int min = 0, max = 50; // default
            char c;

            // create a file with the pid as the name
            char filename[64];
            sprintf(filename, "%d.txt", pid); // getpid() or pid

            // read the range and save it in min and max
            range = fopen("range.txt", "r");
            randv = fopen(filename, "w"); // make the filename = child's pid = argv[1]

            if (randv == NULL)
            {
                perror("Could not open output file for writing");
                exit(EXIT_FAILURE);
            }
            if (range == NULL)
            {
                printf("error \n");
                return 1;
            }

            fscanf(range, "%d %c %d", &min, &c, &max); // to read the range (min and max) from the range file

            // to get a different random value each time
            srand(pid); // getpid or pid
            float value = (float)(rand() % (max - min + 1) + min) + (float)rand() / RAND_MAX;

            fprintf(randv, "%f", value); // print rand value on the file with name=pid

            fclose(range);
            fclose(randv);

            // Inform the parent process that the value is ready to be picked through the pipe, and exit
            close(child_pipes[childN][PIPE_READ_END]);
            write(child_pipes[childN][PIPE_WRITE_END], "READY", 6);
            close(child_pipes[childN][PIPE_WRITE_END]);

            exit(EXIT_SUCCESS);
        }
        else if (childN == 4)
        { // CHILD 5

            child_pipes[childN][PIPE_READ_END] = (int)strtol(argv[3], (char **)NULL, 10);
            child_pipes[childN][PIPE_WRITE_END] = (int)strtol(argv[4], (char **)NULL, 10);

            co_processor_pipe[PIPE_READ_END] = child_pipes[childN][PIPE_READ_END];
            co_processor_pipe[PIPE_WRITE_END] = child_pipes[childN][PIPE_WRITE_END];

            // Read the values from the parent process
            char buf[BUF_SIZE];

            write(co_processor_pipe[PIPE_WRITE_END], "READY", 6);

            read(co_processor_pipe[PIPE_READ_END], buf, BUF_SIZE);

            // Calculate the sums and send them back to the parent process
            float values[4];
            sscanf(buf, "%f,%f,%f,%f", &values[0], &values[1], &values[2], &values[3]);

            float sum1 = values[0] + values[1];
            float sum2 = values[2] + values[3];
            char response[BUF_SIZE];

            sprintf(response, "%f,%f", sum1, sum2);

            write(co_processor_pipe[PIPE_WRITE_END], response, strlen(response) + 1);

            // close unused pipes
            close(co_processor_pipe[PIPE_WRITE_END]);
            close(co_processor_pipe[PIPE_READ_END]);

            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        perror("arguments error");
    }

    return 0;
}

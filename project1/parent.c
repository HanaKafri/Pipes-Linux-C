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
#include <GL/glut.h>
#include <string.h>

#define PLAYERS 5
#define NUM_PLAYERS 4 // Num Of Teams * Num of players Per Team
#define BUF_SIZE 64
#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

int print_final_winner = 0;
int print_team1_score = 0;
int print_team2_score = 0;

void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 0.1, 100.0);
}

void backGround()
{ // Draw background which is green and make it 3D
    glBegin(GL_QUADS);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(-10.0, 0.0, -10.0);
    glVertex3f(-10.0, 0.0, 10.0);
    glVertex3f(10.0, 0.0, 10.0);
    glVertex3f(10.0, 0.0, -10.0);
    glEnd();
}

void Draw_Team1(float x, float z)
{ // Draw team 1 with color red
    glPushMatrix();
    glTranslatef(x, 0.0, z);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidSphere(0.5, 50, 50);
    glPopMatrix();
}

void Draw_Team2(float x, float z)
{ // Draw team 2 with color pink
    glPushMatrix();
    glTranslatef(x, 0.0, z);
    glColor3f(1.0, 0.0, 1.0);
    glutSolidSphere(0.5, 50, 50);
    glPopMatrix();
}

void Draw_Child5()
{ // Draw child 5 with color white
    glPushMatrix();
    glRotatef(0.0, 0.0, 1.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);
    glutSolidSphere(0.3, 50, 50);
    glPopMatrix();
}

void Draw_print_winner(float x, float z)
{ // print the winner team number

    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(x, z); // read its position
    char st[64];
    sprintf(st, "The winner team is %d", print_final_winner); // print the result
    int Length = strlen(st);

    for (int i = 0; i < Length + 1; i++)
    { // print the result via character per character
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st[i]);
    }

    glFlush();
}

void team1Wins(float x, float z)
{
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos2f(x, z); // read its position
    char st[64];
    sprintf(st, "Score of Team 1 = %d", print_team1_score); // print the score of team 1
    int Length = strlen(st);

    for (int i = 0; i < Length + 1; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st[i]); // print the result via character per character
    }
    glFlush();
}

void team2Wins(float x, float z)
{
    glColor3f(1.0, 0.0, 1.0);
    glRasterPos2f(x, z); // read its position
    char st[64];
    sprintf(st, "Score of Team 2 = %d", print_team2_score); // print the score of team 2
    int Length = strlen(st);

    for (int i = 0; i < Length + 1; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st[i]); // print the result via character per character
    }
    glFlush();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 5.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    backGround(); // Draw the background

    Draw_Team1(-5.0, -5.0); // Draw the firt player from team 1
    Draw_Team1(-5.0, 5.0);  // Draw the second player from team 1
    team1Wins(-5.0, 4.0);   // display the score of team 1

    Draw_Team2(5.0, -5.0); // Draw the firt player from team 2
    team2Wins(3.0, 4.0);   // display the score of team 2
    Draw_Team2(5.0, 5.0);  // Draw the second player from team 2

    Draw_print_winner(-1.0, 5.0); // display the winner from team 1 and team 2
    Draw_Child5();                // display child 5
    glutSwapBuffers();
}

volatile sig_atomic_t should_stop = 0;

void handle_signal(int signal)
{
    should_stop = 1;
}

int main(int argc, char *argv[])
{

    int Number_of_rounds = 5;
    if (argc > 1)
    {
        Number_of_rounds = (int)strtol(argv[1], (char **)NULL, 10);
    }
    printf("Number of rounds = %d\n", Number_of_rounds);

    char valuesPerRounds[Number_of_rounds][4]; // Declare a two-dimensional character array to hold values for each round.
    int win1 = 0, win2 = 0;                    // Initialize two integer variables.
    for (int round = 0; round < Number_of_rounds; round++)
    {
    	printf("Start Round %d\n", round + 1);
        // Initialize signal handling
        struct sigaction action;
        memset(&action, 0, sizeof(action));
        action.sa_handler = handle_signal;
        sigaction(SIGINT, &action, NULL);

        // Fork the children processes. creat 5 children
        char buf[BUF_SIZE];
        char *childNum = (char *)malloc(10 * sizeof(char));
        char *childPid = (char *)malloc(10 * sizeof(char));
        char *child_Read_Pipe = (char *)malloc(10 * sizeof(char));
        char *child_Write_Pipe = (char *)malloc(10 * sizeof(char));
        pid_t pids[PLAYERS];

        // Create pipes for communication between parent and children (5 children)
        int child_pipes[PLAYERS][2];
        // create each child pipe and return -1 if an error message is printed and the program exits.
        for (int i = 0; i < PLAYERS; i++)
        { // i = 0 - 4
            if (pipe(child_pipes[i]) == -1)
            {
                perror("Could not create child pipe");
                exit(EXIT_FAILURE);
            }
        }

        // the following loop forks the first 4 child processes and stores their process IDs in the pids array
        for (int i = 0; i < NUM_PLAYERS; i++)
        { // i = 0 - 4
            pids[i] = fork();
            // If fork returns -1, an error message is printed and the program exits.
            if (pids[i] == -1)
            {
                perror("Could not fork child process");
                exit(EXIT_FAILURE);
            }
            else if (pids[i] == 0)
            {
                //  If fork returns 0, then its a Child process

                // This loop iterates over all of the pipes and closes the file descriptors for reading and writing for
                // all pipes except the one associated with the current child process.
                for (int j = 0; j < NUM_PLAYERS; j++)
                {
                    if (j != i)
                    {
                        close(child_pipes[j][PIPE_READ_END]);
                        close(child_pipes[j][PIPE_WRITE_END]);
                    }
                }

                // passes four arguments to the child file
                sprintf(childNum, "%d", i);                                      // passes the current child process's index i
                sprintf(childPid, "%d", getpid());                               // passes the process ID
                sprintf(child_Read_Pipe, "%d", child_pipes[i][PIPE_READ_END]);   // passes the file descriptors for reading
                sprintf(child_Write_Pipe, "%d", child_pipes[i][PIPE_WRITE_END]); // passes the file descriptors for writing

                // executes the child file using the execlp() system call.
                execlp("./child", "./child", childNum, childPid, child_Read_Pipe, child_Write_Pipe, NULL);
                // If execlp() fails, an error message is printed and the child process exits with a status of -1.
                perror("fail");

                kill(getpid(), SIGUSR1);
                exit(0);
            }
            else
            {
                // Parent process
                // after the child process finishes executing the following execute in the parent process
                close(child_pipes[i][PIPE_WRITE_END]);              // This line closes the write end of the pipe of the current child process, so that the parent process can no longer write to it.
                read(child_pipes[i][PIPE_READ_END], buf, BUF_SIZE); // This line reads data from the read end of the pipe of the current child process and stores it in the buffer
                close(child_pipes[i][PIPE_READ_END]);               // This line closes the read end of the pipe of the current child process, so that the parent process can no longer read from it.

                // wait for the child to die or terminate, it blocks the parent process until the child process has exited.
                waitpid(pids[i], NULL, 0);

                // kill the child after it's ready
                kill(pids[i], SIGTERM);
            }
        }

        // generating 2 random values and choosing them as min and max values that are stored in "range.txt" file:
        // Open the range file for writing
        FILE *range_file = fopen("range.txt", "w");
        // check if the file was opened successfully. If not, an error message is printed and the program is terminated
        if (range_file == NULL)
        {
            perror("Could not open range.txt for writing");
            exit(EXIT_FAILURE);
        }
        // initializes the minimum and maximum values of the range.
        int min1 = 0, max1 = 100;
        int min, max;

        // To generate a random number each time
        srand(getpid());

        // generating 2 random values
        int value1 = (int)(rand() % (max1 - min1 + 1) + min1) + (int)rand() / RAND_MAX; // generates a random number and assigns it to the variable value1.
        int value2 = (int)(rand() % (max1 - min1 + 1) + min1) + (int)rand() / RAND_MAX; // generates another random number and assigns it to the variable value2.

        // the following method compares value1 and value2 and assigns the smaller value to min and the larger value to max
        if (value1 <= value2)
        {
            min = value1;
            max = value2;
        }
        else if (value2 < value1)
        {
            min = value2;
            max = value1;
        }

        // write the range on the file
        fprintf(range_file, "%d,%d", min, max);
        fclose(range_file); // close the file

        // Read the values from the first 4 child processes, and save it in values array
        float values[NUM_PLAYERS];
        for (int i = 0; i < NUM_PLAYERS; i++)
        { // i = 0 - 3
            char filename[BUF_SIZE];

            // open the file with the name of the child's pid
            sprintf(filename, "%d.txt", pids[i]);
            FILE *input_file = fopen(filename, "r");

            if (input_file == NULL)
            {
                perror("Could not open input file for reading");
                exit(EXIT_FAILURE);
            }
            // Read the value from the input file into the corresponding index of the values array
            fscanf(input_file, "%f", &values[i]);
            fclose(input_file);
            printf("Value of child %d = %f.\n", i + 1, values[i]); 
            // Save the value as a character in the valuesPerRounds 2D array, which stores values for each round and player
            valuesPerRounds[round][i] = (char)values[i];
        }

        /// CHILD 5

        int co_processor_pipe[2]; // declare an integer array with size of 2 to store pipe file descriptors.
        if (pipe(co_processor_pipe) == -1)
        { // create a pipe and check if the return value is -1, which indicates an error in creating the pipe.
            perror("Could not create co-processor pipe");
            exit(EXIT_FAILURE);
        }

        char *child5Num = (char *)malloc(10 * sizeof(char));         // allocate memory for a character pointer child5Num with size of 10 bytes.
        char *child5Pid = (char *)malloc(10 * sizeof(char));         // allocate memory for a character pointer child5Pid with size of 10 bytes.
        char *child5_Read_Pipe = (char *)malloc(10 * sizeof(char));  // allocate memory for a character pointer child5_Read_Pipe with size of 10 bytes.
        char *child5_Write_Pipe = (char *)malloc(10 * sizeof(char)); // allocate memory for a character pointer child5_Write_Pipe with size of 10 bytes.

        // pipe 4 is for child 5
        co_processor_pipe[PIPE_READ_END] = child_pipes[NUM_PLAYERS][PIPE_READ_END];   // assign the read end of the pipe of child process 5 to the co_processor_pipe array.
        co_processor_pipe[PIPE_WRITE_END] = child_pipes[NUM_PLAYERS][PIPE_WRITE_END]; // assign the write end of the pipe of child process 5 to the co_processor_pipe array.

        // fork child 5
        pids[NUM_PLAYERS] = fork();
        // check if the fork() function failed and handle the error.
        if (pids[NUM_PLAYERS] == -1)
        {
            perror("Could not fork child process");
            exit(EXIT_FAILURE);
            // check if this is child 5 process and execute the child process code.
        }
        else if (pids[NUM_PLAYERS] == 0)
        {
            sprintf(child5Num, "%d", NUM_PLAYERS);                               // convert the integer NUM_PLAYERS to a string and store it in child5Num
            sprintf(child5Pid, "%d", getpid());                                  // convert the current process ID to a string and store it in child5Pid.
            sprintf(child5_Read_Pipe, "%d", co_processor_pipe[PIPE_READ_END]);   // convert the read end of the co-processor pipe to a string and store it in child5_Read_Pipe.
            sprintf(child5_Write_Pipe, "%d", co_processor_pipe[PIPE_WRITE_END]); // convert the write end of the co-processor pipe to a string and store it in child5_Write_Pipe.

            // execute child file with arguments child5Num, child5Pid, child5_Read_Pipe, child5_Write_Pipe, and NULL
            execlp("./child", "./child", child5Num, child5Pid, child5_Read_Pipe, child5_Write_Pipe, NULL);
            // print an error message if the execlp() function failed.
            perror("execlp child 5 fail");
            exit(-2);
        }
        else
        {
            // read data from the read end of the co-processor pipe and store it in buf.
            read(co_processor_pipe[PIPE_READ_END], buf, BUF_SIZE);

            // declare a character array message with size of BUF_SIZE to store the message to be sent to the co-processor.
            char message[BUF_SIZE];
            // Format the values array as a string and store it in the message variable
            sprintf(message, "%f,%f,%f,%f", values[0], values[1], values[2], values[3]);
            // Write the message string to the write end of the co-processor pipe
            write(co_processor_pipe[PIPE_WRITE_END], message, strlen(message) + 1);

            // Wait for the child process to finish
            wait(NULL);

            // Read the sums from the co-processor
            close(co_processor_pipe[PIPE_WRITE_END]);              // Close the write end of the co-processor pipe
            read(co_processor_pipe[PIPE_READ_END], buf, BUF_SIZE); // Read the sums calculated by the co-processor from the read end of the co-processor pipe
            close(co_processor_pipe[PIPE_READ_END]);               // Close the read end of the co-processor pipe

            // Calculate the winner
            // declare two variables to hold the sums of the two teams
            float sum1, sum2;

            // extract the values of the sums from the buffer string that is sent by child 5
            sscanf(buf, "%f,%f", &sum1, &sum2);
	    printf("Sum1 = %f, Sum2 = %f\n", sum1, sum2);
	     
            int winner = (sum1 > sum2) ? 1 : 2;  // determine the winner based on the sums
            // print the winner of each round
            printf("-> Winner of round %d is Team %d <-\n\n", round + 1, winner);
            waitpid(pids[NUM_PLAYERS], NULL, 0); // wait for child 5 to finish

            if (winner == 1)
            {
                win1 = win1 + 1; // increment the wins count for team 1
            }
            else if (winner == 2)
            {
                win2 = win2 + 1; // increment the wins count for team 2
            }
        }

        // remove the text files containing the player values
        for (int i = 0; i < NUM_PLAYERS; i++)
        {
            char filename[20];
            // create the filename of each player's file
            sprintf(filename, "%d.txt", pids[i]);
            // delete the file
            remove(filename);
        }
        // wait for 2 seconds before starting the next round
        sleep(2);
    }
    
    
    int finalWinner = (win1 > win2) ? 1 : 2;    // calculate the final winner based on the scores of the two teams
    print_final_winner = (win1 > win2) ? 1 : 2; // assign the final winner to a variable to be printed later
    print_team1_score = win1;                   // assign the score of team 1 to a variable to be printed later
    print_team2_score = win2;                   // assign the score of team 2 to a variable to be printed later
    
    printf("\n______________The final winner is Team %d______________\n", finalWinner);
        
    // set up the graphics display of the opengl window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Soccer Field");
    glEnable(GL_DEPTH_TEST);
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0; // end the program
}

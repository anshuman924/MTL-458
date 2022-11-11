#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


pthread_mutex_t rng_mutex;

int thread_safe_rng(int min, int max) {
    pthread_mutex_lock(&rng_mutex);
    int r = rand();
    pthread_mutex_unlock(&rng_mutex);
    return min + r % max;
}

/* TODO : can add global vars, structs, functions etc */

pthread_mutex_t A[4], J[4];
pthread_mutex_t dl;

int dead[4];

void arriveLane(int i) {
    /* TODO: add code here */
    pthread_mutex_lock(&A[i]);
    pthread_mutex_lock(&J[i]);

    pthread_mutex_lock(&dl);
    dead[i] = 1;
    pthread_mutex_unlock(&dl);
}

void crossLane(int i) {
    /* TODO: add code here */
    pthread_mutex_lock(&J[i]);

    pthread_mutex_lock(&dl);
    dead[i] = 2;
    pthread_mutex_unlock(&dl);

    usleep(1000 * thread_safe_rng(500, 1000)); // take 500-1000 ms to cross the lane
}

void exitLane(int i) {
    /* TODO: add code here */

    int j = (i + 3) % 4;
    pthread_mutex_unlock(&J[i]);
    pthread_mutex_unlock(&A[i]);
    pthread_mutex_unlock(&J[j]);

    pthread_mutex_lock(&dl);
    dead[i] = 0;
    dead[j] = 0;
    pthread_mutex_unlock(&dl);
}


int convert(char c) {
    if(c == 'N') {
        return 0;
    } else if(c == 'E') {
        return 3;
    } else if(c == 'S') {
        return 2;
    } else if(c == 'W') {
        return 1;
    }

    return -1;
}

char *dir(int x) {
    char *c = malloc(10);
    if(x == 0) {
        strcpy(c, "North");
    } else if(x == 1) {
        strcpy(c, "West");
    } else if(x == 2) {
        strcpy(c, "South");
    } else {
        strcpy(c, "East");
    }

    return c;
}
void *trainThreadFunction(void* arg)
{
    /* TODO extract arguments from the `void* arg` */
    usleep(thread_safe_rng(0, 10000)); // start at random time

    char *c = (char *) arg;
    char* trainDir = c; // TODO set the direction of the train: North/South/East/West.

    int x = convert(trainDir[0]);

    char *direction = dir(x);
    arriveLane(x);
    printf("Train Arrived at the lane from the %s direction\n", direction);
    usleep(1000*1000);

    int y = (x + 3) % 4;
    crossLane(y);

    printf("Train Exited the lane from the %s direction\n", direction);
    exitLane(x);
}

void *deadLockResolverThreadFunction(void * arg) {
    /* TODO extract arguments from the `void* arg` */
    int i = 0;
    while (1) {
        /* TODO add code to detect deadlock and resolve if any */

        int deadLockDetected = 0; // TODO set to 1 if deadlock is detected

        pthread_mutex_lock(&dl);
        if((dead[0] == 1) && (dead[1] == 1) && (dead[2] == 1) && (dead[3] == 1)) 
            deadLockDetected = 1;
        pthread_mutex_unlock(&dl);

        if (deadLockDetected) {
            printf("Deadlock detected. Resolving deadlock...\n");
            /* TODO add code to resolve deadlock */

            pthread_mutex_unlock(&J[i]);

            pthread_mutex_lock(&dl);
            dead[i] = 0;
            pthread_mutex_unlock(&dl);
            
            i = (i + 1) % 4;
        }

        usleep(1000 * 500); // sleep for 500 ms
    }
}




int main(int argc, char *argv[]) {


    srand(time(NULL));

    if (argc != 2) {
        printf("Usage: ./main <train dirs: [NSWE]+>\n");
        return 1;
    }

    pthread_mutex_init(&rng_mutex, NULL);
    for(int i = 0; i < 4; i++) {
        pthread_mutex_init(&A[i], NULL);
        pthread_mutex_init(&J[i], NULL);
    }

    pthread_mutex_init(&dl, NULL);


    /* TODO create a thread for deadLockResolverThreadFunction */
    pthread_t checkDead;
    pthread_create(&checkDead, NULL, deadLockResolverThreadFunction, NULL);

    char* train = argv[1];

    int n = strlen(train);
    pthread_t trains[n];

    int num_trains = 0;

    while (train[num_trains] != '\0') {
        char trainDir = train[num_trains];

        if (trainDir != 'N' && trainDir != 'S' && trainDir != 'E' && trainDir != 'W') {
            printf("Invalid train direction: %c\n", trainDir);
            printf("Usage: ./main <train dirs: [NSEW]+>\n");
            return 1;
        }

        /* TODO create a thread for the train using trainThreadFunction */

        char* cc = malloc(2);
        cc[0] = trainDir;
        cc[1] = '\0';

        pthread_create(&trains[num_trains], NULL, trainThreadFunction, cc);

        num_trains++;
    }

    /* TODO: join with all train threads*/

    for(int i = 0; i < n; i++) {
        pthread_join(trains[i], NULL);
    }

    return 0;
}
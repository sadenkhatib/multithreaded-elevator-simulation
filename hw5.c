#include "elevator.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int pickup_floor[PASSENGERS];
static int destination_floor[PASSENGERS];
static int state[PASSENGERS];
static int serving_elevator[PASSENGERS];
static int served_count;

static int pickup_flag[PASSENGERS];
static int boarded_flag[PASSENGERS];
static int arrived_flag[PASSENGERS];
static int exited_flag[PASSENGERS];

static pthread_mutex_t mlock;
static pthread_cond_t c_pickup[PASSENGERS];
static pthread_cond_t c_boarded[PASSENGERS];
static pthread_cond_t c_arrived[PASSENGERS];
static pthread_cond_t c_exited[PASSENGERS];

// helper, move elevator between floors
static void move2dest(int elevator, int source, int destination,
                      void (*move_elevator)(int, int)) {
    int direction = (destination > source) ? 1 : -1;
    int steps = abs(destination - source);
    for (int i = 0; i < steps; i++) {
        move_elevator(elevator, direction);
    }
}

// Initialize all data and synchronization objects
void initializer(void) {
    pthread_mutex_init(&mlock, NULL);
    for (int i = 0; i < PASSENGERS; i++) {
        state[i] = 0;
        pickup_floor[i] = destination_floor[i] = 0;
        serving_elevator[i] = -1;
        pickup_flag[i] = boarded_flag[i] = arrived_flag[i] = exited_flag[i] = 0;
        pthread_cond_init(&c_pickup[i], NULL);
        pthread_cond_init(&c_boarded[i], NULL);
        pthread_cond_init(&c_arrived[i], NULL);
        pthread_cond_init(&c_exited[i], NULL);
    }
    served_count = 0;
}

void passenger_controller(int passenger, int from_floor, int to_floor,
                          void (*enter_elevator)(int, int), void (*exit_elevator)(int, int)) {
    // signal request
    pthread_mutex_lock(&mlock);
    pickup_floor[passenger] = from_floor;
    destination_floor[passenger] = to_floor;
    state[passenger] = 1;
    pthread_mutex_unlock(&mlock);

    // wait for elevator to arrive at pickup
    pthread_mutex_lock(&mlock);
    while (!pickup_flag[passenger])
        pthread_cond_wait(&c_pickup[passenger], &mlock);
    pickup_flag[passenger] = 0;
    int my_elev = serving_elevator[passenger];
    pthread_mutex_unlock(&mlock);

    // board elevator
    enter_elevator(passenger, my_elev);

    // signal boarded
    pthread_mutex_lock(&mlock);
    boarded_flag[passenger] = 1;
    pthread_cond_signal(&c_boarded[passenger]);
    pthread_mutex_unlock(&mlock);

    // wait for arrival at destination
    pthread_mutex_lock(&mlock);
    while (!arrived_flag[passenger])
        pthread_cond_wait(&c_arrived[passenger], &mlock);
    arrived_flag[passenger] = 0;
    pthread_mutex_unlock(&mlock);

    // exit elevator
    exit_elevator(passenger, my_elev);

    // signal exited
    pthread_mutex_lock(&mlock);
    exited_flag[passenger] = 1;
    pthread_cond_signal(&c_exited[passenger]);
    state[passenger] = 0;  // mark request handled
    pthread_mutex_unlock(&mlock);
}

void elevator_controller(int elevator, int at_floor,
                         void (*move_elevator)(int, int), void (*open_door)(int),
                         void (*close_door)(int)) {
    int pid;
    int src, dst;
    while (1) {
        // pick a pending request
        pthread_mutex_lock(&mlock);
        if (served_count >= PASSENGERS * TRIPS_PER_PASSENGER) {
            pthread_mutex_unlock(&mlock);
            return;
        }
        pid = -1;
        for (int i = 0; i < PASSENGERS; i++) {
            if (state[i] == 1) {
                pid = i;
                state[i] = 0;
                break;
            }
        }
        pthread_mutex_unlock(&mlock);

        if (pid < 0) {
            sched_yield();
            continue;
        }

        // retrieve floors
        pthread_mutex_lock(&mlock);
        src = pickup_floor[pid];
        dst = destination_floor[pid];
        serving_elevator[pid] = elevator;
        pthread_mutex_unlock(&mlock);

        // move to pickup floor and open door
        move2dest(elevator, at_floor, src, move_elevator);
        open_door(elevator);

        // signal pickup
        pthread_mutex_lock(&mlock);
        pickup_flag[pid] = 1;
        pthread_cond_signal(&c_pickup[pid]);
        pthread_mutex_unlock(&mlock);

        // wait for boarded
        pthread_mutex_lock(&mlock);
        while (!boarded_flag[pid])
            pthread_cond_wait(&c_boarded[pid], &mlock);
        boarded_flag[pid] = 0;
        pthread_mutex_unlock(&mlock);

        // close door, move to destination, open door
        close_door(elevator);
        move2dest(elevator, src, dst, move_elevator);
        open_door(elevator);

        // signal arrival
        pthread_mutex_lock(&mlock);
        arrived_flag[pid] = 1;
        pthread_cond_signal(&c_arrived[pid]);
        pthread_mutex_unlock(&mlock);

        // wait for exit
        pthread_mutex_lock(&mlock);
        while (!exited_flag[pid])
            pthread_cond_wait(&c_exited[pid], &mlock);
        exited_flag[pid] = 0;
        // count served
        served_count++;
        pthread_mutex_unlock(&mlock);

        // close door after exit
        close_door(elevator);
        at_floor = dst;
    }
}


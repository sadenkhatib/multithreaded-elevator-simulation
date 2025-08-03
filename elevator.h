#include <pthread.h>
#include <ncurses.h>

/* Change these experiment settings to try different scenarios. Parameters can
     also be passed in using gcc flags, e.g. -DELEVATORS=5 */

#ifndef MAX_CAPACITY
#define MAX_CAPACITY 3
#endif

#ifndef ELEVATORS
#define ELEVATORS 3
#endif

#ifndef FLOORS
#define FLOORS 28
#endif

#ifndef PASSENGERS
#define PASSENGERS 5
#endif

#ifndef TRIPS_PER_PASSENGER
#define TRIPS_PER_PASSENGER 3
#endif



#ifndef DELAY
#define DELAY 50000
#endif


extern pthread_mutex_t log_lock;
extern WINDOW * logwin;
extern FILE * logfile;


#ifndef LOG_LEVEL
#define LOG_LEVEL 9
#endif


#define log(level,format,...) do{ if(level<=LOG_LEVEL) { pthread_mutex_lock(&log_lock); fprintf(logfile,format,__VA_ARGS__); fflush(logfile); wprintw(logwin,format,__VA_ARGS__); wrefresh(logwin); pthread_mutex_unlock(&log_lock);} }while(0);


void initializer();


void passenger_controller(int passenger, int from_floor, int to_floor,
                       void (*enter)(int, int), void(*exit)(int, int));



void elevator_controller(int elevator, int at_floor,
            void(*move_direction)(int, int), void(*open_door)(int), void(*close_door)(int));


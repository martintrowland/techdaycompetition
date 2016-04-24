//
// Mark Rowland
// Galaxy Server for TechDay competition 2016
// 
//

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#define USE_X_WINDOWS

//
// Galaxy Data Structures
//
#define TRUE           1
#define FALSE          0
#define DEBUG          0
#define MAX_PLAYERS   10
#define MAX_BUFFER  1000
#define MAX_PLANETS  100
#define MAX_EVENTS  5000
#define NEUTRAL       -1

FILE *logFile;

typedef struct{
  int  x,y,ships,ownerIdx,prod;
  char name;
}planetType;
typedef struct{
  int   attackerIdx, sourceIdx, targetIdx, ships;
  double arrivalTime,travelTime;
}eventType;
typedef struct{
  char name[MAX_BUFFER];
  char exec[MAX_BUFFER];
  pid_t pid;
  int   from, to;
  int   in[2],out[2];
}playerType;
typedef struct {
  int playerCount;
  int planetCount;
  int eventCount;
  int waitForPrompt;
  int seed;
  int turn;
  int max_x;
  int max_y;
  playerType player[MAX_PLAYERS];
  planetType planet[MAX_PLANETS];
  eventType  event[MAX_EVENTS];
}galaxyType;
void clearScreen();
void flushScreen();
void createCanvas(galaxyType *galaxy);
void drawMessage(galaxyType *galaxy, char *s, int player);
void drawEvents(galaxyType *galaxy);
void drawGalaxy(galaxyType *galaxy);

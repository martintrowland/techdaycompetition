#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

//
// Galaxy Data Structures
//
#define TRUE  1
#define FALSE 0
#define MAX_PLAYERS 20
#define MAX_BUFFER 200
#define MAX_PLANETS 100
#define MAX_EVENTS 5000
#define NO_OWNER -1
#define NAME Player1

typedef struct{
  int  x,y,ships,ownerIdx,prod;
  char name;
  char owner[MAX_BUFFER];
}planetType;

typedef struct{
  int   attackerIdx, planetIdx, ships;
  float arrivalTime;
}eventType;

typedef struct{
  char name[MAX_BUFFER];
}playerType;

typedef struct {
  int playerCount;
  int planetCount;
  int eventCount;
  int max_x;
  int max_y;
  char playerName[MAX_BUFFER];
  playerType player[MAX_PLAYERS];
  planetType planet[MAX_PLANETS];
  eventType  events[MAX_EVENTS];
}galaxyType;

FILE *logFile;

//
// Initialize Game
//
void processCommandLine(int argc, char *argv[],galaxyType *galaxy){
  if(argc<1){
    printf("usage: player <name>");
    exit(1);
  }else{
    sscanf(argv[1],"%s\n",&(galaxy->playerName));
  }
}

//
// ReadLine
//
void readLine(char *buf){
  if(fgets(buf,100,stdin)){
    //fprintf(stderr,"%s\n",buf);
  }else{
    fprintf(stderr,"EOF read!!\n");
    exit(1);
  }
  fputs(buf,logFile);
  fflush(logFile);
}

void writeLine(char *buf){
  fputs(buf,logFile);
  fputc('\n',logFile);
  fflush(logFile);
  write(1,buf,strlen(buf));
}
//
// Setup
//
void setup(galaxyType *galaxy){
  int i,j,collision;
  planetType *p=&(galaxy->planet[i]);
}
//
// Read Planet update
//
int readPlanetUpdate(galaxyType *galaxy){
  //*Planet Update
  int j,x,y,s;
  char buf[MAX_BUFFER],
    token[MAX_BUFFER],
    token2[MAX_BUFFER],
    name[MAX_BUFFER];
  int i=0;
  char p;

  readLine(buf);
  i=0;
  do{
    readLine(buf);
    j=sscanf(buf,"Planet %c Owner %s X %d Y %d Ships %d Prod %d",
             &(galaxy->planet[i].name),
             &galaxy->planet[i].owner,
             &(galaxy->planet[i].x),
             &(galaxy->planet[i].y),
             &(galaxy->planet[i].ships),
             &( galaxy->planet[ i ].prod ) );

    if(j==6){
      i++;
      galaxy->planetCount=i;
    }
  }while((buf[0]!='*'));
  return(i);
}
//
// Calculate Travel Time
//
double calculateTravelTime(galaxyType *galaxy, int sourceIdx, int targetIdx){
  double time=sqrt(pow((double)(galaxy->planet[sourceIdx].x-
				galaxy->planet[targetIdx].x),2.0)+
                   pow((double)(galaxy->planet[sourceIdx].y-
				galaxy->planet[targetIdx].y),2.0));
  return(time);
}
//
// Make Moves
//
void moveShips(galaxyType *galaxy){
  int i, totalShips=0, planetWithLeastShips=10000, planetWithLeastShipsIdx=-1;
  char cmd[MAX_BUFFER];
  char buf[MAX_BUFFER];
  //Examine the planets
  for(i=0;i<galaxy->planetCount;i++){
    if(!strcmp(galaxy->planet[i].owner,galaxy->playerName)){
      totalShips+=(0.8*galaxy->planet[i].ships);
    }else{
      if(planetWithLeastShipsIdx== -1){
	planetWithLeastShipsIdx=i;
      }
      if(galaxy->planet[i].ships<galaxy->planet[planetWithLeastShipsIdx].ships){
	planetWithLeastShipsIdx=i;
      }
    }
  }
  // Should we Attack?
  if((planetWithLeastShipsIdx>=0) &&
     (totalShips>galaxy->planet[planetWithLeastShipsIdx].ships)){
    fprintf(logFile,"//Yes attack planet %c has only %d ships\n",
	    (int)(galaxy->planet[planetWithLeastShipsIdx].ships),
	    galaxy->planet[planetWithLeastShipsIdx].name);
    fflush(logFile);
    for(i=0;i<galaxy->planetCount;i++){
      if(!strcmp(galaxy->planet[i].owner,galaxy->playerName)){
	sprintf(cmd,"move %c to %c %d ships",galaxy->planet[i].name,
		galaxy->planet[planetWithLeastShipsIdx].name,
		(int)(galaxy->planet[i].ships*0.8));
	writeLine(cmd);
      }
    }
  }
  sprintf(buf,"end turn %s",galaxy->playerName);
  writeLine(buf);
}



int main (int argc, char *argv[]){
  galaxyType galaxy;
  char fileName[MAX_BUFFER];

  //Report the Robot Name (no spaces!)
  processCommandLine(argc,argv,&galaxy);
  sprintf(fileName,"%s.log",galaxy.playerName);
  if((logFile=fopen(fileName,"w"))==NULL){
    printf("ERROR opening %s file\n",fileName);
  }
  writeLine(galaxy.playerName);
  do{
    readPlanetUpdate(&galaxy);
    moveShips(&galaxy);
  }while(TRUE);

  fclose(logFile);
  return(0);
}

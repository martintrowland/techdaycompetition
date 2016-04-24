#include "galaxy.h"
#include <time.h>

//
// Initialize Game
//
void processCommandLine(int argc, char *argv[],galaxyType *galaxy){
  fprintf(stderr, "argc %d\n",argc);
  if(argc<7){
    fprintf(stderr,"usage: galaxy <number of neutral planets> <size x> <size y> <wait for prompt 0/1> <exectuable> <executable Name>...");
    exit(-1);
  }else{
    sscanf(argv[1],"%d",&(galaxy->planetCount));
    sscanf(argv[2],"%d",&(galaxy->max_x));
    sscanf(argv[3],"%d",&(galaxy->max_y));
    sscanf(argv[4],"%d",&(galaxy->waitForPrompt));

    galaxy->playerCount=0;
    galaxy->eventCount=0;
    int i;
    for(i=5; i<argc; i+=2){
      sprintf(galaxy->player[galaxy->playerCount].exec,"%s %s",argv[i],argv[i+1]);
      (galaxy->playerCount)++;

      fprintf(stderr, "[%s]\n",galaxy->player[galaxy->playerCount].exec);

      if(galaxy->playerCount>=MAX_PLAYERS){
	fprintf(stderr,"ERROR: To many players. Only %d are supported\n",
		MAX_PLAYERS);
	exit(-1);
      }
    }
    galaxy->planetCount+=galaxy->playerCount;
    if(galaxy->planetCount>=MAX_PLANETS){
      fprintf(stderr,"ERROR: To many planets. Only %d are supported\n",
	      MAX_PLANETS);
      exit(-1);
    }

    galaxy->turn=1;
    galaxy->eventCount=0;
  }
}

//
// Launch Executables
//
int launchExectuables(galaxyType *galaxy) {
  int playerIdx;
  for(playerIdx=0;playerIdx<galaxy->playerCount;playerIdx++){
    printf("exec [%s]\n", galaxy->player[playerIdx].exec);
    playerType *p=&(galaxy->player[playerIdx]);
    if(pipe(p->in)) return -1;
    if(pipe(p->out)) return -1;
    p->pid = fork();
    if(p->pid < 0) return p->pid; /* Fork failed */
    if(p->pid == 0) { /* child */
      close(p->in[1]);
      dup2(p->in[0], 0);
      close(p->out[0]);
      dup2(p->out[1], 1);
      execl("/bin/sh","sh","-c", p->exec, NULL);
      perror("execl");
      exit(99);
    }
    p->to = p->in[1];
    p->from = p->out[0];
  }
  return 0;
}

//
// Create Galaxy
//
void createGalaxy(galaxyType *galaxy){

  int i,j,collision;

  srand(time(NULL));

  for(i=0;i<galaxy->planetCount; i++){
    planetType *p=&(galaxy->planet[i]);
    p->x=(rand()%galaxy->max_x);
    p->y=(rand()%galaxy->max_y);
    p->name=(char)('A'+i);
    p->prod=(rand()%8+4);
    p->ownerIdx=NO_OWNER;
    p->ships=50;
    if(DEBUG){
      printf("planet %c %d %d\n",galaxy->planet[i].name,
             galaxy->planet[i].x,galaxy->planet[i].y);
    }
    collision=FALSE;
    for(j=0;j<i;j++){
      if((p->x==galaxy->planet[j].x)&&
         (p->y==galaxy->planet[j].y)){
        collision=TRUE;
      }
    }
    if(collision){
      i--;
    }
  };  // end for loop

  for(j=0;j<galaxy->playerCount;j++){
    galaxy->planet[j].ownerIdx=j;
    galaxy->planet[j].ships=100;
    galaxy->planet[j].prod=10;
  };  // end for each planet

};  // end createGalaxy


//
// lookupPlanetIdx
//
int lookupPlanetIdx(galaxyType *galaxy, char name){
  int i;
  for(i=0;i<galaxy->planetCount;i++)
    if(galaxy->planet[i].name==name)
      return(i);
  return(-1);
}

//
// Calculate Distance
//
double calculateTravelTime(galaxyType *galaxy, int sourceIdx, int targetIdx){
  double time=sqrt(pow((double)(galaxy->planet[sourceIdx].x-
				galaxy->planet[targetIdx].x),2.0)+
                   pow((double)(galaxy->planet[sourceIdx].y-
				galaxy->planet[targetIdx].y),2.0));
  return(time);
}
//
// PrintEvents
//
void printEvents(galaxyType *galaxy){
  int i;
  for(i=0;i<galaxy->eventCount;i++){
    fprintf(stderr,"%d: time %5.1lf %s %c to %c ships %d\n",
	    i,
	    galaxy->event[i].arrivalTime,
	    galaxy->player[galaxy->event[i].attackerIdx].name,
	    galaxy->planet[galaxy->event[i].sourceIdx].name,
	    galaxy->planet[galaxy->event[i].targetIdx].name,
	    galaxy->event[i].ships);
  }
}
//
// update Production
//
void updateProduction(galaxyType *galaxy){
  int i;
  for(i=0;i<galaxy->planetCount;i++){
    galaxy->planet[i].ships+=galaxy->planet[i].prod;
  }
}
//
// simulate Battle
//
void simulateBattle(galaxyType *galaxy){
  int attackerIdx=galaxy->event[0].attackerIdx;
  int attackerShips=galaxy->event[0].ships;
  int defenderIdx=galaxy->planet[galaxy->event[0].targetIdx].ownerIdx;
  int defenderShips=galaxy->planet[galaxy->event[0].targetIdx].ships;
  int i;
  char *name;
  char s[MAX_BUFFER];
  if(attackerIdx==defenderIdx){
    sprintf(s,"Event: %s has reinforced planet %c with %d ships and now has %d ships",
	    galaxy->player[galaxy->event[0].attackerIdx].name,
	    galaxy->planet[galaxy->event[0].targetIdx].name,
	    galaxy->event[0].ships,
	    galaxy->planet[galaxy->event[0].targetIdx].ships+attackerShips);
    fprintf(stderr,"%s\n",s);
#ifdef USE_X_WINDOWS
    drawMessage(galaxy,s,attackerIdx);
#endif
    galaxy->planet[galaxy->event[0].targetIdx].ships+=attackerShips;
    return;
  }
  // battle here
  while(attackerShips && defenderShips){
    if(rand()%2){
      --defenderShips;
    }else{
      --attackerShips;
    }
  }
  if((i=galaxy->planet[galaxy->event[0].targetIdx].ownerIdx)!=NO_OWNER){
    name=galaxy->player[galaxy->planet[galaxy->event[0].targetIdx].ownerIdx].name;
  }else{
    name="NEUTRAL";
  }
  if(attackerShips>0){
    sprintf(s,"Event: %s has attacked %s with %d ships on planet %c with a defense of %d ships and has won with %d ships left",
	    galaxy->player[galaxy->event[0].attackerIdx].name,
	    name,
	    galaxy->event[0].ships,
	    galaxy->planet[galaxy->event[0].targetIdx].name,
	    galaxy->planet[galaxy->event[0].targetIdx].ships,
	    attackerShips);
    fprintf(stderr,"%s\n",s);
#ifdef USE_X_WINDOWS
    drawMessage(galaxy,s,attackerIdx);
#endif
    galaxy->planet[galaxy->event[0].targetIdx].ships=attackerShips;
    galaxy->planet[galaxy->event[0].targetIdx].ownerIdx=galaxy->event[0].attackerIdx;
  }else{
    sprintf(s,"Event: %s has attacked %s with %d ships on planet %c with a defense of %d ships and has lost leaving %d ships",
	    galaxy->player[galaxy->event[0].attackerIdx].name,
	    name,
	    galaxy->event[0].ships,
	    galaxy->planet[galaxy->event[0].targetIdx].name,
	    galaxy->planet[galaxy->event[0].targetIdx].ships,
	    defenderShips);
    fprintf(stderr,"%s\n",s);
#ifdef USE_X_WINDOWS
    drawMessage(galaxy,s,attackerIdx);
#endif
    galaxy->planet[galaxy->event[0].targetIdx].ships=defenderShips;
  }
}
//
// Remove Event
//
void removeEvent(galaxyType *galaxy){
  int i;
  galaxy->eventCount-=1;
  for(i=0;i<galaxy->eventCount;i++){
    galaxy->event[i]=galaxy->event[i+1];
  }
}

//
// insertEvent
//
int insertEvent(galaxyType *galaxy, eventType event){
  int i,j;
  int insertIdx=galaxy->eventCount;
  for(i=0; i<galaxy->eventCount; i++){
    if(event.arrivalTime<galaxy->event[i].arrivalTime){
      for(j=galaxy->eventCount+1;j>i;--j){
	galaxy->event[j]=galaxy->event[j-1];
      }
      insertIdx=i;
      break;
    }
  }
  galaxy->event[insertIdx]=event;
  if((galaxy->eventCount+=1)>MAX_EVENTS){
    fprintf(stderr,"ERROR: event queue out of space!\n");
    exit(-1);
  }
  return(galaxy->eventCount);
}

//
// schedule Event
//
int scheduleEvent(galaxyType *galaxy,int playerIdx,
		  char src, char dest,int ships){
  int destIdx,sourceIdx;
  double travelTime;
  eventType  event;

  if((sourceIdx=lookupPlanetIdx(galaxy,src))== -1){
    fprintf(stderr,"Foul: Player %s selected source planet %c and it does not exist\n",
	    galaxy->player[playerIdx].name,src);
    return(-1);
  }
  if(galaxy->planet[sourceIdx].ownerIdx!=playerIdx){
    fprintf(stderr,"Foul: Player %s does not own source planet %c\n",
	    galaxy->player[playerIdx].name,src);
    return(-1);
  }
  if((destIdx=lookupPlanetIdx(galaxy,dest))== -1){
    fprintf(stderr,"Foul: Player %s selected destination planet %c and it does not exist\n",
	    galaxy->player[playerIdx].name,dest);
    return(-1);
  }

  if(ships>galaxy->planet[sourceIdx].ships){
    fprintf(stderr,"Foul: move command from %s from %c to %C requested %d ships but only has %d ships. Command ignored\n",
	    galaxy->player[playerIdx].name,
	    galaxy->planet[sourceIdx].name,
	    galaxy->planet[destIdx].name,
	    ships,
	    galaxy->planet[destIdx].ships);
    return(-1);
  }
  travelTime=calculateTravelTime(galaxy,sourceIdx,destIdx);
  event.arrivalTime=galaxy->turn+travelTime;
  event.travelTime=travelTime;
  event.ships=ships;
  event.attackerIdx=playerIdx;
  event.sourceIdx=sourceIdx;
  event.targetIdx=destIdx;
  insertEvent(galaxy,event);
  galaxy->planet[sourceIdx].ships-=ships;
  return(TRUE);
}

//
// process Events
//
void processEvents(galaxyType *galaxy){
  int i=0;
  while((i<galaxy->eventCount)&&(galaxy->event[i].arrivalTime<galaxy->turn)){
    simulateBattle(galaxy);
    removeEvent(galaxy);
  }
}

//
// Print Galaxy
//
int printGalaxy(galaxyType *galaxy){

  int i,j;
  char *owner;
  fprintf(stderr,"* Planet Update\n");
  fprintf(logFile,"* Planet Update\n");
  for(i=0;i<galaxy->planetCount;i++){
    planetType *p=&(galaxy->planet[i]);
    if(p->ownerIdx==NO_OWNER){
      owner="NEUTRAL";
    }else{
      owner=galaxy->player[p->ownerIdx].name;
    }
    fprintf(stderr,"Planet %c Owner %s X %d Y %d Ships %d\n",
	   p->name, owner,p->x,p->y,p->ships);
    fprintf(logFile,"Planet %c Owner %s X %d Y %d Ships %d\n",
	   p->name, owner,p->x,p->y,p->ships);
  }
  fflush(logFile);
}
//
// Victory
//
int victory(galaxyType *galaxy){
  int i;
  char buf[MAX_BUFFER];
  for(i=0;i<galaxy->planetCount; i++){
    if(galaxy->planet[0].ownerIdx!=galaxy->planet[i].ownerIdx){
      return(FALSE);
    }
  }
  for(i=0;i<galaxy->eventCount;i++){
    if(galaxy->planet[0].ownerIdx!=galaxy->event[i].attackerIdx){
      return(FALSE);
    }
  }
  sprintf(buf,"VICTORY has been achieved for %s!!!\n",
	  galaxy->player[galaxy->planet[0].ownerIdx].name);
  fprintf(stderr,"%s",buf);
  fprintf(logFile,"%s",buf);
  fflush(logFile);
#ifdef USE_X_WINDOWS
  clearScreen();
  drawMessage(galaxy,buf,galaxy->planet[0].ownerIdx);
#endif
  return(TRUE);
}

//
// Print Planet Update
//
int printPlanetUpdate(galaxyType *galaxy,int playerIdx){
  int i,j,ships;
  char buf[MAX_BUFFER];
  char *owner;
  sprintf(buf,"*Planet Update turn %d %s\n",
	  galaxy->turn,galaxy->player[playerIdx].name);
  fprintf(stderr,"%s",buf);
  write(galaxy->player[playerIdx].to,buf,strlen(buf));
  for(i=0;i<galaxy->planetCount;i++){
    planetType *p=&(galaxy->planet[i]);
    if(p->ownerIdx==NO_OWNER){
      owner="NEUTRAL";
    }else{
      owner=galaxy->player[p->ownerIdx].name;
    }
    sprintf(buf,"Planet %c Owner %s X %d Y %d Ships %d Prod %d\n",
            p->name, owner,p->x,p->y,p->ships, p->prod );
    fprintf(stderr,"%s",buf);
    fprintf(logFile,"%s",buf);
    write(galaxy->player[playerIdx].to,buf,strlen(buf));
  }
  sprintf(buf,"*Planet Update Complete\n");
  fprintf(stderr,"%s",buf);
  fprintf(logFile,"%s",buf);
  fflush(logFile);
  write(galaxy->player[playerIdx].to,buf,strlen(buf));
}
//
// Read Move Commands
//
int readMoveCmds(galaxyType *galaxy,int playerIdx){
  int i,j,done,ships;
  char buf[MAX_BUFFER];
  char source,dest;

  FILE *f=fdopen(galaxy->player[playerIdx].from,"r");
  while(fscanf(f,"move %c to %c %d ships",&source,&dest,&ships)==3){
    fprintf(stderr,"%s: move %c to %c ships %d\n",
	    galaxy->player[playerIdx].name,source,dest,ships);
    scheduleEvent(galaxy,playerIdx,source,dest,ships);
  }
  fprintf(stderr,"%s: end turn\n",galaxy->player[playerIdx].name);
  return(1);
}


//
// Trim Return
//
void trimReturn(char *buf){
  if(buf[strlen(buf)-1]=='\n'){
    buf[strlen(buf)-1]='\0';
  }
}

//
// Read Names
//
int readNames(galaxyType *galaxy){
  int i;
  for(i=0;i<galaxy->playerCount;i++){
    playerType *p=&(galaxy->player[i]);
    write(p->to,"Galaxy\n",7);
    read(p->from,&(p->name),MAX_BUFFER);
    printf("[%s]",p->name);
  }
}


int main (int argc, char *argv[]){
  char buf[MAX_BUFFER];
  galaxyType galaxy;
  int status;
  int i,j;

  processCommandLine(argc,argv,&galaxy);
  createGalaxy(&galaxy);
  if((status=launchExectuables(&galaxy))!=0){
    printf("ERROR: launch executable failed %d\n",status);
    exit(-1);
  }

  if((logFile=fopen("mark","w"))==NULL){
    printf("ERROR opening mark.log file\n");
  }

#ifdef USE_X_WINDOWS
  createCanvas(&galaxy);
#endif

  printf("count of players: %d\n",galaxy.playerCount);
  for(i=0; i<galaxy.playerCount; i++){
    playerType *p=&(galaxy.player[i]);
    printf("starting command %s\n",p->exec);
    memset(buf, 0, MAX_BUFFER);
    read(p->from, &(p->name),1000);
    trimReturn(p->name);
  }
  do{
    for(i=0;i<galaxy.playerCount;i++){
      printPlanetUpdate(&galaxy,i);
    }
    for(i=0;i<galaxy.playerCount;i++){
      readMoveCmds(&galaxy,i);
    }
    updateProduction(&galaxy);
#ifdef USE_X_WINDOWS
    clearScreen();
    drawGalaxy(&galaxy);
    drawEvents(&galaxy);
#endif
    processEvents(&galaxy);
#ifdef USE_X_WINDOWS
    flushScreen();
#endif
    printEvents(&galaxy);
    galaxy.turn+=1;
    if(galaxy.waitForPrompt){
      fprintf(stderr,"Press Enter\n");
      fgets( buf, MAX_BUFFER, stdin );
    }else{
      usleep(1000000);
    }
  }while(!victory(&galaxy));
  fclose(logFile);

#ifdef USE_X_WINDOWS
  drawGalaxy(&galaxy);
  drawEvents(&galaxy);
  flushScreen();
#endif

  for(i=0; i<galaxy.playerCount; i++){
    playerType *p=&(galaxy.player[i]);
    kill(p->pid, 0);
    waitpid(p->pid, NULL, 0);
    kill(p->pid, 0);
  }

  exit(0);
}

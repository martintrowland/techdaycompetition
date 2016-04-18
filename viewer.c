#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <assert.h>  
#include <unistd.h>  
#include <stdio.h>   
#include "galaxy.h"
#define NIL (0)      
#define PLANET_SIZE   3
#define GRID_SIZE    100

int messageCount=0;
const double PI=3.142;
GC gc;   
Display *dpy;
XTextProperty textProperty;
Colormap colormap;
XColor black_col,white_col,red_col,green_col,blue_col,yellow_col;
XColor colorPixel[MAX_PLAYERS];
const int NUM_COLORS=8;
char colors[MAX_PLAYERS][10]={"#FF0000","#00FF00","#00FFFF","#0000FF",
			      "#FF00FF","#006400","#640000","#000064"};
char black_bits[] = "#000000";
char white_bits[] = "#FFFFFF";
char red_bits[] = "#FF0000";
char green_bits[] = "#00FF00";
char blue_bits[] = "#0000FF";
char yellow_bits[] = "#FFFF00";
Window w;
Font font;

void createCanvas(galaxyType *galaxy)
{ 
  char keyboard_input[100];
  dpy = XOpenDisplay(NIL); assert(dpy);
  int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
  //int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));
  // Define the colors we want to use
  colormap = DefaultColormap(dpy, 0);
  XParseColor(dpy, colormap, black_bits, &black_col); XAllocColor(dpy, colormap, &black_col);
  XParseColor(dpy, colormap, white_bits, &white_col); XAllocColor(dpy, colormap, &white_col);
  XParseColor(dpy, colormap, red_bits, &red_col); XAllocColor(dpy, colormap, &red_col);
  XParseColor(dpy, colormap, green_bits, &green_col);XAllocColor(dpy, colormap, &green_col);
  XParseColor(dpy, colormap, blue_bits, &blue_col);XAllocColor(dpy, colormap, &blue_col);
  XParseColor(dpy, colormap, yellow_bits, &yellow_col);XAllocColor(dpy, colormap, &yellow_col);

  int i;
  for(i=0;i<NUM_COLORS;i++){
    XParseColor(dpy, colormap, colors[i], &colorPixel[i]); XAllocColor(dpy, colormap, &colorPixel[i]);
  }
    
  w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
			  galaxy->max_x*GRID_SIZE,
			  (galaxy->max_y+1)*GRID_SIZE, 0,
			  blackColor, blackColor);
 
  XSelectInput(dpy, w, StructureNotifyMask);
  XMapWindow(dpy, w);

  char *title="Galaxy";
  XStringListToTextProperty(&title,1,&textProperty);
  XSetWMName(dpy,w,&textProperty);
 
 for(;;){XEvent e; XNextEvent(dpy,&e); if(e.type == MapNotify) break;}
  gc = XCreateGC(dpy, w, 0, NIL);    
}
void clearScreen(){
  XClearWindow(dpy,w);
}
void flushScreen(){
  messageCount=0;
  XFlush(dpy);
}
void drawMessage(galaxyType *galaxy, char *s,int player){
  XColor color=colorPixel[player%NUM_COLORS];
  XSetForeground(dpy,gc,color.pixel);
  XDrawString(dpy,w,gc,
	      20,
	      ((galaxy->max_y+1)*GRID_SIZE)-20-15*messageCount++,
	      s,strlen(s));
}
void drawEvents(galaxyType *galaxy){
  int i,j;
  char s[MAX_BUFFER];
  for(i=0;i<galaxy->eventCount;i++){
    int r=(galaxy->event[i].ships/5)+1;
    int srcx=galaxy->planet[galaxy->event[i].sourceIdx].x;
    int srcy=galaxy->planet[galaxy->event[i].sourceIdx].y;
    int destx=galaxy->planet[galaxy->event[i].targetIdx].x;
    int desty=galaxy->planet[galaxy->event[i].targetIdx].y;
    double percent=(double)(galaxy->turn-galaxy->event[i].arrivalTime)
			    /galaxy->event[i].travelTime;

    if(percent>0.0){
      percent=0.0;
    }
    double distanceTraveledX=(double)labs(destx-srcx)*percent;
    double distanceTraveledY=(double)labs(desty-srcy)*percent;
    /*
    fprintf(stderr,"initial (%d %d) (%d %d) AT %lf turn %d ships %d\n",
	    srcx,srcy,destx,desty,
	    galaxy->event[i].arrivalTime, galaxy->turn,
	    galaxy->event[i].ships);
    fprintf(stderr,"x percent %.2lf distance %d travel %.2lf %d x %.2lf %d\n",
      percent,labs(destx-srcx),
      distanceTraveledX,
      (int)distanceTraveledX,
      (srcx<destx)?destx+distanceTraveledX:destx-distanceTraveledX,
      (srcx<destx)?(int)(destx+distanceTraveledX):(int)(destx-distanceTraveledX));
    fprintf(stderr,"y percent %.2lf distance %d travel %.2lf %d y %.2lf %d\n",
      percent,labs(desty-srcy),
      distanceTraveledY,
      (int)distanceTraveledY,
      (srcy<desty)?desty+distanceTraveledY:desty-distanceTraveledY,
      (srcy<desty)?(int)(desty+distanceTraveledY):(int)(desty-distanceTraveledY));
    */
    double transitx;
    double transity;
    if(srcx<destx){
      transitx=destx+distanceTraveledX;
    }else{
      transitx=destx-distanceTraveledX;
    }
    if(srcy<desty){
      transity=desty+distanceTraveledY;
    }else{
      transity=desty-distanceTraveledY;
    }
    int player=galaxy->event[i].attackerIdx;
    XColor color=colorPixel[player%NUM_COLORS];
    XSetForeground(dpy,gc,color.pixel);
    double theta=0.0;
    for(j=0;j<galaxy->planet[i].ships;j++){
      theta+=PI/(double)(galaxy->event[i].ships+1);
      int x=(int)(r*cos(theta))+(int)(transitx*GRID_SIZE+rand()%r+15);
      int y=(int)(r*sin(theta))+(int)(transity*GRID_SIZE+rand()%r+15);
      XDrawPoint(dpy,w,gc,x,y);
    }
  }
}
void drawGalaxy(galaxyType *galaxy){
  int i,j;
  char s[MAX_BUFFER];
  for(i=0; i<galaxy->planetCount; i++){
    int player=galaxy->planet[i].ownerIdx;
    XColor color;
    if(player!=NO_OWNER){
      color=colorPixel[player%NUM_COLORS];
    }else{
      color=yellow_col;
    }
    XSetForeground(dpy, gc, color.pixel);  
    XFillArc(dpy, w, gc,
	     galaxy->planet[i].x*GRID_SIZE+15,
	     galaxy->planet[i].y*GRID_SIZE+15,
	     galaxy->planet[i].prod*PLANET_SIZE,
	     galaxy->planet[i].prod*PLANET_SIZE,
	     0*64, 360*64);
    sprintf(s,"%c",galaxy->planet[i].name);
    XSetForeground(dpy,gc,white_col.pixel);
    XDrawString(dpy,w,gc,
		galaxy->planet[i].x*GRID_SIZE+GRID_SIZE/4,
		galaxy->planet[i].y*GRID_SIZE+GRID_SIZE/4,
		s,strlen(s));
    sprintf(s,"Turn %d",galaxy->turn);
    XSetForeground(dpy,gc,white_col.pixel);
    XDrawString(dpy,w,gc,
		(galaxy->max_x*GRID_SIZE)/2,
		((galaxy->max_y+1)*GRID_SIZE)-5,
		s,strlen(s));
    
    XSetForeground(dpy, gc, color.pixel);  
    double theta=0.0;
    int r=galaxy->planet[i].prod*PLANET_SIZE;
    for(j=0;j<galaxy->planet[i].ships;j++){
      theta+=2*PI/galaxy->planet[i].ships;
      int x=(int)(r*cos(theta))+(galaxy->planet[i].x*GRID_SIZE)+15+rand()%4+
	galaxy->planet[i].prod*PLANET_SIZE/2;
      int y=(int)(r*sin(theta))+(galaxy->planet[i].y*GRID_SIZE)+15+rand()%4+
	galaxy->planet[i].prod*PLANET_SIZE/2;
      XDrawPoint(dpy,w,gc,x,y);		
    }
  }
}

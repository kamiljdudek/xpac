/*
 * Copyright 1996 P. Warden (wardenp@cs.man.ac.uk)
 * This software may be freely copied and used, but no warranty is given
 */
/* include files *********************************************************/

/* include the files needed for Xlib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

/* include files for io and stdlib & time for the rand function */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* definitions and enumerations ******************************************/

/* application's event mask */
#define MASK ExposureMask | KeyPressMask | StructureNotifyMask | FocusChangeMask

#define MAXOPTION 9

/* maximum number of ghosts */
#define MAXGHOSTS (64)

/* possible values for the playing area map */
enum mtype { EMPTY,WALL,PILL, EATPILL };

/* values for the redraw mask */
enum rtype { CLEAN,DIRTY };

/* possible game states */
enum gstate { INGAME=1, PAUSED=2, START=4, NEWLEVEL=8, WIN=16, EAT=32 };

/* directions */
enum dtype { UP=0, RIGHT, DOWN, LEFT, DEAD };

/* structure for holding colour map information */
struct cstruct {

  int black;
  int white;
  int red;
  int green;
  int blue;
  int darkblue;
  int lightblue;
  int yellow;
  int grey;

};

/* structure to hold position information */
struct point {
  int x;
  int y;
};

struct ghostinfo {
  int x;
  int y;
  int oldx;
  int oldy;
  enum dtype dir;
  XImage *image;
};

union anyptr {

  int *intptr;
  char *charptr;
  float *floatptr;

};

/* function prototypes *************************************************/

void ld_font(XFontStruct **);
void getGC(Window ,GC *,XFontStruct *);
void make_image(Window ),setup_blank(XImage *,int),setup_empty(XImage *);
void setup_wall(XImage *),setup_game(void);
void draw_y_line(XImage *,int,int,int,int);
void update_image_from_map(Window ,GC),update_image(Window, GC);
void setup_map_memory(void),free_memory(void),print_usage(void);
void reset_display_map(void),setup_maze(void);
void maze_draw(int,int,int,int),draw_maze_point(int,int,enum mtype);
void draw_maze_edges(void),draw_x_line(XImage *,int,int,int,int);
void setup_pill(XImage *),add_pills(int,int),setup_pacmen(void);
void setup_pacmanu(void),setup_pacman(XImage *),plot_pacman(Window,GC);
void update_game(void);
struct point newpacpos(void);
void setup_pacmanr(void),setup_pacmand(void),setup_pacmanl(void);
void setup_ghosts(void),setup_ghost(XImage *,int);
void plot_sprite(Window,GC,XImage *,int,int,int,int),plot_ghosts(Window,GC);
void plot_map_tile(Window,GC,int,int);
struct point newghostpos(struct ghostinfo);
void update_pacman_position(void),update_ghost_position(struct ghostinfo *);
void update_ghosts(void);
int valid_position(int,int);
void init_game(void);
int test_collide(int,int,int,int),test_ghost_collide(void);
void plot_ghosts_eat(Window,GC),update_image_eat(Window,GC);
void update_game_eat(void),update_ghosts_eat(void),update_ghosts_eat(void);
struct point newghostpos_eat(struct ghostinfo);
void update_ghost_position_eat(struct ghostinfo *),setup_eatpill(XImage *);

/* global variables ****************************************************/

/* variable to hold the game's current state */
enum gstate gamestate;

/* variable to store closest pixel values to desired colours */
struct cstruct cvals;

/* variables to hold the current display and screen */
Display *display;
int screen;

/* program name (from argv[0]) */
static char *pname;

/* pointers to the various x format images used in this program */
XImage *empty,*wall,*pill,*pacmanu2,*pacmanu1,*pacmanu0,*pacmanr2,*pacmanr1,
*pacmanr0,*pacmand2,*pacmand1,*pacmand0,*pacmanl2,*pacmanl1,*pacmanl0,*ghostr;
XImage *ghostw,*ghostb,*ghostg,*ghostgy,*eatpill;

/* pointers to the data areas for all the images */
char *emptyd,*walld,*pilld,*pacmanu2d,*pacmanu1d,*pacmanu0d,*pacmanr2d,
*pacmanr1d,*pacmanr0d,*pacmand2d,*pacmand1d,*pacmand0d,*pacmanl2d,*pacmanl1d,
*pacmanl0d,*ghostrd,*ghostwd,*ghostbd,*ghostgd,*ghostgyd,*eatpilld;

/* pointer to permanent map */
enum mtype *map;

/* pointer to map used for display purposes, & which bank is currently used */
enum mtype *dmap;
int cdmap;

/* bit depth all graphics are to be created in */
int depth;

/* size of game area, in 16 pixel units */
int width=33,height=33;

/* difficulty level */
int difficulty=0;

/* random seed */
int randseed=5755585;

/* number of pills left on screen */
int pillnum;

/* pacman's postion */
struct point pacpos;
struct point oldpacpos;

/* pacman's direction */
enum dtype pacdir;

/* direction player wants to turn */
enum dtype newdir;

/* array for the ghost's info */
struct ghostinfo ghosts[MAXGHOSTS];

/* number of ghosts to start with */
int ghostnum=4;

/* current number of ghosts */
int cghostnum;

/* time left to eat ghosts */
int ecount;

/* main code **************************************************************/

/* function to control the main execution of the program */

int main(int argc, char *argv[]) {

  Window win; /* id of window */
  int wwidth,wheight,x,y,border_width=4; /* window geometry information */
  /* title bar string */
  char *window_title;
  XSizeHints size_hints; /* desired window size */
  XWMHints wm_hints; /* general hints for the window manager */
  XClassHint class_hints; /* class hints for the window manager */
  XTextProperty window_title_property; /* window name as a property */

  XEvent event; /* structure to hold events */
  GC gc; /* holds the graphics context id */
  XFontStruct *font; /* handle to the current font */
  char *display_name=NULL; /* current display name */
  
  char buffer[20]; /* buffer for decoding key events */
  int bufsize=20; /* buffer size */
  KeySym key; /* key symbol for decoding key events */
  XComposeStatus compose; /* needed for decoding key events */
  /* player`s keys */
  char upkey='\'',rightkey='z',downkey='/',leftkey='x',quitkey='q'; 
  int count; /* counter for arg parsing loop */
  int rate=15;
  int wait;

  struct timeval newt,oldt;
  struct timezone dontcare;
  int newtval,oldtval;

  enum optnum { WIDTH, HEIGHT, LEFTKEY, RIGHTKEY, UPKEY, DOWNKEY, QUITKEY,
  LEVEL, FRAMERATE };
  char *optstr[MAXOPTION]={"width", "height", "leftkey", "rightkey",
"upkey", "downkey", "quitkey", "level", "framerate"};
  union anyptr optadr[MAXOPTION];
  enum otype { NUM, STR, TGL, CHAR } opttype[MAXOPTION]={NUM,NUM,CHAR,CHAR,CHAR,
CHAR,CHAR,NUM,NUM };
  char clargstr[20];
  int clrecognised,clarg,printusage=False;

  optadr[WIDTH].intptr=&width;
  optadr[HEIGHT].intptr=&height;
  optadr[LEFTKEY].charptr=&leftkey;
  optadr[RIGHTKEY].charptr=&rightkey;
  optadr[UPKEY].charptr=&upkey;
  optadr[DOWNKEY].charptr=&downkey;
  optadr[QUITKEY].charptr=&quitkey;
  optadr[LEVEL].intptr=&difficulty;
  optadr[FRAMERATE].intptr=&rate;

  /* set program name to that given on command line */
  pname=argv[0];

  /* parse the command line arguments */
  clarg=1;

  while (clarg<argc) {

    clrecognised=False;
    (void) sscanf(argv[clarg]+1,"%[A-Za-z]",clargstr);

    for (count=0; count<MAXOPTION; count++) {
      if (strcmp(optstr[count],clargstr)==0) {
        clrecognised=True;

        switch (opttype[count]) {

          case NUM :
            if (sscanf(argv[clarg],"-%*[A-Za-z]%d",optadr[count].intptr)==EOF)
            if (clarg+1<argc)
            if (sscanf(argv[clarg+1],"%d",optadr[count].intptr)!=EOF) clarg++;
            else printf("No value found for option %s\n",clargstr);
            else printf("No value found for option %s\n",clargstr);
            break;

          case STR :
            if (clarg+1<argc)
            if (sscanf(argv[clarg+1],"%s",optadr[count].charptr)!=EOF) clarg++;
            else printf("No string found for option %s\n",clargstr);
            else printf("No string found for option %s\n",clargstr);
            break;

          case TGL :
            *optadr[count].intptr=!*optadr[count].intptr;
            break;

          case CHAR :
            if (clarg+1<argc)
            if (sscanf(argv[clarg+1],"%c",optadr[count].charptr)!=EOF) clarg++;
            else printf("No character found for option %s\n",clargstr);
            else printf("No character found for option %s\n",clargstr);
            break;

        }
      }
    }

    if (!clrecognised) { printf("Option %s not recognised.\n ",clargstr);
                         printusage=True;
                       }
    clarg++;
  
  }

  if (printusage) {
    printf("Usage: %s ",pname);
    for (count=0; count<MAXOPTION; count++) {
      printf("[-%s ",optstr[count]);
      switch (opttype[count]) {

        case NUM:printf("<number>] "); break;
        case STR:printf("<string>] "); break;
        case TGL:printf("] ");break;
        case CHAR:printf("<character>] "); break;

      }
    }
    printf("\n");
  }

  wait=1000/rate;

  window_title=malloc(sizeof(char)*256);

  sprintf(window_title,"XPacman-%c up %c down %c left %c right %c quit use -slow to adjust speed\0",upkey,downkey,leftkey,rightkey,quitkey);
   
  /* connect to the X server, or report an error if not possible */
  if ((display=XOpenDisplay(display_name))==NULL) {
    (void) fprintf(stderr,"%s: can't connect to X server %s.\n",
    pname,display_name);
    exit(0); }

  /* get screen number */
  screen=DefaultScreen(display);

  /* get bit depth for graphics */
  depth=DefaultDepth(display,screen);

  /* set window position */
  x=0;
  y=0;

  /* set window size */
  wwidth=width*16;
  wheight=height*16;

  /* create window on server */
  win=XCreateWindow(display,RootWindow(display,screen),x,y,wwidth,wheight,
  border_width,depth,InputOutput,DefaultVisual(display,screen),0,0);

  /* set size hints for the window manager */
  /* note, this asks the window manager to create a window of an exact size */
  /* if this request is not honoured, it may cause problems playing the game */

  size_hints.flags=PPosition | PSize | PMinSize | PMaxSize;
  size_hints.min_width=wwidth;
  size_hints.min_height=wheight;
  size_hints.max_width=wwidth;
  size_hints.max_height=wheight;

  /* create window_title_property */
  if (XStringListToTextProperty(&window_title,1,&window_title_property)==0)
 {
    (void) fprintf(stderr,"%s: structure window_title_property can't be allocated.\n",pname); }

  /* set various hints */
  wm_hints.initial_state=NormalState;
  wm_hints.input=True;
  wm_hints.flags=StateHint | InputHint;
  class_hints.res_name=pname;
  class_hints.res_class="Snake";

  /* pass hints to the window manager */
  XSetWMProperties(display,win,&window_title_property,NULL,argv,argc,
  &size_hints,&wm_hints,&class_hints);

  /* select wanted events */
  XSelectInput(display,win,MASK);

  /* load font */
  ld_font(&font);

  /* create the graphics context */
  getGC(win,&gc,font);

  /* map window onto display */
  XMapWindow(display,win);

  /* create the game's graphics */
  make_image(win);

  /* initialise the games state */
  gamestate=PAUSED | NEWLEVEL | INGAME;
  init_game();

  /* main game loop */
  while (True) {

  /* reset timing counter */
    if (gettimeofday(&oldt,&dontcare)!=0) {
      fprintf(stderr,"%s: Problem with gettimeofday()!\n",pname);
      exit(1);
    }
    oldtval=1000*oldt.tv_sec+(oldt.tv_usec/1000);

    /* branch control according to game state */

    /* set up new game if NEWLEVEL is set in gamestate, & clear flag */
    if (gamestate & NEWLEVEL) {
      setup_maze();
      setup_game();
      gamestate&=~NEWLEVEL;
      reset_display_map();
      update_image_from_map(win,gc);
    }

    /* restart game afresh if START is set */
    if (gamestate & START) {
      setup_game();
      gamestate&=~START;
      reset_display_map();
      update_image_from_map(win,gc);
    }

    /* only do anything if PAUSED is false in gamestate */
    if (!(gamestate & PAUSED)) {
      if (gamestate & INGAME) {
        if (gamestate & EAT) {

          update_image_eat(win,gc);
          update_game_eat();
          ecount--;
          if (ecount<=0) gamestate&=~EAT;

        } else {

          update_image(win,gc);
          update_game();

        }
      }
    }

    /* this loop here to regulate the game's speed */
    do {

      /* while more events remain, grab an event and process it */
      while (XCheckMaskEvent(display,MASK,&event)) {

        /* switch on event type */
        switch (event.type) {

          case Expose:
            if (event.xexpose.count!=0) break; /*more exposes to come*/
            reset_display_map(); /* get everything redrawn */
            update_image_from_map(win,gc);
            update_image(win,gc); /* draw graphics */
            break;

          case ConfigureNotify:
            /* to be filled in! */
            break;

          case KeyPress:
            /* work out actual key pressed as a string */
            XLookupString(&(event.xkey),buffer,bufsize,&key,&compose);

            if (tolower(buffer[0])==upkey) newdir=UP;
            if (tolower(buffer[0])==rightkey) newdir=RIGHT;
            if (tolower(buffer[0])==downkey) newdir=DOWN;
            if (tolower(buffer[0])==leftkey) newdir=LEFT;

            /* quit if it's the quit key */
            if (tolower(buffer[0])==quitkey) {
              free_memory(); /* free all malloc'd memory */
              XUnloadFont(display,font->fid);
              XFreeGC(display,gc);
              XCloseDisplay(display);
              exit(1); }

             break;

          case FocusIn:
            gamestate&=~PAUSED; break;

          case FocusOut:
            gamestate|=PAUSED; break;  

          default:
            /* do nothing with any other events */
            break;
        } /* end of event switch */
      } /* end of event while */

      /* no more events to process */

      if (gettimeofday(&newt,&dontcare)!=0) {
        fprintf(stderr,"%s: Problem with gettimeofday()!\n",pname);
        exit(1);
      }
      newtval=1000*newt.tv_sec+(newt.tv_usec/1000);

    } while (newtval<oldtval+wait); /* don't end until looped speed times */

  } /* end main game loop */

} /* end of main function */

/* function to load the font for x */
void ld_font(XFontStruct **font) {

  char *fontname="9x15"; /* name of font to load */

  if ((*font=XLoadQueryFont(display,fontname))==NULL) {
    (void) fprintf(stderr,"%s: cannot open font '9x15'.\n",pname);
    exit(-1);
  }
}

/* function to make and return a graphics context */
void getGC(Window win, GC *gc, XFontStruct *font) {

  unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
  XGCValues values;
  unsigned int line_width = 1;
  int line_style = LineSolid;
  int cap_style = CapRound;
  int join_style = JoinRound;
  int dash_offset = 0;
  static char dash_list[] = {12, 24};
  int list_length = 2;

  /* Create default Graphics Context */
  *gc = XCreateGC(display, win, valuemask, &values);

  /* specify font */
  XSetFont(display, *gc, font->fid);

  /* specify black foreground */
  XSetForeground(display, *gc, BlackPixel(display,screen));

  /* set line attributes */
  XSetLineAttributes(display, *gc, line_width, line_style, 
			cap_style, join_style);

  /* set dashes */
  XSetDashes(display, *gc, dash_offset, dash_list, list_length);

}

/* routine to set up all the graphics related things */
void make_image(Window win) {

  XWindowAttributes a; /* needed to get the colormap */
  XColor b,c; /* for the return values of allocated colours */

  /* get the window attributes, so you can get the colour map */
  XGetWindowAttributes(display,win,&a);

  /* allocate memory for the image data */
  emptyd=(char *) malloc(sizeof(char)*4*16*16);
  walld=(char *) malloc(sizeof(char)*4*16*16);
  pilld=(char *) malloc(sizeof(char)*4*16*16);
  pacmanu2d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanu1d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanu0d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanr2d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanr1d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanr0d=(char *) malloc(sizeof(char)*4*16*16);
  pacmand2d=(char *) malloc(sizeof(char)*4*16*16);
  pacmand1d=(char *) malloc(sizeof(char)*4*16*16);
  pacmand0d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanl2d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanl1d=(char *) malloc(sizeof(char)*4*16*16);
  pacmanl0d=(char *) malloc(sizeof(char)*4*16*16);
  ghostrd=(char *) malloc(sizeof(char)*4*16*16);
  ghostwd=(char *) malloc(sizeof(char)*4*16*16);
  ghostbd=(char *) malloc(sizeof(char)*4*16*16);
  ghostgd=(char *) malloc(sizeof(char)*4*16*16);
  ghostgyd=(char *) malloc(sizeof(char)*4*16*16);
  eatpilld=(char *) malloc(sizeof(char)*4*16*16);

  /* create the blank images */

  empty=XCreateImage(display,a.visual,depth,ZPixmap,0,emptyd,16,16,8,0);
  wall=XCreateImage(display,a.visual,depth,ZPixmap,0,walld,16,16,8,0);
  pill=XCreateImage(display,a.visual,depth,ZPixmap,0,pilld,16,16,8,0);
  pacmanu2=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanu2d,16,16,8,0);
  pacmanu1=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanu1d,16,16,8,0);
  pacmanu0=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanu0d,16,16,8,0);
  pacmanr2=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanr2d,16,16,8,0);
  pacmanr1=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanr1d,16,16,8,0);
  pacmanr0=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanr0d,16,16,8,0);
  pacmand2=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmand2d,16,16,8,0);
  pacmand1=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmand1d,16,16,8,0);
  pacmand0=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmand0d,16,16,8,0);
  pacmanl2=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanl2d,16,16,8,0);
  pacmanl1=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanl1d,16,16,8,0);
  pacmanl0=XCreateImage(display,a.visual,depth,ZPixmap,0,pacmanl0d,16,16,8,0);
  ghostr=XCreateImage(display,a.visual,depth,ZPixmap,0,ghostrd,16,16,8,0);
  ghostw=XCreateImage(display,a.visual,depth,ZPixmap,0,ghostwd,16,16,8,0);
  ghostb=XCreateImage(display,a.visual,depth,ZPixmap,0,ghostbd,16,16,8,0);
  ghostg=XCreateImage(display,a.visual,depth,ZPixmap,0,ghostgd,16,16,8,0);
  ghostgy=XCreateImage(display,a.visual,depth,ZPixmap,0,ghostgyd,16,16,8,0);
  eatpill=XCreateImage(display,a.visual,depth,ZPixmap,0,eatpilld,16,16,8,0);

  /* get the closest colours available to the ones wanted */

  XAllocNamedColor(display,a.colormap,"black",&b,&c);
  cvals.black=b.pixel;
  XAllocNamedColor(display,a.colormap,"white",&b,&c);
  cvals.white=b.pixel;
  XAllocNamedColor(display,a.colormap,"red",&b,&c);
  cvals.red=b.pixel;
  XAllocNamedColor(display,a.colormap,"green",&b,&c);
  cvals.green=b.pixel;
  XAllocNamedColor(display,a.colormap,"slate blue",&b,&c);
  cvals.blue=b.pixel;
  XAllocNamedColor(display,a.colormap,"blue",&b,&c);
  cvals.darkblue=b.pixel;
  XAllocNamedColor(display,a.colormap,"sky blue",&b,&c);
  cvals.lightblue=b.pixel;
  XAllocNamedColor(display,a.colormap,"yellow",&b,&c);
  cvals.yellow=b.pixel;
  XAllocNamedColor(display,a.colormap,"grey",&b,&c);
  cvals.grey=b.pixel;

  /* ensure graphics can be seen on a monochrome screen */
  if (depth==1) {

    cvals.black=BlackPixel(display,screen);
    cvals.white=WhitePixel(display,screen);
    cvals.red=WhitePixel(display,screen);
    cvals.green=WhitePixel(display,screen);
    cvals.blue=WhitePixel(display,screen);
    cvals.darkblue=WhitePixel(display,screen);
    cvals.lightblue=WhitePixel(display,screen);
    cvals.yellow=WhitePixel(display,screen);
    cvals.grey=WhitePixel(display,screen);

  }

  /* draw the graphics onto the images */

  setup_empty(empty);
  setup_wall(wall);
  setup_pill(pill);
  setup_pacmen();
  setup_ghosts();
  setup_eatpill(eatpill);

}
      
/* paints an image completely in a colour */
void setup_blank(XImage *animage, int colour) {
  int x,y;

  for (y=0; y<16; y++) {
    for (x=0; x<16; x++) {
      XPutPixel(animage,x,y,colour);
    }
  }

}

void setup_empty(XImage *animage) {

  setup_blank(animage,cvals.black);

}

void setup_wall(XImage *animage) {

  setup_blank(animage,cvals.blue);

  draw_x_line(animage,0,0,14,cvals.lightblue);
  draw_y_line(animage,0,0,14,cvals.lightblue);

  draw_x_line(animage,15,1,15,cvals.darkblue);
  draw_y_line(animage,15,1,15,cvals.darkblue);  


}

/* setup various game things */
void setup_game() {

  int c;

  ghostnum=((width*height)/127)+difficulty;

  add_pills(width/2,height/2);

  /* sets pacman's starting position & direction */
  pacpos.x=(width/2)*16;
  pacpos.y=(height/2)*16;
  pacdir=UP;

  for (c=0; c<ghostnum; c++) {

    switch (c%4) {

      case 0:
        ghosts[c].x=(width/2)*16;
        ghosts[c].y=0;
        ghosts[c].oldx=(width/2)*16;
        ghosts[c].oldy=0;
        ghosts[c].dir=DOWN;
        ghosts[c].image=ghostr;
        break;

      case 1:
        ghosts[c].x=(width/2)*16;
        ghosts[c].y=(height-1)*16;
        ghosts[c].oldx=(width/2)*16;
        ghosts[c].oldy=(height-1)*16;
        ghosts[c].dir=UP;
        ghosts[c].image=ghostw;
        break;

      case 2:
        ghosts[c].x=0;
        ghosts[c].y=(height/2)*16;
        ghosts[c].oldx=0;
        ghosts[c].oldy=(height/2)*16;
        ghosts[c].dir=RIGHT;
        ghosts[c].image=ghostb;
        break;

      case 3:
        ghosts[c].x=(width-1)*16;
        ghosts[c].y=(height/2)*16;
        ghosts[c].oldx=(width-1)*16;
        ghosts[c].oldy=(height/2)*16;
        ghosts[c].dir=LEFT;
        ghosts[c].image=ghostg;
        break;
    }

  }

}

/* draw a line on an image straight down */
void draw_y_line(XImage *animage,int x,int ystart,int yend,int col)
{
  int c;

  for (c=ystart; c<=yend; c++) XPutPixel(animage,x,c,col);

}

/* draw the graphics from the map onto the screen */
void update_image_from_map(Window win,GC gc) {

  int x,y;

  for (y=0;y<(height*16);y+=16) {
    for (x=0;x<(width*16);x+=16) {

      /* only draw if the display map has changed since last time updated */
      if (*(dmap+((x/16)*height)+(y/16)) == DIRTY ) {

        *(dmap+((x/16)*height)+(y/16)) = CLEAN;

        switch (*(map+((x/16)*height)+(y/16))) {

          case EMPTY : XPutImage(display,win,gc,empty,0,0,x,y,16,16); break;
          case WALL : XPutImage(display,win,gc,wall,0,0,x,y,16,16); break;
          case PILL : XPutImage(display,win,gc,pill,0,0,x,y,16,16); break;
          case EATPILL : XPutImage(display,win,gc,eatpill,0,0,x,y,16,16);break; 
 
        } 
      }
    }
  }
}

/* update the display map, and draw it */
void update_image(Window win,GC gc) {

  plot_pacman(win,gc);

  plot_ghosts(win,gc);

  /* wait until the graphics have appeared */
  XSync(display,False);

}

/* update the display map, and draw it */
void update_image_eat(Window win,GC gc) {

  plot_pacman(win,gc);

  plot_ghosts_eat(win,gc);

  /* wait until the graphics have appeared */
  XSync(display,False);

}

/* get the memory needed for the maps */
void setup_map_memory(void) {
 
  map=(enum mtype *) malloc(sizeof(enum mtype)*width*height);
  dmap=(enum rtype *) malloc(sizeof(enum rtype)*width*height);

  if ((map==NULL) || (dmap==NULL)) {
    (void) fprintf(stderr,"%s: Unable to allocate needed memory.\n",pname);
    exit(1); }

}

/* free all allocated memory */
void free_memory(void) {

  /* free image data */
  free(emptyd);
  free(walld);

  /* free map data */
  free(map);
  free(dmap);

}

void print_usage(void) {

  printf("Usage: %s [-w<width>] [-h<height>] [-l<left key>]",pname);
  printf(" [-r<right key>] [-u<up key>] [-d<down key>] [-q<quitkey>] ");
  printf("[-s<slowdown factor>] [-g<difficulty level>]\n");

}

void setup_maze(void) {

  int cx,cy,x,y,w,h,p;

  for (x=0; x<width; x++) {
    for (y=0; y<height; y++) {
      *(map+(x*height)+y)=WALL;
    }
   }

  maze_draw(0,0,width/2,height/2);

  draw_maze_edges();

  pillnum=0;

}

void reset_display_map(void) {

  int x,y;

  for (x=0; x<width; x++) {
    for (y=0; y<height; y++) {
      *(dmap+(x*height)+y)=DIRTY;
    }
   }

}

int get_rand(void) {

  randseed^=(randseed<<13) | (randseed>>19);
  randseed+=1;

  return (randseed<0) ? -randseed : randseed;

}

void draw_passage_rectangle(int xs,int ys,int xe, int ye) {

  int cx,cy;

  for (cx=xs; cx<=xe; cx++) {

    *(map+(cx*height)+ys)=EMPTY;
    *(map+((width-1-cx)*height)+ys)=EMPTY;
    *(map+(cx*height)+(height-1-ys))=EMPTY;
    *(map+((width-1-cx)*height)+(height-1-ys))=EMPTY;

    *(map+(cx*height)+ye)=EMPTY;
    *(map+((width-1-cx)*height)+ye)=EMPTY;
    *(map+(cx*height)+(height-1-ye))=EMPTY;
    *(map+((width-1-cx)*height)+(height-1-ye))=EMPTY;

  }

  for (cy=ys; cy<=ye; cy++) {

    *(map+(xs*height)+cy)=EMPTY;
    *(map+((width-1-xs)*height)+cy)=EMPTY;
    *(map+(xs*height)+(height-1-cy))=EMPTY;
    *(map+((width-1-xs)*height)+(height-1-cy))=EMPTY;

    *(map+(xe*height)+cy)=EMPTY;
    *(map+((width-1-xe)*height)+cy)=EMPTY;
    *(map+(xe*height)+(height-1-cy))=EMPTY;
    *(map+((width-1-xe)*height)+(height-1-cy))=EMPTY;

  }

}

void maze_draw(int sx,int sy,int ex,int ey) {

  int nx,ny;

  draw_passage_rectangle(sx,sy,ex,ey);

  if ((ex-sx)*(ey-sy) > 50) {

    if (((get_rand()&1)==0)&&((ex-sx)>4)) {

      nx=sx+(get_rand()%(ex-sx-4))+2;

      maze_draw(sx,sy,nx,ey);
      maze_draw(nx,sy,ex,ey);

    } else if ((ey-sy)>4) {

      ny=sy+(get_rand()%(ey-sy-4))+2;

      maze_draw(sx,sy,ex,ny);
      maze_draw(sx,ny,ex,ey);

    }

  }

}

void draw_maze_point(int x,int y,enum dtype value) {

  *(map+(x*height)+y)=value;
  *(map+((width-1-x)*height)+y)=value;
  *(map+(x*height)+(height-1-y))=value;
  *(map+((width-1-x)*height)+(height-1-y))=value;

}

void draw_maze_edges() {

  int flag=0,x,y;

  x=0;
  for (y=(height/2)-1; y>0; y--) {

    if (*(map+((x+1)*height)+y)==EMPTY) flag=(flag==0)? 1 : 0;

    if ((*(map+((x+1)*height)+y)==WALL)&&(flag==1)) draw_maze_point(x,y,WALL);

  }

  x=0; y=0;
  if (flag==1) draw_maze_point(x,y,WALL);

  for (x=1; x<(width/2); x++) {

    if (*(map+(x*height)+y+1)==EMPTY) flag=(flag==0)? 1 : 0;

    if ((*(map+(x*height)+y+1)==WALL)&&(flag==1)) draw_maze_point(x,y,WALL);

  }

  x=(width/2)-1; y=0;

  while ((flag==1) && (x>=0)) {

    draw_maze_point(x,y,EMPTY);

    if (*(map+(x*height)+y+1)==EMPTY) flag=(flag==0)? 1 : 0;

    x--;

  }

  x=0; y=1;

  while ((flag==1) && (y<(height/2))) {

    draw_maze_point(x,y,EMPTY);

    if (*(map+((x+1)*height)+y)==EMPTY) flag=(flag==0)? 1 : 0;

    y++;

  }


}

void draw_x_line(XImage *animage,int y,int xstart,int xend,int col)
{
  int c;

  for (c=xstart; c<=xend; c++) XPutPixel(animage,c,y,col);

}

void setup_pill(XImage *animage) {

  setup_blank(animage,cvals.black);

  draw_x_line(animage,5,6,8,cvals.green);
  draw_x_line(animage,6,5,9,cvals.green);
  draw_x_line(animage,7,5,9,cvals.green);
  draw_x_line(animage,8,5,9,cvals.green);
  draw_x_line(animage,9,6,8,cvals.green);

}

void add_pills(int x, int y) {

  if ((*(map+(x*height)+y)==EMPTY)&&(x>=0)&&(x<width)&& (y>=0)&&(y<height)) {

    pillnum++;

    *(map+(x*height)+y)=((get_rand()&127)==0) ? EATPILL : PILL;

    add_pills(x-1,y); add_pills(x+1,y); add_pills(x,y-1); add_pills(x,y+1);

  }

}

void setup_pacmen(void) {

  setup_pacmanu();
  setup_pacmanr();
  setup_pacmand();
  setup_pacmanl();

}

void setup_pacmanu(void) {

  setup_pacman(pacmanu2);

  draw_y_line(pacmanu2,3,0,3,cvals.black);  
  draw_y_line(pacmanu2,4,0,4,cvals.black);
  draw_y_line(pacmanu2,5,0,5,cvals.black);
  draw_y_line(pacmanu2,6,0,6,cvals.black);
  draw_y_line(pacmanu2,7,0,7,cvals.black);
  draw_y_line(pacmanu2,8,0,7,cvals.black);
  draw_y_line(pacmanu2,9,0,6,cvals.black);
  draw_y_line(pacmanu2,10,0,5,cvals.black);
  draw_y_line(pacmanu2,11,0,4,cvals.black);
  draw_y_line(pacmanu2,12,0,3,cvals.black);

  setup_pacman(pacmanu1);

  draw_y_line(pacmanu1,5,0,3,cvals.black);  
  draw_y_line(pacmanu1,6,0,5,cvals.black);
  draw_y_line(pacmanu1,7,0,7,cvals.black);
  draw_y_line(pacmanu1,8,0,7,cvals.black);
  draw_y_line(pacmanu1,9,0,5,cvals.black);
  draw_y_line(pacmanu1,10,0,3,cvals.black);

  setup_pacman(pacmanu0);

}

void setup_pacman(XImage *animage) {

  setup_blank(animage,cvals.black);

  draw_x_line(animage,1,6,9,cvals.yellow);
  draw_x_line(animage,2,4,11,cvals.yellow);
  draw_x_line(animage,3,3,12,cvals.yellow);
  draw_x_line(animage,4,2,13,cvals.yellow);
  draw_x_line(animage,5,2,13,cvals.yellow);
  draw_x_line(animage,6,1,14,cvals.yellow);
  draw_x_line(animage,7,1,14,cvals.yellow);
  draw_x_line(animage,8,1,14,cvals.yellow);
  draw_x_line(animage,9,1,14,cvals.yellow);
  draw_x_line(animage,10,2,13,cvals.yellow);
  draw_x_line(animage,11,2,13,cvals.yellow);
  draw_x_line(animage,12,3,12,cvals.yellow);
  draw_x_line(animage,13,4,11,cvals.yellow);
  draw_x_line(animage,14,6,9,cvals.yellow);

}

void plot_pacman(Window win, GC gc) {

  XImage *plotimage;

  int x,y;

  switch (pacdir) {

    case UP: switch (((pacpos.x+pacpos.y)/4)%4) {

      case 0: plotimage=pacmanu0; break;
      case 1: plotimage=pacmanu1; break;
      case 2: plotimage=pacmanu2; break;
      case 3: plotimage=pacmanu1; break;

    } break;

    case LEFT: switch (((pacpos.x+pacpos.y)/4)%4) {

      case 0: plotimage=pacmanl0; break;
      case 1: plotimage=pacmanl1; break;
      case 2: plotimage=pacmanl2; break;
      case 3: plotimage=pacmanl1; break;

    } break;

    case DOWN: switch (((pacpos.x+pacpos.y)/4)%4) {

      case 0: plotimage=pacmand0; break;
      case 1: plotimage=pacmand1; break;
      case 2: plotimage=pacmand2; break;
      case 3: plotimage=pacmand1; break;

    } break;

    case RIGHT: switch (((pacpos.x+pacpos.y)/4)%4) {

      case 0: plotimage=pacmanr0; break;
      case 1: plotimage=pacmanr1; break;
      case 2: plotimage=pacmanr2; break;
      case 3: plotimage=pacmanr1; break;

    } break;

  }

  plot_sprite(win,gc,plotimage,pacpos.x,pacpos.y,oldpacpos.x,oldpacpos.y);

}

void update_game(void) {

  enum mtype *p;
  int x,y;

  update_pacman_position();

  update_ghosts();

  x=((pacpos.x+4)/16)%width;
  y=((pacpos.y+4)/16)%height;

  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  x=((pacpos.x+11)/16)%width;
  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  y=((pacpos.y+11)/16)%height;
  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  x=((pacpos.x+4)/16)%width;
  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  if (pillnum<=0) gamestate|=NEWLEVEL;

  if (test_ghost_collide()) {  gamestate|=START; }

}

void update_game_eat(void) {

  enum mtype *p;
  int x,y;

  update_pacman_position();

  update_ghosts_eat();

  x=((pacpos.x+4)/16)%width;
  y=((pacpos.y+4)/16)%height;

  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  x=((pacpos.x+11)/16)%width;
  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  y=((pacpos.y+11)/16)%height;
  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  x=((pacpos.x+4)/16)%width;
  p=(map+(x*height)+y);
  if (*p==PILL) { *p=EMPTY; pillnum--; }
  if (*p==EATPILL) { *p=EMPTY; gamestate|=EAT; ecount=127; pillnum--; }

  if (pillnum<=0) gamestate|=NEWLEVEL;

  if (test_ghost_collide()) ghosts[which_ghost_collide()].dir=DEAD;

}

struct point newpacpos() {

  struct point newpos;

  newpos=pacpos;

  switch (pacdir) {

    case UP: newpos.y-=4; break;
    case LEFT: newpos.x+=4; break;
    case DOWN: newpos.y+=4; break;
    case RIGHT: newpos.x-=4; break;

  }

  newpos.x=(newpos.x+width*16)%(width*16);
  newpos.y=(newpos.y+height*16)%(height*16);

  return newpos;

}

void setup_pacmanr(void) {

  setup_pacman(pacmanr2);

  draw_x_line(pacmanr2,3,0,3,cvals.black);  
  draw_x_line(pacmanr2,4,0,4,cvals.black);
  draw_x_line(pacmanr2,5,0,5,cvals.black);
  draw_x_line(pacmanr2,6,0,6,cvals.black);
  draw_x_line(pacmanr2,7,0,7,cvals.black);
  draw_x_line(pacmanr2,8,0,7,cvals.black);
  draw_x_line(pacmanr2,9,0,6,cvals.black);
  draw_x_line(pacmanr2,10,0,5,cvals.black);
  draw_x_line(pacmanr2,11,0,4,cvals.black);
  draw_x_line(pacmanr2,12,0,3,cvals.black);

  setup_pacman(pacmanr1);

  draw_x_line(pacmanr1,5,0,3,cvals.black);  
  draw_x_line(pacmanr1,6,0,5,cvals.black);
  draw_x_line(pacmanr1,7,0,7,cvals.black);
  draw_x_line(pacmanr1,8,0,7,cvals.black);
  draw_x_line(pacmanr1,9,0,5,cvals.black);
  draw_x_line(pacmanr1,10,0,3,cvals.black);

  setup_pacman(pacmanr0);

}

void setup_pacmand(void) {

  setup_pacman(pacmand2);

  draw_y_line(pacmand2,3,12,15,cvals.black);  
  draw_y_line(pacmand2,4,11,15,cvals.black);
  draw_y_line(pacmand2,5,10,15,cvals.black);
  draw_y_line(pacmand2,6,9,15,cvals.black);
  draw_y_line(pacmand2,7,8,15,cvals.black);
  draw_y_line(pacmand2,8,8,15,cvals.black);
  draw_y_line(pacmand2,9,9,15,cvals.black);
  draw_y_line(pacmand2,10,10,15,cvals.black);
  draw_y_line(pacmand2,11,11,15,cvals.black);
  draw_y_line(pacmand2,12,12,15,cvals.black);

  setup_pacman(pacmand1);

  draw_y_line(pacmand1,5,12,15,cvals.black);  
  draw_y_line(pacmand1,6,10,15,cvals.black);
  draw_y_line(pacmand1,7,8,15,cvals.black);
  draw_y_line(pacmand1,8,8,15,cvals.black);
  draw_y_line(pacmand1,9,10,15,cvals.black);
  draw_y_line(pacmand1,10,12,15,cvals.black);

  setup_pacman(pacmand0);

}

void setup_pacmanl(void) {

  setup_pacman(pacmanl2);

  draw_x_line(pacmanl2,3,12,15,cvals.black);  
  draw_x_line(pacmanl2,4,11,15,cvals.black);
  draw_x_line(pacmanl2,5,10,15,cvals.black);
  draw_x_line(pacmanl2,6,9,15,cvals.black);
  draw_x_line(pacmanl2,7,8,15,cvals.black);
  draw_x_line(pacmanl2,8,8,15,cvals.black);
  draw_x_line(pacmanl2,9,9,15,cvals.black);
  draw_x_line(pacmanl2,10,10,15,cvals.black);
  draw_x_line(pacmanl2,11,11,15,cvals.black);
  draw_x_line(pacmanl2,12,12,15,cvals.black);

  setup_pacman(pacmanl1);

  draw_x_line(pacmanl1,5,12,15,cvals.black);  
  draw_x_line(pacmanl1,6,10,15,cvals.black);
  draw_x_line(pacmanl1,7,8,15,cvals.black);
  draw_x_line(pacmanl1,8,8,15,cvals.black);
  draw_x_line(pacmanl1,9,10,15,cvals.black);
  draw_x_line(pacmanl1,10,12,15,cvals.black);

  setup_pacman(pacmanl0);

}

void setup_ghosts() {

  setup_ghost(ghostr,cvals.red);
  setup_ghost(ghostw,cvals.white);
  setup_ghost(ghostb,cvals.yellow);
  setup_ghost(ghostg,cvals.green);
  setup_ghost(ghostgy,cvals.grey);

}

void setup_ghost(XImage *animage,int colour) {

  setup_blank(animage,cvals.black);

  draw_y_line(animage,3,3,14,colour);
  draw_y_line(animage,4,2,13,colour);
  draw_y_line(animage,5,2,12,colour);
  draw_y_line(animage,6,1,13,colour);
  draw_y_line(animage,7,1,14,colour);
  draw_y_line(animage,8,1,14,colour);
  draw_y_line(animage,9,1,13,colour);
  draw_y_line(animage,10,2,12,colour);
  draw_y_line(animage,11,2,13,colour);
  draw_y_line(animage,12,3,14,colour);

}

void plot_sprite(Window win,GC gc,XImage *animage,int x,int y,int ox,int oy) {

  int mx,my;

  mx=(ox/16);
  my=(oy/16);

  *(dmap+(mx*height)+my)=DIRTY;

  mx=((ox+15)/16)%width;
  *(dmap+(mx*height)+my)=DIRTY;

  my=((oy+15)/16)%height;
  *(dmap+(mx*height)+my)=DIRTY;

  mx=(ox/16);
  *(dmap+(mx*height)+my)=DIRTY;

  mx=(ox/16);
  my=(oy/16);
  plot_map_tile(win,gc,mx*16,my*16);

  mx=((ox+15)/16)%width;
  plot_map_tile(win,gc,mx*16,my*16);

  my=((oy+15)/16)%height;
  plot_map_tile(win,gc,mx*16,my*16);

  mx=(ox/16);
  plot_map_tile(win,gc,mx*16,my*16);

  XPutImage(display,win,gc,animage,0,0,x,y,16,16);

}

void plot_ghosts(Window win,GC gc) {

  int c;

  for (c=0; c<ghostnum; c++) {

    if (ghosts[c].dir!=DEAD)
    plot_sprite(win,gc,ghosts[c].image,ghosts[c].x,ghosts[c].y,ghosts[c].oldx,
    ghosts[c].oldy);

  }

}

void plot_ghosts_eat(Window win,GC gc) {

  int c;

  for (c=0; c<ghostnum; c++) {

    if (ghosts[c].dir!=DEAD) {
      if (ecount<32 && (ecount%2==0)) {
      plot_sprite(win,gc,ghosts[c].image,ghosts[c].x,ghosts[c].y,ghosts[c].oldx,
        ghosts[c].oldy);
      } else {
      plot_sprite(win,gc,ghostgy,ghosts[c].x,ghosts[c].y,ghosts[c].oldx,
        ghosts[c].oldy);
      }

    }

  }

}

void plot_map_tile(Window win,GC gc,int x,int y) {

 if (*(dmap+((x/16)*height)+(y/16)) == DIRTY ) {

  *(dmap+((x/16)*height)+(y/16)) = CLEAN;

    switch (*(map+((x/16)*height)+(y/16))) {

      case EMPTY : XPutImage(display,win,gc,empty,0,0,x,y,16,16); break;
      case WALL : XPutImage(display,win,gc,wall,0,0,x,y,16,16); break;
      case PILL : XPutImage(display,win,gc,pill,0,0,x,y,16,16); break;
      case EATPILL : XPutImage(display,win,gc,eatpill,0,0,x,y,16,16); break; 

    } 
  }
}

struct point newghostpos_eat(struct ghostinfo ghost) {

  struct point newpos;

  newpos.x=ghost.x;
  newpos.y=ghost.y;

  switch (ghost.dir) {

    case UP: newpos.y-=2; break;
    case LEFT: newpos.x+=2; break;
    case DOWN: newpos.y+=2; break;
    case RIGHT: newpos.x-=2; break;

  }

  newpos.x=(newpos.x+width*16)%(width*16);
  newpos.y=(newpos.y+height*16)%(height*16);

  return newpos;

}

struct point newghostpos(struct ghostinfo ghost) {

  struct point newpos;

  newpos.x=ghost.x;
  newpos.y=ghost.y;

  switch (ghost.dir) {

    case UP: newpos.y-=4; break;
    case LEFT: newpos.x+=4; break;
    case DOWN: newpos.y+=4; break;
    case RIGHT: newpos.x-=4; break;

  }

  newpos.x=(newpos.x+width*16)%(width*16);
  newpos.y=(newpos.y+height*16)%(height*16);

  return newpos;

}

void update_pacman_position(void) {

  struct point newpos;
  enum mtype a,b,c,d;
  int x,y;
  enum dtype odir;

  oldpacpos=pacpos;

  if ((pacpos.x+pacpos.y)%16==0) {

    odir=pacdir;
    pacdir=newdir;

    newpos=newpacpos();

    x=(newpos.x/16);
    y=(newpos.y/16);

    a=*(map+(x*height)+y);

    x=((newpos.x+15)/16)%width;
    b=*(map+(x*height)+y);

    y=((newpos.y+15)/16)%height;
    c=*(map+(x*height)+y);

    x=((newpos.x/16)+width)%width;
    d=*(map+(x*height)+y);

    if ((a==WALL)||(b==WALL)||(c==WALL)||(d==WALL)||(newpos.x<0)||
         (newpos.x>((width-1)*16))||(newpos.y<0)||(newpos.y>((height-1)*16)))
      pacdir=odir;

  }


  do {

    newpos=newpacpos();

    x=(newpos.x/16);
    y=(newpos.y/16);

    a=*(map+(x*height)+y);

    x=((newpos.x+15)/16)%width;
    b=*(map+(x*height)+y);

    y=((newpos.y+15)/16)%height;
    c=*(map+(x*height)+y);

    x=((newpos.x/16)+width)%width;
    d=*(map+(x*height)+y);

    if ((a==WALL)||(b==WALL)||(c==WALL)||(d==WALL)||(newpos.x<0)||
         (newpos.x>((width-1)*16))||(newpos.y<0)||(newpos.y>((height-1)*16)))
       { pacdir=(pacdir+1)%4; newdir=pacdir; }

  } while ((a==WALL)||(b==WALL)||(c==WALL)||(d==WALL)||(newpos.x<0)||
         (newpos.x>((width-1)*16))||(newpos.y<0)||(newpos.y>((height-1)*16))); 

  pacpos=newpacpos();

}

void update_ghost_position(struct ghostinfo *ghost) {

  struct point newpos;
  enum dtype olddir;

  if (((get_rand()%2==0))&&(((ghost->x+ghost->y)%16)==0)) {

    olddir=ghost->dir;
    ghost->dir=(ghost->dir+((get_rand()%2)*2)+1)%4;

    newpos=newghostpos(*ghost);
    if (!valid_position(newpos.x,newpos.y)) ghost->dir=olddir;

  }

  do {

    newpos=newghostpos(*ghost);

    if (!valid_position(newpos.x,newpos.y))
       { ghost->dir=(get_rand())%4; }

  } while (!valid_position(newpos.x,newpos.y)); 

  ghost->oldx=ghost->x; ghost->oldy=ghost->y;
  ghost->x=newpos.x; ghost->y=newpos.y;

}

void update_ghost_position_eat(struct ghostinfo *ghost) {

  struct point newpos;
  enum dtype olddir;

  if (((get_rand()%2==0))&&(((ghost->x+ghost->y)%16)==0)) {

    olddir=ghost->dir;
    ghost->dir=(ghost->dir+((get_rand()%2)*2)+1)%4;

    newpos=newghostpos_eat(*ghost);
    if (!valid_position(newpos.x,newpos.y)) ghost->dir=olddir;

  }

  do {

    newpos=newghostpos_eat(*ghost);

    if (!valid_position(newpos.x,newpos.y))
       { ghost->dir=(get_rand())%4; }

  } while (!valid_position(newpos.x,newpos.y)); 

  ghost->oldx=ghost->x; ghost->oldy=ghost->y;
  ghost->x=newpos.x; ghost->y=newpos.y;

}

void update_ghosts(void) {

  int c;

  for (c=0; c<ghostnum; c++) {

    update_ghost_position(&(ghosts[c]));

  }

}

void update_ghosts_eat(void) {

  int c;

  for (c=0; c<ghostnum; c++) {

    update_ghost_position_eat(&(ghosts[c]));

  }

}

int valid_position(nx,ny) {

  enum mtype a,b,c,d;
  int x,y;

  x=(nx/16);
  y=(ny/16);

  a=*(map+(x*height)+y);

  x=((nx+15)/16)%width;
  b=*(map+(x*height)+y);

  y=((ny+15)/16)%height;
  c=*(map+(x*height)+y);

  x=((nx/16)+width)%width;
  d=*(map+(x*height)+y);

  if ((a==WALL)||(b==WALL)||(c==WALL)||(d==WALL)||(nx<0)||
       (nx>((width-1)*16))||(ny<0)||(ny>((height-1)*16))) return False;
  else return True;

}

void init_game(void) {

  time_t curtime;

  curtime=time(0);
  randseed=curtime;

  /* allocates the memory needed for the maps */
  setup_map_memory();

}

int test_collide(ax,ay,bx,by) {

  return (((bx-ax)>-15)&&((bx-ax)<15)&&((by-ay)>-15)&&((by-ay)<15));

}

int test_ghost_collide(void) {

  int c;

  for (c=0; c<ghostnum; c++) {

    if ((ghosts[c].dir!=DEAD) && (test_collide(pacpos.x,pacpos.y,ghosts[c].x,ghosts[c].y))) return True;

  }

  return False;

}

int which_ghost_collide(void) {

  int c;

  for (c=0; c<ghostnum; c++) {

    if ((ghosts[c].dir!=DEAD) &&(test_collide(pacpos.x,pacpos.y,ghosts[c].x,ghosts[c].y))) return c;

  }

  return -1;

}

void setup_eatpill(XImage *animage) {

  setup_blank(animage,cvals.black);

  draw_x_line(animage,4,6,9,cvals.red);
  draw_x_line(animage,5,5,10,cvals.red);
  draw_x_line(animage,6,4,11,cvals.red);
  draw_x_line(animage,7,4,11,cvals.red);
  draw_x_line(animage,8,4,11,cvals.red);
  draw_x_line(animage,9,4,11,cvals.red);
  draw_x_line(animage,10,5,10,cvals.red);
  draw_x_line(animage,11,6,9,cvals.red);

}

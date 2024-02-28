#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <libpynq.h>
#include <stdio.h>

// MACROS
// Scaling
#define BS 10 /* block size */
#define WIDTH (DISPLAY_WIDTH/BS)
#define HEIGHT (DISPLAY_HEIGHT/BS)
// Map dimensions 
#define MAP_WIDTH 72
#define MAP_HEIGHT 72
// UI
#define NUM_BUTTONS 4
#define MAX_OPTIONS 4
#define MAX_OPTION_LENGTH 10
// Players
#define MAX_PLAYERS 4
#define MAX_NAME_LENGTH 10

// CONSOLE
#define RED   "\033[31m"
#define GREEN "\033[32m"
#define BLUE  "\033[34m"
#define RESET "\033[0m"

// Struct definitions
typedef struct option_t {
  uint8_t text[MAX_OPTION_LENGTH];
  int color;
} option_t;

typedef struct _pos_t {
  uint8_t x;
  uint8_t y;
  struct _pos_t * next;
} pos_t;

typedef struct player_t {
  uint8_t id;
  bool player;
  char name[MAX_NAME_LENGTH];
  pos_t * snake;
  int xdir;
  int ydir;
} player_t;

typedef struct _ttl_t {
int x;
int y;
int startTTL;
int ttl;
struct _ttl_t *next;
} ttl_t;

typedef struct coord_t {
  int x;
  int y;
} coord_t;

typedef struct rect_t {
  int tx;
  int ty;
  int bx;
  int by;
} rect_t;

// Display
extern display_t display;
extern coord_t displayOrigin;
extern coord_t displayOriginPrev;

// Global reference for the player
extern player_t p1;
// how many bots and players there are in total
extern int playerCount;
// whether the player has won or not
extern int playerWon;

// FONTS

extern FontxFile fontTitle[2]; //title text
extern uint8_t fontWidthTitle;
extern uint8_t fontHeightTitle;

extern FontxFile fontStandard[2];
extern uint8_t fontWidthStandard; // standard text
extern uint8_t fontHeightStandard;


// FUNCTIONS

// slitherio.c
void play();

// game.c
void set_block(display_t *display, int x, int y, int c, bool erasePrev);
void setBlockSnake(display_t *display, int x, int y, int c, pos_t * prev);
int getScore(pos_t * snake);
void drawMapBorders();
void insertPos (pos_t **list, int x, int y);
pos_t removeFirstPos (pos_t **list);
pos_t *lookupPos (pos_t *list, int x, int y);
void updateFromHead(pos_t * node, pos_t * nodePrev, pos_t * tmp);
pos_t * getHead(pos_t * snake);
ttl_t *lookupTTL (ttl_t *list, int x, int y);
ttl_t *insertTTL (ttl_t *list, int x, int y, int startTTL);
int updateTTL (ttl_t *list);
int removeTTL (ttl_t **list);
void removeMiddleTTL (ttl_t **list, int x, int y);
void resetPlayers(player_t players[MAX_PLAYERS]);

// fonts.c
void fontInit();

// bot.c
coord_t closestBonus(ttl_t * bonus, pos_t head);
void removeClosest(ttl_t ** bonus, coord_t closest);
void trackBonus(ttl_t * bonus, player_t * bot);
void killBot(player_t players[MAX_PLAYERS], player_t * bot);
void updateBot(ttl_t ** bonus, player_t players[MAX_PLAYERS], player_t * bot);

// ui.c
int centralizedStart(uint8_t * text, int fontwidth, int containerWidth, int fontHeight, int containerHeight, int * yStart);
void horizontalBorderPattern(int yDir);
void drawOptions(option_t options[MAX_OPTIONS], int length, int selected);
int optionSelector(option_t options[MAX_OPTIONS]);
void introScreen();
void playAgain(int score, player_t players[MAX_PLAYERS]);

#endif
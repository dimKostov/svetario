#include <libpynq.h>
#include <math.h>
#include "mechanics.h"
#include "declarations.h"

// Global variable definitions ************

// Customization
int delay = 300;
int snakeBodyColor = RGB_GREEN;
int snakeHeadColor = RGB_BLUE;

// Display
display_t display;
coord_t displayOrigin;
coord_t displayOriginPrev;

// Global reference for the player
player_t p1;
// how many bots and players there are in total
int playerCount;
// whether the player has won or not
int playerWon;

// Fonts
FontxFile fontTitle[2]; //title text
uint8_t fontWidthTitle;
uint8_t fontHeightTitle;

FontxFile fontStandard[2];
uint8_t fontWidthStandard; // standard text
uint8_t fontHeightStandard;

// Global variable definitions ************

// Play loop function
void play()
{
  // clear screen before start of game
  displayFillScreen(&display, RGB_BLACK);

  // reset count variable
  playerCount = 0;
  // reset win state
  playerWon = 0;

  // set players array with one player and NUM_BOTS bots
  player_t players[MAX_PLAYERS];
  for(int i = 0; i < MAX_PLAYERS; i++){
    // set player
    if(i == 0) players[i] = (player_t){ i, true, "P1", NULL, 0, -1 };
    else {
      if(i > NUM_BOTS) break;
      char name[MAX_NAME_LENGTH] = {'\0', };
      sprintf(name, "BOT%d", i);
      players[i] = (player_t){ i, false, {'\0', }, NULL, 0, -1 };
      strcpy(players[i].name, name);
    }
    // increment player count for every player
    playerCount++;
  }
  pos_t * snakePrev = NULL;

  // for each player, insert the initial length
  int rx, ry;
  for(int i = 0; i < playerCount; i++){
    // assign unique spawn position

    // get unique coordinates
    while(1){
      // take random x and y coordinates
      rx = ((rand() % 3) * 2 + 1) * 12;
      ry = ((rand() % 3) * 2 + 1) * 12;

      // check if coinciding with another player's spawn location
      int j = i;
      while (j > 0){
        if(players[j - 1].snake->x == rx && players[j - 1].snake->y == ry) break;
        j--;
      }

      // if no conflict, proceed with this spawn location
      if(j == 0) break;
    }

    // assign spawn location and assemble snake length
    // coordinates of snake tail
    int y = ry + (INITIAL_LENGTH - 1); // increment by length such that head is exactlly at the spawn location
    int x = rx;

    // insert the specified initial length of the snake
    for(int k = 0; k < INITIAL_LENGTH; k++){
      if(k < INITIAL_LENGTH - 1) insertPos (&(players[i].snake), x, y--);
      else insertPos (&(players[i].snake), x, y); // HEAD (hence y is currently at head)
    }

    // set display origin to position of player head
    if(players[i].player) displayOrigin = (coord_t){rx, ry};
  } 

  // bonus initialization
  ttl_t *bonus = NULL;

  // iterations counter
  int iterations = 0;

  // growth counter (how many times not to cut snake)
  int growth = 0; // replace in general snake update function

  if(get_switch_state(SWITCH1)){
    // intro 
    introScreen();
  }
  
  do {
    // if all bots have been killed, end the game with a victory
    if(playerCount == NUM_PLAYERS) {
      playerWon = 1;
      goto gameOver;
    }

    // set previous origin to current, effectively before changing it with movement
    displayOriginPrev.x = displayOrigin.x;
    displayOriginPrev.y = displayOrigin.y;

    // PLAYER MOVEMENT
    pos_t head;
    for(int i = 0; i < playerCount; i++){
      head = *(getHead(players[i].snake));

      // if player, look for button input
      if(players[i].player) {

        // create button array to receive input
        int button_states[NUM_BUTTONS] = { 0 };
        // wait delay amount of time to see if there is input in any direction
        sleep_msec_buttons_pushed (button_states, delay);
        // update coordinates if there is input
        if (button_states[0] && players[i].xdir != -1) { players[i].xdir = 1; players[i].ydir = 0; } // RIGHT
        else if (button_states[1] && players[i].ydir != 1) { players[i].xdir = 0; players[i].ydir = -1; } // TOP
        else if (button_states[2] && players[i].ydir != -1) { players[i].xdir = 0; players[i].ydir = 1; } // BOTTOM
        else if (button_states[3] && players[i].xdir != 1) { players[i].xdir = -1; players[i].ydir = 0; } // LEFT

        // print them for testing
        // printf(GREEN "PX: %d\n" RESET, head.x + players[i].xdir);
        // printf(GREEN "PY: %d\n" RESET, head.y + players[i].ydir);
      }
      else { // if bot, then perform bonus tracking algorithm
        trackBonus(bonus, &players[i]);

        // print them for testing
        // printf(RED "B%dX: %d\n" RESET, players[i].id, getHead(players[i].snake)->x + players[i].xdir);
        //printf(RED "B%dY: %d\n" RESET, players[i].id, getHead(players[i].snake)->y + players[i].ydir);
      }
    }

    // get new head
    head = *(getHead(players[0].snake));
    coord_t newHead = (coord_t){head.x + players[0].xdir, head.y + players[0].ydir};

    // END GAME IF 
    // the snake collides with any of the snakes
    for(int i = 0; i < playerCount; i++){
      if(lookupPos(players[i].snake, newHead.x, newHead.y) != NULL){
        goto gameOver;
      } 
    }
    if(newHead.x >= MAP_WIDTH - 2 || newHead.x <= 0 || newHead.y >= MAP_HEIGHT - 2 || newHead.y <= 0) goto gameOver; // the snake coincides with map bounds
    
    // check if bonus was eaten
    if(lookupTTL(bonus, newHead.x, newHead.y) != NULL){
      growth++; // indicate snake should grow
      removeMiddleTTL(&bonus, newHead.x, newHead.y);
    }

    // update head position and display origin
    insertPos(&(players[0].snake), newHead.x, newHead.y);
    displayOrigin.x = newHead.x;
    displayOrigin.y = newHead.y;

    // clear and repopulate the previous snake with the snake before update - for display correction
    while(snakePrev != NULL) removeFirstPos(&snakePrev);
    pos_t * node = players[0].snake;
    while(node != NULL){
      insertPos(&snakePrev, node->x, node->y);
      node = node->next;
    }

    updateFromHead(players[0].snake, snakePrev, snakePrev);

     // erase the tail unless there is growth
    if(growth == 0){
      pos_t tail = removeFirstPos(&(players[0].snake));
      set_block(&display,tail.x,tail.y, RGB_BLACK, false);
    } else 
    {
      growth--;
    }

    // update p1 global reference
    p1 = players[0];

    // update all bot snakes
    for(int i = 1; i < playerCount; i++){ // TODO: set the count for "human" players to a more robust variable
      updateBot(&bonus, players, &(players[i]));
    }

    // update bonus list if probability satisfied
    if(rand() % 100 < BONUS_PROB){
      for(int j = 0; j < BONUS_TRIES_TIMEOUT; j++){
        // spawn around the player (based on dip)
        int kx = rand() % MAP_WIDTH - 1 == 0 ? 1 : rand() % MAP_WIDTH - 1;
        int ky = rand() % MAP_HEIGHT - 1 == 0 ? 1 : rand() % MAP_HEIGHT - 1;
        // insert if there is nothing in the way
        int i = 0;
        while(i < playerCount){
          if(lookupPos(players[i].snake, kx, ky) != NULL) break;
          i++;
        }
        if(i == playerCount && lookupTTL(bonus, kx, ky) == NULL){
          bonus = insertTTL(bonus, kx, ky, rand() % BONUS_MAX_DURATION);
          break;
        }
      }
    }
    
    // decrement and remove per cycle
    updateTTL(bonus);
    removeTTL(&bonus);

    //color every bonus block yellow
    ttl_t * bonusBlock = bonus;
    while(bonusBlock != NULL) {
      set_block(&display, bonusBlock->x, bonusBlock->y, RGB_YELLOW, true);
      bonusBlock = bonusBlock->next;
    }

    // draw map borders in red
    drawMapBorders();

    iterations++;

  } while (true);

  gameOver:

  // calculate score
  int score = getScore(players[0].snake);

  // cleanup of linked lists (regardless of replay)s
  // free snake space for all players
  for(int i = 0; i < playerCount; i++) {
    while(players[i].snake != NULL) removeFirstPos(&players[i].snake);
  }
  
  // free bonus space
  while(bonus != NULL) {
    updateTTL(bonus);
    removeTTL(&bonus);
  }

  // Outro prompt for playing again
  playAgain(score, players);

  display_destroy(&display);
}

int main(void) {
  pynq_init();
  buttons_init();
  switches_init();

  // display initialization
  display_init(&display);
  displaySetFontDirection(&display, TEXT_DIRECTION0);

  // initialize the fonts for use in the UI
  fontInit();

  // initialize play loop
  play();

  // final cleanup
  displayFillScreen(&display, RGB_BLACK);
  switches_destroy();
  buttons_destroy();
  pynq_destroy();
  return EXIT_SUCCESS;
}

#include <libpynq.h>
#include <math.h>
#include "mechanics.h"
#include "declarations.h"

// find closest bonus to bot based on its head position coordinates
coord_t closestBonus(ttl_t * bonus, pos_t head)
{
  coord_t closest = (coord_t){MAP_WIDTH * 3, MAP_WIDTH * 3};
  while(bonus != NULL)
    {
        if(abs(head.x - bonus->x) + abs(head.y - bonus->y) < abs(head.x - closest.x) + abs(head.y - closest.y)) 
        {
            closest = (coord_t){bonus->x, bonus->y};
        }
        bonus = bonus->next;
    }
  return closest;
}

void removeClosest(ttl_t ** bonus, coord_t closest){
  while(*bonus != NULL){
    // remove if less than zero
    if((*bonus)->x == closest.x && (*bonus)->y == closest.y){

      // skip this instance
      *bonus = (*bonus)->next; // RISK: You are overwriting the pointer, will the previous bonus know that and point to it?
    } 
    else{
      bonus = &((*bonus)->next);
    }
  }
}

// the function for bot tracking of bonuses that recursively repeats until position and direction specs are satisfied
void trackBonus(ttl_t * bonus, player_t * bot){
        // variable following where the head is
        pos_t interHead = *(getHead(bot->snake));

        coord_t closest = (coord_t)closestBonus(bonus, interHead); // CHOICE: GETS CLOSEST EVERY TIME, HENCE MAY CHANGE DESTINATION
        // if no bonuses yet
        if(closest.x >= MAP_WIDTH){
          // if close to borders, turn
          if( ((*bot).ydir == -1 && interHead.y <= BOT_BORDER_DISTANCE) || ((*bot).ydir == 1 && MAP_HEIGHT - interHead.y <= BOT_BORDER_DISTANCE) ){ // TOP OR BOTTOM BORDER
            (*bot).ydir = 0;
            (*bot).xdir = interHead.x > MAP_WIDTH - interHead.x ? -1 : 1;
          } 
          else if( ((*bot).xdir == -1 && interHead.x <= BOT_BORDER_DISTANCE) || ((*bot).xdir == 1 || MAP_WIDTH - interHead.x <= BOT_BORDER_DISTANCE) ){ // RIGHT OR LEFT BORDER
            (*bot).xdir = 0;
            (*bot).ydir = interHead.y > MAP_HEIGHT - interHead.y ? -1 : 1;
          } 

          // otherwise, continue travelling in the same direction
        }
        else 
        { // if there is a bonus present
          // decide directions
          int xdir = interHead.x - closest.x > 0 ? -1 : interHead.x - closest.x == 0 ? 0 : 1;
          int ydir = interHead.y - closest.y > 0 ? -1 : interHead.y - closest.y == 0 ? 0 : 1;

          // reusable vars
          pos_t * node = NULL;
          pos_t * snake = NULL;

          // Trace all instances leading to the closest bonus whether the head will collide with the snake

          // HORIZONTAL MAJOR

          // if horizontal direction is not opposite and non-zero
          if(xdir != - bot->xdir && xdir != 0){ 
            // make complete copy of the snake for horizontal tracing
            node = bot->snake;
            snake = NULL;
            while(node != NULL) {
              insertPos(&snake, node->x, node->y);
              node = node->next;
            }

            // horizontal movement tracing
            for(int i = 0; i < abs(interHead.x - closest.x); i++){
              // remove last position indicating movement
              removeFirstPos(&snake);

              // check if the new head would coincide with the snake
              node = snake;
              while(node != NULL){
                if(node->x == getHead(snake)->x + xdir && node->y == getHead(snake)->y) break;
                node = node->next;
              }
              // stop the tracing if a collision has been detected
              if(node != NULL) goto verticalMajor;

              // otherwise march the trace snake forward
              insertPos(&snake, getHead(snake)->x + xdir, getHead(snake)->y);
            }

            // continuing from the current layout of the snake and saving the current head as a constant
            interHead = *(getHead(snake));

            // vertical movement tracing
            for(int i = 0; i < abs(interHead.y - closest.y); i++)
            {
              // remove last position indicating movement
              removeFirstPos(&snake);

              // check if the new head would coincide with the snake
              node = snake;
              while(node != NULL)
                {
                    if(node->x == getHead(snake)->x && node->y == getHead(snake)->y + ydir) break;
                    node = node->next;
                }
              // stop the tracing if a collision has been detected
              if(node != NULL) goto verticalMajor;

              // otherwise march the trace snake forward
              insertPos(&snake, getHead(snake)->x, getHead(snake)->y + ydir);
            }

            // free the snake from the memory used for horizontal major tracing
            while(snake != NULL) removeFirstPos(&snake);

            // if it didn't fail, horizontal major direction decided, function complete
            bot->ydir = 0;
            bot->xdir = xdir;
            return;
          } 

          verticalMajor:
          // free the snake from the memory used for horizontal major tracing
          while(snake != NULL) removeFirstPos(&snake);

          // VERTICAL MAJOR

          // if vertical direction is not opposite and non-zero
          if(ydir != - bot->ydir && ydir != 0)
            {
                // make complete copy of the snake for vertical major tracing
                node = bot->snake;
                snake = NULL;
                while(node != NULL) {
                insertPos(&snake, node->x, node->y);
                node = node->next;
                }

                // reset interHead since change to vertical major
                interHead = *(getHead(snake));

                // vertical movement tracing
                for(int i = 0; i < abs(interHead.y - closest.y); i++){
                  // remove last position indicating movement
                  removeFirstPos(&snake);

                  // check if the new head would coincide with the snake
                  node = snake;
                  while(node != NULL){
                      if(node->x == getHead(snake)->x && node->y == getHead(snake)->y + ydir) break;
                      node = node->next;
                  }
                  // stop the tracing if a collision has been detected
                  if(node != NULL) goto newClosest;

                  // otherwise march the trace snake forward
                  insertPos(&snake, getHead(snake)->x, getHead(snake)->y + ydir);
                }

                // continuing from current snake layout for horizontal movement tracing and saving the current head as a constant 
                interHead = *(getHead(snake));

                // horizontal movement tracing
                for(int i = 0; i < abs(interHead.x - closest.x); i++){
                  // remove last position indicating movement
                  removeFirstPos(&snake);

                  // check if the new head would coincide with the snake
                  node = snake;
                  while(node != NULL){
                      if(node->x == getHead(snake)->x + xdir && getHead(snake)->y == snake->y) break;
                      node = node->next;
                  }
                  // stop the tracing if a collision has been detected
                  if(node != NULL) goto newClosest;

                  // otherwise march the trace snake forward
                  insertPos(&snake, getHead(snake)->x + xdir, getHead(snake)->y);
                }

                // free the snake from the memory used for horizontal tracing
                while(snake != NULL) removeFirstPos(&snake);

                // if it didn't fail, vertical major direction decided, function complete
                bot->ydir = ydir;
                bot->xdir = 0;
                return;
            }

          // all failures of current closest lead here
          newClosest:

          // free the snake from the memory used for vertical tracing
          while(snake != NULL) removeFirstPos(&snake);

          // if neither of them work, opt for different closest
          // remove that bonus from search pool (in a copy of the variable)
          ttl_t * bonusCopy = bonus;
          removeClosest(&bonusCopy, closest);
          trackBonus(bonusCopy, bot);
        }
}

void killBot(player_t players[MAX_PLAYERS], player_t * bot){
    // black out all the positions of the snake if they are visible
    pos_t * node = bot->snake;
    while(node != NULL) {
        set_block(&display, node->x, node->y, RGB_BLACK, true);
        node = node->next;
    }

    // free them pointers
    while(bot->snake != NULL) removeFirstPos(&(bot->snake));
    
    // left pull of all players in the array
    for(int i = bot->id; i < playerCount - 1; i++){
      players[bot->id] = players[bot->id + 1];
    }

    // decrement number of players
    playerCount--;

    // maybe drop loot :D
}

void updateBot(ttl_t ** bonus, player_t players[MAX_PLAYERS], player_t * bot){
    
    // get new head
    pos_t head = *(getHead(bot->snake));
    coord_t newHead = (coord_t){head.x + bot->xdir, head.y + bot->ydir};  

    // KILL BOT IF
    // the snake collides with any of the snakes
    for(int i = 0; i < playerCount; i++){
      if(lookupPos(players[i].snake, newHead.x, newHead.y) != NULL) {
        killBot(players, bot);
        return;
      }
    }
    // the snake coincides with the map bounds
    if(newHead.x >= MAP_WIDTH - 2 || newHead.x <= 0 || newHead.y >= MAP_HEIGHT - 2 || newHead.y <= 0) {
        killBot(players, bot);
        return;
    }
    
    int growth = 0;
    // check if bonus was eaten
    if(lookupTTL(*bonus, newHead.x, newHead.y) != NULL){
      growth++; // indicate snake should grow
      removeMiddleTTL(bonus, newHead.x, newHead.y);
    }

    // update head position and display origin
    insertPos(&(bot->snake), newHead.x, newHead.y);

    //color every bot node
    pos_t * node = bot->snake;
    while(node != NULL) {
        if(!node->next) set_block(&display, node->x, node->y, RGB_BLUE, true);
        else set_block(&display, node->x, node->y, RGB_GREEN, true);
        node = node->next;
    }

    // erase the tail unless there is growth
    if(growth == 0){
      pos_t tail = removeFirstPos(&bot->snake);
      set_block(&display, tail.x, tail.y, RGB_BLACK, false);
    } else 
    {
      growth--;
    }

    // print the score of the bot snake:
    printf(GREEN "%s: %d\n" RESET, bot->name, getScore(bot->snake));
}
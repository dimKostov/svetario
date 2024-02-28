#include <libpynq.h>
#include <math.h>
#include "mechanics.h"
#include "declarations.h"

// GAME

// forward declaration of play
// void play();

// paint a block on the display
void set_block(display_t *display, int x, int y, int c, bool erasePrev) {

  // get the coordinates relative to the display
  int relX = x - (displayOrigin.x - WIDTH / 2);
  int relY = y - (displayOrigin.y - HEIGHT / 2);

  // validate previous and current positions
  bool currentPos = relX < WIDTH && relX >= 0 && relY < HEIGHT && relY >= 0;
  bool previousPos = relX +p1.xdir < WIDTH && relX +p1.xdir >= 0 && relY +p1.ydir < HEIGHT && relY +p1.ydir >= 0;

  // black out previous position if it was on screen
  if(previousPos && erasePrev){
    displayDrawFillRect(display, (relX +p1.xdir)*BS, (relY +p1.ydir)*BS, (relX +p1.xdir + 1)*BS-1, (relY +p1.ydir + 1)*BS-1, RGB_BLACK);
  }

  // shade new position if on screen
  if(currentPos){
    displayDrawFillRect(display, relX*BS, relY*BS, (relX+1)*BS-1,(relY+1)*BS-1, c);
  }
}

void setBlockSnake(display_t *display, int x, int y, int c, pos_t * prev) {

  // REMOVE PREVIOUS, if not NULL
  if(prev != NULL){
      // get the coordinates relative to the previous display origin
      int relXPrev = prev->x - (displayOriginPrev.x - WIDTH / 2);
      int relYPrev = prev->y - (displayOriginPrev.y - HEIGHT / 2); 

      // validate previous position
      bool previousPos = relXPrev < WIDTH && relXPrev >= 0 && relYPrev < HEIGHT && relYPrev >= 0;

      // if visible, remove previous
      if(previousPos){
        displayDrawFillRect(display, relXPrev*BS, relYPrev*BS, (relXPrev+1)*BS-1,(relYPrev+1)*BS-1, RGB_BLACK);
      }
  }

  // ADD CURRENT

  // get the coordinates relative to the display
  int relX = x - (displayOrigin.x - WIDTH / 2);
  int relY = y - (displayOrigin.y - HEIGHT / 2);

  // validate current position
  bool currentPos = relX < WIDTH && relX >= 0 && relY < HEIGHT && relY >= 0;

  // if visible, add current
  if(currentPos){
    displayDrawFillRect(display, relX*BS, relY*BS, (relX+1)*BS-1,(relY+1)*BS-1, c);
  }

}

// get the length of the snake
int getScore(pos_t * snake){
  int score = 0;
  while(snake != NULL){
    score++;
    snake = snake->next;
  }
  return score;
}

// draws the borders around the map indicating the end of the map
void drawMapBorders(){

  // TOP

  // border coordinates
  coord_t t1 = {0, 0};
  coord_t t2 = {MAP_WIDTH, 0};

  // current border
  rect_t top = {
    t1.x - (displayOrigin.x - WIDTH / 2) < 0 ? 0 : t1.x - (displayOrigin.x - WIDTH / 2),
    t1.y - (displayOrigin.y - HEIGHT / 2),
    t2.x - (displayOrigin.x - WIDTH / 2) > WIDTH ? WIDTH - 1 : t2.x - (displayOrigin.x - WIDTH / 2),
    t2.y - (displayOrigin.y - HEIGHT / 2),
  };

  // previous border
  rect_t topPrev = {
    t1.x - (displayOrigin.x - WIDTH / 2) + p1.xdir < 0 ? 0 : t1.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t1.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir,
    t2.x - (displayOrigin.x - WIDTH / 2) + p1.xdir > WIDTH ? WIDTH - 1 : t2.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t2.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir
  };

  // check previous border and remove if visible
  if(topPrev.ty >= 0 && topPrev.by < HEIGHT) {
    displayDrawFillRect(&display, (topPrev.tx)*BS, (topPrev.ty)*BS, (topPrev.bx)*BS-1, (topPrev.by + 1)*BS-1, RGB_BLACK);
  }

  // check current border and display if visible
  if(top.ty >= 0 && top.by < HEIGHT) {
    displayDrawFillRect(&display, (top.tx)*BS, (top.ty)*BS, (top.bx)*BS-1, (top.by + 1)*BS-1, RGB_RED);
  }

// RIGHT

  // border coordinates
  t1 = (coord_t){MAP_WIDTH, 0};
  t2 = (coord_t){MAP_WIDTH, MAP_HEIGHT};

  // current border
  rect_t right = {
    t1.x - (displayOrigin.x - WIDTH / 2),
    t1.y - (displayOrigin.y - HEIGHT / 2) < 0 ? 0 : t1.y - (displayOrigin.y - HEIGHT / 2),
    t2.x - (displayOrigin.x - WIDTH / 2),
    t2.y - (displayOrigin.y - HEIGHT / 2) > HEIGHT ? HEIGHT - 1 : t2.y - (displayOrigin.y - HEIGHT / 2)
  };

  // previous border
  rect_t rightPrev = {
    t1.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t1.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir < 0 ? 0 : t1.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir,
    t2.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t2.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir > HEIGHT ? HEIGHT - 1 : t2.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir
  };

  // check previous border and remove if visible
  if(rightPrev.tx >= 0 && rightPrev.bx < WIDTH) {
    displayDrawFillRect(&display, (rightPrev.tx)*BS, (rightPrev.ty)*BS, (rightPrev.bx + 1)*BS-1, (rightPrev.by)*BS-1, RGB_BLACK);
  }

  // check current border and display if visible
  if(right.tx >= 0 && right.bx < WIDTH) {
    displayDrawFillRect(&display, (right.tx)*BS, (right.ty)*BS, (right.bx + 1)*BS-1, (right.by)*BS-1, RGB_RED);
  }

  // BOTTOM

  // border coordinates
  t1 = (coord_t){0, MAP_HEIGHT};
  t2 = (coord_t){MAP_WIDTH, MAP_HEIGHT};

  // current border
  rect_t bottom = {
    t1.x - (displayOrigin.x - WIDTH / 2) < 0 ? 0 : t1.x - (displayOrigin.x - WIDTH / 2),
    t1.y - (displayOrigin.y - HEIGHT / 2),
    t2.x - (displayOrigin.x - WIDTH / 2) > WIDTH ? WIDTH - 1 : t2.x - (displayOrigin.x - WIDTH / 2),
    t2.y - (displayOrigin.y - HEIGHT / 2)
  };

  // previous border
  rect_t bottomPrev = {
    t1.x - (displayOrigin.x - WIDTH / 2) + p1.xdir < 0 ? 0 : t1.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t1.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir,
    t2.x - (displayOrigin.x - WIDTH / 2) + p1.xdir > WIDTH ? WIDTH - 1 : t2.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t2.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir
  };

  // check previous border and remove if visible
  if(bottomPrev.ty >= 0 && bottomPrev.by < HEIGHT) {
    displayDrawFillRect(&display, (bottomPrev.tx)*BS, (bottomPrev.ty)*BS, (bottomPrev.bx)*BS-1, (bottomPrev.by + 1)*BS-1, RGB_BLACK);
  }

  // check current border and display if visible
  if(bottom.ty >= 0 && bottom.by < HEIGHT) {
    displayDrawFillRect(&display, (bottom.tx)*BS, (bottom.ty)*BS, (bottom.bx)*BS-1, (bottom.by + 1)*BS-1, RGB_RED);
  }

// LEFT

  // border coordinates
  t1 = (coord_t){0, 0};
  t2 = (coord_t){0, MAP_HEIGHT};

  // current border
  rect_t left = {
    t1.x - (displayOrigin.x - WIDTH / 2),
    t1.y - (displayOrigin.y - HEIGHT / 2) < 0 ? 0 : t1.y - (displayOrigin.y - HEIGHT / 2),
    t2.x - (displayOrigin.x - WIDTH / 2),
    t2.y - (displayOrigin.y - HEIGHT / 2) > HEIGHT ? HEIGHT - 1 : t2.y - (displayOrigin.y - HEIGHT / 2)
  };

  // previous border
  rect_t leftPrev = {
    t1.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t1.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir < 0 ? 0 : t1.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir,
    t2.x - (displayOrigin.x - WIDTH / 2) + p1.xdir,
    t2.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir > HEIGHT ? HEIGHT - 1 : t2.y - (displayOrigin.y - HEIGHT / 2) + p1.ydir
  };

  // check previous border and remove if visible
  if(leftPrev.tx >= 0 && leftPrev.bx < WIDTH) {
    displayDrawFillRect(&display, (leftPrev.tx)*BS, (leftPrev.ty)*BS, (leftPrev.bx + 1)*BS-1, (leftPrev.by)*BS-1, RGB_BLACK);
  }

  // check current border and display if visible
  if(left.tx >= 0 && left.bx < WIDTH) {
    displayDrawFillRect(&display, (left.tx)*BS, (left.ty)*BS, (left.bx + 1)*BS-1, (left.by)*BS-1, RGB_RED);
  }
}

// SNAKE STUFF

// inserting the head of the snake
void insertPos (pos_t **list, int x, int y){

  // check if list is empty
  if(*list == NULL){
    *list = (pos_t *)malloc(sizeof(pos_t));
    *(*list) = (pos_t) {x, y, NULL};
  } 
  else{
  // get the last node - the head
  while((*list)->next != NULL)
    {
      if((*list)->x == x && (*list)->y == y) {
        break;
      }
      list = &((*list)->next);
    }

      // allocate memory for the new tail
      pos_t * tail = (pos_t *)malloc(sizeof(pos_t));
      *tail = (pos_t) {x, y, NULL};
      (*list)->next = tail;
  }
}

// removing the tail of the snake
pos_t removeFirstPos (pos_t **list){
  // do nothing if empty
  if(*list == NULL) return (pos_t){-1, -1, NULL};

  // else free memory and reassign
  pos_t * temp = (*list)->next;
  pos_t * toRemove = *list;
  pos_t removed  = *toRemove;
  
  free(toRemove);
  *list = temp;
  return removed;
}

// return posistion pointer of some snake node
pos_t *lookupPos (pos_t *list, int x, int y){

  while(list != NULL){
    if(list->x == x && list->y == y) return list;
    list = list->next;
  }
  return NULL;

}

// Go through the snake recursively, such that snake block updates start from the head
// Do the same for the previous snake to remove it from the display
void updateFromHead(pos_t * node, pos_t * nodePrev, pos_t * tmp){
  if(node == NULL)return;

  if(nodePrev == tmp) nodePrev = NULL;
  else nodePrev = tmp;

  updateFromHead(node->next, nodePrev, tmp != nodePrev ? tmp : tmp->next);
  setBlockSnake(&display, node->x, node->y, (node->next == NULL ? snakeHeadColor /*head*/: snakeBodyColor/*tail*/), nodePrev);
}

// retrieves the last node of the snake - the current head
pos_t * getHead(pos_t * snake){
  if(snake == NULL) return NULL;
  while(snake->next != NULL) snake = snake->next;
  return snake;
}

// BONUS STUFF
// determine if bonus block exists at some location
ttl_t *lookupTTL (ttl_t *list, int x, int y){
  while(list != NULL){
    if(list->x == x && list->y == y) return list;
    list = list->next;
  }
  return NULL;
}

// ordered insert in bonus list
ttl_t *insertTTL (ttl_t *list, int x, int y, int startTTL){
  // check for duplicates
  if(lookupTTL(list, x, y) != NULL) return list;

  if(list == NULL) {
    list = (ttl_t*)malloc(sizeof(ttl_t));
    *list = (ttl_t){x, y, startTTL, startTTL, NULL};
    return list;
  }
  else if(list->ttl >= startTTL){
    // create another pointer to point to the first ttl
    ttl_t * temp = list;
    // allocate new memory to the original pointer variable
    list = (ttl_t*)malloc(sizeof(ttl_t));
    // instantiate the new struct
    *list = (ttl_t){x, y, startTTL, startTTL, temp};
    return list;
  }
  ttl_t * head = list;
  while(list->next != NULL){
    // if next disappears later, then insert new here
    if(list->next->ttl >= startTTL){
      ttl_t * new = (ttl_t*)malloc(sizeof(ttl_t));
      *new = (ttl_t){x, y, startTTL, startTTL, list->next};
      list->next = new;
      // return the original head
      return head;
    }

    // otherwise increment to the next
    list = list->next;
  }

  // if here, then new is second element
  ttl_t * new = (ttl_t*)malloc(sizeof(ttl_t));
  *new = (ttl_t){x, y, startTTL, startTTL, NULL};
  list->next = new;
  return head;
}

// decrement each ttl by 1 on each iteration
int updateTTL (ttl_t *list){
  while(list != NULL){
    (list->ttl)--;
    list = list->next;
  }
  return 0;
}

// remove elements with expired ttl and return the number of removed elements
int removeTTL (ttl_t **list){
  int removed = 0;
  while(*list != NULL){
    // remove if less than zero
    if((*list)->ttl < 0){

      // black out the displayed bonus
      set_block(&display, (*list)->x, (*list)->y, RGB_BLACK, false);

      // free and remove
      ttl_t * next = (*list)->next;
      free(*list);
      *list = next;
      removed++;

    } else{
      list = &((*list)->next);
    }
  }
  return removed;
}

// remove a specific bonus block after being eaten by snake
void removeMiddleTTL (ttl_t **list, int x, int y){
  if(lookupTTL(*list, x, y) == NULL) return;

  // check for first and remove
  if((*list)->x == x && (*list)->y == y){
    // temporarily store next
    ttl_t * next = (*list)->next;
    free(*list);
    *list = next;
    return;
  }

  // else we know it's not the first 
  while((*list)->next != NULL){
    // when found remove
    if((*list)->next->x == x && (*list)->next->y == y) {
      // temporarily store next of next
      ttl_t * next = (*list)->next->next;
      // free the current one
      ttl_t * toRemove = (*list)->next;
      free(toRemove);
      // reassign new next
      (*list)->next = next;

      // due to display following player, clear bonus from display relative to player direction
      set_block(&display, x, y, RGB_BLACK, false);

      return;
    }
    // else increment to next one
    list = &((*list)->next);
  }
}

// resets player params for a new game
void resetPlayers(player_t players[MAX_PLAYERS]){ // TODO: Perhpas make the entire initialization
  for(int i = 0; i < playerCount; i++){
    players[i].xdir = 0;
    players[i].ydir = -1;
  }
}
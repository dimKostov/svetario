#ifndef MECHANICS_H
#define MECHANICS_H

#include <libpynq.h>

// MECHANICS
// how long the snake is initially
#define INITIAL_LENGTH 5 
// the probability that a bonus block spawnws
#define BONUS_PROB 20 
// the maximum duration that a block can stay in number of cycles
#define BONUS_MAX_DURATION 200 
// how many times to try with a random location for a bonus before giving up
#define BONUS_TRIES_TIMEOUT 15

// BOT BEHAVIOR
// how far away does the bot stray from the borders
#define BOT_BORDER_DISTANCE 10
// how many bots are in the game
#define NUM_BOTS 3
// how many human players are in the game
#define NUM_PLAYERS 1

// Customization
extern int delay;
extern int snakeBodyColor;
extern int snakeHeadColor;

#endif
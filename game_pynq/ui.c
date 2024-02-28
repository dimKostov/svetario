#include <libpynq.h>
#include <stdio.h>
#include <pthread.h>
#include "mechanics.h"
#include "declarations.h"

void resetPlayers();

//forward declarations
// void play();

// Get that start x-coordinate (and y-coordinate) such that the text is centralized within its bounds  
int centralizedStart(uint8_t * text, int fontwidth, int containerWidth, int fontHeight, int containerHeight, int * yStart){
  if(containerHeight != 0) *yStart = (containerHeight + fontHeight) / 2;

  return (containerWidth - strlen((char *)text) * fontwidth) / 2;
}

// Draw horizontal snake borders on intro screen
void horizontalBorderPattern(int yDir){
  int width = 16;
  int height = 16;
  int X = 0;
  int Y = yDir == 1 ? height - 1 : DISPLAY_HEIGHT - height;

  // Top pattern
  while(X < 239){
    // first row
    for(int i = 0; i < width; i++){
      displayDrawPixel(&display, X, Y, RGB_GREEN);
      X++;
    }

    if(X >= 239){
      X = 239;
      break;
    }

    // first column
    for(int i = 0; i < height; i++){
      displayDrawPixel(&display, X, Y, RGB_GREEN);
      Y = Y + (-1)*yDir < 0 ? 0 : Y + (-1)*yDir >= DISPLAY_HEIGHT ? DISPLAY_HEIGHT - 1 : Y + (-1)*yDir;
    }
    // second row
    for(int i = 0; i < width; i++){
      displayDrawPixel(&display, X, Y, RGB_GREEN);
      X++;
    }
    // second column
    for(int i = 0; i < height; i++){
      displayDrawPixel(&display, X, Y, RGB_GREEN);
      Y += yDir;
    }
  }  
}

void drawOptions(option_t options[MAX_OPTIONS], int length, int selected){

  int width = ( DISPLAY_WIDTH / length ) - 15;
  int height = 40;
  int margin = (DISPLAY_WIDTH - length * width) / (length + 1);
  int yOffset = 160;

  for(int i = 0; i < length; i++){
    // first black out to reset
    displayDrawFillRect(&display, i * width + (i + 1) * margin, yOffset, (i + 1) * width + (i + 1) * margin - 1, yOffset + height - 1, RGB_BLACK);

    // highlight selected
    if(i == selected){
      displayDrawRect(&display, i * width + (i + 1) * margin, yOffset, (i + 1) * width + (i + 1) * margin - 1, yOffset + height - 1, RGB_GREEN);
    }

    // write out option
    int yStart = 0;
    int xStart = centralizedStart(options[i].text, fontWidthStandard, width, fontHeightStandard, height, &yStart);
    displayDrawString(&display, fontStandard, i * width + (i + 1) * margin + xStart, yOffset + yStart, options[i].text, options[i].color);
  }
}

// returns the index of the option which was selected
int optionSelector(option_t options[MAX_OPTIONS]){

// variable to return
int selected = 0;

// get count 
int count = 0;
while(count < MAX_OPTIONS){
  if(options[count].color == 0) break;
  count++;
}

// draw initial
drawOptions(options, count, selected);

// button loop
while(!get_button_state(BUTTON3)){
  if(get_button_state(BUTTON0)) {
    selected = (count + (selected + 1)) % count;
    drawOptions(options, count, selected);
    wait_until_button_released(BUTTON0);
  }
  else if(get_button_state(BUTTON1)){
    selected = (count + (selected - 1)) % count;
    drawOptions(options, count, selected);
    wait_until_button_released(BUTTON1);
  }
}

// clears options before leave
displayDrawFillRect(&display, 0, 100, DISPLAY_WIDTH - 1, 200, RGB_BLACK);

return selected;
}

// Greeting and game config
void introScreen(){
  // blackout
  displayFillScreen(&display, RGB_BLACK);

  // top border
  horizontalBorderPattern(1);
  horizontalBorderPattern(-1);

  //Greet
  uint8_t gameGreet[] = "Welcome to ";
  uint8_t snakeText[] = "SNAKE";
  uint8_t snakeTextE[] = "E";
  displayDrawString(&display, fontTitle, centralizedStart(gameGreet, fontWidthTitle, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 1.5, gameGreet, RGB_WHITE);
  displayDrawString(&display, fontTitle, centralizedStart(snakeText, fontWidthTitle, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 2.5, snakeText, RGB_GREEN);
  displayDrawString(&display, fontTitle, centralizedStart(snakeText, fontWidthTitle, DISPLAY_WIDTH, 0, 0, NULL) + (strlen((char *)snakeText) - 1) * fontWidthTitle, fontHeightTitle * 2.5, snakeTextE, RGB_BLUE);

  // COLOR QUERRY
  uint8_t colorQuerry[] = "Choose snake color (BTN0/1):";
  displayDrawString(&display, fontStandard, centralizedStart(colorQuerry, fontWidthStandard, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 4, colorQuerry, RGB_WHITE);

  option_t colorOptions[MAX_OPTIONS] = { {"Green", RGB_GREEN}, {"Red", RGB_RED}, {"Blue", RGB_BLUE}, {"GRAY", RGB_GRAY}};
  int color = optionSelector(colorOptions);
  snakeBodyColor = colorOptions[color].color;
  switch (snakeBodyColor)
  {
  case RGB_GREEN:
    snakeHeadColor = RGB_BLUE;
    break;
  
  case RGB_RED:
    snakeHeadColor = RGB_WHITE;
    break;
  
  case RGB_BLUE:
    snakeHeadColor = RGB_PURPLE;
    break;
  
  case RGB_GRAY:
    snakeHeadColor = RGB_CYAN;
    break;
  
  default:
    snakeBodyColor = RGB_GREEN;
    snakeHeadColor = RGB_BLUE;
    break;
  }

  // DIFFICULTY QUERRY
  uint8_t difficultyQuerry[] = "Choose difficulty (BTN0/1):";
  displayDrawString(&display, fontStandard, centralizedStart(difficultyQuerry, fontWidthStandard, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 4, colorQuerry, RGB_WHITE);

  option_t difficultyOptions[MAX_OPTIONS] = { {"Easy", RGB_GREEN}, {"Hard", RGB_YELLOW}, {"Extreme", RGB_RED}, {"-", 0}};
  int difficulty = optionSelector(difficultyOptions);
  switch (difficulty)
  {
  case 0:
    delay = 300;
    break;
  
  case 1:
    delay = 100;
    break;

  case 2:
    delay = 25;
    break;
  
  default:
    delay = 300;
    break;
  }

  // black out screen
  displayFillScreen(&display, RGB_BLACK);

}

// reset and replay
void playAgain(int score, player_t players[MAX_PLAYERS]){
  // Home style
  displayFillScreen(&display, RGB_BLACK);
  horizontalBorderPattern(1);
  horizontalBorderPattern(-1);

  //GAME OVER OR WON
  if(playerWon) {
    uint8_t msg[10] = "YOU WON!";
    displayDrawString(&display, fontTitle, centralizedStart(msg, fontWidthTitle, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 2, msg, RGB_GREEN);
  } else {
    uint8_t msg[10] = "GAME OVER";
    displayDrawString(&display, fontTitle, centralizedStart(msg, fontWidthTitle, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 2, msg, RGB_RED);
  }

  // Score
  uint8_t scoreText[50];

  // check if score is higher than that noted in score.txt (hence check for new record)
  FILE *fh;
  fh = fopen("score.txt", "r");
  int highScore = 0;
  fscanf (fh, "%d", &highScore);
  if(score > highScore){
    // udpate highscore 
    fh = fopen("score.txt", "w");
    fprintf(fh, "%d", score);
    // fflush(fh);
    // print score with exclamation
    sprintf((char *)scoreText, "NEW HIGH SCORE! - %d", score);
    displayDrawString(&display, fontStandard, centralizedStart(scoreText, fontWidthStandard, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 3.5, scoreText, RGB_YELLOW);
  } 
  else 
  { // just print score
    sprintf((char *)scoreText, "Score: %d", score);
    displayDrawString(&display, fontStandard, centralizedStart(scoreText, fontWidthStandard, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 3.5, scoreText, RGB_WHITE);
  }

  fclose(fh);

  // Play again
  uint8_t playAgain[] = "Play again?";
  displayDrawString(&display, fontStandard, centralizedStart(scoreText, fontWidthStandard, DISPLAY_WIDTH, 0, 0, NULL), fontHeightTitle * 4.5, playAgain, RGB_WHITE);

  option_t playAgainOptions[MAX_OPTIONS] = { {"Yes!", RGB_GREEN}, {"Nah", RGB_RED}, {"-", 0}, {"-", 0}};
  int again = optionSelector(playAgainOptions);
  if(again == 0) {
    // reset all players
    resetPlayers(players);
    play();
  }
}
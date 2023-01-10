
#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
#include "images/start.h"
#include "images/ball.h"
#include "images/lose.h"
#include "images/win.h"


/* TODO: */
// Include any header files for title screen or exit
// screen images generated by nin10kit. Example for the provided garbage
// image:
// #include "images/garbage.h"

/* TODO: */
// Add any additional states you need for your app. You are not requried to use
// these specific provided states.
enum gba_state {
  START,
  PLAY,
  WIN,
  LOSE,
};

Brick brickRow[6];
Paddle paddle;
Ball b;
int score = 0;

int main(void) {
  /* TODO: */
  // Manipulate REG_DISPCNT here to set Mode 3. //
  REG_DISPCNT = BG2_ENABLE | MODE3;
  videoBuffer[1208] = 0x7fff;

  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  enum gba_state state = START;

  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons

    /* TODO: */
    // Manipulate the state machine below as needed //
    // NOTE: Call waitForVBlank() before you draw
    waitForVBlank();

    switch (state) {
      case START:
      drawFullScreenImageDMA(start);
      if (KEY_DOWN(BUTTON_START, currentButtons)) {
        state = PLAY;
      }
        break;
      case PLAY:
        drawFullScreenImageDMA(BLACK);
        void gamePlay(enum gba_state *state);
        gamePlay(&state);
        break;
      case WIN:
        drawFullScreenImageDMA(win);
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
        state = PLAY;
        score = 0;
      }
        break;
      case LOSE:
        drawFullScreenImageDMA(lose);
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
          score = 0;
        state = PLAY;
      }
        break;
    }

    previousButtons = currentButtons; // Store the current state of the buttons
  }

  UNUSED(previousButtons); // You can remove this once previousButtons is used
  
  return 0;
}


void graphics(void) {
  
  // Bricks graphics
  u16 colors[6] = {YELLOW, CYAN, MAGENTA, BLUE, GREEN, GRAY};
  int startRow = 50;
  int startColumn = 25;
  for (int brickNum = 0; brickNum < 6; brickNum++) {
    brickRow[brickNum].upperBound = startRow;
    brickRow[brickNum].lowerBound = startRow + 10;
    brickRow[brickNum].leftBound = startColumn;
    brickRow[brickNum].rightBound = startColumn + 30;
    brickRow[brickNum].color = colors[brickNum];
    drawRectDMA(brickRow[brickNum].upperBound, brickRow[brickNum].leftBound, 30, 10, brickRow[brickNum].color);
    startColumn = startColumn + 30;
    }

  // Create paddle
  paddle.upperBound = 150;
  paddle.lowerBound = 159;
  paddle.leftBound = 100;
  paddle.rightBound = 140;
  drawRectDMA(paddle.upperBound, paddle.leftBound, 40, 9, WHITE);


  // ball
  b.upperBound = 145;
  b.lowerBound = 149;
  b.leftBound = 118;
  b.rightBound = 122;
  drawImageDMA(b.upperBound, b.leftBound, 4, 4, ball);

  // SCORE INIT.
  char buffer[51];
  sprintf(buffer, "SCORE:%d", score);
  drawRectDMA(2, 2, 50, 10, BLACK);
  drawString(2, 2, buffer, YELLOW);
  
}


void delay(int n) {
  // delay for n tenths of a second
  volatile int x = 0;
  for (int i=0; i<n*8000; i++)
  x++;
}



void updateScore(void) {
  char buffer[51];
  score++;
  sprintf(buffer, "SCORE:%d", score);
  drawRectDMA(2, 2, 50, 10, BLACK);
  drawString(2, 2, buffer, YELLOW);
}


void gamePlay(enum gba_state *state) {
  graphics();
  u32 button;
  int rd = -1;   // up displacement
  int cd = 1;  // right displacement
  while (1) {
    waitForVBlank();
    button = BUTTONS;
    if (KEY_DOWN(BUTTON_SELECT, button)) {
        *state = START;
        score = 0;
        return;
      }

    if (KEY_DOWN(BUTTON_LEFT, button)) {
      if (paddle.leftBound > 0) {
        drawRectDMA(paddle.upperBound, paddle.leftBound, 40, 9, BLACK);
        paddle.leftBound--;
        paddle.rightBound--;
        drawRectDMA(paddle.upperBound, paddle.leftBound, 40, 9, WHITE);
      }
    }

    if (KEY_DOWN(BUTTON_RIGHT, button)) {
      if (paddle.rightBound < 240) {
        drawRectDMA(paddle.upperBound, paddle.leftBound, 40, 9, BLACK);
        paddle.leftBound++;
        paddle.rightBound++;
        drawRectDMA(paddle.upperBound, paddle.leftBound, 40, 9, WHITE);
      }
    }

    // Ball movement
    drawRectDMA(b.upperBound, b.leftBound, 4, 4, BLACK);
    
    b.upperBound = b.upperBound + rd;
    b.leftBound = b.leftBound + cd;
    b.lowerBound = b.upperBound + 4;
    b.rightBound = b.leftBound + 4;

    // bounce over the borders
    if (b.upperBound < 10) {     // TOP BORDER
      b.upperBound = 10;
      rd = -rd;
    }

    if (b.upperBound > (160 - 4)) { // BOTTOM BORDER
      b.upperBound = (160 - 4);
      rd = -rd;
    }

    if (b.leftBound < 0) {    // LEFT BORDER
      b.leftBound = 0;
      cd = -cd;
    }

    if (b.leftBound > (240 - 4)) {    // RIGHT BORDER
      b.leftBound = (240 - 4);
      cd = -cd;
    }

    // brick bounce and "disable" color

    for (int i = 0; i < 6; i++) {
     
      // bottom side
      if (b.upperBound == brickRow[i].lowerBound && b.leftBound < brickRow[i].rightBound && b.rightBound > brickRow[i].leftBound) {
        if (brickRow[i].color != BLACK) {
          brickRow[i].color = BLACK;
          updateScore();
          rd = -rd;
          drawRectDMA(brickRow[i].upperBound, brickRow[i].leftBound, 30, 10, BLACK);
        }
      }

      //top side
      if (b.lowerBound == brickRow[i].upperBound && b.leftBound < brickRow[i].rightBound && b.rightBound > brickRow[i].leftBound) {
        if (brickRow[i].color != BLACK) {
          brickRow[i].color = BLACK;
          updateScore();
          rd = -rd;
          drawRectDMA(brickRow[i].upperBound, brickRow[i].leftBound, 30, 10, BLACK);
        }
      }

      //left side
      if (b.rightBound == brickRow[i].leftBound && b.upperBound < brickRow[i].lowerBound && b.lowerBound > brickRow[i].upperBound) {
        if (brickRow[i].color != BLACK) {
          brickRow[i].color = BLACK;
          updateScore();
          cd = -cd;
          drawRectDMA(brickRow[i].upperBound, brickRow[i].leftBound, 30, 10, BLACK);
        }
      }

      //right side
      if (b.leftBound == brickRow[i].rightBound && b.upperBound < brickRow[i].lowerBound && b.lowerBound > brickRow[i].upperBound) {
        if (brickRow[i].color != BLACK) {
          brickRow[i].color = BLACK;
          updateScore();
          cd = -cd;
          drawRectDMA(brickRow[i].upperBound, brickRow[i].leftBound, 30, 10, BLACK);
        }
      }
    }

    // bounce over the paddle
    if ((b.leftBound + 4) > paddle.leftBound && b.leftBound < paddle.rightBound && (b.upperBound + 4) == paddle.upperBound) {
      rd = -rd;
      cd = cd;
    } 



    // lose case

    if (b.upperBound > 154) {
      *state = LOSE;
      return;
    }

    // win case 

    if (score == 6) {
      *state = WIN;
      return;
    }

    drawImageDMA(b.upperBound, b.leftBound, 4, 4, ball);
    delay(1);  
  }
  UNUSED(state);
}

#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverMAX7219.h>
#include "OneButton.h"

unsigned long lastTimeChanged = millis();
unsigned long stepDelay = 700;
int lastDirection = 4;

MAX7219<1, 1, 9> mtrx; // CS PIN D9, DIN PIN D13, CLK PIN D11
LiquidCrystal_I2C lcd(0x27, 16, 2);

OneButton leftButton(4, true);  // left button on d4 pin
OneButton rightButton(6, true); // right button on d6 pin
OneButton upButton(7, true);    // up button on d7 pin
OneButton downButton(5, true);  // down button on d5 pin

const char lose_text[] PROGMEM = "GAME OWER";

bool gameIsStarted = true;
bool gameIsOver = false;

struct SnakeNode
{
  int x;
  int y;
  // int direction; // 1-top; 2-bot; 3-left; 4-right
};

struct SnakeListNode
{
  int x;
  int y;

  SnakeListNode *next;
  // int direction; // 1-top; 2-bot; 3-left; 4-right
};

SnakeListNode *snakeHead;

size_t snakeSize;

SnakeNode snake[] = {
    {x : 4, y : 4},
    {x : 3, y : 4},
};

void addNewSnake(int x, int y)
{
  SnakeListNode *newHead = new SnakeListNode();
  newHead->next = snakeHead;
  newHead->x = x;
  newHead->y = y;
  snakeHead = newHead;
  snakeSize++;
}

void initStartScreen()
{
  lcd.setCursor(0, 0);
  lcd.print("Press any key");
  lcd.setCursor(0, 1);
  lcd.print("for start...");
}

void initLoadingScreen()
{
  gameIsStarted = true;
  mtrx.clear();
  mtrx.clearDisplay();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score: ");
  lcd.setCursor(7, 0);
  // lcd.print(getCurrentSize());
  mtrx.dot(4, 4);
  mtrx.dot(3, 4);
  mtrx.update();
}

void startGame()
{
  if (!gameIsStarted)
  {
    initLoadingScreen();
    // createSnake(3, 4);
    return;
  }
}

void moveSnake(int direction)
{
  if (lastDirection == 2 && direction == 1)
    direction = 2;
  else if (lastDirection == 1 && direction == 2)
    direction = 1;
  else if (lastDirection == 3 && direction == 4)
    direction = 3;
  else if (lastDirection == 4 && direction == 3)
    direction = 4;
  const int len = sizeof(snake) / sizeof(snake[0]);
  int prevX = snake[0].x;
  int prevY = snake[0].y;
  // 1-top; 2-bot; 3-left; 4-right
  switch (direction)
  {
  case 1:
    snake[0].y = snake[0].y + 1 == 8 ? 0 : snake[0].y + 1;
    break;
  case 2:
    snake[0].y = snake[0].y - 1 == -1 ? 7 : snake[0].y - 1;
    break;
  case 3:
    snake[0].x = snake[0].x - 1 == -1 ? 7 : snake[0].x - 1;
    break;
  case 4:
    snake[0].x = snake[0].x + 1 == 8 ? 0 : snake[0].x + 1;
    break;
  }
  for (int i = 1; i < len; i++)
  {
    // 1-top; 2-bot; 3-left; 4-right
    int oldX = snake[i].x;
    int oldY = snake[i].y;
    snake[i].x = prevX;
    snake[i].y = prevY;
    prevX = oldX;
    prevY = oldY;
  }
  lastDirection = direction;
}

bool checkLoose()
{
  const int len = sizeof(snake) / sizeof(snake[0]);
  for (int i = 0; i < len; i++)
  {
    for (int j = 1; j < len - 1; j++)
    {
      if (snake[i].x == snake[j].x && snake[i].y == snake[j].y && i != j)
      {
        return true;
      }
    }
  }
  return false;
}

void leftButtonClick()
{
  Serial.println("Left button click");
  if (!gameIsStarted)
  {
    startGame();
  }
  else
  {
    moveSnake(3);
  }
}

void rightButtonClick()
{
  Serial.println("right button click");
  if (!gameIsStarted)
  {
    startGame();
  }
  else
  {
    moveSnake(4);
  }
}

void upButtonClick()
{
  Serial.println("Up button click");
  if (!gameIsStarted)
  {
    startGame();
  }
  else
  {
    moveSnake(2);
  }
}

void downButtonClick()
{
  Serial.println("Down button click");
  if (!gameIsStarted)
  {
    startGame();
  }
  else
  {
    moveSnake(1);
  }
}

void setup()
{
  Serial.begin(113310);
  // lcd.init(); // initialize the lcd
  // lcd.backlight();
  mtrx.begin();      // запускаем
  mtrx.setBright(5); // яркость 0..15

  leftButton.attachClick(leftButtonClick);
  rightButton.attachClick(rightButtonClick);
  upButton.attachClick(upButtonClick);
  downButton.attachClick(downButtonClick);

  snakeHead->next = nullptr;
  snakeHead->x = 3;
  snakeHead->y = 4;
  snakeSize = 1;
}

void loop()
{
  unsigned long timeNow = millis();
  leftButton.tick();
  rightButton.tick();
  upButton.tick();
  downButton.tick();
  // delay(1000);

  // if (!gameIsStarted)
  // {
  //   initStartScreen();
  //   delay(1000);
  //   lcd.blink();
  // }
  // else
  // {
  //   delay(1000);
  //   snakeStep();
  //   mtrx.update();
  // }

  if (timeNow - lastTimeChanged > stepDelay)
  {
    lastTimeChanged = timeNow;
    gameIsOver = checkLoose();
    if (gameIsOver)
    {
      mtrx.clear();
      mtrx.clearDisplay();
      // mtrx.line(0, 0, 0, 0, 1);
    }
    else
    {
      moveSnake(lastDirection);
      mtrx.clear();
      mtrx.clearDisplay();
      const int len = sizeof(snake) / sizeof(snake[0]);
      for (int i = 0; i < len; i++)
      {
        mtrx.dot(snake[i].x, snake[i].y);
      }
      mtrx.update();
    }
  }
}
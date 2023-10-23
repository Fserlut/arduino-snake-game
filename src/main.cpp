#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverMAX7219.h>
#include "OneButton.h"

const int FRUIT_MAX = 2;

unsigned long lastTimeChanged = millis();
unsigned long stepDelay = 300;

unsigned long fruitsDelay = 3000;
unsigned long lastFruitsChanged = millis();

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

struct SnakeListNode
{
  int x;
  int y;

  SnakeListNode *next;
};

struct fruit
{
  int x;
  int y;
};

fruit fruits[FRUIT_MAX] = {
    {x : (int)random(0, 7), y : (int)random(0, 7)},
    {x : (int)random(0, 7), y : (int)random(0, 7)},
};

void generateNewFruits()
{
  for (int i = 0; i < FRUIT_MAX; i++)
  {
    fruits[i] = {x : (int)random(0, 7), y : (int)random(0, 7)};
  }
}

SnakeListNode *snakeHead;

size_t snakeSize;

int getCurrentSize()
{
  return snakeSize;
}

void createSnake(int x, int y)
{
  snakeSize = 1;
  snakeHead = new SnakeListNode();
  snakeHead->next = nullptr;
  snakeHead->x = x;
  snakeHead->y = y;
}

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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score: ");
  lcd.setCursor(7, 0);
  lcd.print(getCurrentSize());
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
  int prevX = snakeHead->x;
  int prevY = snakeHead->y;
  // 1-top; 2-bot; 3-left; 4-right
  switch (direction)
  {
  case 1:
    snakeHead->y = snakeHead->y + 1 == 8 ? 0 : snakeHead->y + 1;
    break;
  case 2:
    snakeHead->y = snakeHead->y - 1 == -1 ? 7 : snakeHead->y - 1;
    break;
  case 3:
    snakeHead->x = snakeHead->x - 1 == -1 ? 7 : snakeHead->x - 1;
    break;
  case 4:
    snakeHead->x = snakeHead->x + 1 == 8 ? 0 : snakeHead->x + 1;
    break;
  }
  SnakeListNode *iterator = snakeHead->next;
  size_t i = 1;
  while (i < snakeSize)
  {
    // 1-top; 2-bot; 3-left; 4-right
    int oldX = iterator->x;
    int oldY = iterator->y;
    iterator->x = prevX;
    iterator->y = prevY;
    prevX = oldX;
    prevY = oldY;
    iterator = iterator->next;
    i++;
  }
  lastDirection = direction;
}

bool checkLoose()
{

  // for (int i = 0; i < len; i++)
  // {
  //   for (int j = 1; j < len - 1; j++)
  //   {
  //     if (snake[i].x == snake[j].x && snake[i].y == snake[j].y && i != j)
  //     {
  //       return true;
  //     }
  //   }
  // }
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

  createSnake(1, 4);
  addNewSnake(2, 4);
  addNewSnake(3, 4);
  addNewSnake(4, 4);
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
      SnakeListNode *iterator = snakeHead;
      for (size_t i = 0; i < snakeSize; i++)
      {
        mtrx.dot(iterator->x, iterator->y);
        iterator = iterator->next;
      }
      for (int i = 0; i < FRUIT_MAX; i++)
      {
        mtrx.dot(fruits[i].x, fruits[i].y, 1);
      }
      if (timeNow - lastFruitsChanged > fruitsDelay)
      {
        lastFruitsChanged = timeNow;
        generateNewFruits();
      }
      mtrx.update();
    }
  }
}
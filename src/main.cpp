#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverMAX7219.h>
#include "OneButton.h"

unsigned long lastTimeChanged = millis();
unsigned long stepDelay = 500;

MAX7219<1, 1, 9> mtrx; // CS PIN D9, DIN PIN D13, CLK PIN D11
LiquidCrystal_I2C lcd(0x27, 16, 2);

OneButton leftButton(4, true);  // left button on d4 pin
OneButton rightButton(6, true); // right button on d6 pin
OneButton upButton(7, true);    // up button on d7 pin
OneButton downButton(5, true);  // down button on d5 pin

const char lose_text[] PROGMEM = "GAME OWER";

bool gameIsStarted = true;

struct CurrentStepState
{
  bool x;
  bool y;
  bool isPositive;
  int moveIndex;
};

struct SnakeNode
{
  int x;
  int y;
  SnakeNode *next;
};

SnakeNode *head;
size_t listSize;

CurrentStepState currentStepState = {x : true, y : false, isPositive : true, moveIndex : -1};

void createLinkedList(int x, int y)
{
  listSize = 1;
  head = new SnakeNode();
  head->next = nullptr;
  head->x = x;
  head->y = y;
}

void insertHead(int x, int y)
{
  SnakeNode *newHead = new SnakeNode();
  newHead->next = head;
  newHead->x = x;
  newHead->y = y;
  head = newHead;
  listSize++;
}

void insertTail(int x, int y)
{
  SnakeNode *newTail = new SnakeNode();
  newTail->next = nullptr;
  newTail->x = x;
  newTail->y = y;

  SnakeNode *enumerator = head;
  for (size_t index = 0; index < listSize - 1; index++)
    enumerator = enumerator->next;

  enumerator->next = newTail;
  listSize++;
}

SnakeNode *getLastNode()
{
  SnakeNode *enumerator = head;
  while (enumerator->next != NULL)
  {
    enumerator = enumerator->next;
  }
  return enumerator;
}

int getCurrentSize()
{
  return listSize;
}

void moveSnake(int direction)
{
  // Нужен какой-то массив, который будут хранить в себе все повороты, чтобы при большой длинне это все работало
  if (direction == 1 && currentStepState.y && currentStepState.isPositive)
    return;
  if (direction == 2 && currentStepState.y && !currentStepState.isPositive)
    return;
  if (direction == 3 && currentStepState.x && !currentStepState.isPositive)
    return;
  if (direction == 4 && currentStepState.x && currentStepState.isPositive)
    return;
  // 1-top; 2-bot; 3-left; 4-right
  SnakeNode *last = getLastNode();
  switch (direction)
  {
  case 1:
    currentStepState.x = false;
    currentStepState.y = true;
    currentStepState.isPositive = false;
    currentStepState.moveIndex = last->x;
    break;
  case 2:
    currentStepState.x = false;
    currentStepState.y = true;
    currentStepState.isPositive = true;
    currentStepState.moveIndex = last->x;
    break;
  case 3:
    currentStepState.x = true;
    currentStepState.y = false;
    currentStepState.isPositive = false;
    currentStepState.moveIndex = last->y;
    break;
  case 4:
    currentStepState.x = true;
    currentStepState.y = false;
    currentStepState.isPositive = true;
    currentStepState.moveIndex = last->y;
    break;
  }
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
  lcd.print(getCurrentSize());
  mtrx.dot(4, 4);
  mtrx.dot(3, 4);
  mtrx.update();
}

void startGame()
{
  if (!gameIsStarted)
  {
    initLoadingScreen();
    createLinkedList(3, 4);
    createLinkedList(4, 4);
    return;
  }
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
    moveSnake(1);
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
    moveSnake(2);
  }
}

void snakeStep()
{
  mtrx.clear();
  mtrx.clearDisplay();
  SnakeNode *enumerator = head;
  size_t i = 0;

  while (i < listSize)
  {
    int nextX = enumerator->x;
    int nextY = enumerator->y;
    if (currentStepState.x)
    {
      if (currentStepState.isPositive)
      {
        if (currentStepState.moveIndex == nextY || currentStepState.moveIndex == -1)
        {
          nextX = enumerator->x + 1 == 8 ? 0 : enumerator->x + 1;
        }
        else
        {
          nextY = nextY + 1;
        }
      }
      else
      {
        if (currentStepState.moveIndex == nextY || currentStepState.moveIndex == -1)
        {
          nextX = enumerator->x - 1 == -1 ? 7 : enumerator->x - 1;
        }
        else
        {
          nextY = nextY + 1;
        }
      }
    }
    else
    {
      if (currentStepState.isPositive)
      {
        if (currentStepState.moveIndex == nextX)
        {
          nextY = enumerator->y + 1 == 8 ? 0 : enumerator->y + 1;
        }
        else
        {
          nextX = nextX + 1;
        }
      }
      else
      {
        if (currentStepState.moveIndex == nextX)
        {
          nextY = enumerator->y - 1 == -1 ? 7 : enumerator->y - 1;
        }
        else
        {
          nextX = nextX + 1;
        }
      }
    }
    enumerator->x = nextX;
    enumerator->y = nextY;
    mtrx.dot(nextX, nextY, 1);
    enumerator = enumerator->next;
    i++;
  }
  mtrx.update();
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

  createLinkedList(1, 4);
  insertTail(2, 4);
  insertTail(3, 4);
  insertTail(4, 4);
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
    snakeStep();
  }
}
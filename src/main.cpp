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

struct SnakeNode
{
  int x;
  int y;
  int direction; // 1-top; 2-bot; 3-left; 4-right
  SnakeNode *next;
  SnakeNode *prev;
};

struct MoveNode
{
  int x;      // Coords when move was created
  int y;      // Coords when move was created
  int moveTo; // 1-top; 2-bot; 3-left; 4-right
  int doneBy; // Counter done all dots or not

  MoveNode *next;
};

MoveNode *moveHead;
SnakeNode *snakeHead;
size_t snakeSize;
size_t moveSize = 0;

MoveNode *getLastMove()
{
  MoveNode *iteratop = moveHead;
  while (iteratop->next != NULL)
  {
    iteratop = iteratop->next;
  }
  return iteratop;
}

void addMove(int x, int y, int moveTo)
{
  MoveNode *newNode = new MoveNode();
  newNode->next = nullptr;
  newNode->x = x;
  newNode->y = y;
  newNode->doneBy = 0;
  newNode->moveTo = moveTo;
  if (moveSize)
  {
    MoveNode *lastMove = getLastMove();
    lastMove->next = newNode;
  }
  else
    moveHead = newNode;
  moveSize = moveSize + 1;
}

void createSnake(int x, int y)
{
  snakeSize = 1;
  snakeHead = new SnakeNode();
  snakeHead->next = nullptr;
  snakeHead->prev = nullptr;
  snakeHead->x = x;
  snakeHead->y = y;
  snakeHead->direction = 4;
}

void addNewSnake(int x, int y)
{
  SnakeNode *newHead = new SnakeNode();
  newHead->next = snakeHead;
  newHead->x = x;
  newHead->y = y;
  newHead->direction = snakeHead->direction;
  snakeHead = newHead;
  newHead->prev = snakeHead;
  snakeSize++;
}

// void addNewSnake(int x, int y)
// {
//   SnakeNode *newTail = new SnakeNode();
//   newTail->next = nullptr;
//   newTail->prev = snakeHead;
//   newTail->x = x;
//   newTail->y = y;

//   SnakeNode *enumerator = snakeHead;
//   for (size_t index = 0; index < snakeSize - 1; index++)
//     enumerator = enumerator->next;

//   newTail->direction = enumerator->direction;
//   enumerator->next = newTail;

//   snakeSize++;
// }

SnakeNode *getLastNode()
{
  SnakeNode *enumerator = snakeHead;
  while (enumerator->next != NULL)
  {
    enumerator = enumerator->next;
  }
  return enumerator;
}

int getCurrentSize()
{
  return snakeSize;
}

void moveSnake(int direction)
{
  // 1-top; 2-bot; 3-left; 4-right
  SnakeNode *last = getLastNode();
  last->direction = direction;
  addMove(last->x, last->y, direction);
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
    createSnake(3, 4);
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

int getNextXStep(SnakeNode *node)
{
  // 1-top; 2-bot; 3-left; 4-right
  switch (node->direction)
  {
  case 3:
    return node->x - 1 == -1 ? 7 : node->x - 1;
  case 4:
    return node->x + 1 == 8 ? 0 : node->x + 1;
  default:
    return node->x;
  }
}

int getNextYStep(SnakeNode *node)
{
  // 1-top; 2-bot; 3-left; 4-right
  switch (node->direction)
  {
  case 1:
    return node->y + 1 == 8 ? 0 : node->y + 1;
  case 2:
    return node->y - 1 == -1 ? 7 : node->y - 1;
  default:
    return node->y;
  }
}

void snakeStep()
{
  mtrx.clear();
  mtrx.clearDisplay();
  // SnakeNode *last = getLastNode();
  SnakeNode *enumerator = getLastNode();
  size_t i = snakeSize;

  while (i > 0)
  {
    Serial.println(i);
    enumerator->x = getNextXStep(enumerator);
    enumerator->y = getNextYStep(enumerator);
    mtrx.dot(enumerator->x, enumerator->y, 1);
    enumerator = enumerator->prev;
    i--;
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
    snakeStep();
  }
}
#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverMAX7219.h>
#include "OneButton.h"

const int FRUIT_MAX = 2;

unsigned long lastTimeChanged = millis();
unsigned long stepDelay = 500;

unsigned long fruitsDelay = 3000;
unsigned long lastFruitsChanged = millis();

int lastDirection = 4;

MAX7219<1, 1, 9> mtrx; // CS PIN D9, DIN PIN D13, CLK PIN D11
LiquidCrystal_I2C lcd(0x27, 16, 2);

OneButton leftButton(4, true);  // left button on d4 pin
OneButton rightButton(6, true); // right button on d6 pin
OneButton upButton(7, true);    // up button on d7 pin
OneButton downButton(5, true);  // down button on d5 pin

const char *lose_text = "GAME OVER";
const char *start_text = "Press any key for start";
const char *score_text = "Score: ";

bool gameIsStarted = false;
bool gameIsOver = false;
bool gameIsPaused = false;

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
  if (gameIsPaused)
  {
    lcd.setCursor(0, 0);
    lcd.print("Game on pause");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("Press any key");
    lcd.setCursor(0, 1);
    lcd.print("for start...");
  }
}

void initLoadingScreen()
{
  gameIsStarted = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(score_text);
  lcd.setCursor(7, 0);
  lcd.print(getCurrentSize());
}

void showGameOver()
{
  gameIsOver = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lose_text);
  lcd.setCursor(0, 1);
  lcd.print(score_text);
  lcd.print(getCurrentSize());
  lcd.blink();
}

void updateCurrentScore()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(score_text);
  lcd.setCursor(7, 0);
  lcd.print(getCurrentSize());
}

void startGame()
{
  if (!gameIsStarted)
  {
    initLoadingScreen();
    return;
  }
}

void fruitWasEated(SnakeListNode *node)
{
  for (int i = 0; i < FRUIT_MAX; i++)
  {
    if (fruits[i].x == node->x && fruits[i].y == node->y)
    {
      // 1-top; 2-bot; 3-left; 4-right
      switch (lastDirection)
      {
      case 1:
        addNewSnake(snakeHead->x, snakeHead->y + 1 == 8 ? 0 : snakeHead->y + 1);
        break;

      case 2:
        addNewSnake(snakeHead->x, snakeHead->y - 1 == -1 ? 7 : snakeHead->y - 1);
        break;

      case 3:
        addNewSnake(snakeHead->x - 1 == -1 ? 7 : snakeHead->x - 1, snakeHead->y);
        break;

      case 4:
        addNewSnake(snakeHead->x + 1 == 8 ? 0 : snakeHead->x + 1, snakeHead->y);
        break;
      }

      fruits[i].x = -2;
      fruits[i].y = -2;
      updateCurrentScore();
    }
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
  fruitWasEated(snakeHead);
  while (i < snakeSize)
  {
    // 1-top; 2-bot; 3-left; 4-right
    int oldX = iterator->x;
    int oldY = iterator->y;
    iterator->x = prevX;
    iterator->y = prevY;
    prevX = oldX;
    prevY = oldY;
    fruitWasEated(iterator);
    iterator = iterator->next;
    i++;
  }
  lastDirection = direction;
}

void checkLoose()
{
  SnakeListNode *iterator = snakeHead;
  SnakeListNode *iteratorNext = snakeHead->next;
  while (iterator->next != NULL)
  {
    while (iteratorNext->next != NULL)
    {
      if (iterator->x == iteratorNext->x && iterator->y == iteratorNext->y)
      {
        showGameOver();
        return;
      }
      iteratorNext = iteratorNext->next;
    }
    iterator = iterator->next;
  }
  return;
}

void leftButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
    return;
  }
  moveSnake(3);
}

void rightButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
    return;
  }
  moveSnake(4);
}

void upButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
    return;
  }
  moveSnake(2);
}

void downButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
    return;
  }
  moveSnake(1);
}

void pauseOrUnpauseGame()
{
  updateCurrentScore();
  gameIsPaused = !gameIsPaused;
}

void setup()
{
  Serial.begin(113310);
  lcd.init();
  lcd.backlight();
  mtrx.begin();
  mtrx.setBright(4);

  leftButton.attachClick(leftButtonClick);
  rightButton.attachClick(rightButtonClick);
  upButton.attachClick(upButtonClick);
  downButton.attachClick(downButtonClick);

  leftButton.attachLongPressStart(pauseOrUnpauseGame);
  rightButton.attachLongPressStart(pauseOrUnpauseGame);
  upButton.attachLongPressStart(pauseOrUnpauseGame);
  downButton.attachLongPressStart(pauseOrUnpauseGame);

  createSnake(1, 4);
  addNewSnake(2, 4);
  addNewSnake(3, 4);
  addNewSnake(4, 4);
  updateCurrentScore();
}

void loop()
{
  unsigned long timeNow = millis();
  leftButton.tick();
  rightButton.tick();
  upButton.tick();
  downButton.tick();

  if (!gameIsStarted || gameIsPaused)
  {
    initStartScreen();
  }
  else
  {
    if (timeNow - lastTimeChanged > stepDelay)
    {
      lastTimeChanged = timeNow;
      checkLoose();
      if (gameIsOver)
      {
        mtrx.clear();
        mtrx.clearDisplay();
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
}
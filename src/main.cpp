#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverMAX7219.h>
#include "OneButton.h"

MAX7219<1, 1, 9> mtrx; // CS PIN D9, DIN PIN D13, CLK PIN D11
LiquidCrystal_I2C lcd(0x27, 16, 2);

OneButton leftButton(4, true);  // left button on d4 pin
OneButton rightButton(6, true); // right button on d6 pin
OneButton upButton(7, true);    // up button on d7 pin
OneButton downButton(5, true);  // down button on d5 pin

const char lose_text[] PROGMEM = "GAME OWER";

int currentSize = 1;
bool gameIsStarted = false;

struct node
{
  int x;
  int y;
  node *next;
};

node *head;
size_t listSize;

void createLinkedList(int x, int y)
{
  listSize = 1;
  head = new node();
  head->next = nullptr;
  head->x = x;
  head->y = y;
}

void insertHead(int x, int y)
{
  node *newHead = new node();
  newHead->next = head;
  newHead->x = x;
  newHead->y = y;
  head = newHead;
  listSize++;
}

void insertTail(int x, int y)
{
  node *newTail = new node();
  newTail->next = nullptr;
  newTail->x = x;
  newTail->y = y;

  node *enumerator = head;
  for (size_t index = 0; index < listSize - 1; index++)
    enumerator = enumerator->next;

  enumerator->next = newTail;
  listSize++;
}

int getCurrentSize()
{
  return currentSize + 1;
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
  if (!gameIsStarted)
  {
    startGame();
  }
  Serial.println("Left button click");
}

void rightButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
  }
  Serial.println("right button click");
}

void upButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
  }
  Serial.println("Up button click");
}

void downButtonClick()
{
  if (!gameIsStarted)
  {
    startGame();
  }
  Serial.println("Down button click");
}

void setup()
{
  Serial.begin(113310);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  mtrx.begin();      // запускаем
  mtrx.setBright(5); // яркость 0..15

  leftButton.attachClick(leftButtonClick);
  rightButton.attachClick(rightButtonClick);
  upButton.attachClick(upButtonClick);
  downButton.attachClick(downButtonClick);
  mtrx.dot(4, 4);
  mtrx.dot(3, 4);
  // mtrx.line(0, 0, 6, 6);
  // mtrx.line(7, 0, 0, 7);
  mtrx.update();
}

void loop()
{
  leftButton.tick();
  rightButton.tick();
  upButton.tick();
  downButton.tick();
  // lcd.blink();
  // You can implement other code in here or just wait a while
  delay(10);

  if (!gameIsStarted)
  {
    initStartScreen();
    delay(1000);
    lcd.blink();
  }
}
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

class snakeNode
{
public:
  int x;
  int y;
  snakeNode *next;
  snakeNode *prev;
};

bool gameIsStarted = false;

const char lose_text[] PROGMEM = "GAME OWER";

int currentSize = 1;

snakeNode *createNewNode(int x, int y, snakeNode *prev)
{
  auto n = malloc(sizeof(snakeNode));
  if (n == NULL)
  {
    Serial.print("GAME ERROR");
    return;
  }
  snakeNode n = {
    x : 3,
    y : 4,
    prev : (snakeNode *)&prev,
  };
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
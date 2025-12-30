#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // Updated for 20x4 LCD

const int xPin = A0;
const int yPin = A1;
const int buttonPin = 2;

int snakeX = 0, snakeY = 0;
int foodX, foodY;
int bombX, bombY;
int direction = 0;
bool gameOver = false;
bool gamePaused = false;
bool bombVisible = false;
bool foodVisible = false;
int score = 0;

int snakeBodyX[80], snakeBodyY[80]; // Adjusted for 20x4 LCD (20x4 = 80 positions)
int snakeLength = 1;
const int maxsnakeLength = 10; // Increase max snake length for larger LCD
int applesEaten = 0;

unsigned long lastMoveTime = 0;
unsigned long bombSpawnTime = 0;
unsigned long foodSpawnTime = 0;
unsigned long moveInterval = 250;
const unsigned long bombDuration = 5000;
const unsigned long foodInterval = 2000;
const unsigned long minMoveInterval = 100;
const unsigned long speedIncreaseInterval = 2;

void showInitialScreen();
void showCountdown();
void resetGame();
void readJoystick();
void moveSnake();
void checkCollision();
void generateFood();
void generateBomb();
void updateLCD();
void displayGameOver();
void displayPaused();
bool isOnSnake(int x, int y);
bool isInFrontOfSnake(int x, int y);

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(buttonPin, INPUT_PULLUP);

  showInitialScreen();
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    if (gameOver) {
      resetGame();
    } else if (gamePaused) {
      gamePaused = false;
      showCountdown();
    } else {
      gamePaused = true;
    }
    delay(500);
  }

  if (gamePaused) {
    displayPaused();
    return;
  }

  if (gameOver) {
    displayGameOver();
    return;
  }

  if (millis() - lastMoveTime >= moveInterval) {
    lastMoveTime = millis();
    moveSnake();
    checkCollision();
    updateLCD();
  }

  if (!foodVisible && millis() - foodSpawnTime >= foodInterval) {
    generateFood();
  }

  if (bombVisible && millis() - bombSpawnTime >= bombDuration) {
    bombVisible = false;
  }

  readJoystick();
}

void showInitialScreen() {
  lcd.clear();
  lcd.setCursor(5, 1); // Adjusted for 20x4
  lcd.print("Snake Game");
  lcd.setCursor(4, 2); // Adjusted for 20x4
  lcd.print("Press to Play");
  while (digitalRead(buttonPin) == HIGH) {}
  delay(500);
  showCountdown();
  resetGame();
}

void showCountdown() {
  lcd.clear();
  for (int i = 3; i > 0; i--) {
    lcd.setCursor(9, 1); // Centered on 20x4
    lcd.print(i);
    delay(1000);
  }
  lcd.clear();
}

void resetGame() {
  snakeX = 0;
  snakeY = 0;
  direction = 0;
  snakeLength = 1;
  applesEaten = 0;
  score = 0;
  gameOver = false;
  gamePaused = false;
  moveInterval = 250;
  bombVisible = false;
  foodVisible = false;

  for (int i = 0; i < 80; i++) {
    snakeBodyX[i] = -1;
    snakeBodyY[i] = -1;
  }
  snakeBodyX[0] = snakeX;
  snakeBodyY[0] = snakeY;

  generateFood();
  foodSpawnTime = millis();
  updateLCD();
}

void readJoystick() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  int deadzone = 512;
  int threshold = 100;

  if (xValue < deadzone - threshold) {
    direction = 2;
  } else if (xValue > deadzone + threshold) {
    direction = 0;
  }

  if (yValue < deadzone - threshold) {
    direction = 3;
  } else if (yValue > deadzone + threshold) {
    direction = 1;
  }
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeBodyX[i] = snakeBodyX[i - 1];
    snakeBodyY[i] = snakeBodyY[i - 1];
  }

  snakeBodyX[0] = snakeX;
  snakeBodyY[0] = snakeY;

  if (direction == 0) snakeX++;
  if (direction == 1) snakeY++;
  if (direction == 2) snakeX--;
  if (direction == 3) snakeY--;

  if (snakeX < 0) snakeX = 19;
  if (snakeX > 19) snakeX = 0;
  if (snakeY < 0) snakeY = 3;
  if (snakeY > 3) snakeY = 0;

  if (foodVisible && snakeX == foodX && snakeY == foodY) {
    if (snakeLength < maxsnakeLength) {
      snakeLength++;
    }

    applesEaten++;
    score = applesEaten * 10;
    foodVisible = false;
    foodSpawnTime = millis();
    generateBomb();

    if (applesEaten % speedIncreaseInterval == 0) {
      if (moveInterval > minMoveInterval) {
        moveInterval -= 15;
      }
    }
  }

  if (bombVisible && snakeX == bombX && snakeY == bombY) {
    gameOver = true;
  }
}

void checkCollision() {
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX == snakeBodyX[i] && snakeY == snakeBodyY[i]) {
      gameOver = true;
      break;
    }
  }
}

void generateFood() {
  int attempts = 0;
  do {
    foodX = random(0, 20);
    foodY = random(0, 4);
    attempts++;
  } while ((abs(foodX - snakeX) < 3 && abs(foodY - snakeY) < 3) || isOnSnake(foodX, foodY));

  foodVisible = true;
  foodSpawnTime = millis();
}

void generateBomb() {
  if (random(0, 10) >= 3) return;

  int attempts = 0;
  do {
    bombX = random(0, 20);
    bombY = random(0, 4);
    attempts++;
  } while ((abs(bombX - snakeX) < 3 && abs(bombY - snakeY) < 3) || isOnSnake(bombX, bombY));

  bombVisible = true;
  bombSpawnTime = millis();
}

bool isOnSnake(int x, int y) {
  for (int i = 0; i < snakeLength; i++) {
    if (snakeBodyX[i] == x && snakeBodyY[i] == y) {
      return true;
    }
  }
  return false;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(snakeBodyX[0], snakeBodyY[0]);
  lcd.print("O");

  for (int i = 1; i < snakeLength; i++) {
    lcd.setCursor(snakeBodyX[i], snakeBodyY[i]);
    lcd.print("o");
  }

  if (foodVisible) {
    lcd.setCursor(foodX, foodY);
    lcd.print("F");
  }

  if (bombVisible) {
    lcd.setCursor(bombX, bombY);
    lcd.print("B");
  }
}

void displayGameOver() {
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(6, 1);
  lcd.print("Score ");
  lcd.print(score);
}

void displayPaused() {
  lcd.clear();
  lcd.setCursor(7, 1);
  lcd.print("PAUSED");
  lcd.setCursor(4, 2);
  lcd.print("Press to Resume");
}
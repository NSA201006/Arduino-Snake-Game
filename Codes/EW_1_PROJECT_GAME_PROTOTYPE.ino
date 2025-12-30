#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

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

int snakeBodyX[32], snakeBodyY[32];
int snakeLength = 1;
const int maxsnakeLength = 5;  // Max length of the snake
int applesEaten = 0;  // Count the number of apples eaten

unsigned long lastMoveTime = 0;
unsigned long bombSpawnTime = 0;
unsigned long foodSpawnTime = 0;
unsigned long moveInterval = 250;  // Initial move speed (250ms per move)
const unsigned long bombDuration = 5000;
const unsigned long foodInterval = 2000;
const unsigned long minMoveInterval = 100; // Minimum move speed (cap)
const unsigned long speedIncreaseInterval = 2;  // Increase speed after eating 2 apples

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
  lcd.setCursor(2, 0);
  lcd.print("Snake Game");
  lcd.setCursor(3, 1);
  lcd.print("Press to Play");
  while (digitalRead(buttonPin) == HIGH) {}
  delay(500);
  showCountdown();
  resetGame();
}

void showCountdown() {
  lcd.clear();
  for (int i = 3; i > 0; i--) {
    lcd.setCursor(7, 0);
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
  applesEaten = 0;  // Reset apple counter
  score = 0;
  gameOver = false;
  gamePaused = false;
  moveInterval = 250; // Reset speed to initial value
  bombVisible = false;
  foodVisible = false;

  for (int i = 0; i < 32; i++) {
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
  // Move the body: Start from the end of the body and move each segment to the next position
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeBodyX[i] = snakeBodyX[i - 1];
    snakeBodyY[i] = snakeBodyY[i - 1];
  }

  // Now update the position of the head
  snakeBodyX[0] = snakeX;
  snakeBodyY[0] = snakeY;

  // Move the head based on the direction
  if (direction == 0) snakeX++;  // Right
  if (direction == 1) snakeY++;  // Down
  if (direction == 2) snakeX--;  // Left
  if (direction == 3) snakeY--;  // Up

  // Wrap around the screen edges
  if (snakeX < 0) snakeX = 15;
  if (snakeX > 15) snakeX = 0;
  if (snakeY < 0) snakeY = 1;
  if (snakeY > 1) snakeY = 0;

  // Handle food consumption
  if (foodVisible && snakeX == foodX && snakeY == foodY) {
    if (snakeLength < maxsnakeLength) {
      snakeLength++;  // Increase length up to max length
    }

    applesEaten++;  // Increase apples eaten count
    score = applesEaten * 10;  // Score is 10 * apples eaten
    foodVisible = false;
    foodSpawnTime = millis();
    generateBomb();

    // Speed increase logic: Increase speed after eating 2 apples
    if (applesEaten % speedIncreaseInterval == 0) {
      // Gradually decrease the moveInterval (game speed)
      if (moveInterval > minMoveInterval) {
        moveInterval -= 15;  // Reduce speed by 15ms
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
    foodX = random(0, 16);
    foodY = random(0, 2);
    attempts++;
  } while ((abs(foodX - snakeX) < 3 && abs(foodY - snakeY) < 3) || isOnSnake(foodX, foodY));
  
  foodVisible = true;
  foodSpawnTime = millis();
}

void generateBomb() {
  if (random(0, 10) >= 3) return;

  int attempts = 0;
  do {
    bombX = random(0, 16);
    bombY = random(0, 2);
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

  // Draw the body
  for (int i = 1; i < snakeLength; i++) {
    lcd.setCursor(snakeBodyX[i], snakeBodyY[i]);
    lcd.print("o");
  }

  // Draw food and bomb
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
  lcd.setCursor(4, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(4, 1);
  lcd.print("Score: ");
  lcd.print(score);
}

void displayPaused() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("PAUSED");
  lcd.setCursor(0, 1);
  lcd.print("Press to Resume");
  if (digitalRead(buttonPin) == LOW) {
    // Countdown before resuming
    for (int i = 3; i > 0; i--) {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print(i);
      delay(1000); 
    }
    lcd.clear(); 
    gamePaused = false; 
  }
}

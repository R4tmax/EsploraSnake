/* 
*Initial libraries allocation and button mapping.
*/

#include <Esplora.h>
#include <TFT.h>   
#include <EEPROM.h>

#define DOWN   SWITCH_1 //Mapped buttons
#define RIGHT  SWITCH_4
#define LEFT   SWITCH_2
#define UP     SWITCH_3

//Size of the gameArea
#define HORIZONTAL 18 //x Axis
#define VERTICAL 14 //y Axis
/*
 *  Each grid is 8x8 pixels wide
 *  This setup fits the TFT and gives me some extra room.
 */

/* 
*Enumeration segment
*/
/*
 * This is the primary FSM reference enumeration.
 * It is reference via the loop function 
 * to keep track of the program state.
 */
enum game_states {
  MENU, 
  DIFF_SELECT, 
  SCOREBOARD,
  GAME,
  POST_GAME
};

/*
 * Main menu options
 */
enum menu_positions {
  PLAY,
  HIGHSCORES,
  RESET  
};

/*
* These options are available as difficulties
* to play.
 */

enum difficulty_selector {
  EASY,
  MEDIUM,
  HARD,
  VERYHARD
};
/*
 *  Enumeration of snake possible directions
 */
enum heading {
  EAST,
  WEST,
  NORTH,
  SOUTH
};
/*
 * Enumeration for objects of the game
 */

enum object_types {
  EMPTY,
  SNAKE,
  FOOD
};


enum heading SNAKE_DIRECTION;

enum game_states MASTER;
enum game_states MASTER_NEXT;

enum menu_positions MENU_POS;
enum menu_positions MENU_NEXT;

enum difficulty_selector DIFF_POS;
enum difficulty_selector DIFF_NEXT;



/*
 * Game variables section
 */

 //Info about the snake
object_types gameArea[HORIZONTAL][VERTICAL]; //2D array storing info about the game area
int snakePosX[HORIZONTAL*VERTICAL]; //coords of the snake head
int snakePosY[HORIZONTAL*VERTICAL];
unsigned int snakeLength = 0; //how long is the snake

//gamestate
unsigned int score = 0; 
unsigned int timerSaved = 0; //We use two different timers refferencing 
unsigned int timerCur = 0; //Eplora internal clock to modify the game speed
unsigned int tickRate = 0; //basically reffers to the snake speed, this value will be set at the start of each game in accordance with the desired difficulty.

// auxilary variables
char scoreOutput[4]; //This setup allows for convinient conversion of
String scoreConv; //previously unknown number into char array. 
bool permeableWalls; //Keeps track of whether walls are passable or not.

/*
*Button reading function, taken from:
* https://courses.fit.cvut.cz/BI-ARD/tutorials/04/index.html
*/
byte buttonFlag = 0;

bool buttonEvent(int button)
{
  switch(button)
  {
    case DOWN:
     if (Esplora.readButton(DOWN) == LOW)
     {
       buttonFlag |= 1;
     }
     else if (buttonFlag & 1)
     {
       buttonFlag ^= 1;
       return true;
     }
     break;

    case RIGHT:
     if (Esplora.readButton(RIGHT) == LOW)
     {
       buttonFlag |= 2;
     }
     else if (buttonFlag & 2)
     {
       buttonFlag ^= 2;
       return true;
     }
     break;

    case LEFT:
     if (Esplora.readButton(LEFT) == LOW)
     {
       buttonFlag |= 4;
     }
     else if (buttonFlag & 4)
     {
       buttonFlag ^= 4;
       return true;
     }
     break;

    case UP:
     if (Esplora.readButton(UP) == LOW)
     {
       buttonFlag |= 8;
     }
     else if (buttonFlag & 8)
     {
       buttonFlag ^= 8;
       return true;
     }
  }
   return false;
}

/*
 * Auxiliary functions for using
 * the TFT display.
 * 
 */
void writeBlack(){
  EsploraTFT.stroke(0,0,0);
}

void writeWhite(){
  EsploraTFT.stroke(255,255,255);
}


void remStroke(){
  EsploraTFT.noStroke();
}

void backgroundBlack(){
  EsploraTFT.background(0,0,0);
}

void fillBlack () {
  EsploraTFT.fill(0,0,0);
}

void fillWhite () {
  EsploraTFT.fill(255,255,255);
}

void fillRed () {
  EsploraTFT.fill(0,0,240); //For reasons unknown to me, this fill function looks as Red on my TFT,
                            //in spite of what my understanding of the RGB in the documentation is.
}

void fillGreen(){
  EsploraTFT.fill(0,255,0);
}

void backgroundWhite(){
  EsploraTFT.background(255,255,255);
}

/*
 * Menu layer functions.
 */
//Draws the main menu
void primaryMenu(){
  backgroundBlack();
  writeWhite();
  EsploraTFT.setTextSize(1);
  
  EsploraTFT.text("EsploraSnake",40,5);
  EsploraTFT.text("1. New Game",10,40);
  EsploraTFT.text("2. Scoreboard",10,60);
  EsploraTFT.text("3. Reset Score",10,80);
  EsploraTFT.text("*",3,40);
 
}
//draws the play menu
void diffMenu(){
  backgroundBlack();
  writeWhite();
  EsploraTFT.setTextSize(1);
  
  EsploraTFT.text("EsploraSnake",40,5);
  EsploraTFT.text("1. Easy",10,40);
  EsploraTFT.text("2. Normal",10,60);
  EsploraTFT.text("3. Hard",10,80);
  EsploraTFT.text("4. Very hard",10,100);
  EsploraTFT.text("*",3,60);
}

//clears the star markers of the main menu
void mainRedraw() {
  writeBlack();
  EsploraTFT.text("*",3,40);
  EsploraTFT.text("*",3,60);
  EsploraTFT.text("*",3,80);
  writeWhite();
  switch (MENU_NEXT) {
    case PLAY:
      EsploraTFT.text("*",3,40);
      break;

    case HIGHSCORES:
      EsploraTFT.text("*",3,60);
      break;

    case RESET:
      EsploraTFT.text("*",3,80);
      break;      
    
  }

  
}

//clears the star markers of the difficulty selection menu
void diffRedraw(){
    writeBlack();
  EsploraTFT.text("*",3,40);
  EsploraTFT.text("*",3,60);
  EsploraTFT.text("*",3,80);
  EsploraTFT.text("*",3,100);
  writeWhite();
  switch (DIFF_NEXT) {
    case EASY:
      EsploraTFT.text("*",3,40);
      break;

    case MEDIUM:
      EsploraTFT.text("*",3,60);
      break;

    case HARD:
      EsploraTFT.text("*",3,80);
      break; 

    case VERYHARD:
      EsploraTFT.text("*",3,100);
      break;  
  }          
}

//Shows the scoreboard submenu.
void showScore() {
  backgroundBlack();
  writeWhite();
  EsploraTFT.setTextSize(1);
  EsploraTFT.text("Highscores:",5,10);
  EsploraTFT.text("1:",5,20);
  EsploraTFT.text("2:",5,30);
  EsploraTFT.text("3:",5,40);

  int loadVal;
  for(int i = 0 ; i < 3; i++)
  {
    loadVal = EEPROM.read(i);
    scoreConv = String(loadVal);
    scoreConv.toCharArray(scoreOutput, 4);

    if (i == 0)
      {
        EsploraTFT.text(scoreOutput,20,20);
      }
    else if (i == 1)
    {
      EsploraTFT.text(scoreOutput,20,30);
    }
    else if (i == 2)
    {
      EsploraTFT.text(scoreOutput,20,40);
    }    
    
  }
}

//Clears the used positions for the top scores.
void clearScore() {
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
}


/*
 * Active Game functions
 */

/*
 * Auxiliary funtion, can scan and determine which "object" occupies each coord.
 * It is used when reading coords relative to the snake head is not optimal.
 */
object_types readArea(int x, int y) {
  return gameArea[x][y];
}

/*
 * Called on game setup. 
 * Fills the initial snake data sets 
 * and draws the body.
 */
void spawnSnake() {
  gameArea[8][7] = SNAKE; //override the void fields
  gameArea[7][7] = SNAKE; //I start at the eighth "position", with the first direction being right
  gameArea[6][7] = SNAKE; //this puts snake head in the middle of the game field after intial tick,giving player some reaction time


  snakePosX[0] = 8; // Stores the intial position into the "snake head" array
  snakePosY[0] = 7; // This will be used for "moving" the snake
  snakePosX[1] = 7;
  snakePosY[1] = 7;
  snakePosX[2] = 6;
  snakePosY[2] = 7;

  //Draws the Snake in green
  fillGreen();
  remStroke();
  EsploraTFT.rect(6 + (8 * 8), 6 + (7 * 8),8,8); // gameArea is split into 8 by 8 grids
  EsploraTFT.rect(6 + (7 * 8), 6 + (7 * 8),8,8); // we draw the rectangles "at" the desired coords
  EsploraTFT.rect(6 + (6 * 8), 6 + (7 * 8),8,8); 
  snakeLength = 3;
  
  
  
}

/*
 * Called whenever we need to add point to the gamearea.
 * Tries to plant point randomly, when that fails,
 * scans for the any valid position from the top left.
 */
void spawnPoint() {
  for(int i = 0 ; i < 10 ; i++) {
    int x = random(HORIZONTAL);
    int y = random(VERTICAL);
    if (gameArea[x][y] == EMPTY) {
      gameArea[x][y] = FOOD;
      fillRed();
      remStroke();
      EsploraTFT.rect(6 + (x * 8), 6 + (y * 8),8,8);
      return;
    }
  }

  //Failsafe for failed RNG

  for(int x = 0 ; x < HORIZONTAL ; x++ ) {
    for(int y = 0; y < VERTICAL ; y++) {
      if(gameArea[x][y] == EMPTY) {
      gameArea[x][y] = FOOD;
      fillRed();
      remStroke();
      EsploraTFT.rect(6 + (x * 8), 6 + (y * 8),8,8);
      return;
      }
    }
  }
  
}

/*
 * Called after difficulty is selected in the menus.
 * Applies difficulty setting and calls spawning fucntions.
 * It also clears some variables to prevent conflicts and 
 * calls the timer for the first time.
 */
void gameSetup (){

 if ( DIFF_NEXT == EASY ) {
  tickRate = 300;
  permeableWalls = true;
 }
  
 if ( DIFF_NEXT == MEDIUM ) {
  tickRate = 250;
  permeableWalls = true;  
 }

 if ( DIFF_NEXT == HARD ) {
  tickRate = 225;
  permeableWalls = true;  
 }
 
 if ( DIFF_NEXT == VERYHARD ) {
  tickRate = 200;
  permeableWalls = false;
 }
      
  timerSaved = millis();
  score = 0; //Reset the score if multiple games were played in row.
  backgroundBlack(); //draw the game field
  writeWhite();
  fillBlack();
  EsploraTFT.rect(5,5,146,114); //144 = 18*8 + 2 , 112 = 14*8 +2 

  //nullify the coord array
  for(int i = 0; i < HORIZONTAL; i++) {
    for(int j = 0; j < VERTICAL; j++) {
      gameArea[i][j] = EMPTY;
    }
  }

  spawnSnake(); //draws snake in the middle of the screen
  spawnPoint(); //spawns the initial point

  SNAKE_DIRECTION = EAST; //Initial direction
  
}


/*
 * Cleans the tail grid after the snake leaves it.
 * It is also used separetly to emulate "sinking" of the
 * snake during "teleports".
 */
void moveTail(){
  fillBlack();
  EsploraTFT.rect(6 + (snakePosX[snakeLength-1] * 8), 6 + (snakePosY[snakeLength-1] * 8), 8, 8);
  gameArea[snakePosX[snakeLength-1]][snakePosY[snakeLength-1]] = EMPTY;
}

/*
 * Movement auxiliary function.
 * Shifts the data in the arrays 
 * to properly reflect snake position.
 */
void updateCoords(){
    for (int i = snakeLength - 1 ; i >= 0; i--){
    snakePosX[i + 1] = snakePosX[i];
    snakePosY[i + 1] = snakePosY[i];
  }
}


/*
* Primary movement fucntion. Calling this exectues all of the auxilary steps.
* Removes the tail.
* Then looks at the direction of the snake, draws a new grid.
* Updates the arrays and finally, sets the new snake head coords.
*/

void moveSnake() {

  moveTail();

  fillGreen();
  remStroke();
   
  switch(SNAKE_DIRECTION) {
    case NORTH:
      EsploraTFT.rect(6 + (snakePosX[0] * 8), 6 + (snakePosY[0] - 1) * 8, 8, 8);
      gameArea[snakePosX[0]][snakePosY[0] - 1] = SNAKE;
      break;
    case SOUTH:
      EsploraTFT.rect(6 + (snakePosX[0] * 8), 6 + (snakePosY[0] + 1) * 8, 8, 8);
      gameArea[snakePosX[0]][snakePosY[0] + 1] = SNAKE;
      break;
    case EAST:
      EsploraTFT.rect(6 + ((snakePosX[0] + 1) * 8), 6 + (snakePosY[0]) * 8, 8, 8);
      gameArea[snakePosX[0] + 1][snakePosY[0]] = SNAKE;
      break;
    case WEST:
      EsploraTFT.rect(6 + ((snakePosX[0] - 1) * 8), 6 + (snakePosY[0]) * 8, 8, 8);
      gameArea[snakePosX[0] - 1][snakePosY[0]] = SNAKE;
      break;
  }

updateCoords();


  switch(SNAKE_DIRECTION) {
    case NORTH:
      snakePosX[0] = snakePosX[1];
      snakePosY[0] = snakePosY[1] - 1;
      break;
    case SOUTH:
      snakePosX[0] = snakePosX[1];
      snakePosY[0] = snakePosY[1] + 1;
      break;
    case EAST:
      snakePosX[0] = snakePosX[1] + 1;
      snakePosY[0] = snakePosY[1];
      break;
    case WEST:
      snakePosX[0] = snakePosX[1] - 1;
      snakePosY[0] = snakePosY[1];
      break;  
    }

}

/*
 * Speeds the tickRate of the game, 
 * until a fixed treshold is reached.
 */
void shortenTickrate() {
  if ( (score % 10 == 0) && (tickRate >= 150 )) {
    tickRate = tickRate - 5; 
  }
}
/*
 * Handles the score progression.
 * calls the movement function, extends the length
 * and spawns new point.
 * Also, checks if we should speed things up a bit.
 */

void growSnake() {

 moveSnake();
 snakeLength++;
 score++;
 spawnPoint();
 shortenTickrate();
}

/*
 * Handles out of bounds scenario.
 * It is called via the game logic function,
 * when we reached the outer limit of the 2D array.
 * Double checks for food, overrides snake head position
 * once finished.
 */
void teleport() {
  switch(SNAKE_DIRECTION) {
    case NORTH:
      if(readArea(snakePosX[0],VERTICAL -1 ) != FOOD) { 
        moveTail();
      }
      else if (readArea(snakePosX[0],VERTICAL - 1) == FOOD) { 
        snakeLength++;
        score++;
        spawnPoint();
      }

      gameArea[snakePosX[0]][VERTICAL - 1] = SNAKE; 
      fillGreen();
      EsploraTFT.rect(6 + (snakePosX[0]) * 8, 6 + (VERTICAL - 1) * 8, 8, 8);
      updateCoords();
      snakePosX[0] = snakePosX[1];
      snakePosY[0] = VERTICAL - 1;
      break;    

    case SOUTH:
      if(readArea(snakePosX[0],0) != FOOD) {
        moveTail();
      }
      else if (readArea(snakePosX[0],0) == FOOD) {
        snakeLength++;
        score++;
        spawnPoint();
      }

      gameArea[snakePosX[0]][0] = SNAKE; 
      fillGreen();
      EsploraTFT.rect(6 + (snakePosX[0]) * 8, 6, 8, 8);
      updateCoords();
      snakePosX[0] = snakePosX[1];
      snakePosY[0] = 0;
      break;
    
    case EAST:
      if(readArea(0,snakePosY[0]) != FOOD) { 
        moveTail();
      }
      else if (readArea(0,snakePosY[0]) == FOOD) { 
        snakeLength++;
        score++;
        spawnPoint();
      }

      gameArea[0][snakePosY[0]] = SNAKE; 
      fillGreen();
      EsploraTFT.rect(6, 6 + (snakePosY[0]) * 8, 8, 8);
      updateCoords();
      snakePosX[0] = 0;
      snakePosY[0] = snakePosY[1];
      break;
    case WEST:
      if(readArea(HORIZONTAL - 1,snakePosY[0]) != FOOD) { 
        moveTail();
      }
      else if (readArea(HORIZONTAL - 1,snakePosY[0]) == FOOD) { 
        snakeLength++;
        score++;
        spawnPoint();
      }

      gameArea[HORIZONTAL -1 ][snakePosY[0]] = SNAKE; 
      fillGreen();
      EsploraTFT.rect(6 + 8*(HORIZONTAL - 1), 6 + (snakePosY[0]) * 8, 8, 8);
      updateCoords();
      snakePosX[0] = HORIZONTAL-1;
      snakePosY[0] = snakePosY[1];
      break;


  }
}

/*
 * This function returns either true or false.
 * TRUE = Snake made a move thats in order.
 * FALSE = Snake crashed into a wall or eaten itself,gameover.
 * 
 * Depending on the snake direction checks coordinate we are about to enter.
 * If it determines we are in out-of-bounds scenario, 
 * Checks the in game variables and the otherside of the wall whether we are 
 * making legal move.
 * If yes, then it determines whether to call the point collection function,
 * end the game or just proceed with moving the snake.
 */

bool gameLogic (){
    switch(SNAKE_DIRECTION) {
    case NORTH:
      if(snakePosY[0] == 0) {
        if(gameArea[snakePosX[0]][VERTICAL - 1] == SNAKE || !permeableWalls) { 
          return false;
        }
        else {
          teleport();
        }
      }
      else if(gameArea[snakePosX[0]][snakePosY[0] - 1] == SNAKE) { 
        return false;
      }
      else if(gameArea[snakePosX[0]][snakePosY[0] - 1] == FOOD) { 
        growSnake();
      }
      else { 
        moveSnake();
      }
      break;
      
    case SOUTH:
     if(snakePosY[0] + 1 == VERTICAL) {
        if(gameArea[snakePosX[0]][0] == SNAKE || !permeableWalls) { 
          return false;
        }
        else {
          teleport();
        }
      }
      else if(gameArea[snakePosX[0]][snakePosY[0] + 1] == SNAKE) { 
        return false;
      }
      else if(gameArea[snakePosX[0]][snakePosY[0] + 1] == FOOD) { 
        growSnake();
      }
      else { 
        moveSnake();
      }
      break;
      
    case EAST:
     if(snakePosX[0] + 1 == HORIZONTAL) {
        if(gameArea[0][snakePosY[0]] == SNAKE || !permeableWalls) { 
          return false;
        }
        else {
          teleport();
        }
      }
      else if(gameArea[snakePosX[0] + 1][snakePosY[0]] == SNAKE) { 
        return false;
      }
      else if(gameArea[snakePosX[0] + 1][snakePosY[0]] == FOOD) { 
        growSnake();
      }
      else { 
        moveSnake();
      }
      break;
      
    case WEST:
     if(snakePosX[0] == 0) {
        if(gameArea[HORIZONTAL - 1][snakePosY[0]] == SNAKE || !permeableWalls) { 
          return false;
        }
        else {
          teleport();
        }
      }
      else if(gameArea[snakePosX[0] - 1][snakePosY[0]] == SNAKE) { 
        return false;
      }
      else if(gameArea[snakePosX[0] - 1][snakePosY[0]] == FOOD) { 
        growSnake();
      }
      else { 
        moveSnake();
      }
      break; 
    }
  return true;
}

/*
 * Draws the endgame screen.
 * Runs the score calculation.
 * Reads eeprom for saved scores,
 * if we reached a new height, overrides the value.
 * 
 * I originally intended to have this work as BO5.
 * But I decided against increasing memory demand of the code.
 * 
 * Multiplier for Very Hard is +2, becuase of the 
 * added "No teleport" difficulty.
 */
void gameOver() {
  backgroundBlack();
  writeWhite();
  EsploraTFT.setTextSize(3);
  EsploraTFT.text("GG WP!",10,20);

  EsploraTFT.setTextSize(1);
  EsploraTFT.text("Final Score:",10,45);


  if(DIFF_NEXT == MEDIUM) {
    score *= 2;
  }

  
  else if(DIFF_NEXT == HARD) {
    score *= 3;
  }

  
 else if (DIFF_NEXT == VERYHARD) {
    score *= 5;
  }


  scoreConv = String(score);
  scoreConv.toCharArray(scoreOutput,3);
  EsploraTFT.text(scoreOutput,100, 45);



  int first = EEPROM.read(0); // reads old high score values
  int second = EEPROM.read(1);
  int third = EEPROM.read(2);


  if(score>=first) { // if new score is higher, write it to a given position
    EEPROM.write(0, score);
  }
  else if(score>=second) {
    EEPROM.write(1, score);
  }
  else if(score>=third) {
    EEPROM.write(2, score);
  }


  if(score>=third) { // if new high score is achieved, write a text
    EsploraTFT.text("New highscore!",10,60);
    }
  
}




//Arduino default functions
void setup() {
 EsploraTFT.begin();
  MASTER = MENU;
  MENU_POS = PLAY;
  MENU_NEXT = PLAY;
  primaryMenu();
 
}

void loop() {
  switch (MASTER) {
    case MENU:
      switch(MENU_POS) {

        case PLAY:
          if(buttonEvent(UP)){
            MENU_NEXT = RESET;
            mainRedraw();      
          }

           if(buttonEvent(DOWN)){
            MENU_NEXT = HIGHSCORES;
            mainRedraw();      
          } 

             if(buttonEvent(RIGHT)){
            MENU_NEXT = PLAY;
            MASTER_NEXT = DIFF_SELECT;
            DIFF_POS = MEDIUM;
            DIFF_NEXT = MEDIUM;
            diffMenu();
                 
          }   
          break;
        
        case HIGHSCORES:

          if(buttonEvent(UP)){
            MENU_NEXT = PLAY;
            mainRedraw();      
          }

          if(buttonEvent(DOWN)){
            MENU_NEXT = RESET;
            mainRedraw();      
          } 

          if(buttonEvent(RIGHT)){
            MENU_NEXT = HIGHSCORES;
            MASTER_NEXT = SCOREBOARD;
            showScore();
                 
          }  
          break;

        case RESET:

          if(buttonEvent(UP)){
            MENU_NEXT = HIGHSCORES;
            mainRedraw();      
          }

          if(buttonEvent(DOWN)){
            MENU_NEXT = PLAY;
            mainRedraw();      
          } 

          if(buttonEvent(RIGHT)){
            MENU_NEXT = RESET;
            clearScore();                
          }  
          break;                   
      }
     MENU_POS = MENU_NEXT; 
  
  break;

  case SCOREBOARD:
      if(buttonEvent(DOWN) || buttonEvent(UP) || buttonEvent(LEFT) || buttonEvent(RIGHT)) {
            MASTER_NEXT = MENU;
            MENU_POS = HIGHSCORES;
            MENU_NEXT = HIGHSCORES;
            backgroundBlack();
            primaryMenu();
            mainRedraw();
      }
      break;
  case DIFF_SELECT:
   switch(DIFF_POS) {
    case EASY:
       if(buttonEvent(UP)) {
          DIFF_NEXT = VERYHARD;
          diffRedraw();           
          }
    
       if(buttonEvent(DOWN)) {
          DIFF_NEXT = MEDIUM;
          diffRedraw();           
          }



       if(buttonEvent(RIGHT)) {
            MASTER_NEXT = GAME;
            gameSetup();
          }

          if(buttonEvent(LEFT)) {
          MASTER_NEXT = MENU;
          primaryMenu();                   
          }   
        break; 

    case MEDIUM:
       if(buttonEvent(UP)) {
          DIFF_NEXT = EASY;
          diffRedraw();           
          }
    
       if(buttonEvent(DOWN)) {
          DIFF_NEXT = HARD;
          diffRedraw();           
          }



       if(buttonEvent(RIGHT)) {
            MASTER_NEXT = GAME;
            gameSetup();
          }

          if(buttonEvent(LEFT)) {
          MASTER_NEXT = MENU;
          primaryMenu();                    
          }   
        break; 

        
    case HARD:
       if(buttonEvent(UP)) {
          DIFF_NEXT = MEDIUM;
          diffRedraw();           
          }
    
       if(buttonEvent(DOWN)) {
          DIFF_NEXT = VERYHARD;
          diffRedraw();           
          }


       if(buttonEvent(RIGHT)) {
            MASTER_NEXT = GAME;
            gameSetup();  
          }

          if(buttonEvent(LEFT)) {
          MASTER_NEXT = MENU;
          primaryMenu();                    
          }   
        break;     
   

    case VERYHARD:
       if(buttonEvent(UP)) {
          DIFF_NEXT = HARD;
          diffRedraw();           
          }
    
       if(buttonEvent(DOWN)) {
          DIFF_NEXT = EASY;
          diffRedraw();           
          }


       if(buttonEvent(RIGHT)) {
            MASTER_NEXT = GAME;
            gameSetup();
          }

        if(buttonEvent(LEFT)) {
            MASTER_NEXT = MENU;
            primaryMenu();               
          }   
        break; 
       }
       DIFF_POS=DIFF_NEXT;

        break;

    case GAME:
      timerCur = millis(); 
      
      if(buttonEvent(UP)) {
        SNAKE_DIRECTION = NORTH;          
      }
      else if(buttonEvent(DOWN)) {
       SNAKE_DIRECTION = SOUTH;          
      }
      else if(buttonEvent(RIGHT)) {
       SNAKE_DIRECTION = EAST;       
      }
      else if(buttonEvent(LEFT)) {
       SNAKE_DIRECTION = WEST; 
      }

      if ((timerCur - timerSaved) > tickRate){
        if(!gameLogic()) {
          MASTER_NEXT = POST_GAME;
          gameOver();
        }

        timerSaved = timerCur;
      }
      break;  

     case POST_GAME:
      if(buttonEvent(DOWN) || buttonEvent(UP) || buttonEvent(LEFT) || buttonEvent(RIGHT)) {
            MASTER_NEXT = MENU;
            MENU_POS = PLAY;
            MENU_NEXT = PLAY;
            backgroundBlack();
            primaryMenu();
      }
      break;
  }
 MASTER = MASTER_NEXT; 
}

#include <GL/glut.h>

#include <stdio.h>
#include "stdbool.h"
#include <stdlib.h> 
#include <math.h>

#include "windows.h"

#include "graphics.h"

//Key input setup
#define KEY_COUNT 256 

bool keys[KEY_COUNT];

//Customizable
#define BALL_SIZE 15
#define PADDLE_WIDTH 30
#define PADDLE_HEIGHT 80
#define PADDLE_DISTANCE_FROM_WALL 10

//Frame timer
unsigned long lastUpdate = 0;

//Ball speed
const unsigned long FRAME_TIME = 2; 

//Chance of winning is this subtracted by 1
#define ENEMY_SPEED_MULTIPLIER 0.91

//Score text padding from top
#define SCORE_PADDING 50

//Game values (non customizable)
float CENTER_X = (WINDOW_W / 2);
float CENTER_Y = (WINDOW_H / 2);

//Positional values
Vector2 enemyPos = (Vector2){WINDOW_W - PADDLE_DISTANCE_FROM_WALL, WINDOW_H / 2};

Vector2 mousePos = (Vector2){0, 0};

Vector2 ballPos = (Vector2){0, 0};
Vector2 ballVel = (Vector2){0, 0};

Vector2 lastBallPos = (Vector2){0, 0};

//X is player and Y is enemy
Vector2 score = (Vector2){0, 0};

char playerScore[1000];
char enemyScore[1000];

void init()
{
  glClearColor(0, 0, 0, 0);

  gluOrtho2D(0, WINDOW_W, WINDOW_H, 0);
}

void ResetScore()
{
  //Reset score
  score = (Vector2){0, 0};

  // Convert scores to string buf
  itoa(score.y, enemyScore, 10);
  itoa(score.y, playerScore, 10);
}

void GameStart()
{
  RandomInitialization();

  ballPos.x = CENTER_X;
  ballPos.y = CENTER_Y;

  //Pick velocity
  ballVel.x = PickVelocity();
  ballVel.y = PickVelocity();
}

void display() 
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Draw squares
  glBegin(GL_QUADS);

  //Ball
  glColor3f(0, 1, 1);

  glVertex2f(ballPos.x, ballPos.y + BALL_SIZE);
  glVertex2f(ballPos.x + BALL_SIZE, ballPos.y + BALL_SIZE);
  glVertex2f(ballPos.x + BALL_SIZE, ballPos.y);
  glVertex2f(ballPos.x, ballPos.y);

  //Paddles
  glColor3f(1, 1, 1);

  //Friendly paddle
  glVertex2f(PADDLE_DISTANCE_FROM_WALL, mousePos.y - PADDLE_HEIGHT);
  glVertex2f(PADDLE_DISTANCE_FROM_WALL + PADDLE_WIDTH, mousePos.y - PADDLE_HEIGHT);
  glVertex2f(PADDLE_DISTANCE_FROM_WALL + PADDLE_WIDTH, mousePos.y);
  glVertex2f(PADDLE_DISTANCE_FROM_WALL, mousePos.y);

  //Enemy paddle
  glVertex2f(WINDOW_W - PADDLE_DISTANCE_FROM_WALL, enemyPos.y - PADDLE_HEIGHT);
  glVertex2f(WINDOW_W - PADDLE_DISTANCE_FROM_WALL - PADDLE_WIDTH, enemyPos.y - PADDLE_HEIGHT);
  glVertex2f(WINDOW_W - PADDLE_DISTANCE_FROM_WALL - PADDLE_WIDTH, enemyPos.y);
  glVertex2f(WINDOW_W - PADDLE_DISTANCE_FROM_WALL, enemyPos.y);

  glEnd();

  //Draw text
  glBegin(GL_BITMAP);

  //Player score
  drawText(PADDLE_WIDTH + PADDLE_WIDTH, SCORE_PADDING, GLUT_BITMAP_HELVETICA_18, &playerScore, 1, 1, 1);

  //Enemy score
  drawText(WINDOW_W - PADDLE_WIDTH - PADDLE_WIDTH - PADDLE_DISTANCE_FROM_WALL, SCORE_PADDING, GLUT_BITMAP_HELVETICA_18, &enemyScore, 1, 1, 1);

  glutSwapBuffers();
}

void keyDown(unsigned char key, int x, int y) 
{
  keys[key] = true; 
}

void keyUp(unsigned char key, int x, int y) 
{
  keys[key] = false; 
}

void update()
{
  //R to restart the game
  if (keys['r'])
  {
    ResetScore();
    GameStart();
  }

  ///Frame timer
  unsigned long currentTime = glutGet(GLUT_ELAPSED_TIME);

  if (currentTime - lastUpdate < FRAME_TIME) return;

  lastUpdate = currentTime;

  //Update ball position
  ballPos.x += ballVel.x;
  ballPos.y += ballVel.y;

  //Move enemy to ball position, only if the balls coming in its direction
  if (ballPos.x > lastBallPos.x)
  {
    if (ballPos.y > enemyPos.y - PADDLE_HEIGHT / 2) enemyPos.y += 1 * ENEMY_SPEED_MULTIPLIER;
    else if (ballPos.y < enemyPos.y - PADDLE_HEIGHT / 2) enemyPos.y -= 1 * ENEMY_SPEED_MULTIPLIER;
  }
  else //Move to center if balls going away
  {
    if (enemyPos.y - PADDLE_HEIGHT / 2 < CENTER_Y) enemyPos.y += 1 * ENEMY_SPEED_MULTIPLIER;
    else if (enemyPos.y - PADDLE_HEIGHT / 2 > CENTER_Y) enemyPos.y -= 1 * ENEMY_SPEED_MULTIPLIER;
  }

  //Check collisions

  //Enemy score
  if (ballPos.x <= 0 || ballPos.x + BALL_SIZE <= 0)
  {
    score.y += 1;

    // Convert score to string buf
    itoa(score.y, enemyScore, 10);

    //Reset ball
    GameStart();
  }

  //Player score
  if (ballPos.x >= WINDOW_W || ballPos.x + BALL_SIZE >= WINDOW_W)
  {
    score.x += 1;

    // Convert score to string buf
    itoa(score.x, playerScore, 10);

    //Reset ball
    GameStart();
  }

  //Deflect off top or bottom of screen
  if (ballPos.y >= WINDOW_H || ballPos.y + BALL_SIZE >= WINDOW_H || ballPos.y <= 0 || ballPos.y + BALL_SIZE <= 0) ballVel.y = -ballVel.y;

  //Check paddle collisions with player
  if (ballPos.x <= PADDLE_WIDTH + PADDLE_DISTANCE_FROM_WALL)
  {
    if (ballPos.y <= mousePos.y && ballPos.y >= mousePos.y - PADDLE_HEIGHT || ballPos.y + BALL_SIZE <= mousePos.y && ballPos.y + BALL_SIZE >= mousePos.y - PADDLE_HEIGHT)
    {
      ballVel.x = -ballVel.x;

      if (ballPos.x < PADDLE_WIDTH + PADDLE_DISTANCE_FROM_WALL) { ballVel.y = -ballVel.y; ballVel.x = -ballVel.x; }
    }
  } //check collisions with enemy
  else if (ballPos.x + BALL_SIZE >= WINDOW_W - PADDLE_DISTANCE_FROM_WALL - PADDLE_WIDTH)
  {
    if (ballPos.y <= enemyPos.y && ballPos.y >= enemyPos.y - PADDLE_HEIGHT || ballPos.y + BALL_SIZE <= enemyPos.y && ballPos.y + BALL_SIZE >= enemyPos.y - PADDLE_HEIGHT)
    {
      ballVel.x = -ballVel.x;

      if (ballPos.x + BALL_SIZE > WINDOW_W - PADDLE_DISTANCE_FROM_WALL - PADDLE_WIDTH) { ballVel.y = -ballVel.y; ballVel.x = -ballVel.x; }
    }
  }

  //Set last ball position for ai enemy
  lastBallPos.x = ballPos.x;
  lastBallPos.y = ballPos.y;

  glutPostRedisplay();
}

void mouseMotion(int x, int y) 
{
  mousePos = (Vector2){x, y};
}

void windowResize(int width, int height)
{
  glutReshapeWindow(WINDOW_W, WINDOW_H);
}

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);    

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);        
    glutInitWindowSize(WINDOW_W, WINDOW_H);        
    
    glutCreateWindow("Chockey");  

    init();

    ResetScore();
    GameStart();

    for (int i = 0; i < KEY_COUNT; i++) 
    {
      keys[i] = false;
    }

    glutDisplayFunc(display); 
    glutIdleFunc(update);      

    glutPassiveMotionFunc(mouseMotion);
    glutKeyboardFunc(keyDown);  
    glutKeyboardUpFunc(keyUp);  

    glutReshapeFunc(windowResize);

    glutSetCursor(GLUT_CURSOR_NONE);

    glutMainLoop();      
    return 0;   
}
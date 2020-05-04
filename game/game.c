#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <shape.h>
#include <abCircle.h>
#include <p2switches.h>
#include <sound.h>
#include "game.h"

#define RED_LED BIT6
#define BALL_SPEED 3

static Region              fieldFence;

u_int                      bgColor               = COLOR_BLACK;
int                        redrawScreen          = 0;

static short               count                 = 0;
static unsigned int        scorePlayerLeft       = 0;
static unsigned int        scorePlayerRight      = 0;

const static AbRect        rectPaddleRight       = {
                                                    abRectGetBounds,
                                                    abRectCheck,
                                                    {12,1}
};
const static AbRect        rectPaddleLeft        = {
                                                    abRectGetBounds,
                                                    abRectCheck,
                                                    {12,1}
};
const static AbRectOutline outlineField          = {
                                                    abRectOutlineGetBounds,
                                                    abRectOutlineCheck,
                                                    {
                                                     screenWidth/2 - 10,
                                                     screenHeight/2 - 1
                                                    }
};
static Layer               layerField            = {
                                                    (AbShape *) &outlineField,
                                                    {screenWidth/2, screenHeight/2},
                                                    {0,0}, {0,0},
                                                    COLOR_WHITE,
                                                    0
};
static Layer               layerPaddleLeft        = {
                                                     (AbShape *)&rectPaddleLeft,
                                                     {screenWidth/2, 10},
                                                     {0,0}, {0,0},
                                                     COLOR_WHITE,
                                                     &layerField,
};
static Layer               layerPaddleRight       = {
                                                     (AbShape *)&rectPaddleRight,
                                                     {screenWidth/2, screenHeight-10},
                                                     {0,0}, {0,0},
                                                     COLOR_WHITE,
                                                     &layerPaddleLeft,
};
static Layer               layerBall               = {
                                                      (AbShape *)&circle2,
                                                      {(screenWidth/2), (screenHeight/2)},
                                                      {0,0}, {0,0},
                                                      COLOR_WHITE,
                                                      &layerPaddleRight,
};

static transform_t         transformPaddleLeft      = {
                                                       &layerPaddleLeft,
                                                       { 0 , 0 },
                                                       HandleCollidePaddleLeft,
                                                       0
};
static transform_t         transformPaddleRight     = {
                                                       &layerPaddleRight,
                                                       { 0 , 0 },
                                                       HandleCollidePaddleRight,
                                                       &transformPaddleLeft
};
static transform_t         transformBall            = {
                                                       &layerBall,
                                                       { 1 , -BALL_SPEED },
                                                       0,
                                                       &transformPaddleRight
};

/*
========================================
IsGameOver

  Check for player score limit and
  freeze cpu.
========================================
*/
static void IsGameOver() {
  if ( scorePlayerLeft >= 3 || scorePlayerRight >= 3 ) {
    drawString5x7(screenWidth/2 - 40, screenHeight/2 - 20, "GAME", COLOR_WHITE, COLOR_BLACK);
    drawString5x7(screenWidth/2 + 20, screenHeight/2 - 20, "OVER", COLOR_WHITE, COLOR_BLACK);
    for(int i=400; i <= 800; i+=10) {
      set_buzzer(i);
      __delay_cycles(0.1*8000000);
    }
    stop_buzzer();
    or_sr(0x10);                                            // Paralyze CPU
    WDTCTL = WDTPW | WDTHOLD;                               // Stop Watchdog
  }
  return;
}

/*
========================================
DoRenderLayers

  Redraw layers based on updated
  transform components.
========================================
*/
static void DoRenderLayers(transform_t *transforms, Layer *layers)
{
  int row, col;
  transform_t *transform;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (transform = transforms; transform; transform = transform->next) { /* for each moving layer */
    Layer *l = transform->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */

  for (transform = transforms; transform; transform = transform->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(transform->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer;
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break;
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color);
      } // for col
    } // for row
  } // for moving layer being updated
}

/*
========================================
DoCollideWalls

  Apply transforms to geometry layers
  and check horizontal wall collisions.
========================================
*/
static inline void DoCollideWalls(transform_t *transform, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; transform; transform = transform->next) {

    vec2Add( &newPos, &transform->layer->posNext, &transform->velocity );
    abShapeGetBounds( transform->layer->abShape, &newPos, &shapeBoundary );

    if (
        shapeBoundary.topLeft.axes[0]  < fence->topLeft.axes[0]      ||
        shapeBoundary.botRight.axes[0] > fence->botRight.axes[0]
        )
      {
        set_buzzer(900);
        int velocity = transform->velocity.axes[0] = -transform->velocity.axes[0];
        newPos.axes[0] += (2*velocity);
      }

    transform->layer->posNext = newPos;
  } /**< for transform */
}

/*
========================================
DoCollideGoals

  Check vertical ball - wall collisions.
========================================
*/
static inline void DoCollideGoals(transform_t *ball, Region *goal)
{
  unsigned char goalTouched = 0;
  Vec2 newPos;
  u_char axis;
  Region ballEdge;

  vec2Add( &newPos, &ball->layer->posNext, &ball->velocity );
  abShapeGetBounds( ball->layer->abShape, &newPos, &ballEdge );
  if ( ballEdge.topLeft.axes[1] < goal->topLeft.axes[1] ) {
    goalTouched = 1;
    scorePlayerRight ++;
    ball->velocity.axes[1] = BALL_SPEED;
  }

  if ( ballEdge.botRight.axes[1] > goal->botRight.axes[1] ) {
    goalTouched = 1;
    scorePlayerLeft ++;
    ball->velocity.axes[1] = -BALL_SPEED;
  }

  if ( goalTouched ) {
    set_buzzer(600);
    ball->layer->posNext.axes[0] = screenWidth/2;
    ball->layer->posNext.axes[1] = screenHeight/2;
    IsGameOver();
    count = -300;
  }

  return;
}

/*
========================================
HandleCollidePaddleLeft

  Calculate vertical collision for
  left player.
========================================
*/
static char HandleCollidePaddleLeft(transform_t *ball, transform_t *paddle) {
  Vec2 newPos;
  Region ballEdge;
  Region paddleEdge;
  vec2Add(&newPos, &ball->layer->posNext, &ball->velocity);
  abShapeGetBounds(ball->layer->abShape, &newPos, &ballEdge);
  abShapeGetBounds(paddle->layer->abShape, &paddle->layer->pos, &paddleEdge);
  if ( ballEdge.topLeft.axes[1] < paddleEdge.botRight.axes[1] )
    return 1;
  return 0;
}

/*
========================================
HandleCollidePaddleRight

  Calculate vertical collision for
  right player.
========================================
*/
static char HandleCollidePaddleRight(transform_t *ball, transform_t *paddle) {
  Vec2 newPos;
  Region ballEdge;
  Region paddleEdge;
  vec2Add(&newPos, &ball->layer->posNext, &ball->velocity);
  abShapeGetBounds(ball->layer->abShape, &newPos, &ballEdge);
  abShapeGetBounds(paddle->layer->abShape, &paddle->layer->pos, &paddleEdge);
  if ( ballEdge.botRight.axes[1] > paddleEdge.topLeft.axes[1] )
    return 1;
  return 0;
}

/*
========================================
DoCollidePaddle

  Generic paddle collision check.
========================================
*/
static inline void DoCollidePaddle(transform_t *ball, transform_t *paddle)
{
  Vec2 newPos;
  Region ballEdge;
  Region paddleEdge;
  vec2Add(&newPos, &ball->layer->posNext, &ball->velocity);
  abShapeGetBounds(ball->layer->abShape, &newPos, &ballEdge);
  abShapeGetBounds(paddle->layer->abShape, &paddle->layer->pos, &paddleEdge);
  if (
      paddle->CollisionCheck(ball, paddle)                     &&
      ballEdge.botRight.axes[0] > paddleEdge.topLeft.axes[0]   &&
      ballEdge.topLeft.axes[0]  < paddleEdge.botRight.axes[0]
      )
    {
      set_buzzer(440);
      int velocity;
      velocity = ball->velocity.axes[1] = -ball->velocity.axes[1];
      newPos.axes[1] += (velocity);
      ball->layer->posNext = newPos;
    }
}

/*
============================================================

    Main

============================================================
*/
void main() {

  /*
  ====================

      Initialization

  ====================
  */

  // Prepare Activity Light
  P1DIR |= RED_LED;
  P1OUT |= RED_LED;

  // Initialize Library Functions
  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);
  init_buzzer();
  shapeInit();

  // Initialize Geometry Layers
  layerInit(&layerBall);
  layerDraw(&layerBall);
  layerGetBounds(&layerField, &fieldFence);

  // Enable automatic dog feeder
  enableWDTInterrupts();

  // Enable Interrupts
  or_sr(0x8);

  /*
  ====================

      Game Loop

  ====================
  */

  for(;;) {

    // Paralyze CPU and flicker activity light
    while (!redrawScreen) {
      P1OUT &= ~RED_LED;
      or_sr(0x10);
    }
    P1OUT |= RED_LED;

    // Handle Physics in sync with Watchdog
    redrawScreen = 0;
    DoCollidePaddle(&transformBall, &transformPaddleLeft);
    DoCollidePaddle(&transformBall, &transformPaddleRight);
    DoCollideWalls(&transformBall, &fieldFence);
    DoCollideGoals(&transformBall, &fieldFence);
    DoRenderLayers(&transformBall, &layerBall);

    // Update Score Charts
    char scoreStringLeft [2] = { '0'+scorePlayerLeft, '\0' };
    drawString5x7(3, 20, scoreStringLeft, COLOR_WHITE, COLOR_BLACK);
    char scoreStringRight [2] = { '0'+scorePlayerRight, '\0' };
    drawString5x7(screenWidth-7, screenHeight-20, scoreStringRight, COLOR_WHITE, COLOR_BLACK);

    // Stop Sounds in sync with Watchdog
    stop_buzzer();
  }
}


/*
========================================
wdt_c_handler

  Feed the dog, process inputs, enable
  pausing for players to catchup after
  goal.
========================================
*/
void wdt_c_handler() {
  count ++;
  if (count == 10) {
    unsigned int state = p2sw_read();

    if (!(state & 4))
      transformPaddleRight.velocity.axes[0] = -4;
    else if (!(state & 8))
      transformPaddleRight.velocity.axes[0] = 4;
    else {
      transformPaddleRight.velocity.axes[0] = 0;
    }
    if (!(state & 1))
      transformPaddleLeft.velocity.axes[0] = -4;
    else if (!(state & 2))
      transformPaddleLeft.velocity.axes[0] = 4;
    else {
      transformPaddleLeft.velocity.axes[0] = 0;
    }
    redrawScreen = 1;
    count = 0;
  }
}

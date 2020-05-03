#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <shape.h>
#include <abCircle.h>
#include <p2switches.h>

#define RED_LED BIT6

/*
================================================================================

    Geometry Layers

================================================================================
*/
                                                //
                                //
unsigned int                    score;

const AbRect                    rectPaddleRight = {abRectGetBounds, abRectCheck, {12,1}};
const AbRect                    rectPaddleLeft  = {abRectGetBounds, abRectCheck, {12,1}};
const AbRectOutline             outlineField    = {
        abRectOutlineGetBounds, abRectOutlineCheck,
        {screenWidth/2 - 1, screenHeight/2 - 1}
};

Layer layerField = {
  (AbShape *) &outlineField,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  0
};

Layer layerPaddleLeft = {
  (AbShape *)&rectPaddleLeft,
  {screenWidth/2, 10},
  {0,0}, {0,0},
  COLOR_WHITE,
  &layerField,
};

Layer layerPaddleRight = {
  (AbShape *)&rectPaddleRight,
  {screenWidth/2, screenHeight-10},
  {0,0}, {0,0},
  COLOR_WHITE,
  &layerPaddleLeft,
};

Layer layerBall = {
  (AbShape *)&circle2,
  {(screenWidth/2), (screenHeight/2)},
  {0,0}, {0,0},
  COLOR_WHITE,
  &layerPaddleRight,
};

typedef struct transform_s {
  Layer *layer;
  Vec2 velocity;
  char (*CollisionCheck)(struct transform_s*, struct transform_s*);
  struct transform_s *next;
} transform_t;

static char CollisionPaddleLeft(struct transform_s *ball, struct transform_s *paddle);
static char CollisionPaddleRight(struct transform_s *ball, struct transform_s *paddle);

transform_t transformPaddleLeft = { &layerPaddleLeft, { 0 , 0 }, CollisionPaddleLeft, 0 };
transform_t transformPaddleRight = { &layerPaddleRight, { 0 , 0 }, CollisionPaddleRight, &transformPaddleLeft };
transform_t transformBall = { &layerBall, { 1 , -3 }, 0, &transformPaddleRight };

/*
================================================================================

    Geometry Functions

================================================================================
*/


/*
========================================
DoRenderLayer

  Redraw layers based on updated
  transform components.
========================================
*/

static void DoRenderLayer(transform_t *transforms, Layer *layers)
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
DoGenericPhysics

  Apply transforms to geometry layers
  and check horizontal wall collisions.
========================================
*/
static void DoGenericPhysics(transform_t *transform, Region *fence)
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
        int velocity = transform->velocity.axes[0] = -transform->velocity.axes[0];
        newPos.axes[0] += (2*velocity);
      }

    transform->layer->posNext = newPos;
  } /**< for transform */
}

/*
========================================
CollisionGoal

  Check vertical ball - wall collisions.
========================================
*
static void CollisionGoal(transform_t *ball, Region *goal)
{
  Vec2 newPos;
  u_char axis;
  Region ballEdge;

  vec2Add( &newPos, &ball->layer->posNext, &ball->velocity );
  abShapeGetBounds( ball->layer->abShape, &newPos, &ballEdge );
  if (
      ballEdge.topLeft.axes[1]  < goal->topLeft.axes[1]      ||
      ballEdge.botRight.axes[1] > goal->botRight.axes[1]
      )
    {
      int velocity = ball->velocity.axes[0] = -ball->velocity.axes[0];
      newPos.axes[0] += (2*velocity);
    }

  ball->layer->posNext = newPos;
}
**/

/*
========================================
CollisionPaddleLeft

  Calculate vertical collision for
  left player.
========================================
*/
static char CollisionPaddleLeft(transform_t *ball, transform_t *paddle) {
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
CollisionPaddleRight

  Calculate vertical collision for
  right player.
========================================
*/
static char CollisionPaddleRight(transform_t *ball, transform_t *paddle) {
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
DoPaddleCollision

  Calculate collision for bottom paddle.
========================================
*/
static inline void DoPaddleCollision(transform_t *ball, transform_t *paddle)
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
      int velocity;
      velocity = ball->velocity.axes[1] = -ball->velocity.axes[1];
      newPos.axes[1] += (2*velocity);
      ball->layer->posNext = newPos;
    }
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/*
============================================================

    Main Loop

============================================================
*/
void main() {
  P1DIR |= RED_LED;
  P1OUT |= RED_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();

  layerInit(&layerBall);
  layerDraw(&layerBall);


  layerGetBounds(&layerField, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  for(;;) {
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~RED_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= RED_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    DoPaddleCollision(&transformBall, &transformPaddleLeft);
    DoPaddleCollision(&transformBall, &transformPaddleRight);

    DoGenericPhysics(&transformBall, &fieldFence);
    DoRenderLayer(&transformBall, &layerBall);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler() {
  static short count = 0;
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

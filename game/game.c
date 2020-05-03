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
AbRect rectPaddleLeft = {abRectGetBounds, abRectCheck, {12,1}};
AbRect rectPaddleRight = {abRectGetBounds, abRectCheck, {12,1}};

AbRectOutline outlineField = {	/* playing field */
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

Layer layerPaddleRight = {
  (AbShape *)&rectPaddleRight,
  {screenWidth/2, 10},
  {0,0}, {0,0},
  COLOR_WHITE,
  &layerField,
};

Layer layerPaddleLeft = {
  (AbShape *)&rectPaddleLeft,
  {screenWidth/2, screenHeight-10},
  {0,0}, {0,0},
  COLOR_WHITE,
  &layerPaddleRight,
};

Layer layerBall = {
  (AbShape *)&circle2,
  {(screenWidth/2), (screenHeight/2)},
  {0,0}, {0,0},
  COLOR_WHITE,
  &layerPaddleLeft,
};

typedef struct transform_s {
  Layer *layer;
  Vec2 velocity;
  struct transform_s *next;
} transform_t;

transform_t transformPaddleRight = { &layerPaddleRight, { 0 , 0 }, 0 };
transform_t transformPaddleLeft = { &layerPaddleLeft, { 0 , 0 }, &transformPaddleRight };
transform_t transformBall = { &layerBall, { -4 , 0 }, &transformPaddleLeft };

/*
================================================================================

    Geometry Functions

================================================================================
*/


/*
========================================
DoRenderTransformLayer

  Redraw layers based on updated
  transform components.
========================================
*/

void DoRenderLayer(transform_t *transforms, Layer *layers)
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

//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/*
========================================
DoLayerTransform

  Apply transforms to geometry layers.
========================================
*/
void DoLayerTransform(transform_t *transform, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; transform; transform = transform->next) {
    vec2Add(&newPos, &transform->layer->posNext, &transform->velocity);
    abShapeGetBounds(transform->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = transform->velocity.axes[axis] = -transform->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */
    transform->layer->posNext = newPos;
  } /**< for transform */
}

void DoCollisionLeft(transform_t *ball, Region *paddle)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  vec2Add(&newPos, &ball->layer->posNext, &ball->velocity);
  if (
       ( ball->layer->posNext->axes[1] > paddle->layer->pos->axes[1] ) && // Check pass bottom
       ( ball->layer->posNext->axes[0] > paddle->botRight.axes[axis] ) && // Check pass b_right over p_left
      ) {
    int velocity = ball->velocity.axes[axis] = -ball->velocity.axes[axis];
    newPos.axes[axis] += (2*velocity);
  }	/**< if outside of fence */
  ball->layer->posNext = newPos;
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED,
 *  and handles the rendering for the screen
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
    DoLayerTransform(&transformBall, &fieldFence);
    DoRenderLayer(&transformBall, &layerBall);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler() {
  static short count = 0;
  P1OUT |= RED_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 10) {
    unsigned int state = p2sw_read();

    if (!(state & 1))
      transformPaddleLeft.velocity.axes[0] = -4;
    else if (!(state & 2))
      transformPaddleLeft.velocity.axes[0] = 4;
    else {
      transformPaddleLeft.velocity.axes[0] = 0;
    }
    if (!(state & 4))
      transformPaddleRight.velocity.axes[0] = -4;
    else if (!(state & 8))
      transformPaddleRight.velocity.axes[0] = 4;
    else {
      transformPaddleRight.velocity.axes[0] = 0;
    }



    redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~RED_LED;		    /**< Green LED off when cpu off */
}

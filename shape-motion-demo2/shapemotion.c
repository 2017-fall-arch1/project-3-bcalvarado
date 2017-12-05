/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

char str[3];
int numL = 0;
int numR = 0;

AbRect rect10 = {abRectGetBounds, abRectCheck, {3,15}}; /**< 10x10 rectangle */ //RED RECTANGLE
AbRect rect11 = {abRectGetBounds, abRectCheck, {3,15}}; /**< 10x10 rectangle */ //BLACK RECTANGLE
//AbRArrow rightArrow = {abRArrowGetBounds, abRArrowCheck, 30};

AbRectOutline fieldOutline = {	/* playing field */ //line drawn
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - .5, screenHeight/2 - .5}
};

Layer fieldLayer = {		/**< Layer outline */
  (AbShape *)&fieldOutline,
  {(screenWidth/2), (screenHeight/2)}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_VIOLET,
  0
};


Layer layer2 = {		/* Layer with a black square */
  (AbShape *) &rect11,
  {(screenWidth/10)-5, (screenHeight/2)},/**< left side */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  &fieldLayer
};


Layer layer1 = {		/**< Layer with a red square */
  (AbShape *)&rect10,
  {screenWidth-20, screenHeight/2}, /**< right side */ //{screenWidth-5, screenHeight/2}, /**< right side */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer2,
};

/**method to add 1 to layer 0**/// LOOOKK ATTT THIIISSS

Layer layer0 = {		/**< Layer with an orange circle */
  (AbShape *)&circle7,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml2 = { &layer2, {0,0}, 0 }; // {whats moving, direction, }
MovLayer ml1 = { &layer1, {0,0}, 0 }; // {whats moving, direction, }
MovLayer ml0 = { &layer0, {3,3}, 0 }; /**< not all layers move */ //circle moving

//MovLayer ml1 = { &layer1, {1,2}, &ml3 }; 
//MovLayer ml0 = { &layer0, {2,1}, &ml1 }; 




void movLayerDraw(MovLayer *movLayers, Layer *layers){
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
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


Region fence;// = {{screenWidth-5, (screenHeight/2)}, {0, 0}}; /**< Create a fence region */


/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, MovLayer *ml1, MovLayer *ml2,  Region *fence)
{
//char str[1];
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
      buzzer_set_period(0);
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);//ball will disapear
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);//ball boundary
    for (axis = 0; axis < 2; axis ++) {
         drawString5x7(10,10, "YOU:", COLOR_BLACK, COLOR_WHITE);
         drawString5x7(70,10, "COMPUTER:", COLOR_BLACK, COLOR_WHITE);
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])){// axes at pos 0 contains L&R and pos  T&B
        drawString5x7(40,60, "HIT T/B", COLOR_BLACK, COLOR_WHITE);
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
        
        /*LEFT*/
        else if (shapeBoundary.topLeft.axes[0] < fence->topLeft.axes[0]){// axes at pos 0 contains L&R and pos 1
            drawString5x7(40,60, "HIT LEFT", COLOR_BLACK, COLOR_WHITE);
            buzzer_set_period(1000);
            numL += 1;
            //mlAdvance(&ml1, &layer1);
            //newPos.axes[0] += 0; // (screenWidth/2);
            //newPos.axes[1] += 0;//(screenHeight/2);
            if(numL > 2){
                drawString5x7(40,60, "YOU LOSE!:", COLOR_BLACK, COLOR_WHITE);
                 buzzer_advance_frequency();
                layerGetBounds(&fieldLayer, &layer1);//this restarts the game!!!!!!!!!!!!!
                numL = 0;
                numR = 0;
                break;
            }
            
        //reset ball
        }
      
      /*RIGHT*/
        else if (shapeBoundary.botRight.axes[0] > fence->botRight.axes[0]){
            drawString5x7(40,60, "HIT RIGHT", COLOR_BLACK, COLOR_WHITE);
            numR += 1;
           // mlAdvance(&ml1, &layer1);
            //p2sw_init(1);
            //newPos.axes[0] += 0;// (screenWidth/2);
            //newPos.axes[1] += 0;//(screenHeight/2);
            if(numR > 2){
                drawString5x7(40,60, "YOU WIN!:", COLOR_BLACK, COLOR_WHITE);
                layerGetBounds(&fieldLayer, &layer1);//this restarts the game!!!!!!!!!!!!!
                numR = 0;
                numL = 0;
                //clearScreen(COLOR_BLACK);
                break;
             }
            //reset ball
        }
         ml->layer->posNext = newPos;
        str[0] = '0' + numL;
        str[1] = '0' + numR;
        drawString5x7(40,30, str, COLOR_BLACK, COLOR_VIOLET);
    } /**< for axis */
  } /**< for ml */

}


u_int bgColor = COLOR_VIOLET;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */

void blackU(Layer *l){
 Vec2 pos;
 Vec2 update = {0, -3};
 vec2Add(&pos, &l->posNext, &update);
 l->posNext = pos; 
}

void blackD(Layer *l){
 Vec2 pos;
 Vec2 update = {0, 3};
 vec2Add(&pos, &l->posNext, &update);
 l->posNext = pos;
}

void redU(Layer *l){
  Vec2 pos;
 Vec2 update = {0, -3};
 vec2Add(&pos, &l->posNext, &update);
 l->posNext = pos;   
}

void redD(Layer *l){
    Vec2 pos;
 Vec2 update = {0, 3};
 vec2Add(&pos, &l->posNext, &update);
 l->posNext = pos; 
}

/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(1);
  buzzer_init();

  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);
  


  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml0, &layer0);
    movLayerDraw(&ml1, &layer0);
    movLayerDraw(&ml2, &layer0);
  }
  
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
   mlAdvance(&ml0, &ml1, &ml2, &fieldFence);
  switch (p2sw_read()){
       case 1: 
            drawString5x7(40,60, "YOU WIN!:", COLOR_BLACK, COLOR_WHITE);
           blackU(&layer2);
            break;
       case 2:
           blackD(&layer2);
           break;
       case 3:
           redU(&layer1);
           break;
       case 4:
           redD(&layer1);
           break;
   }

    /*if(p2sw_read() == 1){
        blackD(&layer2);
    }
    else if(p2sw_read() == 2){
       blackU(&layer2);
    }
    else if(p2sw_read() == 3){
       redD(&layer1);
    }
    else if(p2sw_read() == 4){
       redU(&layer1);
    }*/
      redrawScreen = 1;
      count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}

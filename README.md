# Project 3: Battle for the Dominion of Valhalla
## The Story So Far

Heimdall investigates the murders at the gates of gods.

You are caught tampering with the evidence at the murder site of Odin.

The physics of this world cannot undo your crimes against the wise god,
and you are taken captive to be thrown to Hel; but you have escaped,
clawing your way to the deepest grottoes of Valhalla to finish what you've
started.

## Lay Waist to the Gods of Aesgard!

Your sword drawn, the Aesir draw ever closer with axe and spell at the ready,
and by surrounding you have closed the parameter around you.

Yet you are not locked in the arena with them, they are locked in the area with you...

One by one, you must slaughter your enemies through the penultimate show of metal:

Pong.

## How to build

```
cd lcdtoy-syxxcalypso
make && make load
```

## How to Play

Left Player refers to the top paddle
- Buttons 1 & 2 move top paddle left and right, respectively

Right Player refers to the bottom paddle
- Buttons 3 & 4 move bottom paddle left and right, respectively

Each player is meant to hold the device with one hand, using
their thumb to rock back and forth in order to play the game.

After executing your enemy via. scoring a goal three times, press
onboard switch #1 (P1S2/Reset) to bring forth the next poor bastard
who dares to engage in trial by pong.

## Requirements Met
Your game should meet the following minimum criteria:

- Dynamically renders graphical elements that move and change, has multiple sounds

- Text rendered from ASCII strings.
- Graphical rendering performed by an interruptable foreground task
-- that sleeps when ball stops moving.
- Behavior changes in response to button presses and the progress of time
-- Sensitivity to time and buttons is interrupt driven

Unmet:
-- including one original algorithmically rendered graphical element 
-- using a font other than 5x7

Additional Features:

- Handles collisions properly with shapes and area boundaries
- Produces sounds triggered by game events
-- in a manner that does not cause the game to pause
- Communicates with the player using text
- Responds to user input from buttons

## Grading Criteria

- relevant development tools such as make and emacs
Proudly made using Spacemacs and GNU Make
- use of timer interrupts to control program timing
Watchdog used to drive timing, inputs, and frame delays
- use of switch interrupts to determine when swiches change
Switches using TI-GCC interrupt vector 4 via. p2swLib
- modularization into multiple source files (including header files)
Game logic is not modularized on purpose, all game logic exists in 'game.c'
as it is not driven by separate components other than the included libs.

There are times it is considered good practice to break apart code such as
when there is reuse of components, this is not the case here and therefore
to split any logic from this single file serves to unnecessarily obfuscate
the code i.e. it would be bad practice.

- use of explicit state machines to implement program functionality
Input processors are glorified state machines, they do not use a switch
mechanism as to do so would require convoluted workarounds that are both
hard to read and defeat the efficiency that a jump table would have otherwise
afforded. Wisdom taken from John Carmack.
- ability to develop or modify (and of course use) linked data structures in c
- mature programming
My consistent C styling brings all the boys to the yard.
-- readable, appropriate algorithms, modularization, data structures, symbol names etc
I'm considering a cleanup of included libs.


- Pong Scores
 - that advance through multiple rounds of play
 - that the ball either moves in-front-of or behind
 This makes the rendering ugly, I opted to place scores on the outlier.

## Libraries

- sound
soundLib from project2 refactored for use here.

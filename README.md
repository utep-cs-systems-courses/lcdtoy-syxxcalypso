# Project 3: Battle for the Dominion of Valhalla
## The Story So Far

Heimdall investigates the murders at the gates of gods.

You are caught tampering with the evidence at the murder site of Odin.

The physics of this world cannot undo your crimes against the wise god,
and you are taken captive to be thrown to Hel; but you have escaped,
clawing your way to the deepest grottoes of Valhalla to finish what you've
started.

## Lay Waste to the Gods of Aesgard!

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
- Graphical rendering performed by an interruptable foreground task that sleeps when ball stops moving.
- Behavior changes in response to button presses and the progress of time
- Sensitivity to time and buttons is interrupt driven

Unmet:
- Include one original algorithmically rendered graphical element 

No new elements were written, unless the manipulation of existing element struct data counts.

- using a font other than 5x7

The fonts that you ripped from RogG's EduKit could not be adapted, had a strange issue where the font characters rendered sideway, and at random would be overwritten by other screen elements even if no elements occupied their space.

Additional Features:

- Handles collisions properly with shapes and area boundaries
- Produces sounds triggered by game events in a manner that does not cause the game to pause, except where intentional (In the game over state)
- Communicates with the player using text
- Responds to user input from buttons

## Grading Criteria

- Relevant development tools such as make and emacs

Proudly made using Spacemacs and GNU Make

- Use of timer interrupts to control program timing

Watchdog used to drive timing, inputs, and frame delays. Buttons technically can't be used without interrupts anyway.

- Use of switch interrupts to determine when swiches change

Switches using TI-GCC interrupt vector 4 via. p2swLib

- modularization into multiple source files (including header files)

Game logic is not modularized on purpose, all game logic exists in 'game.c' as the only game-driving systems are rendering and collision, both of which are tightly coupled for efficiency.

To split any logic from this single file serves to unnecessarily obfuscate
the code i.e. it would be bad practice.

- use of explicit state machines to implement program functionality

Input processors are glorified state machines, they do not use switch-case semantics as to do so would require convoluted workarounds that are both
hard to read and defeat the efficiency that a jump table would have otherwise afforded.

- Ability to develop or modify (and of course use) linked data structures in c

This is only necessary in processing the geometry layers and transforms within a single function call, since it walks the list rather than explicitly calling on each layer.

- Mature Programming

My consistent C styling brings all the boys to the yard.

- Readable! Appropriate Algorithms, Modularization, Data Structures, Symbol Names etc

Compare my code to the ones provided in the libs and demos.

- Keeps a Score
- Advances through multiple rounds of play, up to a maximum of 5 per-game
- The ball does not move behind or in front of the score, this seemed a bad practice and the LCD handles multiple geometry layers overlapping very poorly.

## Libraries

- sound
soundLib from project2 refactored for use here.

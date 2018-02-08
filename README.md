### Context

Dragster is an Atari 2600 game released in 1980.

In 1982, Todd Rogers claimed the world record for the game, with an impressive 5.51 time. Activision approved the score, and so did Twig Galaxies, a website dedicated to tracking video game world records.

In 2017, OmniGamer disassembled the game to work on a tool-assisted speedrun, and concluded that it was impossible to get better than a [5.57 time](http://tasvideos.org/5517S.html). 


### This project

This project intends to prove OmniGamer's claim, by finding the most optimal input sequence.

### Usage

```gcc dragster.c -O3 -Wall -Wextra -lm -o dragster && ./dragster```

### Output

```
Now testing all configurations with an initial frame counter equal to 0.
Now testing all configurations with an initial frame counter equal to 2.
Now testing all configurations with an initial frame counter equal to 4.
Now testing all configurations with an initial frame counter equal to 6.
Now testing all configurations with an initial frame counter equal to 8.
Now testing all configurations with an initial frame counter equal to 10.
Now testing all configurations with an initial frame counter equal to 12.
Now testing all configurations with an initial frame counter equal to 14.

The best possible race is 5.57s.
The best subdistance reachable with a 5.57s timer is 98.
49133488 simulations were performed.
```

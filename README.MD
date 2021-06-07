# Celular Automata 2D
Engine for 2D Cellular automata.

There's not much to it right now. I got basic Win32 Software renderer, and a simple example for rendering stretched (destination) pixels. That's it.
*NOTE* I keep the `Win32.cpp` file with the rendering code

**TODO**
- *ongoing* experiment with API design, it's a simple small app so far, so it doesn't really need API to get the job done, but this makes easy to experiment with the structure, so **DO IT**!
- Expand this readme on how to build.
- Timing
- Random genereator
- Get Conway's Game of Life running.
- Make rule engine
  * for now support only "life" rules as they are simpler
  * make rule parser
  * expand to "generations" rule
  * expand to other rules too

  ## How to build
  Just use `build.bat` script.


# References
- [Cellular Automata Rules Catalogue](https://catagolue.hatsya.com/rules)
- [conwaylife wiki (seeds)](https://conwaylife.com/wiki/OCA:Seeds)
- [Cellular Automata Rules](http://psoup.math.wisc.edu/mcell/ca_rules.html)
  * [Life Rules](http://psoup.math.wisc.edu/mcell/rullex_life.html)
  * [Generations Rules](http://psoup.math.wisc.edu/mcell/rullex_gene.html)
- [Life-like cellular automata](https://www.conwaylife.com/wiki/Cellular_automaton#Life-like_cellular_automata)
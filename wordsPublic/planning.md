### To add at some point:
- add an option to choose to scan specific maps regions only.
- More UX focused things (Loading bar for scans, for example).
- Add a search bar to TargetPUp.
- Mark user processes in green in TargetPUp.
- Add an option to start out with unknown target value (snapshots and whatnot).
- Multithreading. Basically required considering GUI always needs to be
  responsive while we do heavy work in the background.
- Random but: Effective Cancel buttons for tasks.
- Definltely refine the UI. I'm essentially using the defaults for now.
- Keyboard shortcuts.
- Sections like "help", "save" and "load" (for saving variable names...Oh. I
    haven't even set up that window yet.) 
- Add a window for "freeze values" and/or for giving a name to certain variables that live in memory, star them and easily edit/view them. 
    (might do both in a single window) 
- Add more functionality to Logs? (Like doing things from command line in logs? Though...That'd be kind of working back to a terminal LOL. Although this might
    be more relevant when we add more functionality to this program.)
- Advanced: Hex viewer and editor? Like what ghidra or windbg has. That'd be a whole beast
    though. If I really start getting into advanced territory (using a debugger,
    following pointers, struct finding and more...) things will get very complex
    so I'll only entertain them as real implementations only after finishing the
    basics.

## Done:

*Just about every single Window has been worked and improved upon.
*Scanner logic is progressing.
*Still struggling with abstractions and managing objects.


Next:

Copied from HitsW.cpp since it's important.

// I really really don't wanna have this read bytes live,
// but the chosen one IS the only one that needs their bytes updated. everything
// else is value. should I really be updating the bytes of every single hit?
// seems redundant. value on the other hand is a must.
// But then there would be times where "bytes around" are just really stale and
// their freshenss depends on being explicitly called by this one object. Is
// that okay?

// verdict:
// we should NOT refresh everything every x miliseconds or something by default.
// Since this is live, data WILL get stale. However it will represent a stable
// snapshot of what "was". We *should* have a button to refresh values and tag
// them and display their previous value. we should also be able to have the
// user be able to choose automatic refresh. Even when opening byte screen, only
// refresh ON DEMAND OR explicit automatic refresh. We CANNOT just arbitarily
// refresh them.
// ***

I've set up hits view. I've set up a...very clunky looking byte view as well.
(Ohh we can add addresses to the left maybe.)

Either way I shouldn't worry about UI look *too* much for now. We can polish it to our heart's content after we get something working. (Although I must admit I haven't ever worked with front end before, so I never had to think about "coloring" "layout" or such things. Hell, this is the first time I'm ever making a non CLI program. We'll see to it later.).




- *Definitely* implement multithreading. Scanner should do most things on another thread, otherwise the GUI dies while all of that heavy work is happening.

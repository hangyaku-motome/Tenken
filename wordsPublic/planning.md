### To add at some point:
- add an option to choose to scan specific maps regions only.
- More UX focused things (Loading bar for scans, for example).
- Add a search bar to TargetPUp.
- Mark user processes in green in TargetPUp.
- Add an option to start out with unknown target value (snapshots and whatnot).
- Multithreading. Basically required considering GUI always needs to be
  responsive while we do heavy work in the background.
- Random but: Effective Cancel buttons for tasks.
- Definitely refine the UI. I'm essentially using the defaults for now.
- Keyboard shortcuts.
- In favourites, a way to config a address so that a notification sounds is made when it changes in value...Or to a specific value.
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

I've done both a lot for these 2 days, and at the same time not enough.

Unfortunately, I did not have much time the day before.

A **lot** has happened.

Switched to docking branch of Dear ImGui. Rewrote some stuff to fit that.

Changed window returns to "Action" struct type.

Added FavouriteW...That's still mostly unfinished.

Some more stuff that frankly I won't bother to check to see and name.


## Next:

- Finish implementation for FavouritesW. Needs editing, naming, bytes viewing, removing favourite, freezing, and logic that also compares to previous bytes. The table is also kind of ugly right now.

Probably a little bit later:

Don't forget for a way for regular refreshing values.

- Still...implement multithreading. I'd assume this'll be easy to do. Since it's clear which functions do the heavy work and should be moved aside to another thread. (various functions in scanner, for example.). This would also mean implementing some sort of "loading bar" while things of interest are happening in the background.

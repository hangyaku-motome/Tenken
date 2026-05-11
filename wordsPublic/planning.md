### To add at some point:
- add an option to choose to scan specific maps regions only.
- More UX focused things (Loading bar for scans, for example).
- Add a search bar to TargetPUp.
- Mark user processes in green in TargetPUp.
- Add an option to start out with unknown target value (snapshots and whatnot).
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

Regular refreshing complete! An almost fully complete context window for favourite (just needs old bytes.)

Regular Refreshing for favourites is *per item* for favourites. so you can choose to update each one at your own wish. (Frankly, this is overengineered.)

For hits, it's a single universal automatic refresh option.

Settled on a name.

This is acceptable enough to call it version 0.1



## Next:

We'll also need to add previous bytes to favourite. It should also print them comparatively...Well, the question is do we only tag changed/unchanged or also add higher/lower logic for the bytes.

option to remove from favourites. the backend is there. just the frontend is lacking. Very simple to add.

After those, we can just move over to windows implementation. Or, we stay here a bit more and refine the UX a bit more.

At the very least:

A search option for target pop up.

More descriptive log (rescans add nothing).







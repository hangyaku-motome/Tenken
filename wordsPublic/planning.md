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

- Implement HitW hit showing, along with refreshing bytes every 200 ms or so.
- Polish the general codespace more. I still think things are too messy. Also, there are definitely inefficiencies here there. However, we shouldn't get stuck on optimization to much from the get-go. First let's work on actually getting it up and working. I think I heard a quote from somewhere from a certain programmer.
"Premature optimization kills programs."? Something along those lines. Makes sense.

- *Definitely* implement multithreading. Scanner should do most things on another thread, otherwise the GUI dies while all of that heavy work is happening.

Oh also I noticed some small debugging leftovers (printf statements and whatnot) but they might as well stay. Not like any of this is anywhere usable code still. These early gits are really there for my own convenience and seeing the journey. Only after do I make something satisfactory would I even start thinking of publishing this out to the public.
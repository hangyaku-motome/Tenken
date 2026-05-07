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

  // again...we might wanna add unsigned or signed-ness into TargetType. We can
  // just remove IsUnsigned with that.

^^ okay I'll actually do that right now. Done.

Changed to GPLv3 from MIT because it is better :)

Added previous value and relative value to previous value to hit table.

Added filtering logic and implemented the second window of SearchW.

Changed some enum structures, and names.

Other general structure changes in SearchW and some others.

Next:

Hit edit logic! I think that is the only major necessity we haven't added. After that, it *should* be technically ready for actual usage.

Although, the goal isn't something "technically ready for actual usage". Not to mention it's missing a lot of things, it is also definitely full of bugs that I need to observe and fix with usage.

Tomorrow, I will finish up the edit logic, add the new "address of interest" window. Refine things.

Optionally, If I do find myself with more time on my hands: Windows Implementation as well.

Probably a little bit later:

Don't forget for a way for regular refreshing values.

- Still...implement multithreading. I'd assume this'll be easy to do. Since it's clear which functions do the heavy work and should be moved aside to another thread. (various functions in scanner, for example.). This would also mean implementing some sort of "loading bar" while things of interest are happening in the background.

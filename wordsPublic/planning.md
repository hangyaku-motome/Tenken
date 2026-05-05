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

Moved window info into each window object.

Implemented rescan logic on the byte level (no comparative rescaning yet).
Added refresh buttons to HitsW.

Next:

showing previous value.
showing previous value comparative tagging (higher, lower, unchanged, for hit labels).
setting up rescan logic in SearchW (changed, unchanged, higher, lower, specific value for filtering hits).

With this kind of setup, even if you choose "changed" you aren't going to just get hits that are all labeled "changed". you'd get all changed ones which would be higher or lower ones.

Even if you choose specific value, it will still show you if it is higher or lower as a value.

(for string higher or value obviously shouldn't apply. We'll need to specially config some things to work with string.)

Then...Hit edit logic...

Ideally, I was imagining being able to just double click on the hit value in hits table to edit the value. after editing, we would ONLY update value, old value, ONLY hit bytes inside around_bytes.

A bit more later:

New window for "addresses of interest". Where you can tag addresses to:
Regularly monitor their values.
Freeze them at a certain value.




- *Definitely* implement multithreading. Scanner should do most things on another thread, otherwise the GUI dies while all of that heavy work is happening.

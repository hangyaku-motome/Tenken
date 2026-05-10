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

Multhithreading implemented.

Heavy scans are given to another thread.

Freezing has it's own thread.

Favourites window is functional. Can freeze values, can label them, and can refresh the value. Previous byte comparision lacking. Won't be hard to add though.

I also did some rewrites here and there. It all still feels like a mess, just less like one.

I think I will keep the hard line for not allowing windows to edit values outside what they directly own. So...After checking the freeze button, it's not set as so inside the object, it will return an action to resolve in main that does that. 

The reason is...Well, it just feels like a more flexible way of doing it. State changes themselves shouldn't be dependent on windows. Only the requests should be done through the window.

^^ Though on that note SearchW does not follow that. We might want to change that as well.



## Next:

No regular refreshing option, still. This wouldn't be hard to implement...Probably. A small field next to "refresh all hits" for refresh x second logic. For favourite, we should set it up per entry, and it'll be next to refresh context button.

...Each change to the entry can emit an action. Action{REFRESH_REGULAR, HIT, [seconds somewhere, probably new field.]}, each time we recieve this we'll have a thread that keeps up with it's pace. When set to 0, it will be joined.

Same logic is...Not necessary for favourite. since the scale is really small. end of each field we can call RefreshFavourite and will refresh a favourite if it's time arrives. Although I'll have to learn how to keep track of time properly like that.

We'll also need to add previous bytes to favourite. It should also print them comparatively...Well, the question is do we only tag changed/unchanged or also add higher/lower logic for the bytes.

Oh, and an option to remove from favourites. the backend is there. just the frontend is lacking. Very simple to add.




After those, we can just move over to windows implementation. Or, we stay here a bit more and refine the UX a bit more.

At the very least:

A search option for target.

More descriptive log (rescans add nothing).







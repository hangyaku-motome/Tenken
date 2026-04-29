### To add at some point:
- add an option to choose to scan specific maps regions only.
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

## Next to do:
Build up SearchW. Confirm each choice is reflected in log; as should final
scan params be. After the GUI, comes the linux implementation...which
requires: Reading maps,and then mem using that as the guide. For something as
big as all of a process' memory...I guess we'll just have a struct of sections of raw bytes. We shouldn't
encapsulate this data to a single object. (especially considering we might
wanna do other things with it in the future. Like a potential hex viewer.)
We probably wanna keep not just mem contents but also maps contents in main
as well. since both platforms have these, they can be universally worked on. It'd be unnecessary or maybe even nonsense to encapsulate them.


(Also something I should point out. I use "we" often however it is just me, a
single person. I sometimes use "we". Just know it's a singular person.)


### The biggest 2 leaps this project has that I never really implemented before is:
- A GUI, and cross-platform thinking (especially windows. I'll have to learn
how that side does some things.).
+ It's fun. It forces you to think about boundaries and roles more clearly.


An impractical program is as good as fish in a desert. Not to mention, a hefty weight to bear. I should always try to make something practical, first to me, then to others. Both distinctly as important.

My vision for this program is something lightweight yet powerful. Functional, practical, and easy to expand and work on. I'll see to it that I will realise this to the best degree ***I* can**.
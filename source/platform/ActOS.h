// We are supposed to make a:
// A class with virtual members, that will be filled in platform_linux or
// platform_windows. For now since I'll need to finish the linux side first,
// I'll only make platform_linux. However, this is preperation for when this
// program does need to work on windows as well. Also one of the reasons why I
// decided to abstract the windows with LogW, SearchW and so on. Main was
// starting to get messy anyways.

// Uhh this is broken for now I'll actually set it up later for now I'm just
// going to put the relevant linux functions in platform_linux and work from
// there.
#pragma once

// class ActOS {
// public:
//   virtual ~ActOS() = default;
// };

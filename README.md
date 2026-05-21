# Tenken

A memory scanner made in C++ with Dear ImGui that is cross-platform (Linux and Windows).

**Heavily work in progress and unstable as of now.**

## Main Features
- Search for values in primitive types (int8-64, float, double), in strings, or in AOB with wildcards, which returns "hits".
- Filter those hits based on relative change (higher, lower, changed, unchanged) or by certain value.
- Edit the value of said hits.
- A favourite section to pin addresses of interest, plus give them descriptions; and "freeze" them at certain values.
- A hex viewer, where you can view and edit the bytes around an address in a detailed fashion (you can copy the address of a hit by right clicking, or you can copy the address of a region from Utils -> View Regions).

![screenshot](docs/screenshot.png)

## Building

### Dependencies

- CMake 3.15+
- C++20 compiler
- OpenGL

GLFW is fetched automatically by CMake. Dear ImGui is included as a git submodule.

### Linux

```bash
git clone --recursive https://github.com/hangyaku-motome/Tenken
cd Tenken
./build.sh
```

Other build modes:

```bash
./build.sh debug
./build.sh san     # address + undefined sanitizer
./build.sh tsan    # thread sanitizer
```

### Windows (cross-compiled from Linux)

```bash
# Fedora
sudo dnf install mingw64-gcc mingw64-gcc-c++

./build.sh win
```

## Running

#### **Linux:** Requires root or a relaxed `ptrace_scope` setting to read other processes' memory.

Recommended:

```bash
sudo ./build-linux/Tenken
```
Otherwise:

```bash
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
```


#### **Windows:** Run as administrator.

Note: Windows Defender will probably flag the executable as malware. This happens because the program uses calls that malware may use (like ReadProcessMemory and WriteProcessMemory). You'll need to add an exception for it from Windows Defender.

## License

GPL-3.0-or-later. See [LICENSE.txt](LICENSE.txt).

## Contributing

I would highly appreciate any forms of feedback, criticism, and bug reports. You're encouraged to open an issue with that in mind.

## Author

Created and maintained by Hangyaku.

### Other Remarks

This is a personal project, align your expectations accordingly.

Also see [TODO.md](docs/TODO.md) for goals.
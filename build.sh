#!/usr/bin/env bash

if [ "$1" = "win" ]; then
    cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -DCMAKE_BUILD_TYPE=Release
    cmake --build build-win
elif [ "$1" = "debug" ]; then
    cmake -B build-linux -DCMAKE_BUILD_TYPE=Debug
    cmake --build build-linux
    ln -sf build-linux/compile_commands.json compile_commands.json
elif [ "$1" = "san" ]; then
    cmake -B build-linux -DCMAKE_BUILD_TYPE=Debug -DENABLE_SAN=ON
    cmake --build build-linux
    ln -sf build-linux/compile_commands.json compile_commands.json
elif [ "$1" = "tsan" ]; then
    cmake -B build-linux -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
    cmake --build build-linux
    ln -sf build-linux/compile_commands.json compile_commands.json
else
    cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
    cmake --build build-linux
    ln -sf build-linux/compile_commands.json compile_commands.json
fi


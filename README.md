# TENASCII

TENASCII is a C-based ASCII art engine and terminal rendering library.

## Project Structure

```
TENASCII/
├── include/
│   └── tenascii.h     # Header file with struct and function definitions
├── src/
│   ├── main.c         # Application entry point
│   └── tenascii.c     # ASCII canvas implementation
├── Makefile           # GCC build script
├── CMakeLists.txt     # CMake build configuration
└── README.md
```

## Quick Start & Building

### Option 1: Direct GCC Compilation

```powershell
gcc -Wall -Iinclude src/main.c src/tenascii.c -o tenascii.exe
.\tenascii.exe
```

### Option 2: Using Makefile

```powershell
mingw32-make
.\tenascii.exe
```

### Option 3: Using CMake

```powershell
mkdir build
cd build
cmake ..
cmake --build .
.\bin\tenascii.exe
```

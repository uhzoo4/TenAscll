# TENASCII & TENET ⏳

A high-density terminal particle playground and ASCII art engine built in C. 

Inspired by Nolan's *TENET*, it uses Unicode braille sub-pixel rendering (220×168 effective resolution in a 110×42 terminal) to simulate **temporal pincer movements**, **spatial turnstile entropy inversion**, and **matter-antimatter paradox collisions**.

---

## 🚀 Quick Start

### Prerequisites
- Any standard C compiler (`gcc`, `clang`, or MinGW)
- Terminal supporting UTF-8 and ANSI colors (Windows Terminal, PowerShell, or any Linux/macOS terminal)

### Build & Run

```bash
# Build both the ASCII Engine & TENET Simulation using Makefile
make

# Run the TENET Time-Inversion Playground
./tenet.exe    # Or ./tenet on Linux/macOS

# Run the ASCII Engine demo
./tenascii.exe
```

*Or compile directly with GCC:*
```bash
gcc -O2 -o tenet.exe src/tenet_sim.c -lm
```

---

## 🎮 Controls (`tenet`)

| Key | Action | Description |
|:---:|:---|:---|
| `SPACE` | **Forward Wave** | Spawns forward entropy particles (Blue) advancing under gravity |
| `p` | **Pincer Squad** | Launches a forward squad from the opposite direction |
| `a` | **The Algorithm** | Scripted temporal pincer choreography (Squad A → Turnstile → Squad B) |
| `i` | **Force Invert** | Instantly rewinds all active particles |
| `r` | **Reset** | Clears the simulation field |
| `q` | **Quit** | Exits the simulation |

---

## 🌀 Key Concepts

- **Braille Sub-Pixel Framebuffer**: Uses Unicode braille patterns (`U+2800`–`U+28FF`) to fit 8 sub-pixels in every character cell, providing 4x visual resolution.
- **The Turnstile**: Passing through the central spatial ring automatically flips a particle's entropy. Instead of generating reverse physics, it rewinds its exact recorded trajectory frame-by-frame.
- **Paradox Collisions**: When forward matter (Blue) meets its own inverted self (Red), both are annihilated in a white-hot smouldering ember core.

---

## 🛠️ Project Structure

```
TENASCII/
├── include/
│   └── tenascii.h     # Core ASCII canvas & ANSI styling header
├── src/
│   ├── tenet_sim.c    # Interactive braille temporal particle simulation
│   ├── tenascii.c     # ASCII canvas & box drawing implementation
│   └── main.c         # TENASCII engine entry point
├── BLUEPRINT.md       # Architectural blueprint & mechanics spec
├── Makefile           # GCC build script
└── CMakeLists.txt     # Cross-platform CMake configuration
```

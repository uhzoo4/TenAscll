# TENASCII & TENET ⏳

A high-density terminal particle playground built in C using modular architecture.

Inspired by Nolan's *TENET*, it uses Unicode braille sub-pixel rendering (220×168 effective resolution in a 110×42 terminal) to simulate **temporal pincer movements**, **spatial turnstile entropy inversion**, and **matter-antimatter paradox collisions**.

---

## 🚀 Quick Start

### Prerequisites
- Any standard C compiler (`gcc`, `clang`, or MinGW)
- Terminal supporting UTF-8 and ANSI colors (Windows Terminal, PowerShell, or Linux/macOS terminal)

### Build & Run

**On Windows (PowerShell / CMD):**
```powershell
# Refresh environment PATH in active session if needed
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Build executable using MinGW make
mingw32-make

# Run the TENET Interactive Simulation!
.\tenet.exe
```

**On Linux / macOS:**
```bash
make
./tenet
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

## 🛠️ Modular Project Structure

```
TENASCII/
├── CMakeLists.txt     # Modular CMake configuration
├── Makefile           # GCC build script for modular sources
├── .gitignore         # Build exclusions
├── README.md          # Project documentation
├── BLUEPRINT.md       # Architecture spec & mechanics blueprint
├── include/
│   └── tenet.h        # Unified header with structs & module prototypes
└── src/
    ├── main.c         # Application entry point & game loop
    ├── term.c         # Cross-platform raw terminal & non-blocking I/O
    ├── particle.c     # Particle physics, turnstile inversion & paradox logic
    ├── algorithm.c    # Scripted temporal pincer choreography
    └── render.c       # Unicode Braille 2x4 sub-pixel render engine
```

# Quiz Planet 🪐

> A trivia quiz game inspired by Discord's Quiz Planet — built in **C++17 with Raylib**  
> Supports **Single Player** and **Local PvP (2 players)**

---

## Features

- Animated starfield menu with a pulsing planet and 1280x720 HD Scaling
- Dynamic Category Selection (Science, Math, History, Computing, Geography) derived automatically from JSON
- Customizable Game Lengths (e.g. 10, 20, or 30 questions)
- 15-second countdown timer per question
- Score system: 100 base + time bonus + streak bonus
- PvP mode with alternating turns (P1: A/B/C/D — P2: ←/↑/→/↓)
- Sleek animated horizontal sliding banner for Correct/Wrong/Timeout feedback
- Results screen with winner cards (PvP) or performance grades (Solo)
- Advanced greedy word wrapping for dynamically scaled questions and answers

---

## Project Structure

```text
QuizPlanet/
├── .vscode/               ← VS Code config (launch.json, tasks.json, c_cpp_properties.json)
├── lib/
│   └── json.hpp           ← Lightweight JSON parsing library
├── assets/
│   └── fonts              ← fonts
│   └── questions.json     ← questions database
├── src/
│   ├── main.cpp           ← Entry point
│   ├── Game.h / Game.cpp  ← Main controller + game loop
│   ├── Question.h         ← Question data struct
│   ├── Player.h           ← Player state (score, streak, accuracy)
│   ├── Timer.h            ← Countdown timer
│   ├── QuizManager.h      ← Load + shuffle from JSON, category filtering
│   └── scenes/
│       ├── Scene.h        ← Abstract base class
│       ├── UIStyle.h      ← Centralized UI graphics & text rendering
│       ├── MenuScene.h    ← Main menu
│       ├── CategoryScene.h← Subject selection menu
│       ├── QuestionCountScene.h ← Game length selection
│       ├── QuizScene.h    ← Core gameplay loop
│       └── ResultScene.h  ← End game statistics & grades
├── questions.json         ← Question bank database
└── CMakeLists.txt         ← Build system
```

---

## Controls

| Action   | Player 1 | Player 2 (PvP) |
| -------- | -------- | -------------- |
| Choice A | `A` key  | `←` Arrow      |
| Choice B | `B` key  | `↑` Arrow      |
| Choice C | `C` key  | `→` Arrow      |
| Choice D | `D` key  | `↓` Arrow      |

---

## Scoring System

| Event           | Points                                   |
| --------------- | ---------------------------------------- |
| Correct answer  | **+100** base                            |
| Time bonus      | **+0 to 150** (faster = more)            |
| Streak bonus    | **+10 × streak** per consecutive correct |
| Wrong / Timeout | **0** pts, streak resets                 |

---

## 🛠️ VS Code Setup & Debugging (Windows)

This project is pre-configured to build and debug flawlessly entirely within **Visual Studio Code**. Follow these steps to set up your environment:

### 1. Install Prerequisites

1. **Visual Studio Code**: Ensure VS Code is installed.
2. **C/C++ Extension**: Open VS Code, go to the Extensions tab (`Ctrl+Shift+X`), search for **C/C++** (by Microsoft) and install it. This is required for IntelliSense and the Debugger.
3. **MinGW-w64 (GCC Compiler)**: Download and install MinGW-w64. Extract it to your `C:\` drive so that the compiler is located exactly at `C:\mingw64\bin\gcc.exe`. Ensure `C:\mingw64\bin` is added to your Windows System Environment Variables `PATH`.
4. **Raylib Setup**:
   - Download the precompiled **Raylib** library for MinGW.
   - Extract the folder precisely into `C:\raylib\raylib\`.
   - The VS Code configurations rely on the include headers being at `C:\raylib\raylib\src` and the static library being found there.

### 2. VS Code Configuration Details

Everything needed to compile and debug this game is already provided in the `.vscode/` folder:

- **`c_cpp_properties.json`**: Configures IntelliSense to find Raylib headers in `C:/raylib/raylib/src/**` and uses the GCC compiler path.
- **`tasks.json`**: Defines the `make` build commands (`build debug` and `build release`), automatically linking to `-lraylib`, `-lopengl32`, `-lgdi32`, and `-lwinmm`.
- **`launch.json`**: Configures the `gdb` debugger so you can step through the code dynamically and view variables.

### 3. How to Run & Debug

Because the workspace is fully configured:

1. Open the **QuizPlanet** folder in VS Code.
2. Open `main.cpp` (or any source file).
3. **Press `F5`** (or go to `Run` > `Start Debugging`).

VS Code will automatically trigger the `build debug` task (compiling all `.cpp` files in `src/` and `src/scenes/`), link the Raylib library, and immediately attach the debugger to `QuizPlanet.exe`.

> If the game crashes or hits an unexpected error, the debugger will immediately pause execution and highlight the exact line of code causing the issue in VS Code!

---

## OOP Design

| Concept           | Applied In                                                               |
| ----------------- | ------------------------------------------------------------------------ |
| **Inheritance**   | `MenuScene`, `QuizScene`, `ResultScene` all inherit from `Scene`         |
| **Polymorphism**  | `Game` calls `update()`/`draw()` on any `Scene*` via `unique_ptr`        |
| **Encapsulation** | `Player`, `Timer`, `QuizManager` each own their data privately           |
| **Abstraction**   | `Scene` defines a pure-virtual interface; `Game` never knows which scene |

---

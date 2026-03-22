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
│   ├── fonts/             ← UI Fonts (Outfit)
│   └── questions.json     ← The main question database
├── src/
│   ├── main.cpp           ← Minimal entry point
│   ├── core/              ← Engine & Foundation
│   │   ├── Game.h / .cpp  ← Scene manager & main loop
│   │   └── Scene.h        ← Abstract base class for all scenes
│   ├── entities/          ← Data Models
│   │   ├── Player.h       ← Player stats (score, streak, sabotages)
│   │   └── Question.h     ← Question data structure
│   ├── service/             ← Business Logic
│   │   ├── QuizManager.h  ← JSON parsing, shuffling, & filtering
│   │   └── Timer.h        ← Core gameplay timer
│   ├── ui/                ← Visual Presentation
│   │   └── UIStyle.h      ← Global theme, colors, and drawing helpers
│   └── scenes/            ← Gameplay States
│       ├── MenuScene.h
│       ├── CategoryScene.h
│       ├── QuestionCountScene.h
│       ├── QuizScene.h
│       └── ResultScene.h
└── Makefile               ← Cross-platform build script
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

VS Code will automatically trigger the `build debug` task (compiling all `.cpp` files in `src/` and `src/core/`), link the Raylib library, and immediately attach the debugger to `QuizPlanet.exe`.

> If the game crashes or hits an unexpected error, the debugger will immediately pause execution and highlight the exact line of code causing the issue in VS Code!

---

## Object-Oriented Programming (OOP) Design

The project is built on solid OOP foundations to ensure the code is scalable and maintainable:

| Principle                 | Implementation in Quiz Planet                                                                                                                                                   |
| :------------------------ | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Inheritance**           | All gameplay states (e.g., `QuizScene`, `MenuScene`) inherit from the `Scene` abstract base class.                                                                              |
| **Polymorphism**          | The `Game` engine manages a `std::unique_ptr<Scene>`. It calls the virtual `update()` and `draw()` methods without needing to know the specific type of scene currently active. |
| **Encapsulation**         | State-heavy components like `Player`, `Timer`, and `QuizManager` encapsulate their data. For example, `Player` handles its own score calculation and streak logic.              |
| **Abstraction**           | The `Scene` interface hides the complexity of individual scene implementations from the `Game` controller, allowing for easy addition of new game states.                       |
| **Composition**           | The `Game` class uses composition by holding a `GameState` object which aggregates various game components like the `QuizManager` and `Player` array.                           |
| **Single Responsibility** | Each class has one job: `QuizManager` only handles questions, `Timer` only handles time, and `UIStyle` only handles aesthetic presentation.                                     |

---

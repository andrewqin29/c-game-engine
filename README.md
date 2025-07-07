# C Game Engine

A 2D game engine written in C with SDL2. Key implementations and final game were built for Caltech CS3 SP25.

**Members:** Andrew Qin, Alex Kumar, Aaron Dumas

---

## Engine: Core Tech

### Physics
* **Rigid Bodies (`body.c`)**:
    * Bodies are represented as convex polygons.
    * Tracks position, velocity, rotation, and accumulates forces/impulses.
    * `body_tick` updates state using a velocity-averaging integration step.
* **Collision Detection (`collision.c`)**:
    * Uses the **Separating Axis Theorem (SAT)** for convex polygon intersection.
    * Finds the axis of minimum penetration for collision response.
* **Force System (`forces.c`)**:
    * Scene-level force creators apply forces to bodies.
    * Built-in forces: Newtonian gravity, Hooke's Law (springs), and velocity-proportional drag.
    * Custom collision handlers can be registered for any two bodies.
    * Built-in handlers for physics-based elastic collisions (impulses) and simple destructive collisions.

### Rendering & Assets
* **SDL2 Wrapper (`sdl_wrapper.c`)**:
    * Abstracts SDL windowing, input handling, and rendering.
    * Draws textured polygons and text.
    * Maps world coordinates to screen coordinates.
* **Asset System (`asset.c`, `asset_cache.c`)**:
    * An asset cache prevents loading duplicate resources (images, fonts).
    * Assets can be attached directly to a physics body, automatically syncing position and rotation.
    * Supports basic sprite animation by cycling through textures.

### Scene & Game Loop
* **Scene Management (`scene.c`)**:
    * The `scene_t` struct holds all bodies and force creators.
    * `scene_tick` drives the main loop: applies forces, ticks bodies, and removes objects marked for deletion.
* **Main Loop (`emscripten.c`)**:
    * Provides the main `emscripten_main` loop structure, compatible with web compilation.

---

## Game: Debug Dash

The engine is used to build `Debug Dash`, a 2D endless runner.

* **Gameplay**: Player jetpacks to avoid obstacles and collect coins. A C-language quiz system is used for power-ups.
* **Engine Features Used**:
    * Player movement uses the physics engine (gravity force + key-press impulse).
    * Obstacles, coins, and power-ups are procedurally generated.
    * Game logic is driven entirely by collision handlers (e.g., `player_obstacle_collision_handler`, `coin_collected_handler`).
    * The UI (score, timers, quiz panel) is rendered using the engine's text and image capabilities.
    * A simple state machine (`game_mode_t`) switches between playing, quiz, and game-over states.

---

## Build & Run

### Dependencies
* `gcc` / `clang`
* `make`
* `SDL2`
* `SDL2_image`
* `SDL2_ttf`
* `SDL2_mixer`

### Commands
```bash
# Compile
make

# Run
./out/game

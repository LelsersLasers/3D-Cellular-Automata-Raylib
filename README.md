# 3D Cellular Automata [Raylib/C++]

TODO: GIF

- [Quick notes](#quick-notes)
    - [Download](#download)
    - [Raylib](#raylib)
- [Definition of Cellular Automata](#definition-of-cellular-automata)
- [Cell rules explained](#cell-rules-explained)
    - [Survival](#survival)
    - [Spawn](#spawn)
    - [State](#state)
    - [Neighborhoods](#neighborhoods)
    - [Examples](#some-examples)
- [rules.json](#rules.json)
    - [Rules JSON](#rules-json)
    - [Settings](#settings)
        - [cellSize](#cellsize)
        - [cellBounds](#cellbounds)
        - [aliveChanceOnSpawn](#alivechanceonspawn)
        - [threads](#threads)
        - [targetFPS](#targetfps)
- [Simulation](#simulation)
    - [Controls](#controls)
        - [Camera controls](#camera-controls)
        - [Window controls](#window-controls)
        - [Simulation controls](#simulation-controls)
    - [Draw modes](#draw-modes)
        - [Dual color](#dual-color)
        - [RGB](#rgb)
        - [Dual color dying](#dual-color-dying)
        - [Single color](#single-color)
        - [Distance from center](#distance-from-center)
    - [Tick modes](#tick-modes)
    - [Other info](#other-info)
- [Showcase](#showcase)
- [Optimizations](#optimizations)
    - [Indexing over iteration](#indexing-over-iteration)
    - [1 demensional vector](#1-demensional-over-3-demensional)
    - [Branching at the highest level](#branching-at-the-highest-level)
    - [Branchless programing](#branchless-programming)
    - [Multithreading](#multithreading)
        - [Update at the same time as rendering](#update-at-the-same-time-as-rendering)
        - [Multiple threads for updating](#multiple-threads-for-updating)
- [Compiling](#compiling)


## Quick notes

### Download
To quickly download the latest executable and needed files, click [here](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/3D-Cellular-Automata-Raylib.zip).
Note: this was only build for Windows, but all libraries used are fully cross platform (I think).
Information about compiling for non-Windows can be found [here](compiling).

### Raylib
For graphics, the [Raylib](https://www.raylib.com/) library is used.
At first I wanted to use [wgpu](https://wgpu.rs/) with Rust because there were many tutorials online.
However, I quickly found that it was overkill for what I needed and I wanted to focus on the simulation rather than writing complex shader code.
Raylib is very simple to use, just <code>DrawCube(pos, w, l, h, color)</code> and a 3d cube appears.
Raylib is written purely in C99, but has [bindings](https://github.com/raysan5/raylib/blob/master/BINDINGS.md) to many languages, including Python, Java, and Rust.
I was tempted to use one of the bindings, but many of the binding were converted to fit the languages paradigms and did not match 1 to 1 with the documentation.
The library itself without any bindings is fully compatible with C++, and I really am used to using classes, so I chose C++ over C.
(Note: there is a [C++ binding](https://github.com/robloach/raylib-cpp) that follows the C++ pardigm rather than the C one, but I chose not to use it for simplicity.)


## Definition of cellular automata
> A cellular automaton is a collection of "colored" cells on a grid of specified shape that evolves through a number of discrete time steps according to a set of rules based on the states of neighboring cells. The rules are then applied iteratively for as many time steps as desired.
> 
> &mdash; <cite>Wolframe MathWorld</cite>

> A cellular automaton consists of a regular grid of cells, each in one of a finite number of states, such as on and off (in contrast to a coupled map lattice). The grid can be in any finite number of dimensions. For each cell, a set of cells called its neighborhood is defined relative to the specified cell. An initial state (time t = 0) is selected by assigning a state for each cell. A new generation is created (advancing t by 1), according to some fixed rule (generally, a mathematical function) that determines the new state of each cell in terms of the current state of the cell and the states of the cells in its neighborhood. Typically, the rule for updating the state of cells is the same for each cell and does not change over time, and is applied to the whole grid simultaneously.
> 
> &mdash; <cite>Wikipedia</cite>

TODO:?
PERSONAL/HUMAN DEFINTION


## Cell rules explained

There are 4 rules: surivival, spawn, state, and neighborhoods.
A cell can be in one of 3 state types: alive, dead, dying.

### Survival
- [X, Y, ...]
    - Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a signle number (ex: [2])
- If a cell is alive, it will remain alive if it has X, Y, or ... neighbors
- If it does not have X, Y, or ... neighbors, it will begin dying

### Spawn
- [X, Y, ...]
    - Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a signle number (ex: [2])
    - Can have any amount of overlap with survival
- If a cell is dead, it will come alive if it has X, Y, or ... neighbors

### State
- X
    - Must be a single number (ex: 6)
- Once a cell begins dying, it has X game ticks to live before disappearing
- Both survival and spawn rules will no longer affect the cell while it decays

### Neighborhoods
- "M" or "VN"
- How neighbors are counted
- "M": Moore: faces + counts diagonal neighbors, think rubics cube (3^3 - 1 = 26 possible neighbors)
- "VN": Von Neuman: only counts neighors where the faces touch (6 possible)

### Some examples
(can just copy/replace in rules.json)

- Slow build up: <9-18/5-7,12-13,15/6/M>
    ```
    "survival": [9, 10, 11, 12, 13, 14, 15, 16, 17, 18],
    "spawn": [5, 6, 7, 12, 13, 15],
    "state": 6,
    "neighborhood": "M",
    ```
- Outward expansion + hallow center: <2,6,9/4,6,8-9/10/M>
    ```
    "survival": [2, 6, 9],
    "spawn": [4, 6, 8, 9],
    "state": 10,
    "neighborhood": "M",
    ```


## rules.json

The rules and settings for the simulation can be found in rules.json.
When editing the file, make sure that all the keys are still there, and that the types of the values (number, list, string) are not changed.
The simulation loads the settings from the file when it is started, so the simulation must be restarted to see any changes to rules.json.

### Rules JSON

The first 4 keys are the rules for the simulation.
The explainations for these rules are [above](#cell-rules-explained) (as well as their types).

### Settings

Defaults:
```
"cellSize": 1.0,
"cellBounds": 96,
"aliveChanceOnSpawn": 0.15,
"threads": 8,
"targetFPS": 15
```

#### cellSize
- The size (length of the edge of cube) for each cell in the simulation
- Used by Raylib
- Changing this should have almost no effect on the simulation as the camera posistion is relative to this number
- Type: float

#### cellBounds
- The number of cells in each direction (x, y, z)
- The total number of cells is cellBounds<sup>3</sup>
- Higher cellBounds will make the simulation slower, but the simulation will also be more complex and less likely to become static or fully die out
- Type: int

#### aliveChanceOnSpawn
- When the simulation starts, only the middle section will have a chance to spawn
    - The middle section is the middle 1/9 of the simulation (1/3 of each edge)
- This might need to be changed depending on the rules to make sure everything doesn't instantly die out
- 1.0 = 100% chance to spawn
- Type: float

#### threads
- It is not the total number of the treads used by the simulation
- The total number of threads used by the simulation is threads + 2 because the main thread and the update thread (which then creates thread threads)
- See the [mulithreading](#multiple-threads-for-updating) section for more info
- Type: int

#### targetFPS
- Used for [dynamic tick mode](#dynamic)
    - See the section below for more info
- Type: int


## Simulation

### CONTROLS

Keyboard and mouse inputs are only checked once per frame. So on lower FPS, the controls will be less responsive.

#### Camera controls:
- Q/E : zoom in/out
- W/S : rotate camera up/down
- A/D : rotate camera left/right
- Space : reset camera
- The camera works on latitude/longitude system where W/A/S/D cause the camera to orbit the simulation
- Note: can also use the arrow keys instead W/A/S/D and page up/down to zoom in/out
- Camera movement is relative to the delta time between frames

#### Window controls:
- Enter : toggle fullscreen
- O : toggle true fullscreen
    - The Raylib implementation of fullscreen is not garenteed to scale correctly
    - Using Enter is highly recommended

#### Simulation controls
- R : re-randomize cells
    - See [aliveChanceOnSpawn](#alivechanceonspawn) for more info
- B : show/hide bounds
    - Draws a blue outline of the simulation bounds
- P : show/hide left bar
- C : toggle cross section view
    - Shows just half the simulation
    - Useful for seeing the center/core as it grows
    - Note: the hidden cells still update, they are just not rendered
- Mouse click : pause/unpause
    - Simply stops the game ticks
    - All other controls are still available
- M : change between draw modes
    - See [draw modes](#draw-modes) for more info
- U : change between tick modes
    - See [tick modes](#draw-modes) for more info
- X/Z : if the tick mode is [manual](#manual): increase/decrease tick speed

### Draw modes

```
enum DrawMode {
    DUAL_COLOR = 0,
    RGB_CUBE = 1,
    DUAL_COLOR_DYING = 2,
    SINGLE_COLOR = 3,
    CENTER_DIST = 4
};
```

#### Dual color

TODO:image

- Displays the cell's state as a color from green to red
- Green = alive
- Anything else: dying
    - Closer to green = more time to live
    - Closer to red = closer to dead

#### RGB

TODO:image

- Maps the cell's posistion to a color
    - X * K = red intensity, Y * K = green, Z * K = blue
- Because there is no shading, it is hard to tell the difference between cells
    - This draw mode makes it easier to see the cells as each cell is (slightly) different color at the cost of not displaying the cell's state

#### Dual color dying

TODO:image

- Alive = red
- Dying = scales from white to black based on how close the cell is to dead
- Easiest to see the difference between alive and dead cells at the cost of your eyes

#### Single color

TODO:image

- Like dual color, but instead of scaling from green to red, it scales from red to dark red/black

#### Distance from center

TODO:image

- A scale of how far each cell is from the center of the simulation from black to white
- Like RGB, it is easier to see the difference between cells at the cost of not displaying the cell's state

### TICK MODES

```
enum TickMode {
    FAST = 0,
    DYNAMIC = 1,
    MANUAL = 2
};
```

#### Fast
- The simulation progresses one tick forward per frame
- The speed is often limited by the rendering of the cells, so the simulation tick speed and frame rate will decrease as more cells are rendered

#### Dynamic
- Tries to progress the simulation as fast as possible while keeping the simulation running above the [target FPS](#targetfps)
- Does this by alternating between based on the current FPS:
    1) updating the cells and drawing them
    2) just drawing the cells
- If the time between the last update was more than 1/desiredUpdateSpeed, it will run 1, else it will run 2
- Drawing the cells is still slow, so it might just end up as 1 tick per second on higher bounds

#### Manual
- Increase/decrease ticks per second with X/Z
- Increasing above the current FPS effectively makes the simulation run as if it was on Fast
- It works the same way as dynamic (alternating between updating and drawing and just drawing based on time between updates)

### OTHER INFO
TODO:?

## Showcase

TODO:

## Optimization

The simulation is optimized for speed, but it still can be slow on higher bounds.
I have made it as fast as I can, but I am sure there are ways to make it faster.
Here are some of the things I have done to improve the speed:


### Indexing over iteration

Before, to see if a dead cell should come I alive I had to do this:
```
for (int value : spawnNumbers) {
    if (neighbors == value) {
        state = ALIVE;
        break;
    }
}
```
However, I could replace the array of spawnNumbers with an array of booleans where the index is the spawn number.
Then I could just do:
```
if (spawn[neighbors]) state = ALIVE;
```
It was hard to edit the rules when it was a boolean array, but it was easy to convert a list of spawn numbers to a boolean array like:
```
for (size_t value : rules["spawn"]) spawn[value] = true;
```
I did a similar thing for the survival numbers.


### 1 demensional over 3 demensional

Technically, there is no reason to use a vector over an array.
However, I did it this way because later I might add the ability to change the cell bounds on demand.
My understanding is that a vector and an array are the same speed except for allocating memory and I only have to do that at the start.

However, before, I had a vector of vectors of vectors of cells:
```
vector<vector<vector<Cell>>> cells;
```
After some googling, I found that accessing vectors of vectors (of vectors) is slow, because of the "indirection and lack of locality [which] quickly kills a lot of performance."
To solve this problem, I made the cells a 1 demensional vector, and indexed with:
```
size_t threeToOne(int x, int y, int z) {
    return x * cellBounds * cellBounds + y * cellBounds  + z;
}
```
It seemed to run faster when doing this extra calculation per cell than using a vector of vectors of vectors of cells.


### Branching at the highest level

Before, when doing the different draw modes, I simply went through all the cells and switched on the draw mode.
Note: divisor is for the [cross section view](#simulation-controls)
```
for (int x = 0; x < cellBounds/divisor; x++) {
    for (int y = 0; y < cellBounds; y++) {
        for (int z = 0; z < cellBounds; z++) {
            Color color;
            switch (drawMode) {
                case DUAL_COLOR:
                    Color color = ....;
                    break;
                case RGB_CUBE:
                    Color color = ....;
                    break;
                // reset of switches
            }
            cells[threeToOne(x, y, z)].draw(color);
// appropriate closing paraenthese
```
However, this meant that it had to compare/switch/branch on drawMode for every cell.
Moving the switch outside of the for loop made it so that it only had to compare/branch once (at the code of repeated code).
```
switch (drawMode) {
    case DUAL_COLOR:
        for (int x = 0; x < cellBounds/divisor; x++) {
            for (int y = 0; y < cellBounds; y++) {
                for (int z = 0; z < cellBounds; z++) {
                    cells[threeToOne(x, y, z)].drawDualColor();
        // appropriate closing paraenthese
        break;
    case RGB_CUBE:
        for (int x = 0; x < cellBounds/divisor; x++) {
            for (int y = 0; y < cellBounds; y++) {
                for (int z = 0; z < cellBounds; z++) {
                    cells[threeToOne(x, y, z)].drawRGBCube();
        // appropriate closing paraenthese
        break;
    // reset of switches
}
```


### Branchless programming

I am not actually sure if this is faster. The only thing I really read was [this](https://dev.to/jobinrjohnson/branchless-programming-does-it-really-matter-20j4).
The idea is that having branches (if/switch/conditional assignment/etc) is slower than just doing math.
I am sure at what point this is true, but it seemed to speed up the simulation, so I just left it in.

For example, I replaced (example from earlier): 
```
else if (state == DEAD) {
    if (spawn[neighbors]) {
        state = ALIVE;
        hp = STATE; // STATE is the amount of ticks the cell lives as defined in rules.json
    }
}
```
With:
```
else if (state == DEAD) {
    // the way enum State was set up: alive = 2, dying = 1, dead = 0
    // also makes use of implicit bool -> int conversion
    state = (State)(spawn[neighbors] * ALIVE);
    hp = state/ALIVE * STATE;
}
```
Again, not really sure if it is better, and this code is still within a branch.

TODO: is there any I forgot?


### Multithreading

Raylib does not support multithreaded rendering (see [this post from creator of Raylib](https://twitter.com/raysan5/status/1119273062405373952?lang=en)).

#### Update at the same time as rendering
However, I can still run the update functions while the main thread draws.
This can be seen here (note: shouldUpdate depends on the [tick mode](#tick-mode) where it is always set to true if it is on fast):
```
if (shouldUpdate) {
    cells2 = vector<Cell>(cells); // create copy, note: space for the cells2 vector was already allocated when the program first started
    thread updateThread(updateCells, std::ref(cells2));

    draw(camera, cells, ....);

    updateThread.join();
    cells = vector<Cell>(cells2);
}
else {
    draw(camera, cells, ....;
}
```
On a frame where the cells should be updated, it:
1. Creates a copy of the cells so the cells don't change while the main thread is drawing
2. Creates a thread to update the cells
3. Draws the old cells
4. Waits for the thread to finish and then sets the cells to the new cells
This means that the cells are being updated for the next frame, not the current one.

This could be faster when the tick mode is not on fast by
updating the cells as fast as possible seperate from the draw and saving each new tick
then playing just setting the cells to the oldest not drawn save (and deleting the save).
However, this seems rather complicated, and I almost always use the fast tick mode, so I didn't bother.

#### Multiple threads for updating
Not counting cell2 (see above), this simulation is still "double buffered".
What this means is that as the cells are updated, their state is not immediately changed.
Instead the new state of each cell is only applied after the new state of each cell is calculated.

From this understanding, the update function can be split into 2 parts:
1. Calculate the neighbors of each cell based on nearby alive cells
2. Use the neighbor count to determine new state for each cell
The steps are the "buffers".

However, within each step, the cells can be updated in parallel.
In step 1, the information needed from other cells is the current state of each cell.
The state of each cell does not actually change, so the timing of step 1 for each cell is not important (as long as step 1 comes before step 2).
In step 2, there is no information needed from other cells, so again the order is not imporant (as long as step 2 comes after step 1).

This means that step 1, which goes through every cell, can actually go through every cell in parallel.
The amount of threads used for this is defined in the [threads](#threads) variable in rules.json.

On a frame where update will be called, the flow is as follows:
```
Visualization 1:
Frame loop start        *
shouldUpdate = true     |
Create copy of cells    |
                        |
Draw old cells          |\ Create update thread
                        | \ Create 'threads' threads to do step 1
                        | / Wait for all step 1 threads to finish
                        | \ Create 'threads' threads to do step 2
                        | / Wait for all step 2 threads to finish
                        |/ Rejoin with main thread
Old cells = new cells   |
Frame loop ends         *

Visualization 2:
                                * Frame loop start
                                * shouldUpdate = true
                                * create copy of cells
                               / \ a thread for updating is created with the copy
Main thread renders old cells *   * splits into 'threads' threads to do step 1
                              |   * waits for all threads to finish
                              |   * splits into 'threads' threads to do step 2
                              |   * waits for all threads to finish
                               \ / rejoins to main thread
                Frame loop ends * 
```
This makes it so there are actually 'threads' + 2 (main + overall update) total threads running at the same time.

As mentioned earlier, this could likely still be done better/faster, but it seems to work well and vastly improves performance.


## Compiling

So I don't really understand how multi-file projects work for C++/C, but the C99 version of Raylib came with a Notepad++ script to compile it.
I modified it slightly to work with my C++ compiler and to include the library needed to read the JSON file.
```
SET RAYLIB_PATH=C:\raylib\raylib
SET JSON_PATH=C:\Users\milla\Desktop\coding\3D-Cellular-Automata-Raylib\json\
SET CC=g++
SET CFLAGS=$(RAYLIB_PATH)\src\raylib.rc.data -s -static -Os -Wall -I$(RAYLIB_PATH)\src -I$(JSON_PATH) -Iexternal -DPLATFORM_DESKTOP -std=c++11 -pthread
SET LDFLAGS=-lraylib -lopengl32 -lgdi32 -lwinmm
cd $(CURRENT_DIRECTORY)
cmd /c IF EXIST $(NAME_PART).exe del /F $(NAME_PART).exe
npp_save
$(CC) -o $(NAME_PART).exe $(FILE_NAME) $(CFLAGS) $(LDFLAGS)
ENV_UNSET PATH
cmd /c IF EXIST $(NAME_PART).exe $(NAME_PART).exe
```
I don't fully understand all of it, but it seems to work.
The instuctions for installing/downloading Raylib were pretty simple and can be navigated to from their [website](https://www.raylib.com/).
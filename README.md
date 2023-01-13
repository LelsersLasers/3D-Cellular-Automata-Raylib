# 3D Cellular Automata [Raylib/C++]

![Main showcase gif](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/MainShowcaseGIFLowRez.gif)

- [Quick notes](#quick-notes)
    - [Download](#download)
    - [Raylib](#raylib)
- [Definition of Cellular Automata](#definition-of-cellular-automata)
    - [Uses of cellular automata](#uses-of-cellular-automata)
        - [Epidemiology](#epidemiology)
        - [Anthropology](#anthropology)
        - [Biology](#biology)
        - [Physics](#physics)
        - [Cryptography](#cryptography)
- [Cell rules explained](#cell-rules-explained)
    - [Survival](#survival)
    - [Spawn](#spawn)
    - [State](#state)
    - [Neighborhoods](#neighborhoods)
    - [Examples](#some-examples)
- [How to change the rules, colors, and settings (options.json)](#how-to-change-the-rules-colors-and-settings)
    - [Changing the rules](#changing-the-rules)
    - [Changing the settings](#changing-the-settings)
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
        - [Fast](#fast)
        - [Dynamic](#dynamic)
        - [Manual](#manual)
- [Optimizations](#optimizations)
    - [Indexing over iteration](#indexing-over-iteration)
    - [1 dimensional vector](#1-dimensional-over-3-dimensional)
    - [Branching at the highest level](#branching-at-the-highest-level)
    - [Branchless programing](#branchless-programming)
    - [Multithreading](#multithreading)
        - [Update at the same time as rendering](#update-at-the-same-time-as-rendering)
        - [Multiple threads for updating](#multiple-threads-for-updating)
- [Compiling](#compiling)


## Quick notes

### Download
To quickly download the latest executable and needed files (main.exe and the options.json),
click [here](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/3D-Cellular-Automata-Raylib.zip).
Note: this was only build for Windows, but all libraries used are fully cross platform (I think).
Information about compiling for non-Windows can be found [here](compiling).

### Raylib
For graphics, the [Raylib](https://www.raylib.com/) library is used.
At first I wanted to use [wgpu](https://wgpu.rs/) with Rust because there were many tutorials online.
However, I quickly found that it was overkill for what I needed and I wanted to focus on the simulation rather than writing complex shader code.
Raylib is very simple to use, just <code>DrawCube(pos, w, l, h, color);</code> and a 3d cube appears.
Raylib is written purely in C99, but has [bindings](https://github.com/raysan5/raylib/blob/master/BINDINGS.md) to many languages, including Python, Java, and Rust.
I was tempted to use one of the bindings, but many of the binding were converted to fit the languages paradigms and did not match 1 to 1 with the documentation or examples.
The library itself without any bindings is fully compatible with C++, and I really am used to using classes, so I chose C++ over C.
(Note: there is a [C++ binding](https://github.com/robloach/raylib-cpp) that follows the C++ paradigm rather than the C one, but I chose not to use it for simplicity.)


## Definition of cellular automata
> A cellular automaton is a collection of "colored" cells on a grid of specified shape that evolves through a number of discrete time steps according to a set of rules based on the states of neighboring cells. The rules are then applied iteratively for as many time steps as desired.
> 
> &mdash; <cite>Wolframe MathWorld</cite>

> A cellular automaton consists of a regular grid of cells, each in one of a finite number of states, such as on and off (in contrast to a coupled map lattice). The grid can be in any finite number of dimensions. For each cell, a set of cells called its neighborhood is defined relative to the specified cell. An initial state (time t = 0) is selected by assigning a state for each cell. A new generation is created (advancing t by 1), according to some fixed rule (generally, a mathematical function) that determines the new state of each cell in terms of the current state of the cell and the states of the cells in its neighborhood. Typically, the rule for updating the state of cells is the same for each cell and does not change over time, and is applied to the whole grid simultaneously.
> 
> &mdash; <cite>Wikipedia</cite>

> Cellular automata are discrete, abstract computational systems that have proved useful both as general models of complexity and as more specific representations of non-linear dynamics in a variety of scientific fields. Firstly, cellular automata are (typically) spatially and temporally discrete: they are composed of a finite or denumerable set of homogeneous, simple units, the atoms or cells. At each time unit, the cells instantiate one of a finite set of states. They evolve in parallel at discrete time steps, following state update functions or dynamical transition rules: the update of a cell state obtains by taking into account the states of cells in its local neighborhood (there are, therefore, no actions at a distance). Secondly, cellular automata are abstract: they can be specified in purely mathematical terms and physical structures can implement them. Thirdly, cellular automata are computational systems: they can compute functions and solve algorithmic problems. Despite functioning in a different way from traditional, Turing machine-like devices, cellular automata with suitable rules can emulate a universal [Turing machine (see entry)](https://plato.stanford.edu/entries/turing-machine/), and therefore compute, given [Turing’s thesis (see entry)](https://plato.stanford.edu/entries/church-turing/), anything computable.
>
> &mdash; <cite>Stanford Encyclopedia of Philosophy</cite>

> A cellular automaton is a collection of cells arranged in a grid of specified shape, such that each cell changes state as a function of time, according to a defined set of rules driven by the states of neighboring cells.
>
> &mdash; <cite>TechTarget</cite>

### Uses of cellular automata

#### Epidemiology
Cellular automata are used to study the evolution of disease epidemics through computer modeling.

#### Anthropology
In anthropology, cellular automata with fundamental space-time representations are used to model the formation of civil societies.

#### Sociology
Cellular automata are used to study the causes and effects of civil violence.

#### Biology
Several biological processes and systems are simulated using cellular automata, including the patterns of some seashells, moving wave patterns on the skin of cephalopods and the behaviors of brain neurons.

#### Physics
Cellular automata are used to simulate and study physical phenomena, such as gas and fluid dynamics.

#### Cryptography
Cellular automata automata are proposed for use in public key cryptography.
They can be utilized to construct pseudorandom number generators, and to design error correction codes.

Sources:
- <cite>https://www.techtarget.com/searchenterprisedesktop/definition/cellular-automaton#:~:text=A%20cellular%20automaton%20(CA)%20is,the%20states%20of%20neighboring%20cells.</cite>
- <cite>https://en.wikipedia.org/wiki/Cellular_automaton#Applications</cite>


## Cell rules explained

There are 4 rules: survival, spawn, state, and neighborhoods.
A cell can be in one of 3 states: alive, dying, or dead.

### Survival
- [X, Y, ...]
    - Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a single number (ex: [2])
- If a cell is alive, it will remain alive if it has X, Y, or ... neighbors
- If it does not have X, Y, or ... neighbors, it will begin dying

### Spawn
- [X, Y, ...]
    - Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a single number (ex: [2])
    - Can have any amount of overlap with survival
- If a cell is dead, it will come alive if it has X, Y, or ... neighbors

### State
- X
    - Must be a single number (ex: 6)
- Once a cell begins dying, it has X simulation/update ticks to live before disappearing
- Both survival and spawn rules will no longer affect the cell while it decays

### Neighborhoods
- "M" or "VN"
- How neighbors are counted
- "M" - Moore:
    - Neighbors are any cells where 1 away, including diagonals
    - Think like a Rubik's cube where the current cell is the middle, all the outside/colored cubes are the neighbors
    - 3^3 - 1 = 26 possible neighbors
- "VN" - Von Neumann:
    - Neighbors are only cells where the faces touch
    - 6 possible neighbors

### Some examples
(Note: can just copy/replace in options.json)

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


## How to change the rules, colors, and settings

The rules, colors, and settings for the simulation can be found in options.json.
When editing the file, make sure that all the keys/variables are still there, and that the types of the values (number, list of numbers, string) are not changed.
The simulation loads the settings from the file when it is started, but they can be [reloaded by pressing J](#simulation-controls).

### Changing the rules

The first 4 options (survival, spawn, state, neighborhood) are the rules for the actually cellular automata.
The explanations for these rules are [above](#cell-rules-explained) (as well as their types).

### Changing the colors

Defaults:
```
    "dualColorAlive": [0, 228, 48],
    "dualColorDead": [230, 41, 55],
    "dualColorDyingAlive": [230, 41, 55],
    "singleColorAlive": [255, 20, 20],
    "centerDistMax": [255, 255, 255],
```

See [draw modes](#draw-modes) for more information about how the colors work.
The values are a list of three numbers, representing the RGB values of the color where 255 = 100% color intensity, and 0 = 0%.
(So [255, 0, 0] is red, [0, 0, 0] is black, [255, 255, 255] is white, etc.)

### Changing the settings

Defaults:
```
    "cellBounds": 96,
    "aliveChanceOnSpawn": 0.15,
    "threads": 8,
    "targetFPS": 15
```

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
- The total number of threads used by the simulation is threads + 2 because the main thread and the update thread (which then creates 'threads' threads)
    - See the [multithreading](#multiple-threads-for-updating) section for more info
- Type: int

#### targetFPS
- Used for [dynamic tick mode](#dynamic)
    - See the [dynamic tick mode](#dynamic) section for more info
- Type: int


## Simulation

### Controls

Keyboard and mouse inputs are only checked once per frame.
So on lower FPS, the controls will be less responsive.
Note: holding a key will not cause a rapid toggle. (So if the FPS is low, hold a key to make sure it is down pressed when the inputs are checked.)

#### Camera controls
- Q/E : zoom in/out
- W/S : rotate camera up/down
- A/D : rotate camera left/right
- Space : reset camera
- The camera works on latitude/longitude system where W/A/S/D cause the camera to orbit the simulation
- Note: can also use the arrow keys instead W/A/S/D and page up/down to zoom in/out
- Camera movement is relative to the delta time between frames

#### Window controls
- Enter : toggle fullscreen
- O : toggle true fullscreen
    - The Raylib implementation of fullscreen is not guaranteed to scale correctly
    - Using Enter is highly recommended

#### Simulation controls
- R : re-randomize cells
    - See [aliveChanceOnSpawn](#alivechanceonspawn) for more info
    - Note: key intentionally does not have 'rapid toggle protection'
- J : reload from JSON
    - See [options.json](#how-to-change-the-rules-and-settings) for more info
- B : show/hide bounds
    - Draws a blue outline of the simulation bounds
    - If cross section mode is on, it will draw the outline around just the drawn cells

    | Bounds off | Bounds on |
    :-:|:-:
    | ![Bounds off image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/default.PNG) | ![Bounds on image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/BoundsOn.PNG)  |

- P : show/hide left bar

    | Bar off | Bar on |
    :-:|:-:
    | ![Cross section image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/BarOff.PNG) | ![Cross section on image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/default.PNG) |

- C : toggle cross section view
    - Shows just half the simulation
    - Useful for seeing the center/core of the simulation
    - Note: the hidden cells still update, they are just not rendered

    | Cross section off | Cross section on |
    :-:|:-:
    | ![Cross section image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/CrossSectionOff.PNG) | ![Cross section on image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/CrossSectionOn.PNG)  |

- Mouse click : pause/unpause
    - Simply stops the update ticks
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

// Color settings in options.json
"dualColorAlive": [R, G, B],
"dualColorDead": [R, G, B],
"dualColorDyingAlive": [R, G, B],
"singleColorAlive": [R, G, B],
"centerDistMax": [R, G, B],
```

In all draw modes, fully dead cells are not rendered.

#### Dual color

![DUAL_COLOR image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/DUAL_COLOR.PNG)

- Displays the cell's state as a scale from color from dualColorAlive to dualColorDead
- If the cell is dualColorAlive, it is fully alive
- Anything else: the cell is dying
    - Closer to dualColorAlive = more time to live
    - Closer to dualColorDead = closer to dead


#### RGB

![RGB_CUBE image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/RGB_CUBE.PNG)

- Maps the cell's position (X, Y, Z) to a color
    - X * k = red intensity, Y * k = green, Z * k = blue
- Because there is no shading, it is hard to tell the difference between cells
    - This draw mode makes it easier to see the cells as each cell is (slightly) different color at the cost of not displaying the cell's state

#### Dual color dying

![DUAL_COLOR_DYING image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/DUAL_COLOR_DYING.PNG)

- Alive = dualColorDyingAlive
- Dying = scales from white to black based on how close the cell is to dead
- Easiest to see the state/difference between alive and dead cells at the cost of your eyes

#### Single color

![SINGLE_COLOR image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/SINGLE_COLOR.PNG)

- Like dual color, but instead of scaling from dualColorAlive to dualColorDead, it scales from singleColorAlive to dark/black

#### Distance from center

![CENTER_DIST image](https://github.com/LelsersLasers/3D-Cellular-Automata-Raylib/raw/main/Showcase/CENTER_DIST.PNG)

- A scale of how far each cell is from the center of the simulation from black to centerDistMax
- Like RGB, it is easier to see the difference between cells at the cost of not displaying the cell's state
    - It is likely easier on the eyes than RGB, but there are cells that have the same color

### Tick modes

```
enum TickMode {
    FAST = 0,
    DYNAMIC = 1,
    MANUAL = 2
};
```

#### Fast
- The simulation progresses one tick forward every frame

#### Dynamic
- Tries to progress the simulation as fast as possible while keeping the simulation running above the [target FPS](#targetfps)
- Does this by alternating between based on the current FPS:
    1) updating the cells and drawing them
    2) just drawing the cells
- If the time between the last update was more than 1/desiredUpdateSpeed, it will run 1, else it will run 2
- It will adjust desiredUpdateSpeed based on the different between the current FPS and the target FPS.
- Drawing the cells is still slow, so it might just end up as 1 tick per second on higher bounds

#### Manual
- Increase/decrease ticks per second with X/Z
- Increasing above the current FPS effectively makes the simulation run as if it was on Fast
- It works the same way as dynamic (alternating between updating and drawing and just drawing based on time between updates)


## Optimization

The simulation is optimized for speed, but it still can be slow on higher bounds.
I have made it as fast as I can, but I am sure there are ways to make it faster.

One of these ways could be doing all the calculations on the GPU.
Right now (from my understanding), the calculations are all done on the CPU, and then streamed to the GPU when they are rendered.
(Which I think is why 1 CPU thread is always at max usage, it is the one that is in the draw loop and streaming.)
This is all abstracted away by Raylib, but if the calculations are done on the GPU, then there will be no need to stream the data to the GPU.
(Also it seems like most GPUs are faster and can handle parallel calculations much faster/better than CPUs.)

Here are some of the things I have done to improve the speed of the simulation:
Note: a lot of this code is modified to illustrate the point, and may not match 1 to 1 with the actual code.

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
// rules is the JSON file
```
I did a similar thing for the survival numbers.
Now, per cell, instead of having to go through an additional loop, it can index a list which is much much faster.


### 1 dimensional over 3 dimensional

I used a vector over an array so the bounds could be changed without the simulation needing to be restarted.
My understanding is that a vector and an array are the same speed except for allocating memory and I only have to do that at the start.

However, before, I had a vector of vectors of vectors of cells:
```
vector<vector<vector<Cell>>> cells;
```
After some googling, I found that accessing vectors of vectors (of vectors) is slow, because of the "indirection and lack of locality [which] quickly kills a lot of performance."
(All the memory was not nessisarily in the same place, so it had to do a lot of extra work.)
To solve this problem, I made the cells a 1 dimensional vector, and indexed with:
```
size_t threeToOne(int x, int y, int z) {
    return x * cellBounds * cellBounds + y * cellBounds  + z;
}
```
It seemed to run faster when doing this extra calculation per cell than using a vector of vectors of vectors of cells.
(Not sure if was actually worth.)
The main downside is that code/math when reloading from the JSON and changing the bounds is a bit more complicated as every cell has to shuffle around.


### Branching at the highest level

Before, when doing the different draw modes, I simply went through all the cells and switched on the draw mode.
Note: divisor is for the [cross section view](#simulation-controls).
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
// appropriate closing parentheses
```
However, this meant that it had to compare/switch/branch on drawMode for every cell,
but the value it switched on would not change, so redoing the comparison/switch/branching for every cell was excessive and slow.
Moving the switch outside of the for loop made it so that it only had to compare/branch once (at the cost of repeated code).
```
switch (drawMode) {
    case DUAL_COLOR:
        for (int x = 0; x < cellBounds/divisor; x++) {
            for (int y = 0; y < cellBounds; y++) {
                for (int z = 0; z < cellBounds; z++) {
                    cells[threeToOne(x, y, z)].drawDualColor();
        // appropriate closing parentheses
        break;
    case RGB_CUBE:
        for (int x = 0; x < cellBounds/divisor; x++) {
            for (int y = 0; y < cellBounds; y++) {
                for (int z = 0; z < cellBounds; z++) {
                    cells[threeToOne(x, y, z)].drawRGBCube();
        // appropriate closing parentheses
        break;
    // reset of switches
}
```


### Branchless programming

I am not actually sure if this is faster.
The only thing I really read was [this](https://dev.to/jobinrjohnson/branchless-programming-does-it-really-matter-20j4).
The idea is that having branches (if/switch/conditional assignment/etc) is slower than just doing math and using boolean to int conversions.
I am sure at what point this is true, but it seemed to speed up the simulation, so I just left it in.

For example, I replaced (example from earlier): 
```
if (state == ALIVE) {
    if (!survival[neighbors]) {
        state = Dying;
        hp--;
    }
}
else if (state == DEAD) {
    if (spawn[neighbors]) {
        state = ALIVE;
        hp = STATE; // STATE is the amount of ticks the cell lives as defined in options.json
    }
}
else if (state == DYING) {
    hp--;
    if (hp < 0) {
        state = DEAD;
    }
}
```
With:
```
hp = 
    (hp == STATE) * (hp - 1 + survival[neighbors]) + // alive
    (hp < 0) * (spawn[neighbors] * (STATE + 1) - 1) +  // dead
    (hp >= 0 && hp < STATE) * (hp - 1); // dying
```
This works by eliminating the need for the 'state' variable as it was directly related to its hp where:
- When hp == STATE, state would be ALIVE
- When hp < 0, state would be DEAD
- And anything else (hp >= 0 && hp < STATE), state would be DYING
This allowed the code to simplify into 1 line, without branching, by converting each if statement to a int
and because only 1 of the 3 states was possible, only 1 of the statements will be 1 and the rest will be 0,
effectively making hp equal just that 1 case.
Example - if STATE = 6, and hp = 3, then the cell is dying:
```
(3 == 6) * (3 - 1 + SURVIVAL[neighbors])    -> (0) * (whatever) -> 0
(3 < 0) * (SPAWN[neighbors] * (6 + 1) - 1)  -> (0) * (whatever) -> 0
(3 >= 0 && 3 < 6) * (3 - 1)                 -> (1) * (3 - 1)    -> 2
hp = 0 + 0 + 2 = 2;
(2 >= 0 && 2 < 6) -> still dying
```


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
updating the cells as fast as possible separate from the draw and saving each new tick
then playing just setting the cells to the oldest not drawn save (and deleting the save).
However, this seems rather complicated, and I almost always use the fast tick mode, so I didn't bother.

#### Multiple threads for updating
Even when not counting cells2 (see above), this simulation is still "double buffered".
What this means is that as the cells are updated in two parts, their state is not immediately changed.

The update function can be split into 2 parts:
1. Calculate the neighbors of each cell based on nearby alive cells
2. Use the neighbor count to determine the new state for each cell
(The steps are the "buffers".)

However, within each step, the cells can be updated in parallel.
In step 1, the information needed from other cells is the current state of the surrounding cells.
Because the state of each cell does not actually change in this step, 
the timing of step 1 for each cell is not important (as long as step 1 comes before step 2).
In step 2, there is no information needed from other cells, so again the order is not important (as long as step 2 comes after step 1).

This means that step 1, which goes through every cell, can actually go through every cell in parallel.
The amount of threads used for this is defined in the [threads](#threads) variable in options.json.

On a frame where update will be called, the flow is as follows:
```
Visualization 1:
Frame loop start        *
shouldUpdate = true     |
Create copy of cells    |
                        |
                        |\ Create 1 thread
Draw old cells          | | This thread manage the other threads to update the cells
Still drawing           | \ Create 'threads' threads
Still drawing           |  | Divide cells into 'threads' chunks and each thread does step 1 on that chunk
Still drawing           | / Wait for all step 1 threads to finish and rejoin with update thread
Etc                     | \ Create 'threads' threads
                        |  | Divide cells into 'threads' chunks and each thread does step 2 on that chunk
                        | / Wait for all step 2 threads to finish and rejoin with update thread
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

### Windows

So I don't really understand how multi-file projects work for C++/C, but the C99 version of Raylib came with a Notepad++ script to compile it.
I modified it slightly to work with the C++ compiler:
```
SET RAYLIB_PATH=C:\raylib\raylib
SET CFLAGS=$(RAYLIB_PATH)\src\raylib.rc.data -s -static -Os -Wall -I$(RAYLIB_PATH)\src -Iexternal -DPLATFORM_DESKTOP -std=c++11 -pthread
SET LDFLAGS=-lraylib -lopengl32 -lgdi32 -lwinmm
cd $(CURRENT_DIRECTORY)
cmd /c IF EXIST $(NAME_PART).exe del /F $(NAME_PART).exe
npp_save
g++ -o $(NAME_PART).exe $(FILE_NAME) $(CFLAGS) $(LDFLAGS)
ENV_UNSET PATH
cmd /c IF EXIST $(NAME_PART).exe $(NAME_PART).exe
```
I don't fully understand all of it, but it seems to work.
The instructions for installing/downloading Raylib were pretty simple and can be navigated to from their [website](https://www.raylib.com/).

### Linux

Instructions: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux

#### Quick start (Fedora)

Install dependencies:
```
sudo dnf install alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel libatomic wayland-devel libxkbcommon-devel wayland-protocols-devel
```

Install/setup raylib (with Wayland support):
```
git clone https://github.com/raysan5/raylib.git raylib
cd raylib
mkdir build && cd build
cmake -DUSE_WAYLAND=ON ..
make
sudo make install
```

Compile:
```
g++ -o main main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```
# 3D Cellular Automata [Raylib/C++]

GIF


# Definition of Cellular Automata
> A cellular automaton is a collection of "colored" cells on a grid of specified shape that evolves through a number of discrete time steps according to a set of rules based on the states of neighboring cells. The rules are then applied iteratively for as many time steps as desired.
> 
> &mdash; <cite>Wolframe MathWorld</cite>

> A cellular automaton consists of a regular grid of cells, each in one of a finite number of states, such as on and off (in contrast to a coupled map lattice). The grid can be in any finite number of dimensions. For each cell, a set of cells called its neighborhood is defined relative to the specified cell. An initial state (time t = 0) is selected by assigning a state for each cell. A new generation is created (advancing t by 1), according to some fixed rule (generally, a mathematical function) that determines the new state of each cell in terms of the current state of the cell and the states of the cells in its neighborhood. Typically, the rule for updating the state of cells is the same for each cell and does not change over time, and is applied to the whole grid simultaneously.
> 
> &mdash; <cite>Wikipedia</cite>

PERSONAL/HUMAN DEFINTION


# Rules explained

There are 4 rules: surivival, spawn, state, and neighborhoods.
A cell can be in one of 3 state types: alive, dead, dying.

### Survival: [X, Y, ...]
- If a cell is alive, it will remain alive if it has X, Y, or ... neighbors
- If it does not have X, Y, or ... neighbors, it will begin dying
- Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a signle number (ex: [2])

### Spawn: [X, Y, ...]
- If a cell is dead, it will come alive if it has X, Y, or ... neighbors
- Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a signle number (ex: [2])
- Can have any amount of overlap with survival

### State: X
- Once a cell begins dying, it has X game ticks to live before disappearing
- Both survival and spawn rules will no longer affect the cell while it decays
- Must be a single number (ex: 6)
- Note:
    - The way the cell colors are calculated makes seeing low state numbers (< 3) hard
    - But the simulation still works/runs

### Neighborhoods: "M" or "VN"
- How neighbors are counted
- "M": Moore: faces + counts diagonal neighbors, think rubics cube (3^3 - 1 = 26 possible neighbors)
- "VN": Von Neuman: only counts neighors where the faces touch (6 possible)

<!-- - To change rules in this simulation:
    - Edit 'rules.json'
        - Make sure the format is correct (value types and key names must not be changed)
    - The simulation must be restarted to see the changes -->

### Some examples (can just copy/replace in 'rules.json')
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


# 'rules.json'

The rules and settings for the simulation can be found in 'rules.json'.
When editing the file, make sure that all the keys are still there, and that the types of the values (number, list, string) are not changed.
The simulation loads the settings from the file when it is started, so the simulation must be restarted to see any changes to 'rules.json'.

## Rules

The first 4 keys are the rules for the simulation.
The explainations for these rules are above (as well as their types).

## Settings

Defaults:
```
"cellSize": 1.0,
"cellBounds": 96,
"aliveChanceOnSpawn": 0.15,
"threads": 8,
"targetFPS": 15
```

### cellSize
- The size (length of the edge of cube) for each cell in the simulation
- Used by Raylib
- Changing this should have almost no effect on the simulation as the camera posistion is relative to this number
- Type: float

### cellBounds
- The number of cells in each direction (x, y, z)
- The total number of cells is cellBounds<sup>3</sup>
- Higher cellBounds will make the simulation slower, but the simulation will also be more complex and less likely to become static or fully die out
- Type: int

### aliveChanceOnSpawn
- When the simulation starts, only the middle section will have a chance to spawn
    - The middle section is the middle 1/9 of the simulation (1/3 of each edge)
- This might need to be changed depending on the rules to make sure everything doesn't instantly die out
- 1.0 = 100% chance to spawn
- Type: float

### threads
- It is not the total number of the treads used by the simulation
- The total number of threads used by the simulation is threads + 2 because the main thread and the update thread (which then creates thread threads)
- See the section at the bottom about optimization for more info
- Type: int

### targetFPS
- Used for dynamic tick mode
- See the section below for more info
- Type: int


# Simulation specific controls and information

### CONTROLS

### DRAW MODES

### TICK MODES

### OTHER INFO


# Showcase


# Optimization
- things already done
- multithreading


# Other
- COMPILING
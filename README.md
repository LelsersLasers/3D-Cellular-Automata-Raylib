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

- Survival: [A, B, ...]
    - If a cell is alive, it will remain alive if it has A, B, or ... neighbors
    - If it does not have A, B, or ... neighbors, it will begin dying
    - Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a signle number (ex: [2])
- Spawn: [A, B, ...]
    - If a cell is dead, it will come alive if it has A, B, or ... neighbors
    - Can be a list of numbers (ex: [2, 3, 6, 7, 10]) or a signle number (ex: [2])
    - Can have any amount of overlap with survival
- State: A
    - Once a cell begins dying, it has C game ticks to live before disappearing
    - Both survival and spawn rules will no longer affect the cell while it decays
    - Must be a single number (ex: 6)
- Neighborhoods: "M" or "VN"
    - How neighbors are counted
    - "M": Moore: faces + counts diagonal neighbors, think rubics cube (3^3 - 1 = 26 possible neighbors)
    - "VN": Von Neuman: only counts neighors where the faces touch (6 possible)
- To change rules in this simulation:
    - Edit 'rules.json'
        - Make sure the format is correct (value types and key names must not be changed)
    - The simulation must be restarted to see the changes

Some examples (can just copy/replace in 'rules.json')
- Slow build up: 9-18/5-7,12-13,15/6/M
    ```
    "survival": [9, 10, 11, 12, 13, 14, 15, 16, 17, 18],
    "spawn": [5, 6, 7, 12, 13, 15],
    "state": 6,
    "neighborhood": "M"
    ```
- Outward expansion + hallow center: <2,6,9/4,6,8-9/10/M>
    ```
    "survival": [2, 6, 9],
    "spawn": [4, 6, 8, 9],
    "state": 10,
    "neighborhood": "M"
    ```

# Simulation specific controls and information

CONTROLS

DRAW MODES

TICK MODES

OTHER INFO


# Showcase


# Other
- COMPILING
- already optimized things
- how my multithreading works
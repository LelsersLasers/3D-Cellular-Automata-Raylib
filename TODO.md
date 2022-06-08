# TODO

- The actual 3d cellular automata
    - Does it work?

- Fullscreen is rather "janky" feeling when it goes into fullscreen mode

- Improve speed -> cell bounds = 100
    - Optimize: HOW?
        - Re creating const array per cell in updateCells, drawLeftBar, etc?
        - Extra if statements per cell in updateCells, or draw+sync Cells
    - Is VECTOR slower than arrays/other methods?
        - Am I passing it wrong?
    - Multithreading?

- On demand changes for bound, cell rules, (color, etc)
- Menu screens etc?
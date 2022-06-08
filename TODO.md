# TODO

- The actual 3d cellular automata
    - Does it work?
    - How to actucally get patterns?

- Fullscreen is rather "janky" feeling when it goes into fullscreen mode
    - Change to borderless fullscreen? Or even bordered?
- Dynamic update speed option?
    - Set target fps, if fps > target fps, increase update speed, if fps < target fps, decrease update speed
    - given room for 'error' (aka within 5 of target fps is fine)
    - manual, fastest, dynamic
    - save manual tick setting, and revert to that then it is switched back to it

- Improve speed -> cell bounds = 100
    - Optimize: HOW?
        - Re creating const per frame in drawLeftBar, etc?
        - Re creating non const per frame with delta, etc?
        - Extra if statements per cell in updateCells, or draw+sync Cells
    - Is VECTOR slower than arrays/other methods?
        - Am I passing it wrong?
    - Multithreading?

- Cleaning:
    - 'f' on floats? when float, when int?
    - when is casting needed, when is it exessive?

- On demand changes for bound, cell rules, (colors, etc)
- Menu screens etc?
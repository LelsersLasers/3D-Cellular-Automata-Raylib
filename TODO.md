# TODO

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
    - floor division?

- On demand changes for bound, cell rules, (colors, etc)
- Menu screens etc?
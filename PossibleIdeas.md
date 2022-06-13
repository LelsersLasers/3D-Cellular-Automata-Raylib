# Possible Ideas

UPDATE README WITH NO STATE ENUM + BETTER BRANCHLESS


Cleaning:
- 'hard set' offsets in updateCells()
- Where can use branchless? Is it faster?

Possible/advanced/likely won't happen ideas:
- On demand changes for bound, cell rules, (colors, etc)?
    - Or maybe live/key bind to reload from JSON?
- Menu screens, etc?
- Shaders to make cells easier to see/tell apart?
- Further improve performance?
    - Do calculations on GPU?
    - Improve multithreading?
        - Saved updated/synced cells and moderate the playback?
    - Anywhere else to apply branchless programing?
        - Already sort of there, branches at the highest level possible
        - According to google - fastest: new_state = old_state * transition with linear math
            - Possibly make cells into matrix and try to do it all in 1 step?
    - Profiling?
        - See where time is spent -> fix there
- Reorganize project structure to follow C++ paradigms (e.g. header files, etc)
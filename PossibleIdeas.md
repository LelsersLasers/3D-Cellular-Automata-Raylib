# Possible Ideas

Possible/advanced/likely won't happen ideas:
- On live/onscreen reload from JSON?
    - Colors, draw modes, etc?
- Menu screens, etc?
- Shaders to make cells easier to see/tell apart?
- Further improve performance?
    - Do calculations on GPU?
    - Improve multithreading?
        - Saved updated/synced cells and moderate the playback?
    - Anywhere else to apply branchless programing?
        - According to google - fastest: new_state = old_state * transition with linear math
            - Possibly make cells into matrix and try to do it all in 1 step?
            - Could switch to a more function approach where instead of a cell class, it is just the hp number
                - And then could use 1 line to sync the cells
    - Profiling?
        - See where time is spent -> fix there
- Reorganize project structure to follow C++ paradigms (e.g. header files, etc)
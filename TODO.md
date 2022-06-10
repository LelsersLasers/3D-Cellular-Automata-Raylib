# TODO

- Improve speed -> cell bounds = 100
    - Optimize: HOW?
        - (https://www.reddit.com/r/learnprogramming/comments/v7yn13/optimization_questions_about_c_and_cellular/)
    - Profile?
        - See where time is spent -> fix there
    - Things to try:
        - Branchless programming?
            - Profile/etc to figure out if it is actually faster

- Multithreading
    - Must do update -> sync
    - render does not support multithreading
    - multithread randomize() ?
    - A)
        - thread1: update -> sync as fast as possible (update/sync multithreaded)
        - main thread: render based on updatespeed/tick mode
    - B)
        - main thread: multithread update -> sync/draw (current setup)
    - C)
        - can run update in parallel with render
        - same with sync?
        - how would work when tick mode != fastest
    - C) some comibination of A and B


- Cleaning:
    - 'size_t' to index
    - not use std namespace
    - 'hard set' offsets in updateCells()
    - Where can use branchless? Is it faster?

- On demand changes for bound, cell rules, (colors, etc)
    - Maybe first load rules/settings from JSON file?
- Menu screens etc?
- Calc on GPU?
- Shaders to make cells eaiser to see?

- Readme, showcases, etc
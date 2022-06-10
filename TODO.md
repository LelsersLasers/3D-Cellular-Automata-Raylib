# TODO

- Improve speed -> cell bounds = 100
    - Optimize: HOW?
        - (https://www.reddit.com/r/learnprogramming/comments/v7yn13/optimization_questions_about_c_and_cellular/)
    - Profile?
        - See where time is spent -> fix there
    - Things to try:
        - Branchless programming?
            - Profile/etc to figure out if it is actually faster
            - fastest possible is:
                - new_state = old_state * transistion
                - Can do with linear math
        - 1 cell surrounding padding to eleminate checkValidIndex?
            - 102^3/100^3 = 1.06 = 6% more cells

- Multithreading
    - Must do update -> sync
    - render does not support multithreading
    - multithread randomize() ?
    - Current ('||' = parallel)):
        - update + sync || render
        - update and sync are both split into 8 slices
    - A)
        - thread1: update -> sync as fast as possible (update/sync multithreaded)
        - main thread: render based on updatespeed/tick mode
    - C)
        - can run update in parallel with render
        - same with sync?
        - how would work when tick mode != fastest
    - C) some comibination of A and B


- Cleaning:
    - 'hard set' offsets in updateCells()
    - Where can use branchless? Is it faster?

- On demand changes for bound, cell rules, (colors, etc)
    - Maybe first load rules/settings from JSON file?
- Menu screens etc?
- Calc on GPU?
- Shaders to make cells eaiser to see?

- Readme, showcases, etc
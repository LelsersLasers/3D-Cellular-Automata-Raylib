# TODO

- Improve speed -> cell bounds = 100
    - Optimize: HOW?
        - (https://www.reddit.com/r/learnprogramming/comments/v7yn13/optimization_questions_about_c_and_cellular/)
    - Profile?
        - See where time is spent -> fix there
    - Things to try:
        - Calc on GPU?
        - Branchless programming?
            - Profile/etc to figure out if it is actually faster
        - Multithreading?

- Shaders to make cells eaiser to see?

- Cleaning:
    - Where can use branchless?

- On demand changes for bound, cell rules, (colors, etc)
    - Maybe first load rules/settings from JSON file?
- Menu screens etc?
# TODO

Cleaning:
- 'hard set' offsets in updateCells()
- Where can use branchless? Is it faster?

Possible/advanced/likely won't happen ideas
- On demand changes for bound, cell rules, (colors, etc)?
- Menu screens etc?
- - Shaders to make cells eaiser to see?
- Improvde preformance?
    - Calc on GPU?
    - Improve multithreading?
        - Saved updated/synced cells and moderate the playback?
    - Branchless programing?
        - Already sort of there, branches at the highest level possible
        - Fastest: new_state = old_state * transistion with linear math
    - Profiling?
        - See where time is spent -> fix there

Non-code:
- Readme, showcases, json "comment", explainantion of rules, etc
- Organize into multiple files?
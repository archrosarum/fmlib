# Coding conventions

Do not use assertion statements in library code, examples, or tests. Use explicit
condition checks and appropriate error returns or diagnostic messages instead.
Keep test operations active in all build configurations and clean up resources
on failure. After implementation works, review the changed code and refactor
away any assertions before completing the task.

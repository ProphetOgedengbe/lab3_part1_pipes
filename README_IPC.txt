
# Lab 3 — IPC (Processes & Pipes)

This folder contains ready-to-build solutions for the two tasks you described.

## Files
- `pipes_processes1_2way.c` — **Part 1**: Two-way pipe between parent & child.
- `pipes_processes3.c` — **Part 2**: Builds a pipeline `cat scores | grep <arg> | sort`.
- `scores` — Example data file (place in the same directory as the executables).

## Build
```bash
gcc -Wall -Wextra -O2 -o pipes_processes1_2way pipes_processes1_2way.c
gcc -Wall -Wextra -O2 -o pipes_processes3       pipes_processes3.c
```

## Run — Part 1
```bash
./pipes_processes1_2way
# Example interaction:
# Input : www.geeks
# Other string is: howard.edu
# Output : www.geekshoward.edu
# Input : helloworld
# Output : www.geekshoward.eduhelloworldgobison.org
```

## Run — Part 2
Make sure a `scores` file is present in the current directory; then:
```bash
./pipes_processes3 28
# Equivalent to: cat scores | grep 28 | sort
```

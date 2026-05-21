# Memory Hierarchy
This is an utility to see how the memory hierarchy affects the execution of a program on a x86-64 cpu.
The main goal is understand cache hits and misses, data aligment and its importance on optimization.
## Architecture
The specs for my laptop are:
```bash
48 bits virtual and physical size (256 GiB max RAM)

8 physical cores with 2 threads each
Each physical core has:
L1i 265KiB
L1d 265KiB
L2 4MiB

And two complexes (CCX) of 4 cores with:
L3 8MiB each
```
## Cache
So, first we compare the access time difference between each line.
For this we will be doing steps of 16 and using 32 bit integer so that we go from cache line to cache line.

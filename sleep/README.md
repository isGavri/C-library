# `sleep` implementation from scratch, using [Valigo's video](https://www.youtube.com/watch?v=IbibjkI1kIs) as guide.
No libc, rewrite of symbol_start, main, puts, atol, printf and sleep.

## Target code
We are trying to rewrite a program like the on [simple_sleep.c](simple_sleep.c)

## Implementation
### `symbol_sart`
[Systyem V application Binary Interface](https://raw.githubusercontent.com/wiki/hjl-tools/x86-psABI/x86-64-psABI-1.0.pdf)


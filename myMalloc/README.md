# Memory Allocator
Here I learn how to implement and write my own malloc, free, calloc and realloc
I will be typing an overviwew of what i learnt by reading the resources and then polishing it once i write my own implementation

## Theory

### Arena

By [DJ Delorie on glibc](#resources)
> Arena - Structure that is shared among one or more threads which contains references to one or more heaps, as well as linked lists of chunks within those heaps which are "free". Threads assigned to each arena will allocate memory from that arena's free lists.

I understand this is where we will manage our memory (request and everything). So we will leave its definition as how [attractivechaos says](https://gist.github.com/attractivechaos/862fb6e58147b47c9d16bf2d9e12445f) collection of memory chunks that are managed together and typically share the same lifetime

Read the whole blog post.


### Heap

By [DJ Delorie on glibc](#resources)
> Heap - Contiguous regions  of memory that is subdivided into chunks to  be allocated. Each heap belongs to exacly one arena. 

### Chunk

By [DJ Delorie on glibc](#resources)
> Chunk - A small range of memory that can be allocated, freed, or combined with adjacent chunks into large ranges. Wrapper around the block of memory  given to the application. Chunk in one heap and belongs to one arena

On glibc's malloc each chunks has a header where it contains metadata about how big  it is and thus where the adjacent chunks are.
Chunk pointer point to the last word in the previous chunk

### Memory

By [DJ Delorie on glibc](#resources)
> Memory - Portion of the application's addres space which is typically backed by RAM or swap





#### Resources
[ A Malloc Tutorial by Marwan Burelle ]( https://wiki-prog.infoprepa.epita.fr/images/0/04/Malloc_tutorial.pdf )
[ Memory Allocation Strategies](https://www.gingerbill.org/series/memory-allocation-strategies/)

[ Malloc tutorial by Dan Luu](https://danluu.com/malloc-tutorial/)

[Malloc Internals of the glibc wiki](https://sourceware.org/glibc/wiki/MallocInternals)



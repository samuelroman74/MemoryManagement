# Memory Manager
After initializing a certain amount of memory, manages said memory in blocks and properly dynamically allocates (and deallocates) said memory.

Notable aspects of the project include:

* Data structure is a map tha has void pointers as keys and tuples as values. Tuples include 2 integers for marking position in initialized memory of block and a boolean that indicates whether the block is a "hole" or has not been filled with memory yet.
* Arranges memory in two different ways depending on user specification: best fit & worst fit, dependent on how much space is left in a hole if that memory is allocated there.
* Allows for the creation of a file that denotes the start position and length of any holes via the dumpMemoryMap function.

# gba-flash-fs

A log-structured filesystem implementation backed by gameboy advance SRAM or Flash memory. Also tested on aftermarket repro cartridges.
Originally part of https://github.com/evanbowman/skyland-beta.


Run make to build the library, example rom, and run unit tests.


Memory requirements:
Under normal cirumstances, uses three integer variables to track filesystem data, as well as a bloom filter for speeding up file reads. When the filesystem runs out of room and needs to be compacted, the library will allocate up to 64kb of memory in the worst case (briefly, while performing filesystem compaction for an almost-full flash sector for a flash chip. 32kb worst case for SRAM storage). But when not compacting an almost-full filesystem, memory requirements are minimal. By almost-full, I mean full of valid files that cannot be removed by defragmentation.


Testing:
Compiles for desktop targets and includes a number of unit tests. Manually tested fairly extensively. If you find any bugs, please let me know (I will certainly fix them, as I use this library in my own projects).

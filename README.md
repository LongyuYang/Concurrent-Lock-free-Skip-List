# Concurrent-Lock-free-Skip-List
Final course project for CMU [15-618 Parallel Computer Architectures and Programming (Fall 2021)](http://www.cs.cmu.edu/afs/cs/academic/class/15418-f21/www/). More details are shown in our [project webpage](https://lizidong2015.wixsite.com/15618)
## Requirements
### Garbage Collector Library
- Since lazy deletion is performed in our pointer-lock and lock-free implementations, [Boehm Garbage Collector](https://hboehm.info/gc/) library is required.
- Please download the ```gc-7.0``` tar ball from [here](https://hboehm.info/gc/gc_source/) to the root of this repository.
- Execute the following commands:
```bash
tar zxvf gc-7.0.tar.gz
mkdir gc
cd gc-7.0
./configure --prefix=ABSOLUTE_PATH_OF_THIS_REPO/gc --enable-threads=posix --enable-thread-local-alloc --enable-parallel-mark --enable-cplusplus
make
make install
```

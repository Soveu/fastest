# fastest
Simple and efficient tools for testing executables meant to be send to some
kind of online judge.

## Dependencies
* Python 3.6
* GCC/Clang for compiling `procmp`

## Usage
See example.py

## FAQ

### Why Python 3, not 2?
Because `subprocess.communicate()` in Python 2 does not support timeouts

### Why the script is written in Python, but `procmp` is in C++?
Using lower-level functions like `read()` and `write()` in C have a benefit
of not blocking the flow of a program, because they can read/write less data
than its length and it is OK - their return value indicates how many bytes
were actually read/written. In python `read()` and `write()` can lead to
unpleasant deadlocks.

BUT if you have a clever idea how to solve it, feel free to write the code and
push changes here.


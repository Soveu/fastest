# fastest
Simple and efficient tools for testing executables meant to be send to some
kind of online judge.

## Dependencies
* Python 3.6
* GCC/Clang and Unix-like system, which supports `fork`, `pipe`, `read` and `write` \
  for compiling `procmp`

## Usage
See example.py

## FAQ

### Why Python 3, not 2?
Because `subprocess.communicate()` in Python 2 does not support timeouts

### What is `procmp`?
`procmp` is a simple tool that:
* takes input
* duplicates it between two programs
* collects input
* compares it, when it is equal, prints nothing and returns 0, if not, prints the output from both and returns 1

### Why the script is written in Python, but `procmp` is in C++?
Using lower-level functions like `read` and `write` in C have a benefit
of not blocking the flow of a program, because they can read/write less data
than its length and it is OK - their return value indicates how many bytes
were actually read/written. In python `read()` and `write()` can lead to
unpleasant deadlocks.

BUT if you have a clever idea how to solve it, feel free to write the code and
push changes here.

### Why does it not work on Windows?
Inter-process communication is much easier on Unix-like systems with `fork`, `pipe`,
`read`, `write`, but you can give it a try on WSL. (not tested yet)


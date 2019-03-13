#!/usr/bin/python3

import random
import time

import autotesting 

def createInput(x):
    x = random.random()
    return (
            bytearray("import time, sys; time.sleep(%f); sys.stdout.write('123')" % x, encoding='ascii'),
            bytearray('123', encoding='ascii'),
            )

unittests = [
        (b'print(5)', b'5\n'),
        (b'print(6)', b'6\n'),
        (b'print()' , b'\n'),
        (b'print("a")', b'a\n'),
]

def unitTesting(x):
    return unittests[x-1]

sh = '/bin/bash'
def bashtest(i):
    return bytearray("echo %d $RANDOM\n" % i, encoding='ascii')

if __name__ == '__main__':
    autotesting.run('python', createInput, tests=10, timeout=0.95, stripOutput=False)
    autotesting.run('python', unitTesting, tests=len(unittests), timeout=0.3, stripOutput=False)
    autotesting.compare_processes(sh, sh, bashtest);


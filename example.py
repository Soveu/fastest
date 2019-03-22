#!/usr/bin/python3

import random
import time

import autotesting as tests

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
    return bytearray("echo %d\n" % i, encoding='ascii')

if __name__ == '__main__':
    tests.run('python', createInput, tests=10, timeout=0.95, stripOutput=False)
    tests.run('python', unitTesting, tests=len(unittests), timeout=0.35, stripOutput=False)
    tests.procmp(sh, sh, bashtest);


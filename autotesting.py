import subprocess
import time

def run(exe, inputfunc, timeout=1, tests=10, stripOutput=True):
    averagetime = 0

    for i in range(1, tests+1):
        give, expect = inputfunc(i)
        print("[%02d/%02d] " % (i, tests), end='', flush=True)
        timep = time.perf_counter()

        try:
            proc = subprocess.Popen(
                    [exe], 
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                )
            stdout, stderr = proc.communicate(input=give, timeout=timeout)
        except subprocess.TimeoutExpired as e:
            print('FAIL')
            print("Timed out after %.02f seconds" % e.timeout)
            proc.kill()
            stdout, stderr = proc.communicate()
            return

        deltatime = time.perf_counter() - timep
        averagetime += (deltatime - averagetime) / i

        if stripOutput:
            flag = (expect == stdout.strip())
        else:
            flag = (expect == stdout)
    
        if(flag):
            print("Time: %fs, Average: %fs" % (deltatime, averagetime))
        else:
            print('FAIL')
            print("Expected:", expect)
            print("Got:", stdout, stderr)
            return

def procmp(exe1, exe2, inputfunc, tests=10):
    averagetime = 0.0

    for i in range(1, tests+1):
        give = inputfunc(i)
        print("[%02d/%02d] " % (i, tests), end='', flush=True)
        timep = time.perf_counter()

        proc = subprocess.Popen(
                ['./procmp/procmp', exe1, exe2], 
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
        stdout, stderr = proc.communicate(input=give)

        if proc.returncode != 0:
            print("Process returned with status %d\n%s\n" % (proc.poll(), stdout.decode('utf-8')))

        deltatime = time.perf_counter() - timep
        averagetime += (deltatime - averagetime) / i
        print("Time: %fs, Average: %fs" % (deltatime, averagetime))

    return


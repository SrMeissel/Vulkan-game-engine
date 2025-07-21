import time
import random
import sys 

def generate_entity_id():
    time_ns = time.monotonic_ns()
    id_ = random.getrandbits(32)
    return (time_ns << 32) | id_

if len(sys.argv) >= 2:
    for i in range(int(sys.argv[1])) :
        print(generate_entity_id())
        time.sleep(0.1)
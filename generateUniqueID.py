# this program is meant to provide a simple way to retroactively give ID's to old entities.

import time
import random
import sys 

def generate_entity_id():
    time_ns = time.monotonic_ns()
    id_ = random.getrandbits(32)
    return ((time_ns << 32) | id_) & 0xFFFFFFFFFFFFFFFF

if len(sys.argv) >= 2:
    for i in range(int(sys.argv[1])) :
        print(generate_entity_id())
        time.sleep(1)
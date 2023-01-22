# -*- coding: utf-8 -*-
"""
Entry point for MyoVentPy system
@author: Ken Campbell
"""

import sys
import time

from modules.batch.batch import run_batch
from modulues.util.util import util_Frank_Starling

def parse_inputs():

    start_time = time.time()    

    if (sys.argv[1] == "run_batch"):
        run_batch(sys.argv[2])
    
    if (sys.argv[1] == "util_Frank_Starling"):
        
        
    print('MyoVent execution time: %f' % (time.time() - start_time))


if __name__ == "__main__":
    parse_inputs()


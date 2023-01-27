# -*- coding: utf-8 -*-
"""
Entry point for MyoVentPy system
@author: Ken Campbell
"""

import sys
import time

from modules.batch.batch import run_batch
from modules.utilities.utilities import util_Frank_Starling

def parse_inputs():

    start_time = time.time()    

    if (sys.argv[1] == "run_batch"):
        if (len(sys.argv) == 3):
            run_batch(sys.argv[2])
        else:
            run_batch(sys.argv[2], figures_only=True)
    
    if (sys.argv[1] == "util_Frank_Starling"):
        util_Frank_Starling(sys.argv[2])
        
        
    print('MyoVent execution time: %f' % (time.time() - start_time))


if __name__ == "__main__":
    parse_inputs()


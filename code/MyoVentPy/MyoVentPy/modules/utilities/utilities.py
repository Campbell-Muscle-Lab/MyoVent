# -*- coding: utf-8 -*-
"""
Created on Sun Jan 22 12:43:23 2023

@author: ken
"""

import os
import json

from pathlib import Path

from ..batch import batch

def test_Frank_Starling(json_setup_file_string):
    """ Evaluates isovolumic cardiac cycle at different volumes """

    # Load the setup file
    with open(json_setup_file_string, 'r') as f:
        json_data = json.load(f)
        MyoVent_test = json_data['MyoVent_test']
    
    # Load the Frank_Starling structure
    fs = MyoVent_test['Frank_Starling']

    # Get the model file
    if not ('relative_to' in fs):
        model_file_string = fs['model_file']
    elif (fs['relative_to'] == 'this_file'):
        base_dir = Path(json_setup_file_string).parent.absolute()
        model_file_string = os.path.join(base_dir, fs['model_file'])
    else:
        base_dir = fs['relative_to']
        model_file_string = os.path.join(base_dir, fs['model_file'])

    # Load the model
    with open(model_file_string, 'r') as f:
        base_model = json.load(f)
        
        


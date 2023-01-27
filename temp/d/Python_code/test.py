# -*- coding: utf-8 -*-
"""
Created on Mon Jan 23 14:56:14 2023

@author: kscamp3
"""

import os
import json
import copy
import shutil

from pathlib import Path

import numpy as np

def test():
    """ Runs simulation """

    # Variables
    exe_path = 'c:/ken/github/campbellmusclelab/models/myovent/code/myoventcpp/bin/myoventcpp.exe'
    sim_data_folder = '../sim_data'
    batch_file_string = 'batch.json'
    base_model_file_string = '../base/model.json'
    base_options_file_string = '../base/options.json'
    template_file_strings = ['../templates/template_baro.json']
    MyoVentPy_code_dir = 'c:/ken/github/campbellmusclelab/models/myovent/code/myoventpy/myoventpy'
    
    time_step = 0.001
    no_of_time_steps = 800000
    
    baroreflex_start_s = 25
    baroreflex_stop_s = 10000
    
    growth_start_s = 100
    growth_stop_s = 10000
    
    shrink_eccentric_rate = [0,  -0.0005, 0,      -0.0005]
    shrink_concentric_rate = [0,  0,     -0.001, -0.001]
    
    pert = []
    pert.append({'class': 'baroreflex', 'variable': 'baro_P_set',
                 't_start_s': 500, 't_stop_s': 500.1, 'total_change': 40})
    
    no_of_jobs = len(shrink_eccentric_rate)
    
    # Create the batch structure
    b = dict()
    b['MyoVent_batch'] = dict()
    b['MyoVent_batch']['MyoVentCpp_exe'] = dict()
    b['MyoVent_batch']['MyoVentCpp_exe']['exe_file'] = exe_path
    
    # Set the sim_data folder
    sim_data_folder = str(Path(sim_data_folder).absolute().resolve())
        
    # Clean it out
    try:
        print('Trying to clean %s' % sim_data_folder)
        shutil.rmtree(sim_data_folder, ignore_errors = True)
    except OSError as e:
        print("Error: %s : %s" % (sim_data_folder, e.strerror))
        
    # Remake folder
    if not os.path.isdir(sim_data_folder):
        os.makedirs(sim_data_folder)
        
    # Load the base model
    with open(base_model_file_string, 'r') as f:
        base_model = json.load(f)
        
    # Load the base options
    with open(base_options_file_string, 'r') as f:
        base_options = json.load(f)

    # Make jobs
    job = []
    
    for i in range(no_of_jobs):
        
        # Make input and output folders
        sim_input_folder = os.path.join(sim_data_folder, 'sim_input',
                                     ('%i' % (i+1)))
        if not os.path.isdir(sim_input_folder):
            os.makedirs(sim_input_folder)
            
        sim_output_folder = os.path.join(sim_data_folder, 'sim_output',
                                     ('%i' % (i+1)))
        if not os.path.isdir(sim_output_folder):
            os.makedirs(sim_output_folder)
            
        # Copy the model
        new_model = copy.deepcopy(base_model)
        
        # Adapt the shrinkage
        new_model['growth']['shrinkage']['eccentric_rate'] = \
            shrink_eccentric_rate[i]
        new_model['growth']['shrinkage']['concentric_rate'] = \
                shrink_concentric_rate[i]
        
        # Write it
        new_model_file_string = os.path.join(sim_input_folder, 'model.json')
        with open(new_model_file_string, 'w') as f:
            json.dump(new_model, f, indent=4)

        # Copy the options
        new_options = copy.deepcopy(base_options)
        
        # Write it
        new_options_file_string = os.path.join(sim_input_folder, 'options.json')
        with open(new_options_file_string, 'w') as f:
            json.dump(new_options, f, indent=4)
            
        # Create a protocol
        prot = dict()
        prot['protocol'] = dict()
        prot['protocol']['time_step'] = time_step
        prot['protocol']['no_of_time_steps'] = no_of_time_steps
        
        prot['activation'] = []
        
        a = dict()
        a['type'] = "baroreflex"
        a['t_start_s'] = baroreflex_start_s
        a['t_stop_s'] = baroreflex_stop_s
        prot['activation'].append(a)
        
        a = dict()
        a['type'] = "growth"
        a['t_start_s'] = growth_start_s
        a['t_stop_s'] = growth_stop_s
        prot['activation'].append(a)
        
        prot['perturbation'] = []
        for p in pert:
            prot['perturbation'].append(p)            
        
        # Write the protocol to file
        new_protocol_file_string = os.path.join(sim_input_folder, 'protocol.json')
        with open(new_protocol_file_string, 'w') as f:
            json.dump(prot, f, indent=4)

        # Create an output handler
        new_oh_file_string = os.path.join(sim_input_folder, 'output_handler.json')

        oh = dict()
        oh['templated_images'] = []
        for j,t in enumerate(template_file_strings):
            oh_t = dict()
            oh_t['template_file_string'] =  str(Path(t).absolute().resolve())
            oh_t['output_file_string'] = os.path.join(sim_data_folder,
                                                      ('output_%i_%i' % (j+1, i+1)))
            
            oh['templated_images'].append(oh_t)
            
        with open(new_oh_file_string, 'w') as f:
            json.dump(oh, f, indent=4)
            
        # Set the results file
        new_results_file_string = os.path.join(sim_output_folder, 'results.json')
        
        # Make a new job
        j = dict()
        j['model_file'] = new_model_file_string
        j['options_file'] = new_options_file_string
        j['protocol_file'] = new_protocol_file_string
        j['results_file'] = new_results_file_string
        j['output_handler_file'] = new_oh_file_string
        
        # Add to job
        job.append(j)
    
    # Add jobs to batch
    b['MyoVent_batch']['job'] = job
    
    # Write batch file
    batch_file_string = os.path.join(sim_data_folder, batch_file_string)
    with open(batch_file_string, 'w') as f:
        json.dump(b, f, indent=4)
    
    # Generate a command line
    cs = 'pushd \"%s\" & python MyoVentPy.py run_batch %s & popd' % \
            (MyoVentPy_code_dir, batch_file_string)

      # And run it
    os.system(cs)
        

if (__name__ == "__main__"):
    test()
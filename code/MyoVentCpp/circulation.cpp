/**
/* @file		circulation.cpp
/* @brief		Source file for a circulation object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "circulation.h"
#include "cmv_system.h"
#include "cmv_model.h"
#include "cmv_options.h"
#include "cmv_results.h"
#include "hemi_vent.h"

// Constructor
circulation::circulation(cmv_system* set_p_parent_cmv_system = NULL)
{
	//! Constructor

	// Code
	printf("circulation constructor()\n");

	// Initialise

	// Set the pointer to the parent system
	p_parent_cmv_system = set_p_parent_cmv_system;
	p_cmv_model = p_parent_cmv_system->p_cmv_model;

	// Set pointers to safety
	p_cmv_options = NULL;
	p_cmv_results = NULL;

	// Now initialise other objects
	circ_blood_volume = p_cmv_model->circ_blood_volume;

	circ_no_of_compartments = p_cmv_model->circ_no_of_compartments;

	// Set up arrays
	circ_resistance = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_compliance = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_slack_volume = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_pressure = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_volume = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_flow = (double*)malloc(circ_no_of_compartments * sizeof(double));

	// Initialise
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		circ_resistance[i] = p_cmv_model->circ_resistance[i];
		circ_compliance[i] = p_cmv_model->circ_compliance[i];
		circ_slack_volume[i] = p_cmv_model->circ_slack_volume[i];
		circ_pressure[i] = 0.0;
		circ_volume[i] = 0.0;
		circ_flow[i] = 0.0;
	}

	// Fill the ventricle to slack
	circ_volume[0] = circ_slack_volume[0];

	// Put other blood in the last compartment
	circ_volume[circ_no_of_compartments - 1] = circ_blood_volume - circ_slack_volume[0];

	// Make a hemi-vent object
	p_hemi_vent = new hemi_vent(this);

}

// Destructor
circulation::~circulation(void)
{
	//! Destructor

	// Code
	printf("circulation destructor()\n");

	delete p_hemi_vent;

	free(circ_resistance);
	free(circ_compliance);
	free(circ_slack_volume);
	free(circ_pressure);
	free(circ_volume);
	free(circ_flow);
}

// Other functions

void circulation::initialise_simulation(void)
{
	//! Code initialises simulation
	
	// Variables

	// Code

	// Set the options
	p_cmv_options = p_parent_cmv_system->p_cmv_options;

	// Set the results
	p_cmv_results = p_parent_cmv_system->p_cmv_results;

	// Now handle daughter objects
	p_hemi_vent->initialise_simulation();
}

void circulation::implement_time_step(double time_step_s)
{
	//! Code advances by 1 time-step
	
	p_hemi_vent->implement_time_step(time_step_s);

	calculate_flows(circ_volume);

	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		cout << "f[" << i << "]: " << circ_flow[i];
		
		if (i == (circ_no_of_compartments - 1))
			cout << "\n";
		else
			cout << "\t";
	}
}

void circulation::calculate_flows(double v[])
{
	//! Function sets the values of circ_flows[] based on an
	//! array of compartment volumes where
	//! circ_flow[i] is flow into compartment i through resistance i
	
	// Variables
	double* p;

	// Code

	// Initialise pressure array
	p = (double*)malloc(circ_no_of_compartments * sizeof(double));

	// Calculate the lv pressure
	p[0] = p_hemi_vent->return_pressure_for_chamber_volume(v[0]);

	// Calculate the other pressures
	for (int i = 1; i < circ_no_of_compartments; i++)
	{
		p[i] = (v[i] - circ_slack_volume[i]) / circ_compliance[i];
	}

	// Calculate the flows
	for (int i = 1; i < circ_no_of_compartments; i++)
	{
		circ_flow[i] = (1.0 / circ_resistance[i]) *
			(p[i] - p[i-1]);
	}

	// Special case for flow into the ventricle
	if (p[circ_no_of_compartments - 1] > p[0])
	{
		circ_flow[0] = (1.0 / circ_resistance[0]) *
			(p[circ_no_of_compartments - 1] - p[0]);
	}
	else
	{
		circ_flow[0] = 0.0;
	}

	// Special case for flow out of ventricle
	if (p[0] < p[1])
	{
		circ_flow[1] = 0.0;
	}

	// Tidy up
	free(p);

}

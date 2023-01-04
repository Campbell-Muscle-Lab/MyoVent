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
#include "half_sarcomere.h"

#include "gsl_errno.h"
#include "gsl_odeiv2.h"



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

	// Initialise, noting total slack_volume as we go
	// Start with the compartments at slack volume
	circ_total_slack_volume = 0.0;

	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		circ_resistance[i] = p_cmv_model->circ_resistance[i];
		circ_compliance[i] = p_cmv_model->circ_compliance[i];
		circ_slack_volume[i] = p_cmv_model->circ_slack_volume[i];
		circ_pressure[i] = 0.0;
		circ_volume[i] = circ_slack_volume[i];
		circ_flow[i] = 0.0;

		circ_total_slack_volume = circ_total_slack_volume +
			circ_slack_volume[i];
	}

	// Excess blood goes in veins
	cout << "Blood volume: " << circ_blood_volume << "\n";
	cout << "Total slack volume: " << circ_total_slack_volume << "\n";

	if (circ_blood_volume < circ_total_slack_volume)
	{
		cout << "Error: blood volume is less than total slack volume\n";
		exit(1);
	}
	circ_volume[circ_no_of_compartments - 1] = circ_volume[circ_no_of_compartments - 1] +
		(circ_blood_volume - circ_total_slack_volume);

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

	// Add data fields
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		string label = string("pressure_") + to_string(i);
		p_cmv_results->add_results_field(label, &circ_pressure[i]);
	}

	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		string label = string("volume_") + to_string(i);
		p_cmv_results->add_results_field(label, &circ_volume[i]);
	}

	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		string label = string("flow_") + to_string(i);
		p_cmv_results->add_results_field(label, &circ_flow[i]);
	}
}

// This function is not a member of the circulation class but is used to interace
// with the GSL ODE system. It must appear before the circulation class members
// that calls it, and communicates with the circulation class through a pointer to
// the class object

int circ_vol_derivs(double t, const double y[], double f[], void* params)
{
	// Function sets dV/dt for the compartments

	// Variables
	(void)(t);						// Prevents warning for unused variable

	circulation* p_circ = (circulation*)params;
									// Pointer to circulation

	// Code
	
	// Calculate the flows between the compartments
	p_circ->calculate_flows(y);

	// Now adjust volumes
	//! if compartments are
	//! [0] - [1] - [2] - [3] - .... [n-1]
	//! circ_flow[i] is flow from [i-1] to [i] through resistance i
	
	for (int i = 0; i < p_circ->circ_no_of_compartments; i++)
	{
		if (i < (p_circ->circ_no_of_compartments - 1))
			f[i] = p_circ->circ_flow[i] - p_circ->circ_flow[i + 1];
		else
			f[i] = p_circ->circ_flow[i] - p_circ->circ_flow[0];
	}

	// Return
	return GSL_SUCCESS;
}

void circulation::implement_time_step(double time_step_s)
{
	//! Code advances by 1 time-step
	
	// Variables
	int status;

	double t_start_s = 0.0;
	double t_stop_s = time_step_s;
	
	double eps_abs = 1e-2;
	double eps_rel = 1e-4;

	// Code

	// Update the hemi_vent object, which includes
	// updating the daughter objects
	p_hemi_vent->implement_time_step(time_step_s);

	// Now adjust the compartment volumes by integrating flows.
	gsl_odeiv2_system sys =
		{ circ_vol_derivs, NULL, circ_no_of_compartments, this };

	gsl_odeiv2_driver* d =
		gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45,
			0.5 * time_step_s, eps_abs, eps_rel);

	status = gsl_odeiv2_driver_apply(d, &t_start_s, t_stop_s, circ_volume);

	if (status != GSL_SUCCESS)
	{
		cout << "circulation::implement_time_step problem with ODEs\n";
		exit(1);
	}
	
	// Update the hemi_vent with the new volume
	p_hemi_vent->update_chamber_volume(circ_volume[0]);

	// Make sure total volume remains constant
	double holder = 0.0;
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		holder = holder + circ_volume[i];
	}
	// Any adjustment goes in veins
	double adjustment = (circ_blood_volume - holder);
	circ_volume[circ_no_of_compartments - 1] = 
		circ_volume[circ_no_of_compartments - 1] + adjustment;

	// Warning
	if (fabs(adjustment) > 1e-4)
	{
		cout << "Blood volume mismatch\n";
		exit(1);
	}
}

void circulation::calculate_pressures(const double v[], double p[])
{
	//! Function calculates pressures
	
	// Code

	p[0] = p_hemi_vent->return_pressure_for_chamber_volume(v[0]);

	// Calculate the other pressures
	for (int i = 1; i < circ_no_of_compartments; i++)
	{
		p[i] = (v[i] - circ_slack_volume[i]) / circ_compliance[i];
	}
}

void circulation::calculate_flows(const double v[])
{
	//! Function sets the values of circ_flows[] based on an
	//! array of compartment volumes
	//! if compartments are
	//! [0] - [1] - [2] - [3] - .... [n-1]
	//! circ_flow[i] is flow from [i-1] to [i] through resistance i
	
	// Code

	calculate_pressures(v, circ_pressure);
	
	// Calculate the other pressures
	for (int i = 1; i < circ_no_of_compartments; i++)
	{
		circ_pressure[i] = (v[i] - circ_slack_volume[i]) / circ_compliance[i];
	}

	// Calculate the flows
	for (int i = 1; i < circ_no_of_compartments; i++)
	{
		circ_flow[i] = (1.0 / circ_resistance[i]) *
			(circ_pressure[i-1] - circ_pressure[i]);
	}

	// Special case for flow into the ventricle
	if (circ_pressure[circ_no_of_compartments - 1] > circ_pressure[0])
	{
		circ_flow[0] = (1.0 / circ_resistance[0]) *
			(circ_pressure[circ_no_of_compartments - 1] - circ_pressure[0]);
	}
	else
	{
		circ_flow[0] = 0.0;
	}

	// Special case for flow out of ventricle
	if (circ_pressure[0] < circ_pressure[1])
	{
		circ_flow[1] = 0.0;
	}

	/*
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		cout << "p[" << i << "]: " << p[i] << "\t";
	}
	cout << "\n";
	*/
}

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

#include "gsl_math.h"
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
	circ_inertance = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_pressure = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_volume = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_flow = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_last_pressure = (double*)malloc(circ_no_of_compartments * sizeof(double));
	circ_last_flow = (double*)malloc(circ_no_of_compartments * sizeof(double));

	// Initialise, noting total slack_volume as we go
	// Start with the compartments at slack volume
	circ_total_slack_volume = 0.0;

	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		circ_resistance[i] = p_cmv_model->circ_resistance[i];
		circ_compliance[i] = p_cmv_model->circ_compliance[i];
		circ_slack_volume[i] = p_cmv_model->circ_slack_volume[i];
		circ_inertance[i] = p_cmv_model->circ_inertance[i];
		circ_pressure[i] = 0.0;
		circ_volume[i] = circ_slack_volume[i];
		if (i == 1)
			circ_volume[i] = 1.3 * circ_slack_volume[i];
		circ_flow[i] = 0.0;
		circ_last_pressure[i] = 0.0;
		circ_last_flow[i] = 0.0;

		circ_total_slack_volume = circ_total_slack_volume +
			circ_volume[i];
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

	// Set the aortic valve
	circ_aortic_valve_status = 0.0;
	circ_last_aortic_valve_status = 0.0;
	circ_pd = 0.0;

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
	free(circ_inertance);
	free(circ_pressure);
	free(circ_volume);
	free(circ_flow);
	free(circ_last_pressure);
	free(circ_last_flow);
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

	p_cmv_results->add_results_field("circ_aortic_valve", &circ_aortic_valve_status);
	p_cmv_results->add_results_field("circ_pd", &circ_pd);
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

	//cout << "t: " << t << "    t*f[0]: " << (t*f[0]) << "    v[0]: " << p_circ->circ_volume[0] << "    ratio: " << (t*f[0] / p_circ->circ_volume[0]) << "\n";
	

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

	double* vol_calc = (double*)malloc(circ_no_of_compartments * sizeof(double));
	double* flow_calc = (double*)malloc(circ_no_of_compartments * sizeof(double));

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

	// Set the volumes for the calculation
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		vol_calc[i] = circ_volume[i];
	}

	status = gsl_odeiv2_driver_apply(d, &t_start_s, t_stop_s, vol_calc);

	if (status != GSL_SUCCESS)
	{
		cout << "circulation::implement_time_step problem with ODEs\n";
		exit(1);
	}

	// Unpack
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		circ_volume[i] = vol_calc[i];
	}
	
	// Update the hemi_vent with the new volume
	p_hemi_vent->update_chamber_volume(circ_volume[0]);

	/*
	double b = 0.1;
	if (p_parent_cmv_system->cum_time_s > 1.0)
	{
		double dt = p_parent_cmv_system->cum_time_s - 1.0;
		b = b - (0.3 * dt);
	}
	circ_volume[0] = b;
	p_hemi_vent->update_chamber_volume(circ_volume[0]);

	*/

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
/*
	// Hold the last flow and the last pressure
	for (int i = 0; i < circ_no_of_compartments; i++)
	{
		circ_last_flow[i] = circ_flow[i];
		circ_last_pressure[i] = circ_pressure[i];
	}

	circ_last_aortic_valve_status = circ_aortic_valve_status;

	circ_pressure[0] = p_hemi_vent->return_pressure_for_chamber_volume(circ_volume[0]);
*/
	// Tidy up
	free(vol_calc);
	free(flow_calc);
}

void circulation::calculate_pressures(const double v[], double p[])
{
	//! Function calculates pressures
	
	// Code
	
	p[0] = p_hemi_vent->return_pressure_for_chamber_volume(v[0]);

	// Section is for debugging
	//cout << "Done\n";
	if (p[0] == 0.0)
	{
		for (int i = 0; i < circ_no_of_compartments; i++)
		{
			cout << "v[" << i << "]: " << v[i] << "\t";
			if (i == (circ_no_of_compartments - 1))
				cout << "\n";
		}
		//exit(1);
	}

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

	// Variables
	double p_diff;
	
	// Code

	calculate_pressures(v, circ_pressure);

	// Calculate the flows
	// These are flows from the aorta through to the veins
	for (int i = 2; i < circ_no_of_compartments; i++)
	{
		p_diff = (circ_pressure[i - 1] - circ_pressure[i]);
		circ_flow[i] = p_diff / circ_resistance[i];
	}

	// Special case for flow into the ventricle
	if (circ_pressure[circ_no_of_compartments - 1] > circ_pressure[0])
	{
		
		circ_flow[0] = (circ_pressure[circ_no_of_compartments - 1] - circ_pressure[0]) /
			circ_resistance[0];
	}
	else
	{
		circ_flow[0] = 0.0;
	}

	// Special case for flow out of ventricle
	p_diff = circ_pressure[0] - circ_pressure[1];

	if (p_diff > 0)
		circ_flow[1] = p_diff / circ_resistance[1];
	else
		circ_flow[1] = 0.0;

	circ_pd = p_diff;
	/*
	double k = 10;
	double k2 = 10;

	double time_step = 0.001;

	circ_aortic_valve_status = circ_last_aortic_valve_status +
		time_step * (
			(GSL_MAX(0.0, p_diff) * k * (1.0 - circ_last_aortic_valve_status)) -
		(k2 * circ_last_aortic_valve_status));
	
	//cout << "cum_time_s: " << p_parent_cmv_system->cum_time_s << "   p_diff: " << p_diff << "\n";


	circ_aortic_valve_status = GSL_MIN(circ_aortic_valve_status, 1.0);
	circ_aortic_valve_status = GSL_MAX(circ_aortic_valve_status, 0.0);

	//cout << "aortic_valve: " << circ_aortic_valve_status << "\n";

	if (circ_aortic_valve_status > 0.0)
	{
		circ_flow[1] = p_diff * circ_aortic_valve_status / circ_resistance[1];
	}
	else
	{
		circ_flow[1] = 0.0;
	}

	*/
}

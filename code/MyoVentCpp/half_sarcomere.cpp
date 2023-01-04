/**
/* @file		half_sarcomere.cpp
/* @brief		Source file for a half_sarcomere object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "cmv_model.h"
#include "half_sarcomere.h"
#include "hemi_vent.h"
#include "cmv_results.h"
#include "membranes.h"
#include "myofilaments.h"
#include "heart_rate.h"

#include "gsl_errno.h"
#include "gsl_math.h"
#include "gsl_roots.h"

// Constructor
half_sarcomere::half_sarcomere(hemi_vent* set_p_parent_hemi_vent)
{
	//! Constructor

	// Code
	printf("half_sarcomere constructor()\n");

	// Set the pointers to the parent system
	p_parent_hemi_vent = set_p_parent_hemi_vent;
	p_cmv_model = p_parent_hemi_vent->p_cmv_model;

	// Create the daugher objects
	p_heart_rate = new heart_rate(this);
	p_membranes = new membranes(this);
	p_myofilaments = new myofilaments(this);

	// Set safe values
	p_cmv_options = NULL;
	p_cmv_results = NULL;

	// Initialise
	hs_stress = 0.0;
	hs_length = p_cmv_model->hs_reference_hs_length;
}

// Destructor
half_sarcomere::~half_sarcomere(void)
{
	//! Destructor

	// Code
	printf("half_saromere destructor()\n");

	// Tidy up
	delete p_heart_rate;
	delete p_membranes;
	delete p_myofilaments;
}

// Other functions
void half_sarcomere::initialise_simulation(void)
{
	//! Code initialises simulation
	
	// Variables
	double slack_hs_length;
	
	// Code

	// Set options from parent
	p_cmv_options = p_parent_hemi_vent->p_cmv_options;

	// Set results from parent
	p_cmv_results = p_parent_hemi_vent->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("hs_length", &hs_length);
	p_cmv_results->add_results_field("hs_stress", &hs_stress);

	// Now initialise daughter objects
	p_heart_rate->initialise_simulation();
	p_membranes->initialise_simulation();
	p_myofilaments->initialise_simulation();

	// Set the hs_length so that wall stress is zero
	slack_hs_length = return_hs_length_for_stress(0.0);

	change_hs_length(slack_hs_length - hs_length);

	// Now calculate the wall stress
	p_myofilaments->calculate_stresses();

	hs_stress = p_myofilaments->myof_stress_total;
}

void half_sarcomere::implement_time_step(double time_step_s)
{
	//! Implements time-step

	// Variables
	bool new_beat;

	// Code

	new_beat = p_heart_rate->implement_time_step(time_step_s);

	p_membranes->implement_time_step(time_step_s, new_beat);

	p_myofilaments->implement_time_step(time_step_s);
}

void half_sarcomere::change_hs_length(double delta_hsl)
{
	//! Changes half-sarcomere length by delta_hsl

	// Code

	// Adjust this half-sarcomere
	hs_length = hs_length + delta_hsl;

	// And the cross-bridges
	p_myofilaments->move_cb_populations(delta_hsl);

	// Now update wall stress
	p_myofilaments->calculate_stresses();

	hs_stress = p_myofilaments->myof_stress_total;
}

double half_sarcomere::return_wall_stress_after_delta_hsl(double delta_hsl)
{
	//! Function returns wall stress after given delta_hsl
	
	// Variables
	double wall_stress;

	// Code
	wall_stress = p_myofilaments->return_stress_after_delta_hsl(delta_hsl);

	return wall_stress;
}

struct gsl_root_params
{
	half_sarcomere* p_hs_temp;
	double stress_target;
};

double hsl_stress_root_finder(double x, void* params)
{
	//! Function for the root finder
	
	// Variables
	double new_stress;
	double stress_difference;

	// Code
	
	// Unpack the pointer
	struct gsl_root_params* p = (struct gsl_root_params*)params;

	// Calculate the new stress after a length change
	new_stress = p->p_hs_temp->return_wall_stress_after_delta_hsl(x);

	// Calculate the difference between the new stress and the target
	stress_difference = new_stress - p->stress_target;

	// Return the difference
	return stress_difference;
}

double half_sarcomere::return_hs_length_for_stress(double target_stress)
{
	//! Code returns the hs_length at which force is equal to the target

	// Variables
	double x_lo = -500;
	double x_hi = 500;
	double r;

	const gsl_root_fsolver_type* T;
	gsl_root_fsolver* s;

	gsl_function F;
	struct gsl_root_params params = { this, target_stress };

	int status;
	int iter = 0;
	int max_iter = 100;

	double epsabs = 0.01;
	double epsrel = 0.01;

	// Code

	F.function = &hsl_stress_root_finder;
	F.params = &params;

	T = gsl_root_fsolver_brent;
	s = gsl_root_fsolver_alloc(T);
	gsl_root_fsolver_set(s, &F, x_lo, x_hi);

	do
	{
		iter++;
		status = gsl_root_fsolver_iterate(s);

		r = gsl_root_fsolver_root(s);
		x_lo = gsl_root_fsolver_x_lower(s);
		x_hi = gsl_root_fsolver_x_upper(s);
		status = gsl_root_test_interval(x_lo, x_hi, epsabs, epsrel);

	} while ((status == GSL_CONTINUE) && (iter < max_iter));

	gsl_root_fsolver_free(s);

	// Calculate the length

	return (hs_length + r);
}

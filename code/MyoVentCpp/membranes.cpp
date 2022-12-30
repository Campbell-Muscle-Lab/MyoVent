/**
/* @file		membranes.cpp
/* @brief		Source file for a membranes object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "membranes.h"
#include "half_sarcomere.h"
#include "cmv_results.h"

#include "gsl_errno.h"
#include "gsl_odeiv2.h"

// Constructor
membranes::membranes(half_sarcomere* set_p_parent_hs)
{
	//! Constructor

	// Code
	printf("membranes constructor()\n");

	// Set the pointer to the parent system
	p_parent_hs = set_p_parent_hs;

	// Initialize
	memb_Ca_cytosol = 0.0;
	memb_Ca_sr = 0.001;
	memb_activation = 0.0;
	memb_t_active_s = 0.03;
	memb_t_active_left_s = 0.0;
	memb_k_serca = 1.0;
	memb_k_leak = 0.001;
	memb_k_active = 0.5;	
}

// Destructor
membranes::~membranes(void)
{
	//! Destructor

	// Code
	printf("membranes destructor()\n");
}

// Other functions
void membranes::prepare_for_cmv_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Set the pointer to the results object
	p_cmv_results = p_parent_hs->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("memb_Ca_cytosol", &memb_Ca_cytosol);
	p_cmv_results->add_results_field("memb_Ca_sr", &memb_Ca_sr);
	p_cmv_results->add_results_field("memb_activation", &memb_activation);
	p_cmv_results->add_results_field("memb_t_active_left_s", &memb_t_active_left_s);
	p_cmv_results->add_results_field("memb_k_serca", &memb_k_serca);
	p_cmv_results->add_results_field("memb_k_leak", &memb_k_leak);
	p_cmv_results->add_results_field("memb_k_active", &memb_k_active);


	std::cout << "finished prepare for membrane results\n";
}

void membranes::implement_time_step(double time_step_s, bool new_beat)
{
	//! Function updates membrane object by a time-step

	// Code
	if (new_beat)
	{
		memb_t_active_left_s = memb_t_active_s;
	}
	else
	{
		memb_t_active_left_s = memb_t_active_left_s - time_step_s;
	}

	if (memb_t_active_left_s > 0.0)
	{
		memb_activation = 1.0;
	}
	else
	{
		memb_activation = 0.0;
	}
}

int derivs(double t, const double y[], double f[], void* params)
{
	//! Function - returns derivs
	
	// Variables
	membranes* p_memb = params;

	(void)(t);






}
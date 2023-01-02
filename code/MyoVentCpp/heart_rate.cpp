/**
/* @file		heart_rate.cpp
/* @brief		Source file for a heart_rate object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "heart_rate.h"
#include "cmv_system.h"
#include "cmv_results.h"
#include "cmv_model.h"
#include "half_sarcomere.h"

// Constructor
heart_rate::heart_rate(half_sarcomere* set_p_parent_hs)
{
	// Initialise

	// Code
	std::cout << "heart_rate_constructor()\n";

	// Set pointers
	p_parent_hs = set_p_parent_hs;
	p_cmv_model = p_parent_hs->p_cmv_model;

	// Initialize variables
	hr_new_beat = 0.0;
	hr_t_RR_interval_s = p_cmv_model->hr_t_RR_interval_s;
	hr_t_countdown_s = hr_t_RR_interval_s;
}

// Destructor
heart_rate::~heart_rate(void)
{
	//! Heart_rate destructor
	std::cout << "heart_rate_destructor\n";
}

// Other functions
void heart_rate::initialise_simulation(void)
{
	// Set the pointer to the results object
	p_cmv_results = p_parent_hs->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("hr_new_beat", &hr_new_beat);

	std::cout << "heart_rate:: finished initialise simulation\n";

}

bool heart_rate::implement_time_step(double time_step)
{
	//! Function manages the hr_activation variable which determines the
	//	initiation of a heart beat

	// Variables
	bool new_beat;

	// Code

	// Reduce counters
	hr_t_countdown_s = hr_t_countdown_s - time_step;

	// Reset
	if (hr_t_countdown_s <= 0.0)
	{
		hr_t_countdown_s = hr_t_RR_interval_s;
		hr_new_beat = 1.0;
		new_beat = true;
	}
	else
	{
		hr_new_beat = 0.0;
		new_beat = false;
	}

	// Return
	return(new_beat);
}

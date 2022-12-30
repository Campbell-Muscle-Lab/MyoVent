/**
/* @file		half_sarcomere.cpp
/* @brief		Source file for a half_sarcomere object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "half_sarcomere.h"
#include "hemi_vent.h"
#include "cmv_results.h"
#include "membranes.h"
#include "heart_rate.h"

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
	p_membranes = new membranes(this);
	p_heart_rate = new heart_rate(this);

	// Initialise
	hs_force = 0.0;
	hs_length = 0.0;
}

// Destructor
half_sarcomere::~half_sarcomere(void)
{
	//! Destructor

	// Code
	printf("half_saromere destructor()\n");

	// Tidy up
	delete p_membranes;
	delete p_heart_rate;
}

// Other functions
void half_sarcomere::prepare_for_cmv_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Set the pointer to the results object
	p_cmv_results = p_parent_hemi_vent->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("hs_length", &hs_length);
	p_cmv_results->add_results_field("hs_force", &hs_force);

	// Now handle children
	p_membranes->prepare_for_cmv_results();
	p_heart_rate->prepare_for_cmv_results();
}

void half_sarcomere::implement_time_step(double time_step_s)
{
	//! Implements time-step

	// Variables
	bool new_beat;

	// Code

	new_beat = p_heart_rate->implement_time_step(time_step_s);

	p_membranes->implement_time_step(time_step_s, new_beat);
	
	
}

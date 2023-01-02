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
circulation::circulation(cmv_system* set_p_parent_cmv_system=NULL)
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
}


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

// Constructor
half_sarcomere::half_sarcomere(hemi_vent* set_p_parent_hemi_vent)
{
	//! Constructor

	// Code
	printf("half_sarcomere constructor()\n");

	// Set the pointer to the parent system
	p_parent_hemi_vent = set_p_parent_hemi_vent;

	// Create the daugher objects
	p_membranes = new membranes(this);

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

	// Now handle child
	p_membranes->prepare_for_cmv_results();
}

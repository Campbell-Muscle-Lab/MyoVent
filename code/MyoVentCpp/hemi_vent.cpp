/**
/* @file		hemi_vent.cpp
/* @brief		Source file for a hemi_vent object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "hemi_vent.h"
#include "cmv_system.h"
#include "half_sarcomere.h"
#include "cmv_results.h"


// Constructor
hemi_vent::hemi_vent(cmv_system* set_p_parent_cmv_system)
{
	// Initialise

	// Code
	std::cout << "hemi_vent_constructor()\n";

	// Set pointers
	p_parent_cmv_system = set_p_parent_cmv_system;
	p_cmv_results = p_parent_cmv_system->p_cmv_results;

	// Initialize variables
	pressure_ventricle = 0.0;

	// Initialise child half-sarcomere
	p_hs = new half_sarcomere(this);

}

// Destructor
hemi_vent::~hemi_vent(void)
{
	//! hemi_vent destructor
	std::cout << "hemi_vent_destructor\n";

	// Tidy up
	delete p_hs;
}

// Other functions
void hemi_vent::prepare_for_cmv_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Set the pointer to the results object
	p_cmv_results = p_parent_cmv_system->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("pressure_ventricle", &pressure_ventricle);

	// Now add in the children
	p_hs->prepare_for_cmv_results();

	std::cout << "finished prepare for hemi_vent results\n";
}


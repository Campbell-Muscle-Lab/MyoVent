/**
/* @file		circulation.cpp
/* @brief		Source file for a circulation object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "circulation.h"
#include "cmv_system.h"
#include "cmv_results.h"

// Constructor
circulation::circulation(cmv_system* set_p_parent_cmv_system)
{
	//! Constructor

	// Code
	printf("circulation constructor()\n");

	// Initialise
	p_ventricle = 1.0;
	p_aorta = 10.0;

	// Set the pointer to the parent system
	p_parent_cmv_system = set_p_parent_cmv_system;
}

// Destructor
circulation::~circulation(void)
{
	//! Destructor

	// Code
	printf("circulation destructor()\n");
}

// Other functions
void circulation::initialise_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Set the pointer to the results object
	p_cmv_results = p_parent_cmv_system->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("p_ventricle", &p_ventricle);
	p_cmv_results->add_results_field("p_aorta", &p_aorta);
}


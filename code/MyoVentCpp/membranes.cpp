/**
/* @file		membranes.cpp
/* @brief		Source file for a membranes object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "membranes.h"
#include "half_sarcomere.h"
#include "cmv_results.h"

// Constructor
membranes::membranes(half_sarcomere* set_p_parent_hs)
{
	//! Constructor

	// Code
	printf("membranes constructor()\n");

	// Set the pointer to the parent system
	p_parent_hs = set_p_parent_hs;

	// Initialize
	Ca_myofil_conc = 0.0;
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
	p_cmv_results->add_results_field("Ca_myofil_conc", &Ca_myofil_conc);

	std::cout << "finished prepare for membrane results\n";
}

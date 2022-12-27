/**
/* @file		cmv_system.cpp
/* @brief		Source file for a cmv_system object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "cmv_system.h"
#include "circulation.h"
#include "hemi_vent.h"
#include "half_sarcomere.h"
#include "membranes.h"
#include "cmv_results.h"


// Constructor
cmv_system::cmv_system(void)
{
	// Initialise

	// Code
	printf("cmv_constructor()\n");

	// Create constituent objects
	p_circulation = new circulation(this);
	p_hemi_vent = new hemi_vent(this);

	// Run simulation
	run_simulation();

}

// Destructor
cmv_system::~cmv_system(void)
{
	// Initialise

	// Code
	printf("cmv_system desctructor()\n");

	// Tidy up
	delete p_circulation;
	delete p_hemi_vent;
}

void cmv_system::run_simulation(void)
{
	//! Code runs a simulation

	int no_of_time_points = 10;

	// Initialise the results object
	p_cmv_results = new cmv_results(no_of_time_points);

	// Add in the results
	p_circulation->prepare_for_cmv_results();
	p_hemi_vent->prepare_for_cmv_results();

	// Simulation
	for (int i = 0; i < no_of_time_points; i++)
	{
		p_circulation->pressure_aorta = p_circulation->pressure_aorta + 1.0;
		p_hemi_vent->p_hs->hs_length = p_hemi_vent->p_hs->hs_length + 0.1;
		p_hemi_vent->p_hs->p_membranes->Ca_myofil_conc =
			p_hemi_vent->p_hs->p_membranes->Ca_myofil_conc - 0.2;

		p_cmv_results->update_results_vectors(i);
	}

	// Now save data to file
	p_cmv_results->write_data_to_file("c:/temp/ken.txt");

	// Tidying up
	delete p_cmv_results;
}
/**
/* @file		cmv_system.cpp
/* @brief		Source file for a cmv_system object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "cmv_system.h"
#include "circulation.h"
#include "cmv_system.h"
#include "cmv_results.h"


// Constructor
cmv_system::cmv_system(void)
{
	// Initialise

	// Code
	printf("cmv_constructor()\n");

	// Create constituent objects
	p_circulation = new circulation(this);

	// Run simulation
	run_simulation();

}

// Destructor
cmv_system::~cmv_system(void)
{
	// Initialise

	// Code
	printf("cmv_constructor descructor()\n");

	// Tidy up
	delete p_circulation;
}

void cmv_system::run_simulation(void)
{
	//! Code runs a simulation

	int no_of_time_points = 10;

	// Initialise the results object
	p_cmv_results = new cmv_results(no_of_time_points);

	// Add in the results
	p_circulation->initialise_results();

	// Simulation
	for (int i = 0; i < no_of_time_points; i++)
	{
		p_circulation->p_aorta = p_circulation->p_aorta + 1.0;
		p_circulation->p_ventricle = p_circulation->p_ventricle - 0.5;

		p_cmv_results->update_results_vectors(i);
	}

	// Now save data to file
	p_cmv_results->write_data_to_file("c:/temp/ken.txt");

	// Tidying up
	delete p_cmv_results;
}
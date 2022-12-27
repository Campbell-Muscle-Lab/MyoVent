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

	// Initialise variables
	cum_time_s = 0.0;

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

	int no_of_time_points = 500;
	double time_step_s = 0.01;

	// Initialise the results object
	p_cmv_results = new cmv_results(no_of_time_points);

	// Add in the results
	this->prepare_for_cmv_results();
	p_circulation->prepare_for_cmv_results();
	p_hemi_vent->prepare_for_cmv_results();

	// Simulation
	for (int i = 0; i < no_of_time_points; i++)
	{
		/*p_circulation->pressure_aorta = p_circulation->pressure_aorta + 1.0;
		p_hemi_vent->p_hs->hs_length = p_hemi_vent->p_hs->hs_length + 0.1;
		p_hemi_vent->p_hs->p_membranes->Ca_myofil_conc =
			p_hemi_vent->p_hs->p_membranes->Ca_myofil_conc - 0.2;
			*/

		implement_time_step(time_step_s);


		p_cmv_results->update_results_vectors(i);
	}

	// Now save data to file
	p_cmv_results->write_data_to_file("c:/temp/ken.txt");

	// Tidying up
	delete p_cmv_results;
}

void cmv_system::prepare_for_cmv_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Now add the results fields
	p_cmv_results->add_results_field("Time_s", &cum_time_s);
}

void cmv_system::implement_time_step(double time_step_s)
{
	// Update system time
	cum_time_s = cum_time_s + time_step_s;

	p_hemi_vent->implement_time_step(time_step_s);
}
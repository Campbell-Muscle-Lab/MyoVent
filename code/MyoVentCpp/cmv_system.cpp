/**
/* @file		cmv_system.cpp
/* @brief		Source file for a cmv_system object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>
#include <iomanip>

#include "cmv_system.h"
#include "circulation.h"
#include "hemi_vent.h"
#include "half_sarcomere.h"
#include "membranes.h"
#include "myofilaments.h"
#include "kinetic_scheme.h"
#include "cmv_options.h"
#include "cmv_results.h"
#include "cmv_protocol.h"
#include "cmv_model.h"

using namespace std;

// Constructor
cmv_system::cmv_system(string JSON_model_file_string)
{
	// Initialise

	// Code
	printf("cmv_constructor()\n");

	// Code creates a cmv_model object
	p_cmv_model = new cmv_model(JSON_model_file_string);

	// Sets other pointers to safety
	p_cmv_options = NULL;
	p_cmv_protocol = NULL;
	p_cmv_results = NULL;

	// Initialise variables
	cum_time_s = 0.0;

	// Create constituent objects
	p_circulation = new circulation(this);
}

// Destructor
cmv_system::~cmv_system(void)
{
	// Initialise

	// Code
	printf("cmv_system destructor()\n");

	// Tidy up
	delete p_circulation;
	delete p_cmv_model;
}

void cmv_system::run_simulation(string options_file_string,
									string protocol_file_string,
									string results_file_string)
{
	//! Code runs a simulation

	// Test
	
	// Initialises an options object
	p_cmv_options = new cmv_options(options_file_string);

	// Initialise the protocol object
	p_cmv_protocol = new cmv_protocol(protocol_file_string);

	// Initialise the results object
	p_cmv_results = new cmv_results(p_cmv_protocol->no_of_time_steps);

	// Add in the results
	add_fields_to_cmv_results();

	p_circulation->initialise_simulation();

	cout << "kenenn\n";

	// Simulation
	for (int i = 0; i < p_cmv_protocol->no_of_time_steps; i++)
	{
		implement_time_step(p_cmv_protocol->time_step_s);

		p_cmv_results->update_results_vectors(i);

		/*/(if (i > 5000)
		{
			double x = 0.2 * cos(3.14 * ((double)(i) / 1000.0));
			//cout << "x: " << x << "\n";
			p_circulation->p_hemi_vent->p_hs->change_hs_length(x);
		}*/

		// Update simulation
		if (fmod(cum_time_s, 1.0) < p_cmv_protocol->time_step_s)
		{
			cout << "Simulation time: " << cum_time_s << " s\n";
		}
	}

	// Now save data to file
	p_cmv_results->write_data_to_file(results_file_string);

	// Tidying up
	delete p_cmv_options;
	delete p_cmv_protocol;
	delete p_cmv_results;
}

void cmv_system::add_fields_to_cmv_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Now add the results fields
	p_cmv_results->add_results_field("time", &cum_time_s);
}

void cmv_system::implement_time_step(double time_step_s)
{
	// Update system time
	cum_time_s = cum_time_s + time_step_s;

	p_circulation->implement_time_step(time_step_s);
}
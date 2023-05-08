/**
/* @file		cmv_system.cpp
/* @brief		Source file for a cmv_system object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>
#include <iomanip>
#include <cmath>

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

struct stats_structure {
	double mean_value;
	double min_value;
	double max_value;
	double sum;
};

// Constructor
cmv_system::cmv_system(string JSON_model_file_string, int set_system_id)
{
	// Initialise

	// Code

	// Code creates a cmv_model object
	p_cmv_model = new cmv_model(JSON_model_file_string);

	system_id = set_system_id;

	// Sets other pointers to safety
	p_cmv_options = NULL;
	p_cmv_protocol = NULL;
	p_cmv_results_beat = NULL;
	p_cmv_results_summary = NULL;

	// Initialise variables
	cum_time_s = 0.0;

	sim_t_index = 0;
	beat_t_index = 0;
	summary_t_index = 0;

	// Create constituent objects
	p_circulation = new circulation(this);
}

// Destructor
cmv_system::~cmv_system(void)
{
	// Initialise

	// Code

	// Tidy up
	delete p_circulation;
	delete p_cmv_model;
}

void cmv_system::run_simulation(string options_file_string,
									string protocol_file_string,
									string results_file_string)
{
	//! Code runs a simulation

	// Variables
	bool new_beat = false;

	// Code
	
	// Initialises an options object
	p_cmv_options = new cmv_options(options_file_string);

	// Initialise the protocol object
	p_cmv_protocol = new cmv_protocol(this, protocol_file_string);

	// Initialise the cmv_results_beat object
	p_cmv_options->beat_length_points = int(p_cmv_options->beat_length_s /
		p_cmv_protocol->time_step_s);

	// Create the cmv_results_beat object
	p_cmv_results_beat = new cmv_results(this, p_cmv_options->beat_length_points);
	
	// Add in the results
	add_fields_to_cmv_results_beat();

	// Initialise the circulation and daughter objects
	// This adds data to the cmv_results_beat object
	p_circulation->initialise_simulation();

	// Now we have to prepare the cmv_results_summary object

	// First deduce how many points it needs
	p_cmv_options->summary_points = 0;
	double sim_t = 0.0;
	for (sim_t_index = 0; sim_t_index < p_cmv_protocol->no_of_time_steps; sim_t_index++)
	{
		if (abs(remainder(sim_t, p_cmv_options->summary_time_step_s)) < 1e-6)
		{
			p_cmv_options->summary_points = p_cmv_options->summary_points + 1;
		}

		sim_t = sim_t + p_cmv_protocol->time_step_s;
	}

	cout << "Summary points: " << p_cmv_options->summary_points << "\n";

	// Now create it
	p_cmv_results_summary = new cmv_results(this, p_cmv_options->summary_points);

	cout << "Made: " << p_cmv_results_summary->no_of_time_points << "\n";

	// Now make sure that the summary object has the same fields as the beat object
	clone_results_fields(p_cmv_results_beat, p_cmv_results_summary);

	// Simulation

	// Set counters
	beat_t_index = 0;
	summary_t_index = 0;

	for (sim_t_index = 0; sim_t_index < p_cmv_protocol->no_of_time_steps; sim_t_index++)
	{
		new_beat = implement_time_step(p_cmv_protocol->time_step_s);

		p_cmv_results_beat->update_results_vectors(beat_t_index);

		if (new_beat)
		{
			// Update beat metrics
			update_beat_metrics();

			// Update p_cmv_results_summary with beat data
			update_cmv_results_summary();

			// Update the counters
			beat_t_index = 0;
		}
		else
		{
			beat_t_index = beat_t_index + 1;
		}
	}

	// Now save data to file
	p_cmv_results_summary->write_data_to_file(results_file_string);

	// Tidying up
	delete p_cmv_options;
	delete p_cmv_protocol;
	delete p_cmv_results_beat;
	delete p_cmv_results_summary;
}

void cmv_system::clone_results_fields(cmv_results* p_source, cmv_results* p_clone)
{
	// Function ensures p_clone has same fields as p_source where
	// p_clone and p_source are both cmv_results objects

	// Variables

	// Code
	
	// Reset the clone
	p_clone->no_of_defined_results_fields = 0;

	for (int i = 0; i < p_source->no_of_defined_results_fields; i++)
	{
		p_clone->add_results_field(p_source->results_fields[i], NULL);
	}
}

void cmv_system::add_fields_to_cmv_results_beat(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Now add the results fields
	p_cmv_results_beat->add_results_field("time", &cum_time_s);
}

bool cmv_system::implement_time_step(double time_step_s)
{
	// Variable
	bool new_beat = false;

	// Update system time
	cum_time_s = cum_time_s + time_step_s;

	// Impose perturbations
	p_cmv_protocol->impose_perturbations(cum_time_s);

	new_beat = p_circulation->implement_time_step(time_step_s);

	return new_beat;
}

void cmv_system::update_beat_metrics(void)
{
	//! Updates beat metrics in daughter objects

	cout << "System [" << system_id << "], new beat at : " << cum_time_s << " s\n";
	p_circulation->update_beat_metrics();
}

void cmv_system::update_cmv_results_summary(void)
{
	//! Function copies data from cmv_results_beat to
	//! the appropriate row on cmv_results_summary
	
	// Variables
	double y;

	// Code
	
	// We have to run through the entire beat to capture the fields that
	// were back-filled to describe the cardiac cycle

	for (int b_ind = 0; b_ind < beat_t_index; b_ind++)
	{
		// Work out whether this is a time we need
		if (abs(remainder(gsl_vector_get(
			p_cmv_results_beat->gsl_results_vectors[p_cmv_results_beat->time_field_index], b_ind),
			p_cmv_options->summary_time_step_s)) < 1e-6)
		{
			for (int f_ind = 0; f_ind < p_cmv_results_beat->no_of_defined_results_fields; f_ind++)
			{
				double temp;

				// Pull data from results_beat
				temp = gsl_vector_get(p_cmv_results_beat->gsl_results_vectors[f_ind], b_ind);

				// Write data to results_summary
				gsl_vector_set(p_cmv_results_summary->gsl_results_vectors[f_ind],
					summary_t_index, temp);
			}

			summary_t_index = summary_t_index + 1;
		}
	}
}
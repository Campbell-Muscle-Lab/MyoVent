/**
/* @file		cmv_results.cpp
/* @brief		Source file for a cmv_results object
/* @author		Ken Campbell
*/

#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>

#include "cmv_results.h"

#include "gsl_vector.h"
#include "gsl_math.h"

using namespace std;
using namespace std::filesystem;

// Constructor
cmv_results::cmv_results(int set_no_of_time_points)
{
	// Initialise

	// Code
	printf("cmv_results constructor()\n");

	no_of_defined_results_fields = 0;

	no_of_time_points = set_no_of_time_points;

	no_of_beats = 0;
	last_beat_t_index = -1;
}

// Destructor
cmv_results::~cmv_results(void)
{
	// Code
	printf("cmv_results descructor()\n");

	// Delete all of the defined vectors
	for (int i = 0; i < no_of_defined_results_fields; i++)
	{
		gsl_vector_free(gsl_results_vectors[i]);
	}
}

// Other functions

void cmv_results::add_results_field(std::string field_name, double* p_double)
{
	//! Functions adds a field to the results object

	// Variables
	int new_index;			// index of new field

	// Code

	// Get the number of fields that have been defined already
	new_index = no_of_defined_results_fields;

	// Update the results_fields and the data source
	results_fields[new_index] = field_name;
	p_data_sources[new_index] = p_double;

	// Create a gsl_vector to hold the data and initialise to NaN
	gsl_results_vectors[new_index] = gsl_vector_alloc(no_of_time_points);
	gsl_vector_set_all(gsl_results_vectors[new_index], GSL_NAN);

	// Check for new_beat index
	if (field_name == "hr_new_beat")
	{

		new_beat_field_index = new_index;
	}

	// Update the number of defined fields
	no_of_defined_results_fields = no_of_defined_results_fields + 1;
}

void cmv_results::update_results_vectors(int t_index)
{
	// Cycle through the defined data fields

	for (int i = 0; i < no_of_defined_results_fields; i++)
	{
		gsl_vector_set(gsl_results_vectors[i], t_index, *p_data_sources[i]);
	}

	if (new_beat_field_index >= 0)
	{
		if (*p_data_sources[new_beat_field_index] == 1.0)
			calculate_beat_metrics(t_index);
	}
}

void cmv_results::calculate_beat_metrics(int t_index)
{
	//! Function calculates metrics for the beat
	
	// Variables

	// Code

	// Handle first beat
	if (last_beat_t_index < 0)
	{
		last_beat_t_index = t_index;
		return;
	}

	return_sub_vector_statistics(gsl_results_vectors[0], last_beat_t_index, t_index);

	// Prepare for next beat
	last_beat_t_index = t_index;
}

void cmv_results::return_sub_vector_statistics(gsl_vector* gsl_v, int start_index, int stop_index)
{
	//! Function calculates stats
	
	// Variables
	int index;
	double value;
	double holder = 0;
	double mean_value;
	double min_value = GSL_POSINF;
	double max_value = -GSL_POSINF;

	// Code
	for (index = start_index; index <= stop_index; index++)
	{
		value = gsl_vector_get(gsl_v, index);
		holder = holder + value;
		min_value = GSL_MIN(value, min_value);
		max_value = GSL_MAX(value, max_value);
	}
	mean_value = holder / (double)(stop_index - start_index + 1);

	cout << "mean: " << mean_value << "\n";
	cout << "min: " << min_value << "\n";
	cout << "max: " << max_value << "\n";
}

int cmv_results::write_data_to_file(std::string output_file_string)
{
	//! Function writes data to file

	// Variables
	FILE* output_file;

	// Code
	cout << "Writing simulation results to: " << output_file_string << "\n";

	// Make sure results directory exists
	path output_file_path(output_file_string);

	if (!(is_directory(output_file_path.parent_path())))
	{
		if (create_directories(output_file_path.parent_path()))
		{
			cout << "\nCreating folder: " << output_file_path.string() << "\n";
		}
		else
		{
			cout << "\nError: Results folder could not be created: " <<
				output_file_path.parent_path().string() << "\n";
			exit(1);
		}
	}

	// Check file can be opened, abort if not
	errno_t err = fopen_s(&output_file, output_file_string.c_str(), "w");
	if (err != 0)
	{
		cout << "Results file: " << output_file_string << " could not be opened\n";
		exit(1);
	}

	// Write header
	for (int i = 0; i < no_of_defined_results_fields; i++)
	{
		fprintf_s(output_file, "%s", results_fields[i].c_str());
		if (i == (no_of_defined_results_fields - 1))
			fprintf_s(output_file, "\n");
		else
			fprintf_s(output_file, "\t");
	}

	// Now data
	for (int i = 0; i < no_of_time_points; i++)
	{
		for (int j = 0; j < no_of_defined_results_fields; j++)
		{
			fprintf_s(output_file, "%g", gsl_vector_get(gsl_results_vectors[j], i));
			if (j == (no_of_defined_results_fields - 1))
				fprintf_s(output_file, "\n");
			else
				fprintf_s(output_file, "\t");
		}
	}

	cout << "Closing output_file: " << output_file_string << "\n";

	// Tidy up
	fclose(output_file);

	return(1);
}
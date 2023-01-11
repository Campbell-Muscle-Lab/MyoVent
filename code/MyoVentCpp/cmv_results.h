#pragma once

/**
/* @file		cmv_results.h
/* @brief		Header file for a cmv_results object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include <iostream>
#include <string>

#include "global_definitions.h"

#include "gsl_vector.h"

using namespace std;

class cmv_results
{
public:
	/**
	 * Constructor
	 */
	cmv_results(int set_no_of_time_points);

	/**
	* Destructor
	*/
	~cmv_results(void);

	// Variables
	std::string results_fields[MAX_NO_OF_RESULT_FIELDS];
											/**< array of strings defining the
													data fields in a results object */

	gsl_vector* gsl_results_vectors[MAX_NO_OF_RESULT_FIELDS];
											/**< array of gsl_vectors holding data */

	int no_of_defined_results_fields;		/**< integer defining the number of
													results fields that have been defined */

	double* p_data_sources[MAX_NO_OF_RESULT_FIELDS];
											/**< array of pointers to doubles  holding
													the sources of the data */

	int no_of_time_points;					/**< integer defining the number of
													time-points in a result file */

	int no_of_beats;						/**< integer counting number of beats
													written to record */

	int last_beat_t_index;					/**< integer holding the t_index of the
													last new beat */

	int new_beat_field_index;				/**< integer holding the index for the
													new_beat field */

	// Functions

	void add_results_field(std::string field_name, double* p_double);
											/**< function adds a double to the results
													object */

	void update_results_vectors(int t_index);

	int write_data_to_file(string output_file_string);
											/**< write data to file */

	void calculate_beat_metrics(int t_beat_index);

	void return_sub_vector_statistics(gsl_vector* gsl_v, int start_index, int stop_index);

};
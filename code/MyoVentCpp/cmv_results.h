#pragma once

/**
/* @file		cmv_results.h
/* @brief		Header file for a cmv_results object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include <iostream>
#include <cstring>

#include "global_definitions.h"

#include "gsl_vector.h"

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

	// Functions

	void add_results_field(std::string field_name, double* p_double);
											/**< function adds a double to the results
													object */

	void update_results_vectors(int t_index);

	int write_data_to_file(std::string data_file_string);
											/**< write data to file */

};
#pragma once

/**
/* @file		myofilaments.h
/* @brief		Header file for a myofilaments object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include <iostream>

#include "gsl_vector.h"
#include "gsl_matrix.h"

#include "global_definitions.h"

// Forward declararations
class half_sarcomere;
class cmv_model;
class cmv_options;
class cmv_results;

class kinetic_scheme;

class myofilaments
{
public:
	/**
	 * Constructor
	 */
	myofilaments(half_sarcomere* set_p_parent_hs);

	/**
	* Destructor
	*/
	~myofilaments(void);

	// Variables
	half_sarcomere* p_parent_hs;			/**< Pointer to parent half-sarcomere */

	cmv_model* p_cmv_model;					/**< Pointer to cmv_model */

	cmv_options* p_cmv_options;				/**< Pointer to cmv_options */

	cmv_results* p_cmv_results;				/**< Pointer to cmv_results */

	kinetic_scheme* p_m_scheme;				/**< Pointer to a kinetic scheme */

	double myof_cb_number_density;
	double myof_prop_fibrosis;
	double myof_prop_myofilaments;
	double myof_k_cb;

	double myof_a_k_on;						/**< double with a_k_on, rate constant
													for thin filament activation
													M^-1 s^-1 */

	double myof_a_k_off;					/**< double with a_k_off, rate constant
													for thin filament deactivation
													s^-1 */

	double myof_a_k_coop;					/**< double for thin filament
													cooperativity, dimensionless */

	int no_of_bin_positions;				/**< integer with the number of
													positions cross-bridge
													distributions are evaluated
													at */

	gsl_vector* x;							/**< gsl_vector with bin positions */

	int y_length;							/**< integer with the length of the
													system */

	int m_length;							/**< integer with the length of the
													myosin system */

	gsl_vector* y;							/**< gsl_vector with the system */

	int a_off_index;						// indices
	int a_on_index;

	gsl_matrix_int* m_y_indices;			/**< gsl_matrix_int holding the start and
													stop indices in y for
													myosin state */

	gsl_vector* m_state_pops;				/**< gsl_vector holding m_state populations */

	double* m_pop_array;					/**< array of doubles holding state populations
													used to communicate with gsl_results */

	double myof_a_off;						/**< double with proportion of off thin sites */

	double myof_a_on;						/**< double with proportion of on thin sites */

	double myof_m_bound;					/**< double with proportion of myosins
													that are bound */

	double myof_f_overlap;					/**< double defining the proportion
													of filaments that are in overlap */



	// Functions

	void prepare_for_cmv_results(void);

	void update_p_cmv_options(cmv_options* set_p_cmv_options);

	void initialise_simulation(void);

	void implement_time_step(double time_step_s);

	void calculate_m_state_pops(const double y[]);

	void calculate_f_overlap(void);
};

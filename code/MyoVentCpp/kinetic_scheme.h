#pragma once

/**
* @file		kinetic_scheme.h
* @brief	header file for the kinetic_scheme class
* @author	Ken Campbell
*/

#include <iostream>

#include "rapidjson/document.h"
#include "JSON_functions.h"

#include "gsl_vector.h"

#include "global_definitions.h"

// Forward declarations
class cmv_model;
class cmv_options;
class m_state;
class myofilaments;

using namespace std;

class kinetic_scheme
{
public:

	// Variables

	myofilaments* p_parent_myofil;			/**< pointer to parent myofilament object */

	cmv_model* p_cmv_model;					/**< pointer to a cmv_model object */

	cmv_options* p_cmv_options;				/**< pointer to a cmv_options objects */

	int no_of_states;						/**< int defining the number of different states */

	int max_no_of_transitions;				/**< int defining the maximum number of transitions from a state */

	m_state* p_m_states[MAX_NO_OF_KINETIC_STATES];
											/**< pointer to an array of m_state objects */

	// Functions

	/**
	* Constructor
	* takes a cmv_model and parses it to give the kinetic scheme
	*/
	kinetic_scheme(const rapidjson::Value& m_ks, myofilaments* set_p_parent_myofil);

	/**
	* Destructor
	*/
	~kinetic_scheme(void);

	/**
	* void set_transition_types(void)
	* loops through transitions from each state and sets the type, 'a' for attach,
	* 'd' for detach, and 'n' for neither
	* needs to be run after all the states are known so can't be included in the
	* transition constructor as these are built in sequence with each state
	* @return void
	*/
	void set_transition_types(void);

	/**
	* void write_kinetic_scheme_to_file(char output_file_string)
	* writes kinetic_scheme to specified file in JSON format
	* @return void
	*/
	void write_kinetic_scheme_to_file(char output_file_string[]);

	/**
	* void write_myosin_rate_functions_to_file(char output_file_string)
	* as a tab-delimited file
	* @return void
	*/
	void write_rate_functions_to_file(string output_file_string, char file_write_mode[],
										string JSON_append_string);

};
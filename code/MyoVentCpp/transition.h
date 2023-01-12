#pragma once

/**
* @file		transition.h
* @brief	Header file for the transition class
* @author	Ken Campbell
*/

#include "half_sarcomere.h"
#include "m_state.h"
#include "rapidjson/document.h"
#include "JSON_functions.h"

#include "gsl_vector.h"
#include "gsl_math.h"

// Forward declaration
class half_sarcomere;
class cmv_model;
class cmv_options;

class transition
{
public:

	// Variables

	m_state* p_parent_m_state;		/**< pointer to parent m_state */

	cmv_model* p_cmv_model;			/**< pointer to parent model */

	cmv_options* p_cmv_options;		/**< pointer to cmv_options */

	int new_state;					/**< integer defining the new state */

	char rate_type[_MAX_PATH];		/**< char array defining the transition type */

	char transition_type;			/**< char defining 'a' attachment, 'd' detachment, 'n' neutral */

	char ATP_required;				/**< char with 'y' if ATP is required for the transition */

	gsl_vector* rate_parameters;	/**< gsl_vector holding parameter variables */

	// Functions

	// Constructor

	/**
	* Normal constructor called with an entry into a JSON document
	*/
	transition(const rapidjson::Value& m_ks, m_state* set_p_m_state);

	/**
	* Constructor that sets default values
	* Called when the transition is empty
	*/
	transition();

	/**
	* Destructor
	*/
	~transition(void);

	// Functions

	/**
	* double calculate_rate(double x)
	* @param x double defining the cb_x position - the bs_x position
	* @return the rate in units of s^-1
	*/
	/*
	double calculate_rate(double x, double x_ext, double node_force,
							int mybpc_state, int mybpc_iso,
							short int active_neigh = 0,
							half_sarcomere* p_hs = NULL);
	*/

	double calculate_rate(double, double, double force=0.0, double = GSL_NAN);
};

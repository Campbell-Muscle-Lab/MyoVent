#pragma once

/**
/* @file		myofilaments.h
/* @brief		Header file for a myofilaments object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include <iostream>

#include "global_definitions.h"

// Forward declararations
class half_sarcomere;
class cmv_model;
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

	cmv_results* p_cmv_results;				/**< Pointer to cmv_results */

	kinetic_scheme* p_m_scheme;				/**< Pointer to a kinetic scheme */

	double myof_cb_number_density;
	double myof_prop_fibrosis;
	double myof_prop_myofilaments;
	double myof_k_cb;

};

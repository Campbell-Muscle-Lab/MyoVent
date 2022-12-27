#pragma once

/**
/* @file		membranes.h
/* @brief		Header file for a membranes object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class half_sarcomere;
class cmv_results;

class membranes
{
public:
	/**
	 * Constructor
	 */
	membranes(half_sarcomere* set_p_parent_hs);

	/**
	* Destructor
	*/
	~membranes(void);

	// Variables

	half_sarcomere* p_parent_hs;		/**< pointer to parent half-sarcomere */

	cmv_results* p_cmv_results;			/**< pointer to cmv_results object */

	double Ca_myofil_conc;				/**< double with myofilament Ca_conc in M */

	/**
	/* function adds data fields and vectors to the results objet
	*/
	void prepare_for_cmv_results(void);
};

#pragma once

/**
/* @file		baro_activation.h
/* @brief		Header file for a baroreflex object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class cmv_protocol;

class baro_activation
{
public:
	/**
	 * Constructor
	 */
	baro_activation(double set_t_start_s, double t_stop_s);

	/**
	 * Destructor
	 */
	 ~baro_activation(void);

	 // Variables
	 double t_start_s;						/**< double defining when the activation starts */

	 double t_stop_s;						/**< double definining when the activation stops */

	 // Functions

	 int return_status(double t_test_s);
};

#pragma once

/**
/* @file		cmv_model.h
/* @brief		Header file for a cmv_model object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

// Definitions for JSON parsing
#ifndef _RAPIDJSON_DOCUMENT
#define _RAPIDJSON_DOCUMENT
#include "rapidjson/document.h"
#endif

#include "global_definitions.h"

// Forward declararations
class half_sarcomere;
class membranes;
class heart_rate;

using namespace std;

class cmv_model
{
public:
	/**
	* Constructor
	*/
	cmv_model(string JSON_model_file_string);

	/**
	* Destructor
	*/
	~cmv_model(void);

	// Variables

	// Heart rate
	double hr_t_RR_interval_s;			/**< double with RR interval in s */

	// Half-sarcomere
	
	double hs_initial_hs_length;		/**< double with initial hs_length */

	double hs_reference_hs_length;		/**< double with hs length used for normalization */

	// Membranes
	double memb_Ca_content;				/**< double with total Ca_concentration
												in M */
	double memb_t_open_s;				/**< double with open duration in s */

	double memb_k_serca;				/**< double describing rate at which
												SERCA pumps Ca in to
												sarcoplasmic reticulum
												in M^-1 s^-1 */

	double memb_k_leak;					/**< double desribing rate at which
												Ca leaks from sarcoplasmic reticulum
												in M s^-1 */

	double memb_k_active;				/**< double describing rate at which
												Ca is released from sarcoplasmic reticulum
												when the release channels are open
												in M s^-1 */

	// Other functions

	/**
	/* Function initialises a model object from file
	*/
	void initialise_model_from_JSON_file(string JSON_model_file_string);
};
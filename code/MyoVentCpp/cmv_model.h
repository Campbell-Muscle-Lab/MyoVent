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
class kinetic_scheme;

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

	double temperature_K;				/**< double with temperature in K */

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

	// Myofilaments
	double myof_cb_number_density;		/**< double describing the number of
												cross-bridges in a half-sarcomere
												with a cross-sectional area of 1 m^2 */

	double myof_prop_fibrosis;			/**< double describing the proportional
												cross-sectional area occupied by
												fibrosis */

	double myof_prop_myofilaments;		/**< double describing the proportional
												area of myofibrils occupied by
												myofilaments - the remainder is
												mitochondria */


	double myof_int_pas_sigma;			/**< double with sigma for intracellular
												passive stress exponential
												in N m^-2 */

	double myof_int_pas_L;				/**< double with curvature for intracellular
												passive stress exponential
												exponential in nm */

	double myof_int_pas_slack_hsl;		/**< double with slack length for
												intraceullar passive stress
												exponential in nm */

	double myof_ext_pas_sigma;			/**< double with sigma for extracellular
												passive stress exponential
												in N m^-2 */

	double myof_ext_pas_L;				/**< double with curvature for extracellular
												passive stress exponential
												exponential in nm */

	double myof_ext_pas_slack_hsl;		/**< double with slack length for
												extraceullar passive stress
												exponential in nm */

	double myof_fil_compliance_factor;	/**< double holding the myofilament compliance
													factor. When the half-sarcomere changes
													length by x, cross-bridges are displaced
													by x * this variable */

	double myof_k_cb;					/**< double describing cross-bridge stiffness
												in N m^-1 */

	kinetic_scheme* p_m_scheme;			/**< pointer to the kinetic scheme
												for myosin */

	double myof_a_k_on;					/**< double describing actin k_on */

	double myof_a_k_off;				/**< double describing actin k_off */

	double myof_a_k_coop;				/**< double describing actin k_coop */

	// Other functions

	/**
	/* Function initialises a model object from file
	*/
	void initialise_model_from_JSON_file(string JSON_model_file_string);
};
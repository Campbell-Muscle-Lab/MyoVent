#pragma once

/**
/* @file		half_sarcomere.h
/* @brief		Header file for a half_sarcomere object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class cmv_system;
class cmv_results;
class hemi_vent;
class membranes;
class heart_rate;

class half_sarcomere
{
public:
	/**
	* Constructor
	*/
	half_sarcomere(hemi_vent* set_p_parent_hemi_vent);

	/**
	* Destructor
	*/
	~half_sarcomere(void);

	// Variables
	hemi_vent* p_parent_hemi_vent;
	
	cmv_results* p_cmv_results;

	membranes* p_membranes;

	heart_rate* p_heart_rate;

	double hs_length;
	double hs_force;

	/**
	/* function adds data fields and vectors to the results objet
	*/
	void prepare_for_cmv_results(void);

	void implement_time_step(double time_step_s);
};
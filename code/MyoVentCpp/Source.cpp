/**
/* @file		hemi_vent.cpp
/* @brief		Source file for a hemi_vent object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "hemi_vent.h"
#include "cmv_system.h"
#include "half_sarcomere.h"
#include "cmv_results.h"


// Constructor
hemi_vent::hemi_vent(cmv_system* set_p_parent_cmv_system)
{
	// Initialise

	// Code
	std::cout << "hemi_vent_constructor()\n";

	// Set pointer to parent
	p_parent_cmv_system = set_p_parent_cmv_system;
}

// Destructor
hemi_vent::~hemi_vent(void)
{
	//! hemi_vent destructor

	std::cout << "hemi_vent_destructor\n";
}

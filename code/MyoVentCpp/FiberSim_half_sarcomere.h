#pragma once

/**
/* @file		FiberSim_half_sarcomere.h
/* @breif		Header file for a FiberSim half-sarcomere object
/* @author		Ken Campbell
*/

class half_sarcomere;

class FiberSim_half_sarcomere
{
public:
	/**
	* Constructor
	*/
	FiberSim_half_sarcomere(half_sarcomere* set_p_hs);

	/**
	* Destructor
	*/
	~FiberSim_half_sarcomere(void);

	// Variables
	half_sarcomere* p_hs;				/**< pointer to a half_sarcomere */
};
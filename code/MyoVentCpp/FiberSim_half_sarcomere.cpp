/**
/* @file		FiberSim_half_sarcomere.cpp
/* @brief		Source file for a FiberSim_half_sarcomere object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "FiberSim_half_sarcomere.h"

#include "half_sarcomere.h"

// Constructor
FiberSim_half_sarcomere::FiberSim_half_sarcomere(half_sarcomere* set_p_hs)
{
	//! Constructor
	
	p_hs = set_p_hs;
}

// Destructor
FiberSim_half_sarcomere::~FiberSim_half_sarcomere()
{

}
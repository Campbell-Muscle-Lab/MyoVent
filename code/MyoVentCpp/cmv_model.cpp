/**
/* @file		cmv_model.cpp
/* @brief		Source file for a cmv_model object
/* @author		Ken Campbell
*/

#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>

#include "cmv_model.h"
#include "kinetic_scheme.h"
#include "m_state.h"
#include "transition.h"
#include "JSON_functions.h"
#include "kinetic_scheme.h"

#include "rapidjson\document.h"
#include "rapidjson\filereadstream.h"

using namespace std::filesystem;
using namespace std;

// Constructor
cmv_model::cmv_model(string JSON_model_file_string)
{
	// Initialise

	// Code
	cout << "In cmv_model constructor\n";

	// Set safe options
	p_m_scheme = NULL;

	// Set some known parameters
	temperature_K = 315.0;

	// Set rest from file
	initialise_model_from_JSON_file(JSON_model_file_string);
}

// Destructor
cmv_model::~cmv_model(void)
{
	// Code
	std::cout << "cmv_model destructor\n";

	delete p_m_scheme;
}

// Other functions
void cmv_model::initialise_model_from_JSON_file(string JSON_model_file_string)
{
	//! Function initialises the object from file

	// Variables
	errno_t file_error;
	FILE* fp;
	char readBuffer[65536];

	// Code
	file_error = fopen_s(&fp, JSON_model_file_string.c_str(), "rb");
	if (file_error != 0)
	{
		cout << "Error opening JSON model file: " << JSON_model_file_string;
		exit(1);
	}

	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document doc;
	doc.ParseStream(is);

	fclose(fp);

	// Now trying to read file
	cout << "Parse JSON model file: " << JSON_model_file_string << "\n";

	// Load the heart_rate object
	JSON_functions::check_JSON_member_object(doc, "heart_rate");
	const rapidjson::Value& hr = doc["heart_rate"];

	JSON_functions::check_JSON_member_number(hr, "t_RR_interval_s");
	hr_t_RR_interval_s = hr["t_RR_interval_s"].GetDouble();

	// Load the half-sarcomere structure
	JSON_functions::check_JSON_member_object(doc, "half_sarcomere");
	const rapidjson::Value& hs = doc["half_sarcomere"];

	// Load the half_sarcomere parameters
	JSON_functions::check_JSON_member_number(hs, "initial_hs_length");
	hs_initial_hs_length = hs["initial_hs_length"].GetDouble();
		
	// Load the membranes structure
	JSON_functions::check_JSON_member_exists(hs, "membranes");
	const rapidjson::Value& memb = hs["membranes"];

	JSON_functions::check_JSON_member_number(memb, "Ca_content");
	memb_Ca_content = memb["Ca_content"].GetDouble();

	JSON_functions::check_JSON_member_number(memb, "k_leak");
	memb_k_leak = memb["k_leak"].GetDouble();

	JSON_functions::check_JSON_member_number(memb, "k_active");
	memb_k_active = memb["k_active"].GetDouble();

	JSON_functions::check_JSON_member_number(memb, "k_serca");
	memb_k_serca = memb["k_serca"].GetDouble();

	JSON_functions::check_JSON_member_number(memb, "t_open");
	memb_t_open_s = memb["t_open"].GetDouble();

	// Load the myofilaments structure
	JSON_functions::check_JSON_member_exists(hs, "myofilaments");
	const rapidjson::Value& myof = hs["myofilaments"];

	JSON_functions::check_JSON_member_number(myof, "cb_number_density");
	myof_cb_number_density = myof["cb_number_density"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "prop_fibrosis");
	myof_prop_fibrosis = myof["prop_fibrosis"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "prop_myofilaments");
	myof_prop_myofilaments = myof["prop_myofilaments"].GetDouble();

	// Load the myosin structure
	JSON_functions::check_JSON_member_exists(myof, "myosin");
	const rapidjson::Value& myos = myof["myosin"];

	JSON_functions::check_JSON_member_number(myos, "k_cb");
	myof_k_cb = myos["k_cb"].GetDouble();

	JSON_functions::check_JSON_member_exists(myos, "kinetics");
	const rapidjson::Value& mykin = myos["kinetics"];

	// Now add kinetic scheme
	p_m_scheme = new kinetic_scheme(mykin, this);

	// Load the actin structure
	JSON_functions::check_JSON_member_exists(myof, "actin");
	const rapidjson::Value& actin = myof["actin"];

	JSON_functions::check_JSON_member_number(actin, "k_on");
	myof_a_k_on = actin["k_on"].GetDouble();

	JSON_functions::check_JSON_member_number(actin, "k_off");
	myof_a_k_off = actin["k_off"].GetDouble();

	JSON_functions::check_JSON_member_number(actin, "k_coop");
	myof_a_k_coop = actin["k_coop"].GetDouble();
}

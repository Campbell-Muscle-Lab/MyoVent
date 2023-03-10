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

#include "global_definitions.h"
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

// Valve structure
struct cmv_model_valve_structure {
	string name;
	double mass;
	double eta;
	double k;
	double leak;
};

struct cmv_model_rc_structure {
	string level;
	string variable;
	double k_control;
	double k_recov;
	double para_factor;
	double symp_factor;
};

struct cmv_model_gc_structure {
	string type;
	string level;
	string signal;
	double set_point;
	double prop_gain;
	double deriv_gain;
	double max_rate;
};

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

	// Reserve space for compartment variables
	circ_resistance = (double*)malloc(MAX_NO_OF_COMPARTMENTS * sizeof(double));
	circ_compliance = (double*)malloc(MAX_NO_OF_COMPARTMENTS * sizeof(double));
	circ_slack_volume = (double*)malloc(MAX_NO_OF_COMPARTMENTS * sizeof(double));
	circ_inertance = (double*)malloc(MAX_NO_OF_COMPARTMENTS * sizeof(double));

	// Create pointers
	p_av = new cmv_model_valve_structure;
	p_mv = new cmv_model_valve_structure;

	// Set values that might not be defined in the model file
	baro_P_set = GSL_NAN;
	baro_S = GSL_NAN;
	baro_k_drive = GSL_NAN;
	baro_k_recov = GSL_NAN;
	baro_P_compartment = -1;

	// Safe pointers
	for (int i = 0; i < MAX_NO_OF_REFLEX_CONTROLS; i++)
	{
		p_rc[i] = NULL;
	}

	no_of_rc_controls = 0;

	for (int i = 0; i < MAX_NO_OF_GROWTH_CONTROLS; i++)
	{
		p_gc[i] = NULL;
	}

	no_of_gc_controls = 0;

	// Set rest from file
	initialise_model_from_JSON_file(JSON_model_file_string);
}

// Destructor
cmv_model::~cmv_model(void)
{
	// Code

	delete p_m_scheme;

	delete p_av;
	delete p_mv;

	for (int i = 0; i < MAX_NO_OF_REFLEX_CONTROLS; i++)
	{
		if (p_rc[i] != NULL)
			delete p_rc[i];
	}

	free(circ_resistance);
	free(circ_compliance);
	free(circ_slack_volume);
	free(circ_inertance);
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

	// Load the circulation object
	JSON_functions::check_JSON_member_object(doc, "circulation");
	const rapidjson::Value& circ = doc["circulation"];

	JSON_functions::check_JSON_member_number(circ, "blood_volume");
	circ_blood_volume = circ["blood_volume"].GetDouble();

	JSON_functions::check_JSON_member_object(circ, "compartments");
	const rapidjson::Value& comp = circ["compartments"];

	// Pull arrays
	JSON_functions::check_JSON_member_array(comp, "resistance");
	const rapidjson::Value& r_array = comp["resistance"];

	circ_no_of_compartments = (int)r_array.Size();
	cout << "circ_no_of_compartments: " << circ_no_of_compartments << "\n";

	for (rapidjson::SizeType i = 0; i < r_array.Size(); i++)
	{
		circ_resistance[i] = r_array[i].GetDouble();
	}

	JSON_functions::check_JSON_member_array(comp, "compliance");
	const rapidjson::Value& c_array = comp["compliance"];

	for (rapidjson::SizeType i = 0; i < c_array.Size(); i++)
	{
		circ_compliance[i] = c_array[i].GetDouble();
	}

	JSON_functions::check_JSON_member_array(comp, "slack_volume");
	const rapidjson::Value& sv_array = comp["slack_volume"];

	for (rapidjson::SizeType i = 0; i < sv_array.Size(); i++)
	{
		circ_slack_volume[i] = sv_array[i].GetDouble();
	}

	JSON_functions::check_JSON_member_array(comp, "inertance");
	const rapidjson::Value& in_array = comp["inertance"];

	for (rapidjson::SizeType i = 0; i < sv_array.Size(); i++)
	{
		circ_inertance[i] = in_array[i].GetDouble();
	}

	// Load the ventricle object
	JSON_functions::check_JSON_member_object(doc, "ventricle");
	const rapidjson::Value& vent = doc["ventricle"];

	JSON_functions::check_JSON_member_number(vent, "wall_density");
	vent_wall_density = vent["wall_density"].GetDouble();

	JSON_functions::check_JSON_member_number(vent, "wall_volume");
	vent_wall_volume = vent["wall_volume"].GetDouble();

	// Load the valves
	JSON_functions::check_JSON_member_object(vent, "valves");
	const rapidjson::Value& valves = vent["valves"];

	JSON_functions::check_JSON_member_object(valves, "aortic");
	const rapidjson::Value& av = valves["aortic"];

	JSON_functions::check_JSON_member_string(av, "name");
	p_av->name = av["name"].GetString();

	JSON_functions::check_JSON_member_number(av, "mass");
	p_av->mass = av["mass"].GetDouble();

	JSON_functions::check_JSON_member_number(av, "eta");
	p_av->eta = av["eta"].GetDouble();

	JSON_functions::check_JSON_member_number(av, "k");
	p_av->k = av["k"].GetDouble();

	if (JSON_functions::check_JSON_member_exists(av, "leak"))
		p_av->leak = av["leak"].GetDouble();
	else
		p_av->leak = 0.0;

	JSON_functions::check_JSON_member_object(valves, "mitral");
	const rapidjson::Value& mv = valves["mitral"];

	JSON_functions::check_JSON_member_string(mv, "name");
	p_mv->name = mv["name"].GetString();

	JSON_functions::check_JSON_member_number(mv, "mass");
	p_mv->mass = mv["mass"].GetDouble();

	JSON_functions::check_JSON_member_number(mv, "eta");
	p_mv->eta = mv["eta"].GetDouble();

	JSON_functions::check_JSON_member_number(mv, "k");
	p_mv->k = mv["k"].GetDouble();

	if (JSON_functions::check_JSON_member_exists(mv, "leak"))
		p_mv->leak = mv["leak"].GetDouble();
	else
		p_mv->leak = 0.0;

	// Load the heart_rate object
	JSON_functions::check_JSON_member_object(vent, "heart_rate");
	const rapidjson::Value& hr = vent["heart_rate"];

	JSON_functions::check_JSON_member_number(hr, "t_RR_interval_s");
	hr_t_RR_interval_s = hr["t_RR_interval_s"].GetDouble();

	// Load the half-sarcomere structure
	JSON_functions::check_JSON_member_object(vent, "half_sarcomere");
	const rapidjson::Value& hs = vent["half_sarcomere"];

	// Load the half_sarcomere parameters
	JSON_functions::check_JSON_member_number(hs, "reference_hs_length");
	hs_reference_hs_length = hs["reference_hs_length"].GetDouble();

	JSON_functions::check_JSON_member_number(hs, "initial_ATP_concentration");
	hs_initial_ATP_concentration = hs["initial_ATP_concentration"].GetDouble();

	JSON_functions::check_JSON_member_number(hs, "prop_fibrosis");
	hs_prop_fibrosis = hs["prop_fibrosis"].GetDouble();

	JSON_functions::check_JSON_member_number(hs, "prop_myofilaments");
	hs_prop_myofilaments = hs["prop_myofilaments"].GetDouble();

	JSON_functions::check_JSON_member_number(hs, "delta_G_ATP");
	hs_delta_G_ATP = hs["delta_G_ATP"].GetDouble();

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

	// Load the mitochondria structure
	JSON_functions::check_JSON_member_exists(hs, "mitochondria");
	const rapidjson::Value& mito = hs["mitochondria"];

	JSON_functions::check_JSON_member_number(mito, "ATP_generation_rate");
	mito_ATP_generation_rate = mito["ATP_generation_rate"].GetDouble();

	// Load the myofilaments structure
	JSON_functions::check_JSON_member_exists(hs, "myofilaments");
	const rapidjson::Value& myof = hs["myofilaments"];

	JSON_functions::check_JSON_member_number(myof, "cb_number_density");
	myof_cb_number_density = myof["cb_number_density"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "int_pas_sigma");
	myof_int_pas_sigma = myof["int_pas_sigma"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "int_pas_L");
	myof_int_pas_L = myof["int_pas_L"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "int_pas_slack_hsl");
	myof_int_pas_slack_hsl = myof["int_pas_slack_hsl"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "ext_pas_sigma");
	myof_ext_pas_sigma = myof["ext_pas_sigma"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "ext_pas_L");
	myof_ext_pas_L = myof["ext_pas_L"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "ext_pas_slack_hsl");
	myof_ext_pas_slack_hsl = myof["ext_pas_slack_hsl"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "fil_compliance_factor");
	myof_fil_compliance_factor = myof["fil_compliance_factor"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "thick_filament_length");
	myof_thick_fil_length = myof["thick_filament_length"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "bare_zone_length");
	myof_bare_zone_length = myof["bare_zone_length"].GetDouble();

	JSON_functions::check_JSON_member_number(myof, "thin_filament_length");
	myof_thin_fil_length = myof["thin_filament_length"].GetDouble();

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

	// Now try the baroreflex
	if (JSON_functions::check_JSON_member_exists(doc, "baroreflex"))
	{
		// It might not be in the model file
		const rapidjson::Value& baro = doc["baroreflex"];

		JSON_functions::check_JSON_member_number(baro, "baro_P_set");
		baro_P_set = baro["baro_P_set"].GetDouble();

		JSON_functions::check_JSON_member_number(baro, "baro_S");
		baro_S = baro["baro_S"].GetDouble();

		JSON_functions::check_JSON_member_number(baro, "baro_k_drive");
		baro_k_drive = baro["baro_k_drive"].GetDouble();

		JSON_functions::check_JSON_member_number(baro, "baro_k_recov");
		baro_k_recov = baro["baro_k_recov"].GetDouble();

		JSON_functions::check_JSON_member_number(baro, "baro_P_compartment");
		baro_P_compartment = baro["baro_P_compartment"].GetUint();

		JSON_functions::check_JSON_member_array(baro, "control");
		const rapidjson::Value& cont_array = baro["control"];

		for (rapidjson::SizeType i = 0; i < cont_array.Size(); i++)
		{
			const rapidjson::Value& cont = cont_array[i];

			p_rc[i] = new cmv_model_rc_structure();

			JSON_functions::check_JSON_member_string(cont, "level");
			p_rc[i]->level = cont["level"].GetString();

			JSON_functions::check_JSON_member_string(cont, "variable");
			p_rc[i]->variable = cont["variable"].GetString();

			JSON_functions::check_JSON_member_number(cont, "k_control");
			p_rc[i]->k_control = cont["k_control"].GetDouble();

			JSON_functions::check_JSON_member_number(cont, "k_recov");
			p_rc[i]->k_recov = cont["k_recov"].GetDouble();

			JSON_functions::check_JSON_member_number(cont, "para_factor");
			p_rc[i]->para_factor = cont["para_factor"].GetDouble();

			JSON_functions::check_JSON_member_number(cont, "symp_factor");
			p_rc[i]->symp_factor = cont["symp_factor"].GetDouble();

			no_of_rc_controls = no_of_rc_controls + 1;
		}
	}

	// Now try the growth
	if (JSON_functions::check_JSON_member_exists(doc, "growth"))
	{
		// It might not be in the model file
		const rapidjson::Value& grow = doc["growth"];

		JSON_functions::check_JSON_member_number(grow, "master_rate");
		gr_master_rate = grow["master_rate"].GetDouble();

		// Shrinkage

		JSON_functions::check_JSON_member_exists(grow, "shrinkage");
		const rapidjson::Value& shrink = grow["shrinkage"];

		JSON_functions::check_JSON_member_number(shrink, "eccentric_rate");
		gr_shrink_eccentric_rate = shrink["eccentric_rate"].GetDouble();

		JSON_functions::check_JSON_member_number(shrink, "concentric_rate");
		gr_shrink_concentric_rate = shrink["concentric_rate"].GetDouble();

		// Growth controls

		JSON_functions::check_JSON_member_array(grow, "control");
		const rapidjson::Value& grow_array = grow["control"];

		for (rapidjson::SizeType i = 0; i < grow_array.Size(); i++)
		{
			const rapidjson::Value& cont = grow_array[i];

			p_gc[i] = new cmv_model_gc_structure();

			JSON_functions::check_JSON_member_string(cont, "type");
			p_gc[i]->type = cont["type"].GetString();

			JSON_functions::check_JSON_member_string(cont, "level");
			p_gc[i]->level = cont["level"].GetString();

			JSON_functions::check_JSON_member_string(cont, "signal");
			p_gc[i]->signal = cont["signal"].GetString();

			JSON_functions::check_JSON_member_number(cont, "set_point");
			p_gc[i]->set_point = cont["set_point"].GetDouble();

			JSON_functions::check_JSON_member_number(cont, "prop_gain");
			p_gc[i]->prop_gain = cont["prop_gain"].GetDouble();

			if (JSON_functions::check_JSON_member_exists(cont, "deriv_gain"))
			{
				p_gc[i]->deriv_gain = cont["deriv_gain"].GetDouble();
			}
			else
			{
				p_gc[i]->deriv_gain = 0.0;
			}

			JSON_functions::check_JSON_member_number(cont, "max_rate");
			p_gc[i]->max_rate = cont["max_rate"].GetDouble();

			no_of_gc_controls = no_of_gc_controls + 1;
		}
	}
}

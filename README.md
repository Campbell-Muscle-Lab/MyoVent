# MyoVent

````mermaid

classDiagram
    
    CMyoVent <|-- cmv_system
    
    cmv_system <|-- cmv_model
    cmv_system <|-- cmv_protocol
    cmv_system <|-- cmv_results
    cmv_system <|-- cmv_options
    cmv_system <|-- circulation

    cmv_model <|-- FiberSim_model
    cmv_model <|-- MyoSim_model

    cmv_protocol <|-- activation
    cmv_protocol <|-- perturbation

    cmv_options <|-- FiberSim_options
    cmv_options <|-- MyoSim_options

    circulation <|-- hemi_vent
    circulation <|-- VAD
    circulation <|-- baroreflex
    circulation <|-- Growth
    
    hemi_vent <|-- muscle
    hemi_vent <|-- valve
    
    muscle <|-- membranes
    muscle <|-- heart_rate
    muscle <|-- MyoSim_muscle
    muscle <|-- FiberSim_muscle
    muscle <|-- mitochondria

    baroreflex <|-- reflex_control
    
    Growth <|-- Growth_control
    
    FiberSim_model <|-- FiberSim_kinetic_scheme
    FiberSim_kinetic_scheme <|-- FiberSim_m_state
    FiberSim_m_state <|-- FiberSim_transition
    
    MyoSim_model <|-- MyoSim_kinetic_scheme
    MyoSim_kinetic_scheme <|-- MyoSim_m_state
    MyoSim_m_state <|-- MyoSim_transition

    FiberSim_muscle <|-- FiberSim_series_component
    FiberSim_muscle <|-- FiberSim_half_sarcomere

    FiberSim_half_sarcomere <|-- FiberSim_thick_filament
    FiberSim_half_sarcomere <|-- FiberSim_thin_filament

    MyoSim_muscle <|-- MyoSim_half_sarcomere
    MyoSim_muscle <|-- MyoSim_series_component


````

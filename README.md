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
    
    hemi_vent <|-- half_sarcomere
    hemi_vent <|-- valve
    
    half_sarcomere <|-- membranes
    half_sarcomere <|-- heart_rate
    half_sarcomere <|-- MyoSim_half_sarcomere
    half_sarcomere <|-- FiberSim_half_sarcomere
    half_sarcomere <|-- mitochondria

    baroreflex <|-- reflex_control
    
    Growth <|-- Growth_control
    
    FiberSim_model <|-- FiberSim_kinetic_scheme
    FiberSim_kinetic_scheme <|-- FiberSim_m_state
    FiberSim_m_state <|-- FiberSim_transition
    
    MyoSim_model <|-- MyoSim_kinetic_scheme
    MyoSim_kinetic_scheme <|-- MyoSim_m_state
    MyoSim_m_state <|-- MyoSim_transition

    FiberSim_half_sarcomere <|-- FiberSim_thick_filament
    FiberSim_half_sarcomere <|-- FiberSim_thin_filament


````

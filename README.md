# MyoVent

````mermaid

classDiagram

    CMyoVent <|-- cmv_system
    cmv_system <|-- cmv_model
    cmv_system <|-- cmv_protocol
    cmv_system <|-- cmv_results
    cmv_system <|-- circulation
    circulation <|-- hemi_vent
    circulation <|-- VAD
    circulation <|-- baroreflex
    circulation <|-- Growth
    hemi_vent <|-- half_sarcomere
    hemi_vent <|-- valve
    half_sarcomere <|-- membranes
    half_sarcomere <|-- heart_rate
    half_sarcomere <|-- myofilaments
    half_sarcomere <|-- mitochondria
    myofilaments <|--kinetic_scheme
    kinetic_scheme <|-- m_state
    m_state <|-- transition
    baroreflex <|-- reflex_control
    Growth <|-- Growth_control
    cmv_protocol <|-- activation
    cmv_protocol <|-- perturbation
    
    cmv_system: + *cmv_model p_cmv_model
    cmv_system: + run_simulation()

    half_sarcomere: + prop_fibrosis
    half_sarcomere: + prop_myofilaments
    half_sarcomere: + hs_length
    half_sarcomere: + hs_stress
    half_sarcomere: + ATP_concentration




````

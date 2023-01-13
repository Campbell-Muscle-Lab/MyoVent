# MyoVent

````mermaid

classDiagram

    CMyoVent <|-- cmv_system
    cmv_system <|-- cmv_model
    cmv_system <|-- cmv_protocol
    cmv_system <|-- cmv_results
    cmv_system <|-- circulation
    circulation <|-- hemi_vent
    circulation <|-- baroreflex
    circulation <|-- Growth
    hemi_vent <|-- half_sarcomere
    hemi_vent <|-- valve 
    half_sarcomere <|-- membranes
    half_sarcomere <|-- heart_rate
    half_sarcomere <|-- myofilaments
    myofilaments <|--kinetic_scheme
    kinetic_scheme <|-- m_state
    m_state <|-- transition
    baroreflex <|-- reflex_control
    cmv_protocol <|-- baro_activation
    cmv_protocol <|-- growth_activation
    
    cmv_system: + *cmv_model p_cmv_model
    cmv_system: + run_simulation()


````

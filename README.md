# MyoVent

````mermaid

classDiagram

    CMyoVent <|-- cmv_system
    cmv_system <|-- cmv_model
    cmv_system <|-- cmv_protocol
    cmv_system <|-- cmv_results
    cmv_system <|-- circulation
    cmv_system <|-- hemi_vent
    cmv_system <|-- Baroreflex
    cmv_system <|-- Growth
    hemi_vent <|-- half_sarcomere
    half_sarcomere <|-- membranes
    half_sarcomere <|-- Heart_rate
    half_sarcomere <|-- Myofilaments
    
    cmv_system: + *cmv_model p_cmv_model
    cmv_system: + run_simulation()


````

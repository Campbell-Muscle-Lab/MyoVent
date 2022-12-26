# MyoVent

````mermaid

classDiagram

    CMyoVent <|-- CMV_model
    CMyoVent <|-- CMV_protocol
    CMyoVent <|-- CMV_results
    CMyoVent <|-- CV_system
    CV_system <|-- Circulation
    CV_system <|-- HemiVent
    CV_system <|-- Baroreflex
    CV_system <|-- Growth
    HemiVent <|-- Half_sarcomere
    Half_sarcomere <|-- Membranes
    Half_sarcomere <|-- Myofilaments
    
    CMyoVent: + *CMV_model p_CMV_model
    CMyoVent: +run_simulation()

````

# Lumon Industries System Architecture

Welcome to the Lumon Industries internal architecture documentation. The work we do here is mysterious and important.

## Macrodata Refinement Architecture

At the heart of the Severed Floor's operations is the **Macrodata Refinement** system. This system is driven by two primary engines designed to uphold the vision of Kier Eagan.

### CorrelationEngine

The `CorrelationEngine` is responsible for parsing the vast and complex sea of numbers that Innies encounter daily. Under the hood, this engine meticulously correlates:
- ETW (Event Tracing for Windows) File I/O
- FIM (File Integrity Monitoring)
- Network Events

By aggregating these data points, the `CorrelationEngine` successfully identifies **Macrodata Anomalies**—those numbers that elicit emotional responses. Once identified, these anomalies are binned securely.

### ActiveResponse Engine

To ensure compliance and maintain the sanctity of the Severed Floor, we have implemented the rules-based `ActiveResponse` engine. Should an Innie or external factor attempt to disrupt our Macrodata Refinement workflow, the `ActiveResponse` engine triggers **Containment Directives**. 

These directives are absolute and designed to protect the Outie from the burdens of the Severed Floor, honoring Kier's wisdom.

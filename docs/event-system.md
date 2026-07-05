# Severed Floor Event System

The Event System is the lifeblood of Lumon Industries' operational awareness on the Severed Floor. 

## Event Sources

To properly support Macrodata Refinement, our event system leverages the following sources:
- **ETW Monitoring:** We capture low-level Event Tracing for Windows (ETW) File I/O events to track every interaction within the workspace.
- **FIM (File Integrity Monitoring):** Ensures that the sacred texts and critical operational files remain untampered with by Innies or unauthorized personnel.

## Macrodata Correlation

Events are not viewed in isolation. They are fed directly into the `CorrelationEngine`, where ETW, FIM, and Network events are synthesized. This process is crucial for detecting Macrodata Anomalies. When an Innie refines a bin of numbers, the Event System ensures that all systemic correlations are logged and evaluated against Kier Eagan's baseline parameters.

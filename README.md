
# Digital Ear — Industrial Machine Sound Anomaly Detector Prototype

## Overview
Digital Ear is an embedded systems project that explores sound-based anomaly detection for industrial machinery. It uses an ESP32-S3 development board and an INMP441 digital microphone to capture sound and identify unusual readings through baseline/threshold comparison.

## Problem Statement
Changes in machine sound can sometimes indicate changes in operating conditions. This project explores a simple embedded approach for identifying unusual acoustic patterns.

## Hardware Components
- ESP32-S3 development board
- INMP441 digital microphone
- USB connection for programming and serial monitoring

## Working Principle
1. Capture sound using the INMP441 microphone.
2. Establish a normal sound reference.
3. Compare live readings against the implemented baseline or threshold.
4. Display the detection result in the serial monitor.

## Detection Method
The current prototype uses baseline/threshold-based detection. It does not use a trained machine-learning model.

## Testing
The prototype was tested using:
- Fan or machine-like sounds
- Claps
- Other sound inputs

Actual readings and test outcomes will be documented using serial-monitor results.

## Applications
- Acoustic condition monitoring
- Embedded systems experimentation
- Industrial sound anomaly detection research

## Limitations
The current prototype identifies unusual sound readings, but this alone does not confirm a specific machine fault. Performance under different noise conditions requires further testing.

## Future Improvements
- Analyse acoustic features such as RMS and frequency components.
- Test using labelled industrial audio datasets.
- Evaluate false alarms and detection reliability.
- Explore edge-based machine learning.

## Technologies
ESP32-S3, INMP441, embedded programming, baseline/threshold comparison.

## Author
Rohith Kumar  
Electronics and Communication Engineering Student  
East Point College of Engineering and Technology, Bengaluru

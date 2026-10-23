# Digital Ear — Edge AI Industrial Sound Anomaly Detector

## Overview

Digital Ear is an embedded AI-based prototype designed to explore how machine sounds can be monitored for abnormal acoustic patterns. It uses an ESP32-S3 microcontroller, an INMP441 digital microphone, and an Edge Impulse inference model to classify captured audio.

The project also includes a Wi-Fi web dashboard intended to display the predicted sound class and its confidence score.

## Key Features

- Captures digital audio using an INMP441 microphone and I²S.
- Processes audio at a configured sampling rate of 16 kHz.
- Uses an Edge Impulse model for sound classification.
- Supports an uncertain prediction when the highest class score is below a configured confidence threshold.
- Provides serial-monitor output for debugging.
- Includes an HTTP web dashboard that requests classification data from the device.

## Hardware

- ESP32-S3 development board
- INMP441 I²S digital microphone
- Computer or phone to access the dashboard
- Wi-Fi network or mobile hotspot

## Technologies Used

- Embedded C/C++ and Arduino framework
- ESP32-S3
- I²S digital audio acquisition
- Edge Impulse machine-learning inference
- Wi-Fi and asynchronous HTTP server
- HTML, CSS and JavaScript

## How It Works

1. The INMP441 microphone captures sound.
2. The ESP32-S3 reads audio samples through I²S.
3. The firmware passes captured audio to the Edge Impulse inference model.
4. The firmware selects the highest-scoring class, subject to a confidence threshold.
5. The result and associated scores are made available to the web dashboard.

## Current Status

The repository contains the firmware and dashboard implementation. The code includes the main audio-capture, inference and web-monitoring logic.

**Physical operation, classification accuracy and real-world anomaly-detection performance still require documented testing.** No accuracy figures or performance claims are made here.
## Experimental Readings

![Digital Ear sound readings](sound-readings.jpng)
## Future Improvements

- Test with labeled normal and abnormal machine recordings.
- Measure classification accuracy, false alarms and missed anomalies.
- Evaluate the system under different machine speeds and background-noise conditions.
- Improve configuration security and document deployment requirements.
- Measure inference latency and memory usage.

## Repository Contents

- Firmware source code
- README and project documentation

## Important Note

The Edge Impulse model library and its class-label ordering must match the firmware configuration. Wi-Fi credentials should be kept out of public source code.

This is a prototype for exploration and testing, not a certified industrial safety or predictive-maintenance system.

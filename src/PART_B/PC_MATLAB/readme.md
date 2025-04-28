# MATLAB Code for Signal Processing and GPR Image Generation

This directory contains:

- MATLAB code for advanced signal processing and B-scan image generation.
- A `Data` folder with relevant datasets used in the imaging pipeline.

---

## 🧠 Overview

The provided MATLAB script is used to generate the final **Ground Penetrating Radar (GPR) image** (B-scan) from raw signal samples. 

Unlike traditional methods that rely primarily on **Fast Fourier Transform (FFT)** for signal processing, this implementation synthesizes **large apertures** by applying algorithms inspired by **Synthetic Aperture Radar (SAR)** techniques—typically used in airborne radar imaging.

---

## 🛰️ Synthetic Aperture Radar (SAR) Technique

**SAR** is a radar imaging method that leverages the motion of the radar to emulate a physically large antenna aperture. This significantly improves resolution by:

- Focusing radar echoes from multiple pulses.
- Compensating for variations in target distance and angle.
- Enhancing spatial resolution in post-processing.

The same concept is applied in this project to GPR data by mathematically simulating antenna motion and signal focusing in MATLAB.

---

## 📁 Data Folder Contents

- **`sample.txt`**:  
  Contains the signal samples collected during actual scanning in the test environment.

- **`correction.txt`**:  
  Contains reference data collected in a **non-reflective (empty)** environment. This is subtracted from `sample.txt` to **eliminate antenna coupling effects** and background noise.

---

This combination of SAR-inspired processing and environment correction allows for high-resolution, noise-reduced GPR imaging.

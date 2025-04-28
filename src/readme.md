# 🗂️ Source Code Overview

This directory contains the complete source code for both **Part A** and **Part B** of the project.

---

## ⚙️ Part A: STM32 Firmware

Includes the firmware responsible for:

- 📡 Signal acquisition
- 🧠 Signal processing
- 🖥️ Data visualization
- 🤖 Mechanical control

All implemented on the **STM32** microcontroller platform.

---

## ⚙️ Part B: Multi-Stage Implementation

Part B includes multiple components spread across different directories:

- 🔧 **STM32 Firmware**:  
  For mechanical maneuvering, signal generation, collection, and sampling.

- 🐍 **Python Scripts**:  
  Used to read and store data transmitted by the STM32.

- 🧮 **MATLAB Code**:  
  Applies signal processing algorithms on the acquired data to generate the final **Ground Penetrating Radar (GPR) image**.

---

This modular codebase supports end-to-end functionality—from hardware control to final image reconstruction—across both parts of the system.


# 📟 STM32 Project: Signal Processing with Display & Motion Control

This project interfaces the **Nucleo-F103RB** development board with the following hardware components:

- 🖥️ **ILI9488 TFT Display**
- 🎚️ **MCP4725 DAC**
- 🌀 **A4988 Stepper Motor Driver**

Additionally, the code performs **Fast Fourier Transform (FFT)** on the received signal and displays the processed results on the TFT screen.

---

## 🧰 Development Environment

- **IDE**: STM32CubeIDE

---

## ⚙️ Build and Run Instructions

1. **Save and Build the Project**  
   Navigate to `Build > Build Project`  
   This step compiles the code and highlights any errors, if present.

2. **Upload to Board**  
   Connect the **Nucleo-F103RB** via USB.  
   Use `Run > Debug As > STM32 C/C++ Application` to flash the code onto the board.

---

## 📦 External FFT Dependencies

The project uses external FFT libraries from an open-source GitHub repository.  
The following `.c` files (excluding `main.c`) are used for FFT operations:

| File Name                     | Description |
|------------------------------|-------------|
| `arm_bitreversal.c`          | Reorders FFT input/output in bit-reversed order. |
| `arm_radix8_butterfly_f32.c` | Radix-8 FFT butterfly computation for 32-bit float data. |
| `arm_cfft_f32.c`             | Complex FFT (CFFT) and inverse FFT (CIFFT) for float32. |
| `arm_copy_f32.c`             | Copies arrays of float32 values. |
| `arm_cmplx_mag_f32.c`        | Computes magnitude of complex FFT output. |
| `arm_common_tables.c`        | Contains twiddle factors, sine/cosine tables, and constants. |
| `arm_const_structs.c`        | Predefined FFT configuration structures. |
| `arm_mult_f32.c`             | Element-wise multiplication of float32 arrays. |

---

This implementation demonstrates real-time signal acquisition, processing, visualization, and mechanical control using STM32.

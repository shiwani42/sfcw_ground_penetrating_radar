# STM32 Firmware for IVS-362 Module Control and Data Acquisition

This STM32 firmware interfaces the **Nucleo-F103RB** board with the following components:

- 🧪 **MCP4921 DAC**
- ⚙️ **A4988 Stepper Motor Driver**
- 📡 **IVS-362 Module**

The firmware is responsible for signal generation, stepper motor control, and acquisition of IF1 output, which is then transmitted over UART to a connected PC for further processing.

---

## 📐 System Functionality

- **Voltage Sweep Generation**:  
  The **MCP4921 DAC** is programmed to output voltages between `START_VAL = 992` and `END_VAL = 2978`, with a `STEP_SIZE = 2`.  
  These values are communicated via **SPI protocol**, resulting in a total of **993 discrete frequency points** for each scan.

- **Frequency Sweep and Range Compliance**:  
  The voltage sweep corresponds to a frequency sweep in the **IVS-362 module**. The selected voltage range ensures the module operates safely within **0.7V to 2.5V**, as required.

- **IF1 Output Handling**:  
  The **IF1 output** from the IVS-362 is offset by approximately **2.8V**. Since the STM32 ADC operates within **0–3.3V**, a hardware **subtractor circuit** based on an op-amp is used to shift the signal downward by **1.6V**, preventing ADC input overvoltage.

- **Data Buffering and Transmission**:  
  ADC samples are stored in a buffer of size **993**. Once all samples are collected, the entire buffer is transmitted over **UART** to the PC.  
  > ⚠️ Real-time UART transmission of individual samples would introduce significant delay, violating the Nyquist criterion and potentially compromising scan accuracy. Hence, buffered transmission is used to maintain efficiency and precision.

---

## 🧰 Development Environment

- **IDE**: STM32CubeIDE  
- **Microcontroller**: Nucleo-F103RB  
- **Communication Interfaces**:
  - **SPI**: For DAC
  - **UART**: For data transmission to PC
  - **GPIO**: For stepper motor control

---

## 🛠️ Build and Run Instructions

1. **Save and Build**:  
   Open the project in STM32CubeIDE, then go to `Build > Build Project` to compile and check for errors.

2. **Connect and Flash**:  
   - Connect the Nucleo-F103RB board via USB.
   - Select `Run > Debug As > STM32 C/C++ Application` to flash the firmware onto the board.

---

This firmware enables the GPR system to perform high-speed, precision scanning while maintaining signal integrity and operational safety.

# 🔌 LF Board Design Files

This directory contains resources related to the **Low Frequency (LF) board** design:

- 📁 A ZIP file of the complete **KiCAD project**
- 📄 The **schematic** of the LF board
- 📦 The **footprints** used in the design

---

These files provide the necessary information for viewing, modification, or fabrication of the LF board using KiCAD.

The major parts of LF circuit includes Buck convertor(7.4V to 5V),Boost convertor(5V to 12V),Band pass filter,DAC and non-inverting amplifier.

We used 2mm trace width for signals and 5mm trace width for power lines.

We used both the layers for routing.

The Boost convertor has filled potential zones instead of traces as recommended in the datasheet.

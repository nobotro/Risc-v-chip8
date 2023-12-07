## Risc-v-chip8 
This repository contains an extremely simple implementation of the RV32I ISA cpu without 5 stage pipeline and chip8 emulator which is running on that cpu.<br/>
I use tang nano 9k fpga board and 4.3 inch 480xRGBx272 display

## Current Design
- Entirely written in Verilog.
- Used 1 port block ram ip for memory inteface
- Not designed with multiple RISC-V harts .
- The privileged ISA is **not** implemented.
- FENCE, FENCE.I and CSR instructions are not implemented.
- Dont have 5 tage pipeline(be added in future)
 
## User Guide
- "rtl" directory contains verilog files for risc-v cpu and 4.3 inch 480xRGBx272 lcd controller
- "sdk" directory contains utils and libs to compile c program ro risc-v executable rom and generate memory configuration file from it.
- Video demonstrations<br>
  https://www.youtube.com/watch?v=g4ZAb_T6BVA<br>
  https://www.youtube.com/shorts/nkxRnUPcrGY

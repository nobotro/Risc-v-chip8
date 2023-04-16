
rm *.o
rm *.out
riscv32-unknown-elf-gcc -O0  starter.s chip8.c  -Ttext 0 -ffreestanding -fno-stack-protector  -fno-pie -march=rv32i -c&&riscv32-unknown-elf-ld starter.o chip8.o -T link.ld&&rom=$(riscv32-unknown-elf-elf2hex --input a.out --bit-width 32)&&
python romgen.py  "$rom" && riscv-machinsn-decode objfile a.out 

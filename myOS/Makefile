all: os.bin

# ---------- BOOTLOADER ----------
boot.bin:
	nasm -f bin boot/boot.asm -o build/boot.bin

# ---------- KERNEL OBJECTS ----------
kernel.o:
	i686-elf-gcc -ffreestanding -m32 -c kernel/kernel.c -o build/kernel.o

idt.o:
	i686-elf-gcc -ffreestanding -m32 -c kernel/idt.c -o build/idt.o

keyboard.o:
	i686-elf-gcc -ffreestanding -m32 -c kernel/keyboard.c -o build/keyboard.o

keyboard_asm.o:
	nasm -f elf32 kernel/keyboard.asm -o build/keyboard_asm.o

entry.o:
	nasm -f elf32 kernel/entry.asm -o build/entry.o

# ---------- KERNEL LINK ----------
kernel.bin: kernel.o idt.o keyboard.o keyboard_asm.o entry.o
	i686-elf-ld -T linker.ld -o build/kernel.bin \
		build/entry.o \
		build/kernel.o \
		build/idt.o \
		build/keyboard.o \
		build/keyboard_asm.o

# ---------- FINAL OS IMAGE ----------
os.bin: boot.bin kernel.bin
	cat build/boot.bin build/kernel.bin > os.bin

# ---------- RUN ----------
run: os.bin
	qemu-system-i386 \
	  -drive format=raw,file=os.bin \
	  -device loader,file=build/kernel.bin,addr=0x00100000

# ---------- CLEAN ----------
clean:
	rm -rf build/*.o build/*.bin os.bin
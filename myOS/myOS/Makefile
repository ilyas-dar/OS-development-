all: os.bin

boot.bin:
	nasm -f bin boot/boot.asm -o build/boot.bin

kernel.o:
	i686-elf-gcc -ffreestanding -m32 -c kernel/kernel.c -o build/kernel.o
	nasm -f elf32 kernel/entry.asm -o build/entry.o

kernel.bin: kernel.o
	i686-elf-ld -T linker.ld -o build/kernel.bin build/entry.o build/kernel.o

os.bin: boot.bin kernel.bin
	cat build/boot.bin build/kernel.bin > os.bin

run: os.bin
	qemu-system-i386 \
	  -drive format=raw,file=os.bin \
	  -device loader,file=build/kernel.bin,addr=0x00100000

clean:
	rm -rf build/*.o build/*.bin os.bin

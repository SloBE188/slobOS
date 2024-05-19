FILES=./build/start.o ./build/slobos.o ./build/stdlib.o ./build/stdio.o ./build/slobos.asm.o ./build/slobos.o
INCLUDES=-I./src
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ./stdlib.elf

./build/start.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.o

./build/slobos.o: ./src/slobos.asm
	nasm -f elf ./src/slobos.asm -o ./build/slobos.o

./build/stdlib.o: ./src/stdlib.c
	i686-elf-gcc  ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdlib.c -o ./build/stdlib.o

./build/stdio.o: ./src/stdio.c
	i686-elf-gcc  ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdio.c -o ./build/stdio.o

./build/slobos.asm.o: ./src/slobos.asm
	nasm -f elf ./src/slobos.asm -o ./build/slobos.asm.o

./build/slobos.o: ./src/slobos.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/slobos.c -o ./build/slobos.o


clean:
	rm -rf ${FILES}
	rm ./stdlib.elf
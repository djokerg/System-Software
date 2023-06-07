SRCS_AS = src/main_as.cpp src/helpers.cpp src/directive.cpp src/lang_elem.cpp src/instruction.cpp src/addressing.cpp misc/parser.cpp misc/lexer.cpp src/assembler.cpp
SRCS_LN = src/main_ln.cpp src/linker.cpp
SRCS_EM = src/main_em.cpp src/emulator.cpp
ASM_SRCS = 
GPP = g++
ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator

run_tests:
	${ASSEMBLER} -o main.o tests/main.s
	${ASSEMBLER} -o isr_terminal.o tests/isr_terminal.s
	${LINKER} -hex \
		-place=my_code@0x40000000 \
		-place=isr@0x50000000 \
		-o program.hex \
		main.o isr_terminal.o
	${EMULATOR} program.hex

all: assembler linker emulator

assembler: $(SRCS_AS)
	$(GPP) -g $(SRCS_AS) -o assembler

linker: $(SRCS_LN)
	$(GPP) -g $(SRCS_LN) -o linker

emulator: $(SRCS_EM)
	$(GPP) -g $(SRCS_EM) -o emulator

misc/lexer.cpp: misc/lexer.l
	flex misc/lexer.l
	mv lexer.cpp ./misc/
	mv lexer.hpp ./misc/

misc/parser.cpp: misc/parser.y misc/lexer.l
	bison misc/parser.y 
	mv parser.cpp ./misc/
	mv parser.hpp ./misc/

clean_all: clean clean_tests
	rm -rf assembler linker emulator
clean:
	rm -rf *.o misc/lexer.cpp misc/lexer.hpp misc/parser.cpp misc/parser.hpp *.txt *.hex
clean_tests:
	rm -rf ./tests/nivo-a/*.o ./tests/nivo-a/*.txt ./tests/nivo-a/*.hex ./tests/nivo-b/*.o ./tests/nivo-b/*.txt ./tests/nivo-b/*.hex ./tests/*.o ./tests/*.txt ./tests/*.hex 
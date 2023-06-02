SRCS = main.cpp helpers.cpp directive.cpp lang_elem.cpp instruction.cpp addressing.cpp parser.cpp lexer.cpp assembler.cpp
GPP = g++

all: $(SRCS) helpers.hpp directive.hpp instruction.hpp lang_elem.hpp addressing.hpp assembler.hpp
	$(GPP) -g $(SRCS) -o asembler

lexer.cpp: lexer.l helpers.hpp directive.hpp lang_elem.hpp instruction.hpp addressing.hpp assembler.hpp
	flex lexer.l

parser.cpp: parser.y lexer.l helpers.hpp directive.hpp lang_elem.cpp instruction.cpp addressing.cpp assembler.hpp
	bison parser.y

clean:
	rm -rf *.o lexer.cpp lexer.hpp parser.cpp parser.hpp asembler *.txt

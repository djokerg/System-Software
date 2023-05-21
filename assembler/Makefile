SRCS = main.cpp helpers.cpp directive.cpp lang_elem.cpp instruction.cpp addressing.cpp parser.cpp lexer.cpp 
GPP = g++

test: all
	cat sample.asm | ./parser

# if we compile them all together, we should get out what we want!
all: $(SRCS) helpers.hpp directive.hpp instruction.hpp lang_elem.hpp addressing.hpp
	$(GPP) $(SRCS) -o parser

# to regenerate the lexer, we call `flex` on it, which will
# create the lexer.c and lexer.h files
lexer.cpp: lexer.l helpers.hpp directive.hpp lang_elem.hpp instruction.hpp addressing.hpp
	flex lexer.l

# to regenerate the parser, we call `bison` on it, which will
# create the bison.c and bison.h files
parser.cpp: parser.y lexer.l helpers.hpp directive.hpp lang_elem.cpp instruction.cpp addressing.cpp
	bison parser.y

clean:
	rm -rf *.o lexer.cpp lexer.hpp parser.cpp parser.hpp parser

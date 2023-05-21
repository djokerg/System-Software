/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOKEN_COMMA = 258,
    ENDL = 259,
    TOKEN_COMM = 260,
    TOKEN_GLOBAL = 261,
    TOKEN_EXTERN = 262,
    TOKEN_SECTION = 263,
    TOKEN_WORD = 264,
    TOKEN_SKIP = 265,
    TOKEN_ASCII = 266,
    TOKEN_EQU = 267,
    TOKEN_END = 268,
    TOKEN_SYMBOL = 269,
    TOKEN_LABEL = 270,
    TOKEN_LITERAL = 271,
    TOKEN_STRING = 272,
    TOKEN_HALT = 273,
    TOKEN_INT = 274,
    TOKEN_IRET = 275,
    TOKEN_CALL = 276,
    TOKEN_RET = 277,
    TOKEN_JMP = 278,
    TOKEN_BEQ = 279,
    TOKEN_BNE = 280,
    TOKEN_BGT = 281,
    TOKEN_PUSH = 282,
    TOKEN_POP = 283,
    TOKEN_XCHG = 284,
    TOKEN_ADD = 285,
    TOKEN_SUB = 286,
    TOKEN_MUL = 287,
    TOKEN_DIV = 288,
    TOKEN_NOT = 289,
    TOKEN_AND = 290,
    TOKEN_OR = 291,
    TOKEN_XOR = 292,
    TOKEN_SHL = 293,
    TOKEN_SHR = 294,
    TOKEN_LD = 295,
    TOKEN_ST = 296,
    TOKEN_CSRRD = 297,
    TOKEN_CSRWR = 298,
    TOKEN_GP_REGISTER = 299,
    TOKEN_CS_REGISTER = 300,
    TOKEN_IMM = 301,
    TOKEN_LSQB = 302,
    TOKEN_RSQB = 303,
    TOKEN_PLUS = 304
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "parser.y"

	int         num;
	char       *symbol;
	struct arg *arg;
  class Addressing* adr;

#line 114 "parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */

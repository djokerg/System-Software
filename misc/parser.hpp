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
    TOKEN_GLOBAL = 258,
    TOKEN_EXTERN = 259,
    TOKEN_SECTION = 260,
    TOKEN_WORD = 261,
    TOKEN_SKIP = 262,
    TOKEN_ASCII = 263,
    TOKEN_EQU = 264,
    TOKEN_END = 265,
    TOKEN_HALT = 266,
    TOKEN_INT = 267,
    TOKEN_IRET = 268,
    TOKEN_CALL = 269,
    TOKEN_RET = 270,
    TOKEN_JMP = 271,
    TOKEN_BEQ = 272,
    TOKEN_BNE = 273,
    TOKEN_BGT = 274,
    TOKEN_PUSH = 275,
    TOKEN_POP = 276,
    TOKEN_XCHG = 277,
    TOKEN_ADD = 278,
    TOKEN_SUB = 279,
    TOKEN_MUL = 280,
    TOKEN_DIV = 281,
    TOKEN_NOT = 282,
    TOKEN_AND = 283,
    TOKEN_OR = 284,
    TOKEN_XOR = 285,
    TOKEN_SHL = 286,
    TOKEN_SHR = 287,
    TOKEN_LD = 288,
    TOKEN_ST = 289,
    TOKEN_CSRRD = 290,
    TOKEN_CSRWR = 291,
    TOKEN_GP_REGISTER = 292,
    TOKEN_CS_REGISTER = 293,
    TOKEN_COMMA = 294,
    ENDL = 295,
    TOKEN_COMM = 296,
    TOKEN_IMM = 297,
    TOKEN_LSQB = 298,
    TOKEN_RSQB = 299,
    TOKEN_PLUS = 300,
    TOKEN_SYMBOL = 301,
    TOKEN_LABEL = 302,
    TOKEN_LITERAL = 303,
    TOKEN_STRING = 304
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "misc/parser.y"

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

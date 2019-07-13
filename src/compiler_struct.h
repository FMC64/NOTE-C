
#pragma once

typedef enum {
	CTOKEN_OPERATOR,
	CTOKEN_IDENTIFIER
} CTokenType;

typedef struct {
	CTokenType type;
	const char *str;
	CContext ctx;
} CToken;

typedef struct {
	size_t count;
	size_t allocated;
	CToken *token;
} VecCToken;

typedef struct {
	char *input_file_path;
	int input_file;
	VecCToken tokens;
} CBuf;

typedef struct {
	CBuf buf;
} CParser;

typedef enum {
	CSYMBOL_KEYWORD,
	CSYMBOL_VARIABLE,
	CSYMBOL_TYPE
} CSymbolType;

typedef struct {
	CSymbolType type;
	void *data;
} CSymbol;

// CPP REFERENCE FTW
typedef enum {
	CKEYWORD_NONE,
	CKEYWORD_AUTO, // Not what you'd think, this is not yet a CPP compiler
	CKEYWORD_BREAK,
	CKEYWORD_CASE,
	CKEYWORD_CHAR,
	CKEYWORD_CONST,
	CKEYWORD_CONTINUE,
	CKEYWORD_DEFAULT,
	CKEYWORD_DO,
	CKEYWORD_DOUBLE,
	CKEYWORD_ELSE,
	CKEYWORD_ENUM,
	CKEYWORD_EXTERN,
	CKEYWORD_FLOAT,
	CKEYWORD_FOR,
	CKEYWORD_GOTO,
	CKEYWORD_IF,
	CKEYWORD_INLINE,
	CKEYWORD_INT,
	CKEYWORD_LONG,
	CKEYWORD_REGISTER,
	CKEYWORD_RETURN,
	CKEYWORD_SHORT,
	CKEYWORD_SIGNED,
	CKEYWORD_SIZEOF,
	CKEYWORD_STATIC,
	CKEYWORD_STRUCT,
	CKEYWORD_SWITCH,
	CKEYWORD_TYPEDEF,
	CKEYWORD_UNION,
	CKEYWORD_UNSIGNED,
	CKEYWORD_VOID,
	CKEYWORD_VOLATILE,
	CKEYWORD_WHILE
} CKeyword;

typedef enum {
	CTYPE_SIGNED,
	CTYPE_UNSIGNED,
	CTYPE_CONST,
	CTYPE_STATIC,
	CTYPE_VOLATILE,
	CTYPE_REGISTER
} CTypeFlag;

typedef enum {
	CPRIMITIVE_INT,
	CPRIMITIVE_FLOAT,
	CPRIMIVITE_STRUCT,
	CPRIMITIVE_FUNCTION
} CPrimitiveType;

typedef struct {
	CTypeFlag flags;
	CPrimitiveType primitiveType;
	size_t referenceLevel;
	void *primitiveData;	// Struct members, function prototype
} CType;

typedef struct {
	size_t lol;
} CStruct;

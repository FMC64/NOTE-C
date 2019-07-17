
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
	VecCToken vec;
	size_t i;
} StreamCToken;

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

typedef struct {
	StrSonic symbols;
} CBlock;

typedef struct {
	size_t count;
	CBlock *block;
} CScope;

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
	CTYPE_NONE = 0,
	CTYPE_CONST = 1,
	CTYPE_VOLATILE = 2,
} CTypeFullFlag;

typedef enum {
	CPRIMITIVE_NONE,
	CPRIMITIVE_VOID,
	CPRIMITIVE_UINT,
	CPRIMITIVE_SINT,
	CPRIMITIVE_FLOAT,
	CPRIMIVITE_STRUCT,
	CPRIMITIVE_FUNCTION
} CPrimitiveType;

typedef struct {
	void *data;	// For int / float -> bytes count, struct -> ptr to CStruct, function -> CFunction
	char type;	// CPrimitiveType
	char isDataRef;
} CPrimitive;

typedef struct {
	int isUndef;
	size_t size;
} CArrayColumn;

typedef struct {
	unsigned short level;		// Can't go much shorter because of 32 bits alignment
	unsigned short arrayCount;
	CArrayColumn *array;
} CReference;

typedef struct {
	CTypeFullFlag flags;
	CReference ref;
	CPrimitive primitive;
} CTypeFull;

typedef enum {
	CSTORAGE_DEFAULT,
	CSTORAGE_STATIC,
	CSTORAGE_EXTERN,
	CSTORAGE_AUTO,
	CSTORAGE_REGISTER
} CStorageType;

typedef struct {
	char *name;
	size_t address;
	CStorageType storage;
	CTypeFull *type;
} CVariable;

typedef struct {
	size_t size;
	size_t variableCount;
	CVariable *variable;
} CStruct;	// I think this could be used for unions as well

typedef struct {
	CTypeFull *returnType;
	size_t argCount;
	CTypeFull **arg;
} CFunction;

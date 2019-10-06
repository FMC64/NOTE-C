
#pragma once

typedef enum {
	CTOKEN_NONE,
	CTOKEN_BASIC,
	CTOKEN_STRING_SIMPLE,
	CTOKEN_STRING_DOUBLE,
	CTOKEN_STRING_CHEVRON,
	CTOKEN_MACRO
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
	size_t count;
	size_t allocated;
	VecCToken *vec;
} VecVecCToken;

typedef struct {
	VecCToken vec;
	size_t i;
	size_t flushed_count;
} StreamCToken;

typedef struct {
	size_t i;
	size_t i_file;
	size_t quote_start;
	size_t line;
	size_t line_start;
} CTokenParserState;

#define STREAMCTOKEN_BUFSIZE 256

typedef struct {
	const char *filepath;
	int filehandle;
	int isFileDone;
	char *buf;	// malloc'd to reduce vector size
	CTokenParserState parserState;
} CFile;

typedef struct {
	size_t size;
	CFile *stream;
} VecCFile;

typedef struct {
	int isCurrent;
	int hasPassed;
	int hasElsePassed;
	CContext ctx;
} CMacroStackFrame;

typedef struct {
	size_t count;
	size_t allocated;
	CMacroStackFrame *stack;
} VecCMacroStackFrame;

typedef struct {
	char *filepath;
	StreamCToken tokens;
	VecCToken buf;
	VecCFile streams;
	VecCFile terminatedStreams;	// flushed on each token polling pass
	StrSonic macros;
	VecCMacroStackFrame macroStack;
} CStream;

typedef struct {
	char *input_file_path;
	int input_file;
	VecCToken tokens;
} CBuf;

typedef struct {
	CBuf buf;
} CParser;

typedef enum {
	CSYMBOL_NONE,
	CSYMBOL_KEYWORD,
	CSYMBOL_STRUCT,
	CSYMBOL_VARIABLE,
	CSYMBOL_TYPE,
	CSYMBOL_FUNCTION
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
	struct CTypeFull **type;
} VecCTypeFull;

typedef struct {
	struct CTypeFull *t_void;
	VecCTypeFull t_uint;
	VecCTypeFull t_sint;
	VecCTypeFull t_float;
} CTypeFullCached;

typedef struct {
	size_t blockCount;
	CBlock *block;
	CStream *stream;
	CTypeFullCached cachedTypes;
} CScope;

// CPP REFERENCE FTW
typedef enum {
	CKEYWORD_NONE,


	CKEYWORD_INLINE,  // Type keyword start

	CKEYWORD_AUTO, // Not what you'd think, this is not yet a CPP compiler
	CKEYWORD_EXTERN,
	CKEYWORD_STATIC,
	CKEYWORD_REGISTER,

	CKEYWORD_CONST,
	CKEYWORD_VOLATILE,

	CKEYWORD_VOID,
	CKEYWORD_CHAR,
	CKEYWORD_SHORT,
	CKEYWORD_INT,
	CKEYWORD_UNSIGNED,
	CKEYWORD_SIGNED,
	CKEYWORD_LONG,
	CKEYWORD_FLOAT,
	CKEYWORD_DOUBLE,

	CKEYWORD_ENUM,
	CKEYWORD_STRUCT,
	CKEYWORD_UNION,  // Type keyword end


	CKEYWORD_BREAK,  // Flow keyword start
	CKEYWORD_CASE,
	CKEYWORD_CONTINUE,
	CKEYWORD_DEFAULT,
	CKEYWORD_DO,
	CKEYWORD_ELSE,
	CKEYWORD_FOR,
	CKEYWORD_GOTO,
	CKEYWORD_IF,
	CKEYWORD_RETURN,
	CKEYWORD_SWITCH,
	CKEYWORD_WHILE,  // Flow keyword end


	CKEYWORD_SIZEOF,  // Misc.
	CKEYWORD_TYPEDEF
} CKeyword;

typedef enum {
	CTYPE_NONE = 0,
	CTYPE_CONST = 1,
	CTYPE_VOLATILE = 2,
} CTypeFlag;

typedef enum {
	CPRIMITIVE_NONE,
	CPRIMITIVE_VOID,
	CPRIMITIVE_UINT,
	CPRIMITIVE_SINT,
	CPRIMITIVE_FLOAT,
	CPRIMITIVE_STRUCT,
	CPRIMITIVE_FUNCTION
} CPrimitiveType;

typedef struct {
	void *data;	// For int / float -> bytes count, struct -> ptr to CStruct, function -> CFunction
	char type;      // CPrimitiveType
	char isDataNamed;
} CPrimitive;

typedef struct {
	int isUndef;
	size_t size;
} CArray;

typedef struct {
	unsigned short level;		// Can't go much shorter because of 32 bits alignment
	unsigned short arrayCount;
	CArray *array;
} CReference;

typedef struct {
	size_t count;
	CReference *ref;
} VecCReference;

typedef struct CTypeFull {
	CTypeFlag flags;
	VecCReference refs;
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
	size_t flags : 2;	// CTypeFlag
	size_t isTypeNamed : 1;
	size_t isReferenceSimple : 1;
	size_t referenceLevel : 28;
	CTypeFull *full;
} CType;

typedef struct {
	char *name;
	size_t address;
	CStorageType storage;
	CType type;
} CVariable;

typedef struct {
	char *name;
	size_t offset;
	CType type;
} CStructMember;

typedef struct {
	char *name;
	char isDefined;
	char isUnion;
	size_t size;	// Bytes
	size_t memberCount;
	CStructMember *member;
} CStruct;	// I think this could be used for unions as well

typedef struct {
	CType returnType;
	size_t argCount;
	CType *arg;
} CFunction;

typedef struct {
	size_t argumentCount;
	char *argumentBase;
	char *argumentCur;
	char *tokenBase;
	char *tokenCur;
} CMacro;

typedef struct {
	int isBufferedStream;
	StreamCToken *bufferedStream;
	CStream *stream;
} StreamCTokenPoly;

typedef enum {
	COPERATOR_NONE,

	COPERATOR_INC_POST,
	COPERATOR_DEC_POST,
	COPERATOR_PROC_CALL,
	COPERATOR_ARRAY,
	COPERATOR_STRUCT_MEMBER,
	COPERATOR_STRUCT_PTR_MEMBER,

	COPERATOR_INC_PRE,
	COPERATOR_DEC_PRE,
	COPERATOR_UNARY_PLUS,
	COPERATOR_UNARY_MINUS,
	COPERATOR_LOGICAL_NOT,
	COPERATOR_BITWISE_NOT,
	COPERATOR_CAST,
	COPERATOR_DEREF,
	COPERATOR_REF,
	COPERATOR_SIZEOF,

	COPERATOR_MUL,
	COPERATOR_DIV,
	COPERATOR_MOD,

	COPERATOR_ADD,
	COPERATOR_SUB,

	COPERATOR_SHIFT_LEFT,
	COPERATOR_SHIFT_RIGHT,

	COPERATOR_LESS,
	COPERATOR_LESS_EQ,
	COPERATOR_GREATER,
	COPERATOR_GREATER_EQ,

	COPERATOR_EQ,
	COPERATOR_NOT_EQ,

	COPERATOR_BITWISE_AND,

	COPERATOR_BITWISE_XOR,

	COPERATOR_BITWISE_OR,

	COPERATOR_LOGICAL_AND,

	COPERATOR_LOGICAL_OR,

	COPERATOR_TERNARY,

	COPERATOR_COPY,
	COPERATOR_ADD_COPY,
	COPERATOR_SUB_COPY,
	COPERATOR_MUL_COPY,
	COPERATOR_DIV_COPY,
	COPERATOR_MOD_COPY,
	COPERATOR_SHIFT_LEFT_COPY,
	COPERATOR_SHIFT_RIGHT_COPY,
	COPERATOR_AND_COPY,
	COPERATOR_XOR_COPY,
	COPERATOR_OR_COPY,

	COPERATOR_COMMA
} COperatorType;

typedef struct {
	COperatorType type;
	void *data;
	CContext ctx;	// only for cast
} COperator;

typedef enum {
	CNODE_OP,
	CNODE_VALUE
} CNodeType;

typedef struct {
	CNodeType type;
	void *data;
	uinterval tokens;	// tokens enclosing the node
} CNode;

typedef struct {
	COperator op;
	size_t nodeCount;
	CNode *node;
} CNodeOp;

typedef struct {
	CToken token;
} CNodeValue;

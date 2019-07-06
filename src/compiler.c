
#include "headers.h"

CToken CToken_init(CTokenType type, const char *str, CContext ctx)
{
	CToken res;

	res.type = type;
	res.str = str;
	res.ctx = ctx;
	return res;
}

void CToken_destroy(CToken token)
{
	free(token.str);
}

VecCToken VecCToken_init(void)
{
	VecCToken res;

	res.count = 0;
	res.allocated = 0;
	res.token = NULL;
	return res;
}

void VecCToken_add(VecCToken *vec, CToken to_add)
{
	size_t cur = vec->count++;

	if (vec->count > vec->allocated) {
		vec->allocated += 16;
		vec->token = (CToken*)realloc(vec->token, vec->allocated * sizeof(CToken));
	}
	vec->token[cur] = to_add;
}

void VecCToken_print(VecCToken vec)
{
	size_t i;

	terminal_flush();

	printf_term("Tokens: (%u)\n", vec.count);
	for (i = 0; i < vec.count; i++)
		printf_term("'%s' ", vec.token[i].str);

	terminal_show();
}

void VecCToken_destroy(VecCToken vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		CToken_destroy(vec.token[i]);
	free(vec.token);
}

CBuf CBuf_init(char *input_path)
{
	const FONTCHARACTER *path_real = string_to_fontchar(input_path);
	CBuf res;

	res.input_file_path = strdup(input_path);
	res.input_file = Bfile_OpenFile(path_real, _OPENMODE_READ);
	fx_assert(res.input_file, res.input_file_path);
	res.tokens = VecCToken_init();
	free(path_real);
	return res;
}

static int streq_part_in_arr(const char *str, const char **arr, const char **pfound)
{
	size_t i;

	for (i = 0; arr[i] != NULL; i++)
		if (streq_part(str, arr[i])) {
			if (pfound != NULL)
				*pfound = arr[i];
			return 1;
		}
	return 0;
}

static char lower(char to_lower)
{
	if ((to_lower >= 'A') && (to_lower <= 'Z'))
		to_lower -= 32;
	return to_lower;
}

static int is_letter(char to_test)
{
	to_test = lower(to_test);

	return (to_test >= 'a') && (to_test <= 'z');
}

static int is_digit(char to_test)
{
	return (to_test >= '0') && (to_test <= '9');
}

static int is_identifier(char to_test)
{
	return is_letter(to_test) || is_digit(to_test) || (to_test == '_');
}

static char* get_identifier(char *str, size_t *i)
{
	char *res;
	size_t size;

	for (size = 0; is_identifier(str[*i + size]); size++);

	res = (char*)malloc((size + 1) * sizeof(char));
	memcpy(res, &str[*i], size);
	res[size] = 0;

	*i += size;
	return res;
}

int get_escaped_string(Str str, char **pres, CContext ctx)
{
	if (!Str_escape(str, &str, ctx))
		return 0;
	*pres = Str_to_string(str);
	return 1;
}

static int read_tokens(CBuf *buf, char *str)
{
	char *sep[] = {" ", "\t", "\n", NULL};
	char *op[] = {"->", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^|", "|=",
	"++", "--", "<<", ">>", "&&", "||", "==", "!=", "<=", ">=",
	"<", ">", "=", "!", "&", "^", "|", "~",
	"+", "-", "*", "/", "%" ,
	"(", ")", "[", "]", "{", "}", "?", ":", ";", ",", "#", NULL};
	size_t i = 0;
	int is_quote = 0;
	char quote_char;
	size_t quote_start;
	CContext quote_start_ctx;
	size_t line = 0;
	size_t line_start = 0;
	CContext ctx;
	char *found;

	while (str[i] != 0) {
		ctx = CContext_init(buf->input_file_path, line + 1, i - line_start + 1);
		if ((str[i] == '"') || (str[i] == '\'')) {
			if (!is_quote)
				quote_char = str[i];
			if ((!is_quote) || (str[i] == quote_char)) {
				if (is_quote) {
					i++;
					if (!get_escaped_string(Str_init(i - quote_start, &str[quote_start]), &found, ctx))
						return 0;
					VecCToken_add(&buf->tokens, CToken_init(CTOKEN_IDENTIFIER, found, ctx));
				} else {
					quote_start = i;
					quote_start_ctx = ctx;
				}
				is_quote = !is_quote;
				if (!is_quote)
					continue;
			}
		}
		if (is_quote) {
			i++;
			continue;
		}
		if (str[i] == '\n') {
			line++;
			line_start = i + 1;
		}
		if (streq_part_in_arr(&str[i], sep, &found)) {
			i += strlen(found);
			continue;
		}
		if (streq_part_in_arr(&str[i], op, &found)) {
			VecCToken_add(&buf->tokens,
			CToken_init(CTOKEN_OPERATOR, string_create_from_Str(Str_init_from_string(found)), ctx));
			i += strlen(found);
			continue;
		}
		found = get_identifier(str, &i);
		if (strlen(found) == 0) {
			free(found);
			terminal_flush();

			CContext_print_term(ctx);
			printf_term("Unknown character: '%c'\n", str[i]);

			terminal_show();
			return 0;
		}
		VecCToken_add(&buf->tokens, CToken_init(CTOKEN_IDENTIFIER, found, ctx));
	}
	if (is_quote) {
		terminal_flush();

		printf_term("Unfinished string started at:\n");
		CContext_print_term(quote_start_ctx);
		printf_term("with character: %c\n", quote_char);

		terminal_show();
		return 0;
	}
	VecCToken_print(buf->tokens);
	return 1;
}

int CBuf_readTokens(CBuf *buf)
{
	int size = Bfile_GetFileSize(buf->input_file);
	char *str;
	int res;

	fx_assert(size, buf->input_file_path);
	str = (char*)malloc((size + 1) * sizeof(char*));
	fx_assert(Bfile_ReadFile(buf->input_file, str, size, 0), buf->input_file_path);
	str[size] = 0;

	res = read_tokens(buf, str);

	free(str);
	return res;
}

void CBuf_destroy(CBuf buf)
{
	fx_assert(Bfile_CloseFile(buf.input_file), buf.input_file_path);
	free(buf.input_file_path);
	VecCToken_destroy(buf.tokens);
	return;
}

CParser CParser_init(char *source_path)
{
	CParser res;

	res.buf = CBuf_init(source_path);
	return res;
}

int CParser_exec(CParser *parser)
{
	return CBuf_readTokens(&parser->buf);
}

void CParser_destroy(CParser parser)
{
	CBuf_destroy(parser.buf);
	return;
}

void CCompiler(char *path)
{
	int res;

	CParser parser = CParser_init(path);

	res = CParser_exec(&parser);

	terminal_flush();

	if (res)
		printf_term("File %s compiled.\n", path);
	else
		printf_term("File %s can't be compiled.\n", path);
	terminal_show();

	CParser_destroy(parser);
	return;
}

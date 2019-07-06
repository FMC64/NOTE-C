
#include "headers.h"

CBuf CBuf_init(char *input_path)
{
	const FONTCHARACTER *path_real = string_to_fontchar(input_path);
	CBuf res;

	res.input_file_path = strdup(input_path);
	res.input_file = Bfile_OpenFile(path_real, _OPENMODE_READ);
	fx_assert(res.input_file, res.input_file_path);
	res.tokens = VecStr_init();
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

static int parse_tokens(CBuf *buf, char *str)
{
	char *sep[] = {" ", "\t", "\n", NULL};
	char *op[] = {"->", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^|", "|=",
	"++", "--", "<<", ">>", "&&", "||", "==", "!=", "<=", ">=",
	"<", ">", "=", "!", "&", "^", "|", "~",
	"+", "-", "*", "/", "%" ,
	"(", ")", "[", "]", "{", "}", "?", ":", ";", ",", "#", NULL};
	size_t i = 0;
	int is_quote = 0;
	size_t quote_start;
	size_t line = 0;
	size_t line_start = 0;
	char *found;

	while (str[i] != 0) {
		if (str[i] == '"') {
			if (is_quote)
				VecStr_add(&buf->tokens, string_create_from_Str(Str_init(i - quote_start, &str[quote_start])));
			else
				quote_start = i;
			is_quote = !is_quote;
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
			VecStr_add(&buf->tokens, string_create_from_Str(Str_init_from_string(found)));
			i += strlen(found);
			continue;
		}
		found = get_identifier(str, &i);
		if (strlen(found) == 0) {
			free(found);
			terminal_flush();

			printf_term("Line %u(%u)\nUnknown character: '%c'\n", line + 1, i - line_start + 1, str[i]);

			terminal_show();
			return 0;
		}
		VecStr_add(&buf->tokens, found);
	}
	VecStr_print(buf->tokens);
	return 1;
}

int CBuf_getTokens(CBuf *buf)
{
	int size = Bfile_GetFileSize(buf->input_file);
	char *str;
	int res;

	fx_assert(size, buf->input_file_path);
	str = (char*)malloc((size + 1) * sizeof(char*));
	fx_assert(Bfile_ReadFile(buf->input_file, str, size, 0), buf->input_file_path);
	str[size] = 0;

	res = parse_tokens(buf, str);

	free(str);
	return res;
}

void CBuf_destroy(CBuf buf)
{
	fx_assert(Bfile_CloseFile(buf.input_file), buf.input_file_path);
	free(buf.input_file_path);
	VecStr_destroy(buf.tokens);
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
	return CBuf_getTokens(&parser->buf);
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

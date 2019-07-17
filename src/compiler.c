
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

	printf("Tokens: (%u)\n", vec.count);
	for (i = 0; i < vec.count; i++)
		printf("'%s' ", vec.token[i].str);

	terminal_show();
}

int VecCToken_at(VecCToken vec, size_t i, CToken *pres)
{
	if (i < vec.count) {
		if (pres != NULL)
			*pres = vec.token[i];
		return 1;
	}
	return 0;
}

void VecCToken_destroy(VecCToken vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		CToken_destroy(vec.token[i]);
	free(vec.token);
}

StreamCToken StreamCToken_init(VecCToken vec)
{
	StreamCToken res;

	res.vec = vec;
	res.i = 0;
	return res;
}

void StreamCToken_begin(StreamCToken *stream)
{
	stream->i = 0;
}

void StreamCToken_end(StreamCToken *stream)
{
	if (stream->vec.count > 0)
		stream->i = stream->vec.count - 1;
	else
		stream->i = 0;
}

int StreamCToken_forward(StreamCToken *stream)
{
	stream->i++;
}

int StreamCToken_back(StreamCToken *stream)
{
	stream->i--;
}

int StreamCToken_at(StreamCToken *stream, CToken *pres)
{
	return VecCToken_at(stream->vec, stream->i, pres);
}

int StreamCToken_poll(StreamCToken *stream, CToken *pres)
{
	int res;

	res = StreamCToken_at(stream, pres);
	if (res)
		StreamCToken_forward(stream);
	return res;
}

int StreamCToken_pollRev(StreamCToken *stream, CToken *pres)
{
	int res;

	res = StreamCToken_at(stream, pres);
	if (res)
		StreamCToken_back(stream);
	return res;
}

CContext StreamCToken_lastCtx(StreamCToken *stream)
{
	if (stream->vec.count > 0)
		return stream->vec.token[stream->vec.count - 1].ctx;
	else
		return CContext_null();
}

int StreamCToken_pollStr(StreamCToken *tokens, const char *str, CContext *ctx)
{
	CToken cur;
	int res;

	if (!StreamCToken_at(tokens, &cur)) {
		if (ctx != NULL)
			*ctx = StreamCToken_lastCtx(tokens);
		return 0;
	}
	if (ctx != NULL)
		*ctx = cur.ctx;
	res = streq(cur.str, str);
	if (res)
		StreamCToken_forward(tokens);
	return res;
}

int StreamCToken_pollLpar(StreamCToken *tokens, CContext *ctx)
{
	return StreamCToken_pollStr(tokens, "(", ctx);
}

int StreamCToken_pollRpar(StreamCToken *tokens, CContext *ctx)
{
	return StreamCToken_pollStr(tokens, ")", ctx);
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

char char_lower(char to_lower)
{
	if ((to_lower >= 'A') && (to_lower <= 'Z'))
		to_lower -= 32;
	return to_lower;
}

int char_is_letter(char to_test)
{
	to_test = char_lower(to_test);

	return (to_test >= 'a') && (to_test <= 'z');
}

int char_is_digit(char to_test)
{
	return (to_test >= '0') && (to_test <= '9');
}

int char_is_identifier(char to_test)
{
	return char_is_letter(to_test) || char_is_digit(to_test) || (to_test == '_');
}

int str_is_identifier(const char *str)
{
	size_t i;

	if (char_is_digit(str[0]))
		return 0;
	for (i = 0; str[i] != 0; i++)
		if (!char_is_identifier(str[i]))
			return 0;
	return 1;
}

static char* get_identifier(char *str, size_t *i)
{
	char *res;
	size_t size;

	for (size = 0; char_is_identifier(str[*i + size]); size++);

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
	int is_comment = 0;
	int is_comment_single_line = 0;
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
		if (str[i] == '\n') {
			line++;
			line_start = i + 1;
			is_comment_single_line = 0;
		}
		if (is_comment_single_line) {
			i++;
			continue;
		}
		if (is_comment) {
			if (streq_part(&str[i], "*/")) {
				is_comment = 0;
				i += 2;
				continue;
			}
			i++;
			continue;
		}
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
		if (streq_part(&str[i], "//")) {
			is_comment_single_line = 1;
			i += 2;
			continue;
		}
		if (streq_part(&str[i], "/*")) {
			is_comment = 1;
			i += 2;
			continue;
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

			CContext_print(ctx);
			printf("Unknown character: '%c'\n", str[i]);

			terminal_show();
			return 0;
		}
		VecCToken_add(&buf->tokens, CToken_init(CTOKEN_IDENTIFIER, found, ctx));
	}
	if (is_quote) {
		terminal_flush();

		printf("Unfinished string started at:\n");
		CContext_print(quote_start_ctx);
		printf("with character: %c\n", quote_char);

		terminal_show();
		return 0;
	}
	//VecCToken_print(buf->tokens);
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

void CCompiler(char *path)
{
	int res;

	CParser parser = CParser_init(path);

	res = CParser_exec(&parser);

	printf("\n");
	if (res)
		printf("%s compiled.\n", path);
	else
		printf("%s can't be compiled.\n", path);
	terminal_show();

	CParser_destroy(parser);
	return;
}

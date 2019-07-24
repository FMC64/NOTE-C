
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

void VecCToken_flush(VecCToken *vec)
{
	size_t i;

	for (i = 0; i < vec->count; i++)
		CToken_destroy(vec->token[i]);
	free(vec->token);
	vec->count = 0;
	vec->token = NULL;
}

void VecCToken_moveArea(VecCToken *src, size_t src_start, size_t src_size, VecCToken *dst)
{
	size_t i;

	for (i = 0; i < src_size; i++)
		VecCToken_add(dst, src->token[src_start + i]);
	src->count -= src_size;
	for (i = src_start; i < src->count; i++)
		src->token[i] = src->token[i + src_size];
}

void VecCToken_merge(VecCToken *dst, VecCToken *to_append)
{
	VecCToken_moveArea(to_append, 0, to_append->count, dst);
	VecCToken_destroy(*to_append);
}

void VecCToken_destroy(VecCToken vec)
{
	VecCToken_flush(&vec);
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

	return res;
}

int get_escaped_string(Str str, char **pres, CContext ctx)
{
	if (!Str_escape(str, &str, ctx))
		return 0;
	*pres = Str_to_string(str);
	return 1;
}

static void CTokenParserState_forward(CTokenParserState *s, size_t off)
{
	s->i += off;
	s->i_file += off;
}

static int StreamCToken_pollFile(StreamCToken *stream)
{
	if (stream->isFileDone)
		return 1;
	memcpy(stream->buf, &stream->buf[STREAMCTOKEN_BUFSIZE], STREAMCTOKEN_BUFSIZE);
	if (!StreamCToken_pollFileBytes(stream, STREAMCTOKEN_BUFSIZE, STREAMCTOKEN_BUFSIZE))
		return 0;
	stream->parserState.i -= STREAMCTOKEN_BUFSIZE;
	return 1;
}

int StreamCToken_readToken(StreamCToken *stream, CToken *pres, int *is_err)
{
	char *sep[] = {" ", "\t", "\n", NULL};
	char *op[] = {"->", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^|", "|=",
	"++", "--", "<<", ">>", "&&", "||", "==", "!=", "<=", ">=",
	"<", ">", "=", "!", "&", "^", "|", "~",
	"+", "-", "*", "/", "%" ,
	"(", ")", "[", "]", "{", "}", "?", ":", ";", ",", "#", NULL};
	CTokenParserState *s = &stream->parserState;
	CContext ctx;
	char *found;

	*is_err = 0;
	while (stream->buf[s->i] != 0) {
		if ((s->i >= STREAMCTOKEN_BUFSIZE) && (!s->is_quote))
			if (!StreamCToken_pollFile(stream)) {
				*is_err = 1;
				return 0;
			}
		ctx = CContext_init(stream->filepath, s->line + 1, s->i_file - s->line_start + 1);
		if (stream->buf[s->i] == '\n') {
			s->line++;
			s->line_start = s->i_file + 1;
			s->is_comment_single_line = 0;
		}
		if (s->is_comment_single_line) {
			CTokenParserState_forward(s, 1);
			continue;
		}
		if (s->is_comment) {
			if (streq_part(&stream->buf[s->i], "*/")) {
				s->is_comment = 0;
				CTokenParserState_forward(s, 2);
				continue;
			}
			s->i++;
			continue;
		}
		if ((stream->buf[s->i] == '"') || (stream->buf[s->i] == '\'')) {
			if (!s->is_quote)
				s->quote_char = stream->buf[s->i];
			if ((!s->is_quote) || (stream->buf[s->i] == s->quote_char)) {
				if (s->is_quote) {
					CTokenParserState_forward(s, 1);
					if (!get_escaped_string(Str_init(s->i - s->quote_start, &stream->buf[s->quote_start]), &found, ctx)) {
						*is_err = 1;
						return 0;
					}
					*pres = CToken_init(CTOKEN_IDENTIFIER, found, ctx);
				} else {
					s->quote_start = s->i;
					s->quote_start_ctx = ctx;
				}
				s->is_quote = !s->is_quote;
				if (!s->is_quote)
					return 1;
			}
		}
		if (s->is_quote) {
			CTokenParserState_forward(s, 1);
			continue;
		}
		if (streq_part(&stream->buf[s->i], "//")) {
			s->is_comment_single_line = 1;
			CTokenParserState_forward(s, 2);
			continue;
		}
		if (streq_part(&stream->buf[s->i], "/*")) {
			s->is_comment = 1;
			CTokenParserState_forward(s, 2);
			continue;
		}
		if (streq_part_in_arr(&stream->buf[s->i], sep, &found)) {
			CTokenParserState_forward(s, strlen(found));
			continue;
		}
		if (streq_part_in_arr(&stream->buf[s->i], op, &found)) {
			*pres = CToken_init(CTOKEN_OPERATOR, string_create_from_Str(Str_init_from_string(found)), ctx);
			CTokenParserState_forward(s, strlen(found));
			return 1;
		}
		found = get_identifier(stream->buf, &s->i);
		if (strlen(found) == 0) {
			free(found);
			printf_error(ctx, "Unknown character: '%c'\n", stream->buf[s->i]);
			*is_err = 1;
			return 0;
		}
		CTokenParserState_forward(s, strlen(found));
		*pres = CToken_init(CTOKEN_IDENTIFIER, found, ctx);
		return 1;
	}
	if (s->is_quote) {
		printf("Unfinished string started at:\n");
		CContext_print(s->quote_start_ctx);
		printf("with character: %c\n", s->quote_char);
		*is_err = 1;
		return 0;
	}
	return 0;
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

	//res = read_tokens(buf, str);

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

void CCompiler(const char *path)
{
	int res;

	res = CParser_exec(path);

	printf("\n");
	if (res)
		printf("%s compiled.\n", path);
	else
		printf("%s can't be compiled.\n", path);
	terminal_show();

	return;
}

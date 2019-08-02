
#include "headers.h"

CToken CToken_init(CTokenType type, const char *str, CContext ctx)
{
	CToken res;

	res.type = type;
	res.str = str;
	res.ctx = ctx;
	return res;
}

CToken CToken_dup(CToken src)
{
	CToken res;
	size_t size;

	res = src;
	if (CToken_isString(src)) {
		size = CToken_stringSize(src);
		res.str = (char*)malloc(size);
		memcpy(res.str, src.str, size);
	} else
		res.str = strdup(src.str);
	return res;
}

int CTokenType_isString(CTokenType type)
{
	return (type >= CTOKEN_STRING_SIMPLE) && (type <= CTOKEN_STRING_CHEVRON);
}

int CToken_isString(CToken token)
{
	return CTokenType_isString(token.type);
}

size_t CToken_stringSize(CToken token)
{
	return sizeof(size_t) + *(size_t*)token.str;
}

int CToken_isIdentifier(CToken token)
{
	if (token.type != CTOKEN_BASIC)
		return 0;
	return str_is_identifier(token.str);
}

int CToken_streq(CToken token, const char *str)
{
	if (CToken_isString(token))
		return 0;
	return streq(token.str, str);
}

int CToken_isEndBatch(CToken token)
{
	return CToken_streq(token, ";");
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

	printf("Tokens: (%u)\n", vec.count);
	for (i = 0; i < vec.count; i++) {
		if (CToken_isString(vec.token[i])) {
			printf("'");
			Str_print(Str_init_from_CToken(vec.token[i]));
			printf("' ");
		} else
			printf("'%s' ", vec.token[i].str);
	}
	printf("\n");
}

void VecCToken_display(VecCToken vec)
{
	terminal_flush();
	VecCToken_print(vec);
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
	vec->allocated = 0;
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

VecCToken VecCToken_offset(VecCToken vec, size_t off)
{
	VecCToken res = vec;

	res.count -= off;
	res.allocated -= off;
	res.token = &res.token[off];
	return res;
}

void VecCToken_destroy(VecCToken vec)
{
	VecCToken_flush(&vec);
}

VecVecCToken VecVecCToken_init(void)
{
	VecVecCToken res;

	res.count = 0;
	res.allocated = 0;
	res.vec = NULL;
	return res;
}

void VecVecCToken_add(VecVecCToken *vec, VecCToken to_add)
{
	size_t cur = vec->count++;

	if (vec->count > vec->allocated) {
		vec->allocated += 4;
		vec->vec = (VecCToken*)realloc(vec->vec, vec->allocated * sizeof(VecCToken));
	}
	vec->vec[cur] = to_add;
}

void VecVecCToken_print(VecVecCToken vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		VecCToken_print(vec.vec[i]);
}

void VecVecCToken_display(VecVecCToken vec)
{
	terminal_flush();
	VecVecCToken_print(vec);
	terminal_show();
}

void VecVecCToken_destroy(VecVecCToken vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		VecCToken_destroy(vec.vec[i]);
	free(vec.vec);
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
		to_lower += 32;
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

static int get_escaped_string(Str str, char **pres, size_t *res_size, CContext ctx)
{
	if (!Str_escape(str, &str, ctx))
		return 0;
	*res_size = str.size;
	Str_prepend(&str, Str_init(sizeof(size_t), (char*)res_size));
	*pres = Str_to_string(str);
	return 1;
}

static void CTokenParserState_forward(CTokenParserState *s, size_t off)
{
	s->i += off;
	s->i_file += off;
}

static int CFile_pollFile(CFile *stream)
{
	if (stream->isFileDone)
		return 1;
	memcpy(stream->buf, &stream->buf[STREAMCTOKEN_BUFSIZE], STREAMCTOKEN_BUFSIZE);
	if (!CFile_pollFileBytes(stream, STREAMCTOKEN_BUFSIZE, STREAMCTOKEN_BUFSIZE))
		return 0;
	stream->parserState.i -= STREAMCTOKEN_BUFSIZE;
	stream->parserState.quote_start = stream->parserState.i;
	return 1;
}

static char *sep[] = {" ", "\t", "\n", NULL};
static char *op[] = {"->", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^|", "|=",
"++", "--", "<<", ">>", "&&", "||", "==", "!=", "<=", ">=",
"<", ">", "=", "!", "&", "^", "|", "~",
"+", "-", "*", "/", "%" ,
"(", ")", "[", "]", "{", "}", "?", ":", ";", ",", NULL};
static char *op_macro[] = {"##", NULL};

static CTokenType get_string_type(char source)
{
	switch (source) {
	case '\'':
		return CTOKEN_STRING_SIMPLE;
	case '"':
		return CTOKEN_STRING_DOUBLE;
	case '<':
	case '>':
		return CTOKEN_STRING_CHEVRON;
	default:
		return CTOKEN_NONE;
	}
}

int VecCToken_from_CToken(const CToken src, VecCToken *pres)
{
	VecCToken res = VecCToken_init();
	const char *str = src.str;
	CContext ctx;
	size_t i = 0;
	int is_comment = 0;
	int is_quote = 0;
	char quote_char;
	size_t quote_start;
	CContext quote_start_ctx;
	char *found;
	size_t esc_size;

	while (str[i] != 0) {
		ctx = src.ctx;
		ctx.colon += i;
		if (is_comment) {
			if (streq_part(&str[i], "*/")) {
				is_comment = 0;
				i += 2;
				continue;
			}
			i++;
			continue;
		}
		if ((str[i] == '"') || (str[i] == '\'') || (str[i] == '<') || (str[i] == '>')) {
			if (!is_quote)
				quote_char = str[i] == '<' ? '>' : str[i];
			if ((!is_quote) || (str[i] == quote_char)) {
				if (is_quote) {
					i++;
					if (!get_escaped_string(Str_init(i - quote_start - 1, &str[quote_start]), &found, &esc_size, quote_start_ctx))
						goto VecCToken_from_CTokens_end_error;
					if (quote_char == '\'') {
						if (esc_size > 1) {
							printf_error(quote_start_ctx, "only one character can fit in this. you tried to stuff %u instead.", esc_size);
							free(found);
							goto VecCToken_from_CTokens_end_error;
						}
						if (esc_size == 0) {
							printf_error(quote_start_ctx, "you forgot to put a character there !");
							free(found);
							goto VecCToken_from_CTokens_end_error;
						}
					}
					VecCToken_add(&res, CToken_init(get_string_type(quote_char), found, quote_start_ctx));
				} else {
					quote_start = i + 1;
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
		if (streq_part(&str[i], "//"))
			break;
		if (streq_part(&str[i], "/*")) {
			is_comment = 1;
			continue;
		}
		if (streq_part_in_arr(&str[i], sep, &found)) {
			i += strlen(found);
			continue;
		}
		if (streq_part_in_arr(&str[i], op, &found)) {
			VecCToken_add(&res, CToken_init(CTOKEN_BASIC, string_create_from_Str(Str_init_from_string(found)), ctx));
			i += strlen(found);
			continue;
		}
		if (streq_part_in_arr(&str[i], op_macro, &found)) {
			VecCToken_add(&res, CToken_init(CTOKEN_BASIC, string_create_from_Str(Str_init_from_string(found)), ctx));
			i += strlen(found);
			continue;
		}
		found = get_identifier(str, &i);
		if (strlen(found) == 0) {
			free(found);
			printf_error(ctx, "unknown character: '%c'\n", str[i]);
			goto VecCToken_from_CTokens_end_error;
		}
		i += strlen(found);
		VecCToken_add(&res, CToken_init(CTOKEN_BASIC, found, ctx));
	}
	if (is_quote) {
		printf_error(quote_start_ctx, "unfinished string started with character: %c\n", quote_char);
		goto VecCToken_from_CTokens_end_error;
	}
	*pres = res;
	return 1;

VecCToken_from_CTokens_end_error:
	VecCToken_destroy(res);
	return 0;
}

int CFile_readToken(CFile *stream, CToken *pres, int *is_err)
{
	CTokenParserState *s = &stream->parserState;
	CContext ctx;
	char *found;
	int isMacro = 0;
	size_t esc_size;
	Str acc;

	*is_err = 0;
	while (stream->buf[s->i] != 0) {
		if (s->i >= STREAMCTOKEN_BUFSIZE) {
			if (s->is_quote || isMacro)
				Str_append(&acc, Str_init(s->i - s->quote_start, &stream->buf[s->quote_start]));
			if (!CFile_pollFile(stream)) {
				if (s->is_quote || isMacro)
					Str_destroy(acc);
				*is_err = 1;
				return 0;
			}
		}
		ctx = CContext_init(stream->filepath, s->line + 1, s->i_file - s->line_start + 1);
		if (stream->buf[s->i] == '\n') {
			s->line++;
			s->line_start = s->i_file + 1;
			s->is_comment_single_line = 0;
		}
		if (isMacro) {
			if (stream->buf[s->i] == '\n') {
				Str_append(&acc, Str_init(s->i - s->quote_start, &stream->buf[s->quote_start]));
				*pres = CToken_init(CTOKEN_MACRO, string_create_from_Str(acc), s->quote_start_ctx);
				Str_destroy(acc);
				CTokenParserState_forward(s, 1);
				return 1;
			}
			CTokenParserState_forward(s, 1);
			continue;
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
			CTokenParserState_forward(s, 1);
			continue;
		}
		if ((stream->buf[s->i] == '"') || (stream->buf[s->i] == '\'')) {
			if (!s->is_quote)
				s->quote_char = stream->buf[s->i];
			if ((!s->is_quote) || (stream->buf[s->i] == s->quote_char)) {
				if (s->is_quote) {
					CTokenParserState_forward(s, 1);
					Str_append(&acc, Str_init(s->i - s->quote_start - 1, &stream->buf[s->quote_start]));
					if (!get_escaped_string(acc, &found, &esc_size, s->quote_start_ctx)) {
						Str_destroy(acc);
						*is_err = 1;
						return 0;
					}
					Str_destroy(acc);
					if (s->quote_char == '\'') {
						if (esc_size > 1) {
							printf_error(s->quote_start_ctx, "only one character can fit in this. you tried to stuff %u instead.", esc_size);
							free(found);
							*is_err = 1;
							return 0;
						}
						if (esc_size == 0) {
							printf_error(s->quote_start_ctx, "you forgot to put a character there !");
							free(found);
							*is_err = 1;
							return 0;
						}
					}
					*pres = CToken_init(get_string_type(s->quote_char), found, s->quote_start_ctx);
				} else {
					s->quote_start = s->i + 1;
					acc = Str_empty();
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
		if (stream->buf[s->i] == '#') {
			s->quote_start = s->i + 1;
			acc = Str_empty();
			isMacro = 1;
			s->quote_start_ctx = ctx;
			s->quote_start_ctx.colon++;
			continue;
		}
		if (streq_part_in_arr(&stream->buf[s->i], sep, &found)) {
			CTokenParserState_forward(s, strlen(found));
			continue;
		}
		if (streq_part_in_arr(&stream->buf[s->i], op, &found)) {
			*pres = CToken_init(CTOKEN_BASIC, strdup(found), ctx);
			CTokenParserState_forward(s, strlen(found));
			return 1;
		}
		found = get_identifier(stream->buf, &s->i);
		if (strlen(found) == 0) {
			free(found);
			printf_error(ctx, "unknown character: '%c'\n", stream->buf[s->i]);
			*is_err = 1;
			return 0;
		}
		CTokenParserState_forward(s, strlen(found));
		*pres = CToken_init(CTOKEN_BASIC, found, ctx);
		return 1;
	}
	if (s->is_quote) {
		Str_destroy(acc);
		printf_error(s->quote_start_ctx, "unfinished string started with character: %c\n", s->quote_char);
		*is_err = 1;
		return 0;
	}
	if (isMacro) {
		Str_append(&acc, Str_init(s->i - s->quote_start, &stream->buf[s->quote_start]));
		*pres = CToken_init(CTOKEN_MACRO, string_create_from_Str(acc), ctx);
		Str_destroy(acc);
		return 1;
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

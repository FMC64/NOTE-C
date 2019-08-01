
#include "headers.h"

static CTokenParserState CTokenParserState_init(void)
{
	CTokenParserState res;

	res.i = 0;
	res.i_file = 0;
	res.is_comment = 0;
	res.is_comment_single_line = 0;
	res.is_quote = 0;
	res.line = 0;
	res.line_start = 0;
	return res;
}

int CFile_create(const char *filepath, CFile *pres)
{
	CFile res;
	const FONTCHARACTER *path_real = string_to_fontchar(filepath);

	res.filehandle = Bfile_OpenFile(path_real, _OPENMODE_READ);
	free(path_real);
	if (res.filehandle < 0) {
		printf_error(CContext_init(filepath, 0, 0), "file cannot be opened: %s", IML_FILLEERR_str(res.filehandle));
		return 0;
	}
	res.filepath = strdup(filepath);
	res.isFileDone = 0;
	res.buf = (char*)malloc(STREAMCTOKEN_BUFSIZE * 2);
	CFile_pollFileBytes(&res, 0, STREAMCTOKEN_BUFSIZE * 2);
	res.parserState = CTokenParserState_init();
	*pres = res;
	return 1;
}

static int escape_raw(CFile *stream, size_t start, size_t size)
{
	size_t i;
	size_t j;
	char next;

	for (i = 0; i < size; i++)
		if (stream->buf[start + i] == '\\') {
			next = stream->buf[start + i + 1];
			if (next == 0) {
				if (Bfile_ReadFile(stream->filehandle, &next, 1, -1) != 1) {
					printf_error(CContext_init(stream->filepath, 0, 0), "last file character is a \\");
					return 0;
				}
			}
			if (next == '\n') {
				for (j = i; j + 2 < size; j++)	// delete both \ and linefeed
					stream->buf[start + j] = stream->buf[start + j + 2];
				if (!CFile_pollFileBytes(stream, start + size - 2, 2))	// feed 2 missing bytes at the end of the buffer
					return 0;
			}
		}
	return 1;
}

int CFile_pollFileBytes(CFile *stream, size_t buf_start, size_t size)
{
	int got;

	got = Bfile_ReadFile(stream->filehandle, &stream->buf[buf_start], size, -1);
	if (got < 0) {
		printf_error(CContext_init(stream->filepath, 0, 0), "file cannot be read: %s", IML_FILLEERR_str(got));
		return 0;
	}
	stream->buf[buf_start + got] = 0;
	if (got < size)
		stream->isFileDone = 1;
	if (!escape_raw(stream, buf_start, got))
		return 0;
	return 1;
}

void CFile_destroy(CFile stream)
{
	int status = Bfile_CloseFile(stream.filehandle);

	if (status < 0)
		printf_error(CContext_init(stream.filepath, 0, 0), "file cannot be closed: %s", IML_FILLEERR_str(status));
	free(stream.filepath);
	free(stream.buf);
}

VecCFile VecCFile_init(void)
{
	VecCFile res;

	res.size = 0;
	res.stream = NULL;
	return res;
}

void VecCFile_add(VecCFile *vec, CFile to_add)
{
	size_t cur = vec->size++;

	vec->stream = (CFile*)realloc(vec->stream, vec->size * sizeof(CFile));
	vec->stream[cur] = to_add;
}

void VecCFile_flush(VecCFile *vec)
{
	size_t i;

	for (i = 0; i < vec->size; i++)
		CFile_destroy(vec->stream[i]);
	free(vec->stream);
	vec->size = 0;
	vec->stream = NULL;
}

void VecCFile_move(VecCFile *src, size_t src_ndx, VecCFile *dst)
{
	VecCFile_moveArea(src, src_ndx, 1, dst);
}

void VecCFile_moveArea(VecCFile *src, size_t src_start, size_t src_size, VecCFile *dst)
{
	size_t i;

	for (i = 0; i < src_size; i++)
		VecCFile_add(dst, src->stream[src_start + i]);
	src->size -= src_size;
	for (i = src_start; i < src->size; i++)
		src->stream[i] = src->stream[i + src_size];
}

void VecCFile_destroy(VecCFile vec)
{
	VecCFile_flush(&vec);
}

int CStream_create(const char *filepath, CStream **pres)
{
	CStream *res;
	CFile file;

	if (!CFile_create(filepath, &file))
		return 0;
	res = (CStream*)malloc(sizeof(CStream));
	res->tokens = StreamCToken_init(VecCToken_init());
	res->buf = VecCToken_init();
	res->streams = VecCFile_init();
	res->terminatedStreams = VecCFile_init();
	res->macros = StrSonic_init(&StrSonic_CMacro_destroy);
	VecCFile_add(&res->streams, file);
	*pres = res;
	return 1;
}

void CStream_destroy(CStream *stream)
{
	StreamCToken_destroy(stream->tokens);
	VecCToken_destroy(stream->buf);
	VecCFile_destroy(stream->streams);
	VecCFile_destroy(stream->terminatedStreams);
	StrSonic_destroy(&stream->macros);
	free(stream);
}

int CStream_currentStream(CStream *stream, CFile **pres)
{
	if (stream->streams.size == 0)
		return 0;
	*pres = &stream->streams.stream[stream->streams.size - 1];
	return 1;
}

static int is_token_end_batch(CToken token)
{
	return streq(token.str, ";");
}

static int poll_tokens(CStream *stream, VecCToken *pres)
{
	VecCToken res = VecCToken_init();
	CFile *to_poll;
	CToken cur;
	int is_err;

	while (1) {
		if (!CStream_currentStream(stream, &to_poll)) {
			*pres = res;
			return 1;
		}
		if (!CFile_readToken(to_poll, &cur, &is_err)) {
			if (is_err) {
				VecCToken_destroy(res);
				return 0;
			}
			VecCFile_move(&stream->streams, stream->streams.size - 1, &stream->terminatedStreams);
		} else {
			if (cur.type == CTOKEN_MACRO) {
				if (!CStream_parseMacro(stream, cur)) {
					CToken_destroy(cur);
					VecCToken_destroy(res);
					return 0;
				}
				CToken_destroy(cur);
			} else if (CStream_canAddToken(stream)){
				VecCToken_add(&res, cur);
				if (is_token_end_batch(cur))
					break;
			}
		}
	}
	*pres = res;
	return 1;
}

static int feed_tokens(CStream *stream)
{
	VecCToken new_tokens;
	size_t i;

	if (!poll_tokens(stream, &new_tokens))
		return 0;
	VecCToken_merge(&stream->buf, &new_tokens);
	return 1;
}

static int get_first_ending_token(VecCToken buf, size_t *pres)
{
	size_t i;

	for (i = 0; i < buf.count; i++)
		if (is_token_end_batch(buf.token[i])) {
			*pres = i;
			return 1;
		}
	return 0;
}

// Token batches are ended by a ; or {
int CStream_nextBatch(CStream *stream)
{
	CToken cur;
	size_t end;

	VecCFile_flush(&stream->terminatedStreams);
	StreamCToken_flush(&stream->tokens);
	if (!get_first_ending_token(stream->buf, &end))
		if (!feed_tokens(stream))
			return 0;
	if (!get_first_ending_token(stream->buf, &end))
		end = stream->buf.count - 1;
	VecCToken_moveArea(&stream->buf, 0, end + 1, &stream->tokens.vec);
	return 1;
}

int CStream_isEof(CStream *stream)
{
	return stream->tokens.vec.count == 0;
}

StreamCToken StreamCToken_init(VecCToken vec)
{
	StreamCToken res;

	res.vec = vec;
	res.i = 0;
	return res;
}

void StreamCToken_flush(StreamCToken *stream)
{
	VecCToken_flush(&stream->vec);
	stream->i = 0;
}

StreamCToken StreamCToken_offset(StreamCToken *stream)
{
	StreamCToken res;

	res.i = 0;
	res.vec = VecCToken_offset(stream->vec, stream->i);
	return res;
}

void StreamCToken_destroy(StreamCToken stream)
{
	VecCToken_destroy(stream.vec);
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

CContext StreamCToken_atCtx(StreamCToken *stream)
{
	if (stream->i < stream->vec.count)
		return stream->vec.token[stream->i].ctx;
	else
		return StreamCToken_lastCtx(stream);
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
	if (cur.type != CTOKEN_BASIC)
		return 0;
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

void CStream_begin(CStream *stream)
{
	StreamCToken_begin(&stream->tokens);
}

void CStream_end(CStream *stream)
{
	StreamCToken_end(&stream->tokens);
}

int CStream_forward(CStream *stream)
{
	return StreamCToken_forward(&stream->tokens);
}

int CStream_back(CStream *stream)
{
	return StreamCToken_back(&stream->tokens);
}

int CStream_at(CStream *stream, CToken *pres)
{
	return StreamCToken_at(&stream->tokens, pres);
}

int CStream_poll(CStream *stream, CToken *pres)
{
	return StreamCToken_poll(&stream->tokens, pres);
}

int CStream_pollRev(CStream *stream, CToken *pres)
{
	return StreamCToken_pollRev(&stream->tokens, pres);
}

CContext CStream_lastCtx(CStream *stream)
{
	return StreamCToken_lastCtx(&stream->tokens);
}

CContext CStream_atCtx(CStream *stream)
{
	return StreamCToken_atCtx(&stream->tokens);
}

int CStream_pollStr(CStream *stream, const char *str, CContext *ctx)
{
	return StreamCToken_pollStr(&stream->tokens, str, ctx);
}

int CStream_pollLpar(CStream *stream, CContext *ctx)
{
	return StreamCToken_pollLpar(&stream->tokens, ctx);
}

int CStream_pollRpar(CStream *stream, CContext *ctx)
{
	return StreamCToken_pollRpar(&stream->tokens, ctx);
}

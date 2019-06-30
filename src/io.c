
#include "headers.h"

// Unsafe function, to use with care and never let any uncontrolled user input go there
int printf(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	int res;

	va_start(args, fmt);
	res = vsprintf(buf, fmt, args);
	Print((unsigned char*)buf);
	va_end(args);
	return res;
}

FONTCHARACTER* string_to_fontchar(const char *src)
{
	size_t len = strlen(src);
	FONTCHARACTER *res = malloc((len + 1) * sizeof(FONTCHARACTER));
	size_t i;

	for (i = 0; i < len; i++)
		res[i] = src[i];
	res[len] = 0;
	return res;
}

const char* file_shortpath(const char *src)
{
	size_t i = strlen(src);

	for (; i > 0; i--)
		if ((src[i] == '/') || (src[i] == '\\')) {
			i++;
			break;
		}
	return &src[i];
}

static const char* IML_FILLEERR_string(int code)
{
	switch (code)
	{
	case IML_FILEERR_ENTRYNOTFOUND:
		return "ENTRYNOTFOUND";
	case IML_FILEERR_ILLEGALPARAM:
		return "ILLEGALPARAM";
	case IML_FILEERR_ILLEGALPATH:
		return "ILLEGALPATH";
	case IML_FILEERR_DEVICEFULL:
		return "DEVICEFULL";
	case IML_FILEERR_ILLEGALDEVICE:
		return "ILLEGALDEVICE";
	case IML_FILEERR_ILLEGALFILESYS:
		return "ILLEGALFILESYS";
	case IML_FILEERR_ILLEGALSYSTEM:
		return "ILLEGALSYSTEM";
	case IML_FILEERR_ACCESSDENYED:
		return "ACCESSDENYED";
	case IML_FILEERR_ALREADYLOCKED:
		return "ALREADYLOCKED";
	case IML_FILEERR_ILLEGALTASKID:
		return "ILLEGALTASKID";
	case IML_FILEERR_PERMISSIONERROR:
		return "PERMISSIONERROR";
	case IML_FILEERR_ENTRYFULL:
		return "ENTRYFULL";
	case IML_FILEERR_ALREADYEXISTENTRY:
		return "ALREADYEXISTENTRY";
	case IML_FILEERR_READONLYFILE:
		return "READONLYFILE";
	case IML_FILEERR_ILLEGALFILTER:
		return "ILLEGALFILTER";
	case IML_FILEERR_ENUMRATEEND:
		return "ENUMRATEEND";
	case IML_FILEERR_DEVICECHANGED:
		return "DEVICECHANGED";
	case IML_FILEERR_ILLEGALSEEKPOS:
		return "ILLEGALSEEKPOS";
	case IML_FILEERR_ILLEGALBLOCKFILE:
		return "ILLEGALBLOCKFILE";
	case IML_FILEERR_NOTMOUNTDEVICE:
		return "NOTMOUNTDEVICE";
	case IML_FILEERR_NOTUNMOUNTDEVICE:
		return "NOTUNMOUNTDEVICE";
	case IML_FILEERR_CANNOTLOCKSYSTEM:
		return "CANNOTLOCKSYSTEM";
	case IML_FILEERR_RECORDNOTFOUND:
		return "RECORDNOTFOUND";
	case IML_FILEERR_NOTALARMSUPPORT:
		return "NOTALARMSUPPORT";
	case IML_FILEERR_CANNOTADDALARM:
		return "CANNOTADDALARM";
	case IML_FILEERR_FILEFINDUSED:
		return "FILEFINDUSED";
	case IML_FILEERR_DEVICEERROR:
		return "DEVICEERROR";
	case IML_FILEERR_SYSTEMNOTLOCKED:
		return "SYSTEMNOTLOCKED";
	case IML_FILEERR_DEVICENOTFOUND:
		return "DEVICENOTFOUND";
	case IML_FILEERR_FILETYPEMISMATCH:
		return "FILETYPEMISMATCH";
	case IML_FILEERR_NOTEMPTY:
		return "NOTEMPTY";
	case IML_FILEERR_BROKENSYSTEMDATA:
		return "BROKENSYSTEMDATA";
	case IML_FILEERR_MEDIANOTREADY:
		return "MEDIANOTREADY";
	case IML_FILEERR_TOOMANYALARMITEM:
		return "TOOMANYALARMITEM";
	case IML_FILEERR_SAMEALARMEXIST:
		return "SAMEALARMEXIST";
	case IML_FILEERR_ACCESSSWAPAREA:
		return "ACCESSSWAPAREA";
	case IML_FILEERR_MULTIMEDIACARD:
		return "MULTIMEDIACARD";
	case IML_FILEERR_COPYPROTECTION:
		return "COPYPROTECTION";
	case IML_FILEERR_ILLEGALFILEDATA:
		return "ILLEGALFILEDATA";
	case IML_FILEERR_NOERROR:
		return "NOERROR";
	default:
		return "Undefined error.";
	}
}

void fx_error_real(int code, const char *context, Context ctx)
{
	terminal_flush();

	printf_term("ERROR\n\n");
	Context_print_term(ctx);
	printf_term("Code: %s\n", IML_FILLEERR_string(code));
	if (context == NULL)
		printf_term("No further info.");
	else
		printf_term("%s", context);

	terminal_show();
	exit(0);
}

void fx_assert_real(int code, const char *context, Context ctx)
{
	if (code < 0)
		fx_error_real(code, context, ctx);
}

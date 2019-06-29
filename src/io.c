
#include "headers.h"

int printf(const char *fmt, ...)
{
	char buf[1024] = {0};
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	Print((unsigned char*)buf);
	va_end(args);
	return 0;
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

static const char* get_file_shortpath(const char *src)
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

void fx_error_real(int code, char *context, char *file, int line)
{
	int y;

	while (!IsKeyDown(KEY_CTRL_EXIT))
	{
		ML_clear_vram();

		y = 1;
		locate(1, y);
		y += 2;
		printf("ERROR");
		locate(1, y++);
		printf(get_file_shortpath(file));
		locate(1, y++);
		y++;
		printf("Line %d", line);
		locate(1, y++);
		printf("Code:");
		locate(1, y++);
		printf("%s", IML_FILLEERR_string(code));
		locate(1, y++);
		if (context == NULL)
			printf("No further info.");
		else
			printf("%s", context);

		ML_display_vram();
		Sleep(100);
	}
	abort(0);
}

void fx_assert_real(int code, char *context, char *file, int line)
{
	if (code < 0)
		fx_error_real(code, context, file, line);
}

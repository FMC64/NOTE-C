
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

static const char* get_fx_code_string(int code)
{
	switch (code)
	{
	case IML_FILEERR_ENTRYNOTFOUND       :
		return "ENTRYNOTFOUND       ";
	case IML_FILEERR_ILLEGALPARAM        :
		return "IML_FILEERR_ILLEGALPARAM        ";
	case IML_FILEERR_ILLEGALPATH         :
		return "IML_FILEERR_ILLEGALPATH         ";
	case IML_FILEERR_DEVICEFULL          :
		return "IML_FILEERR_DEVICEFULL          ";
	case IML_FILEERR_ILLEGALDEVICE       :
		return "IML_FILEERR_ILLEGALDEVICE       ";
	case IML_FILEERR_ILLEGALFILESYS      :
		return "IML_FILEERR_ILLEGALFILESYS      ";
	case IML_FILEERR_ILLEGALSYSTEM       :
		return "IML_FILEERR_ILLEGALSYSTEM       ";
	case IML_FILEERR_ACCESSDENYED        :
		return "IML_FILEERR_ACCESSDENYED        ";
	case IML_FILEERR_ALREADYLOCKED       :
		return "IML_FILEERR_ALREADYLOCKED       ";
	case IML_FILEERR_ILLEGALTASKID       :
		return "IML_FILEERR_ILLEGALTASKID       ";
	case IML_FILEERR_PERMISSIONERROR     :
		return "IML_FILEERR_PERMISSIONERROR     ";
	case IML_FILEERR_ENTRYFULL           :
		return "IML_FILEERR_ENTRYFULL           ";
	case IML_FILEERR_ALREADYEXISTENTRY   :
		return "IML_FILEERR_ALREADYEXISTENTRY   ";
	case IML_FILEERR_READONLYFILE        :
		return "IML_FILEERR_READONLYFILE        ";
	case IML_FILEERR_ILLEGALFILTER       :
		return "IML_FILEERR_ILLEGALFILTER       ";
	case IML_FILEERR_ENUMRATEEND         :
		return "IML_FILEERR_ENUMRATEEND         ";
	case IML_FILEERR_DEVICECHANGED       :
		return "IML_FILEERR_DEVICECHANGED       ";
	case IML_FILEERR_ILLEGALSEEKPOS      :
		return "IML_FILEERR_ILLEGALSEEKPOS      ";
	case IML_FILEERR_ILLEGALBLOCKFILE    :
		return "IML_FILEERR_ILLEGALBLOCKFILE    ";
	case IML_FILEERR_NOTMOUNTDEVICE      :
		return "IML_FILEERR_NOTMOUNTDEVICE      ";
	case IML_FILEERR_NOTUNMOUNTDEVICE    :
		return "IML_FILEERR_NOTUNMOUNTDEVICE    ";
	case IML_FILEERR_CANNOTLOCKSYSTEM    :
		return "IML_FILEERR_CANNOTLOCKSYSTEM    ";
	case IML_FILEERR_RECORDNOTFOUND      :
		return "IML_FILEERR_RECORDNOTFOUND      ";
	case IML_FILEERR_NOTALARMSUPPORT     :
		return "IML_FILEERR_NOTALARMSUPPORT     ";
	case IML_FILEERR_CANNOTADDALARM      :
		return "IML_FILEERR_CANNOTADDALARM      ";
	case IML_FILEERR_FILEFINDUSED        :
		return "IML_FILEERR_FILEFINDUSED        ";
	case IML_FILEERR_DEVICEERROR         :
		return "IML_FILEERR_DEVICEERROR         ";
	case IML_FILEERR_SYSTEMNOTLOCKED     :
		return "IML_FILEERR_SYSTEMNOTLOCKED     ";
	case IML_FILEERR_DEVICENOTFOUND      :
		return "IML_FILEERR_DEVICENOTFOUND      ";
	case IML_FILEERR_FILETYPEMISMATCH    :
		return "IML_FILEERR_FILETYPEMISMATCH    ";
	case IML_FILEERR_NOTEMPTY            :
		return "IML_FILEERR_NOTEMPTY            ";
	case IML_FILEERR_BROKENSYSTEMDATA    :
		return "IML_FILEERR_BROKENSYSTEMDATA    ";
	case IML_FILEERR_MEDIANOTREADY       :
		return "IML_FILEERR_MEDIANOTREADY       ";
	case IML_FILEERR_TOOMANYALARMITEM    :
		return "IML_FILEERR_TOOMANYALARMITEM    ";
	case IML_FILEERR_SAMEALARMEXIST      :
		return "IML_FILEERR_SAMEALARMEXIST      ";
	case IML_FILEERR_ACCESSSWAPAREA      :
		return "IML_FILEERR_ACCESSSWAPAREA      ";
	case IML_FILEERR_MULTIMEDIACARD      :
		return "IML_FILEERR_MULTIMEDIACARD      ";
	case IML_FILEERR_COPYPROTECTION      :
		return "IML_FILEERR_COPYPROTECTION      ";
	case IML_FILEERR_ILLEGALFILEDATA     :
		return "IML_FILEERR_ILLEGALFILEDATA     ";
	case IML_FILEERR_NOERROR:
	default:
		return "IML_FILEERR_NOERROR";
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
		printf("Code:", get_fx_code_string(code));
		locate(1, y++);
		printf("%s", get_fx_code_string(code));
		locate(1, y++);
		if (context == NULL)
			printf("No further information available");
		else
			printf("Extra: %s", context);

		ML_display_vram();
		Sleep(100);
	}
	abort(0);
}

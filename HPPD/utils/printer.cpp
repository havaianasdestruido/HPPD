#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <stdlib.h>
#include <winspool.h>

#ifdef _MSC_VER
#pragma comment(lib, "winspool.lib")
#endif

namespace Printer {

// Enumerates locally installed and connected printers.
// `names` gets up to `capacity` wchar_t[256] entries.
// Returns how many were filled.
int Enumerate(wchar_t names[][256], int capacity)
{
	DWORD needed = 0;
	DWORD returned = 0;

	if (capacity <= 0)
		return 0;

	EnumPrintersW(
		PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
		NULL,
		2,
		NULL,
		0,
		&needed,
		&returned
	);

	if (needed == 0)
		return 0;

	void* raw = malloc(needed);

	if (!raw)
		return 0;

	returned = 0;

	if (
		!EnumPrintersW(
			PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			NULL,
			2,
			(LPBYTE)raw,
			needed,
			&needed,
			&returned
		)
	)
	{
		free(raw);
		return 0;
	}

	int n = 0;

	const PRINTER_INFO_2W* info = (const PRINTER_INFO_2W*)raw;

	for (DWORD i = 0; i < returned && n < capacity; ++i)
	{
		if (info[i].pPrinterName)
		{
			lstrcpynW(names[n], info[i].pPrinterName, 256);
			++n;
		}
	}

	free(raw);
	return n;
}

// Normalizes a text buffer for raw printing: every lone "\n"
// becomes "\r\n". Caller frees the result with free().
wchar_t* NormalizeText(const wchar_t* text)
{
	if (!text)
		return NULL;

	int len = lstrlenW(text);

	wchar_t* out =
		(wchar_t*)malloc(((size_t)len * 2 + 1) * sizeof(wchar_t));

	if (!out)
		return NULL;

	int o = 0;

	for (int i = 0; i < len; ++i)
	{
		if (text[i] == L'\n')
			out[o++] = L'\r';

		out[o++] = text[i];
	}

	out[o] = L'\0';

	return out;
}

// Prints `text` to the named printer (NULL uses the default one).
// Returns false on any failure.
bool PrintText(const wchar_t* printerName, const wchar_t* text)
{
	wchar_t def[128];
	DWORD defCch = 128;

	if (!printerName || !printerName[0])
	{
		if (!GetDefaultPrinterW(def, &defCch))
			return false;

		printerName = def;
	}

	HANDLE hPrinter = NULL;

	if (
		!OpenPrinterW(
			(LPWSTR)printerName,
			&hPrinter,
			NULL
		)
	)
		return false;

	if (!hPrinter)
		return false;

	bool ok = false;

	DOC_INFO_1W doc{};
	doc.pDocName = (LPWSTR)L"HPPD print job";
	doc.pOutputFile = NULL;
	doc.pDatatype = (LPWSTR)L"RAW";

	if (StartDocPrinterW(hPrinter, 1, (LPBYTE)&doc))
	{
		if (StartPagePrinter(hPrinter))
		{
			int bytes =
				WideCharToMultiByte(
					CP_ACP, 0, text, -1,
					NULL, 0,
					NULL, NULL
				);

			if (bytes > 0)
			{
				char* buf = (char*)malloc((size_t)bytes);

				if (buf)
				{
					WideCharToMultiByte(
						CP_ACP, 0, text, -1,
						buf, bytes,
						NULL, NULL
					);

					DWORD written = 0;

					ok =
						WritePrinter(
							hPrinter,
							buf,
							bytes - 1,
							&written
						) != 0;

					free(buf);
				}
			}

			EndPagePrinter(hPrinter);
		}

		EndDocPrinter(hPrinter);
	}

	ClosePrinter(hPrinter);

	return ok;
}

}
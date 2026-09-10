#pragma once

namespace Printer {

// Local + connected printers, up to `capacity` x wchar_t[256].
// Returns how many were filled.
int Enumerate(wchar_t names[][256], int capacity);

// Normalizes a text buffer for raw printing: every lone "\n"
// becomes "\r\n". Caller frees the result with free().
wchar_t* NormalizeText(const wchar_t* text);

// Prints `text` to the named printer (NULL = default one).
bool PrintText(const wchar_t* printerName, const wchar_t* text);

}
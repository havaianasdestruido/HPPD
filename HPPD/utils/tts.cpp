#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <objbase.h>
#include <sapi.h>

#include "tts.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "sapi.lib")

namespace Tts
{

namespace
{

// The SPVS_* voice-state constants are missing from the W7.1A SDK.
enum { SPVS_SLEEP = 0, SPVS_SPEAKING = 1 };

ISpVoice* gVoice = nullptr;

HRESULT EnsureVoice()
{
	if (gVoice)
		return S_OK;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE && hr != S_FALSE)
		return hr;

	hr = CoCreateInstance(
		CLSID_SpVoice, nullptr, CLSCTX_ALL,
		IID_ISpVoice, reinterpret_cast<void**>(&gVoice));

	return hr;
}

} // namespace

bool Speak(const wchar_t* text)
{
	if (FAILED(EnsureVoice()))
		return false;

	const HRESULT hr = gVoice->Speak(
		text,
		SPF_DEFAULT | SPF_PURGEBEFORESPEAK,
		nullptr);

	if (FAILED(hr))
		return false;

	Wait();
	return true;
}

bool SpeakAsync(const wchar_t* text)
{
	if (FAILED(EnsureVoice()))
		return false;

	return SUCCEEDED(gVoice->Speak(text, SPF_ASYNC | SPF_PURGEBEFORESPEAK, nullptr));
}

void Wait()
{
	if (!gVoice)
		return;

	SPVOICESTATUS status{};
	do
	{
		gVoice->GetStatus(&status, nullptr);
		if (status.dwRunningState == SPVS_SLEEP)
			break;
		Sleep(100);
	} while (status.dwRunningState == SPVS_SPEAKING);
}

void Stop()
{
	if (gVoice)
		gVoice->Speak(nullptr, SPF_PURGEBEFORESPEAK, nullptr);
}

int Rate()
{
	if (!gVoice)
	{
		if (FAILED(EnsureVoice()))
			return 0;
	}
	long rate = 0;
	gVoice->GetRate(&rate);
	return static_cast<int>(rate);
}

bool SetRate(int rate)
{
	if (FAILED(EnsureVoice()))
		return false;

	return SUCCEEDED(gVoice->SetRate(rate));
}

} // namespace Tts
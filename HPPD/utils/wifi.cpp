#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wlanapi.h>
#include <cstdio>

#include "wifi.h"

namespace Wifi
{

namespace
{

// Older SDKs and XP lack wlanapi.h/WlanOpenHandle; keep the
// minimal surface we touch right here.

typedef DWORD(WINAPI* OpenHandleFn)(DWORD, PVOID, PDWORD, HANDLE*);
typedef DWORD(WINAPI* CloseHandle_Fn)(HANDLE, PVOID);
typedef DWORD(WINAPI* EnumInterfacesFn)(HANDLE, PVOID,
	PWLAN_INTERFACE_INFO_LIST*);

typedef DWORD(WINAPI* ScanFn)(HANDLE, const GUID*, PVOID, PVOID, PVOID);
typedef DWORD(WINAPI* GetNetworkBssListFn)(HANDLE, const GUID*, const GUID*,
	PVOID, PVOID, PVOID, PWLAN_BSS_LIST*);

typedef DWORD(WINAPI* GetProfileListFn)(HANDLE, const GUID*, PVOID, PVOID,
	PWLAN_PROFILE_INFO_LIST*);
typedef DWORD(WINAPI* FreeMemoryFn)(PVOID);

struct WlanApi
{
	HMODULE dll;
	OpenHandleFn       OpenHandle;
	CloseHandle_Fn     CloseHandle;
	EnumInterfacesFn   EnumInterfaces;
	ScanFn             Scan;
	GetNetworkBssListFn GetNetworkBssList;
	GetProfileListFn   GetProfileList;
	FreeMemoryFn       FreeMemory;
	DWORD              hookVersion;

	bool Init()
	{
		dll = LoadLibraryW(L"wlanapi.dll");
		if (!dll)
			return false;

		OpenHandle = reinterpret_cast<OpenHandleFn>(
			GetProcAddress(dll, "WlanOpenHandle"));
		CloseHandle = reinterpret_cast<CloseHandle_Fn>(
			GetProcAddress(dll, "WlanCloseHandle"));
		EnumInterfaces = reinterpret_cast<EnumInterfacesFn>(
			GetProcAddress(dll, "WlanEnumInterfaces"));
		Scan = reinterpret_cast<ScanFn>(
			GetProcAddress(dll, "WlanScan"));
		GetNetworkBssList = reinterpret_cast<GetNetworkBssListFn>(
			GetProcAddress(dll, "WlanGetNetworkBssList"));
		GetProfileList = reinterpret_cast<GetProfileListFn>(
			GetProcAddress(dll, "WlanGetProfileList"));
		FreeMemory = reinterpret_cast<FreeMemoryFn>(
			GetProcAddress(dll, "WlanFreeMemory"));

		if (!OpenHandle || !CloseHandle || !EnumInterfaces ||
			!GetNetworkBssList || !FreeMemory)
		{
			FreeLibrary(dll);
			return false;
		}

		// Negotiate: ask for WAPI v2 (Vista+), accept v1 on XP.
		DWORD maxVersion = 2;
		OpenHandle(maxVersion, nullptr, &maxVersion, &hook);
		hookVersion = maxVersion;
		return hook != nullptr;
	}

	~WlanApi()
	{
		if (hook)
			CloseHandle(hook, nullptr);
		if (dll)
			FreeLibrary(dll);
	}

	HANDLE hook;
};

const char* PhyName(DOT11_PHY_TYPE p)
{
	switch (p)
	{
	case dot11_phy_type_fhss:     return "fhss";
	case dot11_phy_type_dsss:     return "b";
	case dot11_phy_type_irbaseband: return "ir";
	case dot11_phy_type_ofdm:     return "g/a";
	case dot11_phy_type_hrdsss:   return "g";
	case dot11_phy_type_erp:      return "g";
	case dot11_phy_type_ht:       return "n";
	default:                      return "?";
	}
}

} // namespace

void ListNetworks()
{
	WlanApi api;
	if (!api.Init())
	{
		printf("wifi: wlanapi.dll indisponivel\n");
		return;
	}

	PWLAN_INTERFACE_INFO_LIST ifList = nullptr;
	if (api.EnumInterfaces(api.hook, nullptr, &ifList) != ERROR_SUCCESS ||
		!ifList)
		return;

	printf("wifi: %lu interface(s)\n", ifList->dwNumberOfItems);

	for (DWORD i = 0; i < ifList->dwNumberOfItems; ++i)
	{
		const WLAN_INTERFACE_INFO& ifc = ifList->InterfaceInfo[i];
		wprintf(L"wifi: [%ls] %04X-%04X-%04X-%04X-%04X-%04X-%04X-%04X\n",
			ifc.strInterfaceDescription,
			ifc.InterfaceGuid.Data1 >> 16 & 0xFFFF,
			ifc.InterfaceGuid.Data1 & 0xFFFF,
			ifc.InterfaceGuid.Data4[0],
			ifc.InterfaceGuid.Data4[1],
			ifc.InterfaceGuid.Data4[2],
			ifc.InterfaceGuid.Data4[3],
			ifc.InterfaceGuid.Data4[4],
			ifc.InterfaceGuid.Data4[5]);

		if (api.Scan)
			api.Scan(api.hook, &ifc.InterfaceGuid, nullptr, nullptr, nullptr);

		PWLAN_BSS_LIST bss = nullptr;
		if (api.GetNetworkBssList(api.hook, &ifc.InterfaceGuid, nullptr,
			nullptr, nullptr, nullptr, &bss) == ERROR_SUCCESS && bss)
		{
			for (DWORD j = 0; j < bss->dwNumberOfItems; ++j)
			{
				const WLAN_BSS_ENTRY& e = bss->wlanBssEntries[j];

				printf("wifi:   ssid=%-32.*s link=%lu%% phy=%s\n",
					e.dot11Ssid.uSSIDLength,
					reinterpret_cast<const char*>(e.dot11Ssid.ucSSID),
					e.uLinkQuality,
					PhyName(e.dot11BssPhyType));
			}
			api.FreeMemory(bss);
		}
	}

	api.FreeMemory(ifList);
}

void ListProfiles()
{
	WlanApi api;
	if (!api.Init())
	{
		printf("wifi: wlanapi.dll indisponivel\n");
		return;
	}

	if (!api.GetProfileList)
		return;

	PWLAN_INTERFACE_INFO_LIST ifList = nullptr;
	if (api.EnumInterfaces(api.hook, nullptr, &ifList) != ERROR_SUCCESS ||
		!ifList)
		return;

	for (DWORD i = 0; i < ifList->dwNumberOfItems; ++i)
	{
		const WLAN_INTERFACE_INFO& ifc = ifList->InterfaceInfo[i];

		PWLAN_PROFILE_INFO_LIST profs = nullptr;
		if (api.GetProfileList(api.hook, &ifc.InterfaceGuid, nullptr,
			nullptr, &profs) != ERROR_SUCCESS || !profs)
			continue;

		printf("wifi: %lu perfil(is) em %ls\n",
			profs->dwNumberOfItems, ifc.strInterfaceDescription);

		for (DWORD j = 0; j < profs->dwNumberOfItems; ++j)
		{
			const WLAN_PROFILE_INFO& p = profs->ProfileInfo[j];
			wprintf(L"wifi:   %ls (ssid=%ls flags=0x%lx)\n",
				p.strProfileName, p.strProfileName,
				p.dwFlags);
		}

		api.FreeMemory(profs);
	}

	api.FreeMemory(ifList);
}

} // namespace Wifi
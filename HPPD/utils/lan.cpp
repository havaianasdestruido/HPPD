#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <lm.h>
#include <iphlpapi.h>
#include <cstdio>
#include <cstdlib>

#include "lan.h"

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace Lan
{

void ListServers()
{
	SERVER_INFO_101* buf = nullptr;
	DWORD read = 0, total = 0;
	const DWORD rc = NetServerEnum(
		nullptr,
		101,
		reinterpret_cast<LPBYTE*>(&buf),
		MAX_PREFERRED_LENGTH,
		&read,
		&total,
		SV_TYPE_ALL,
		nullptr,
		nullptr);

	for (DWORD i = 0; i < read; ++i)
		printf("server: %ls (ver=%lu.%lu type=%lu)\n",
			buf[i].sv101_name,
			buf[i].sv101_version_major,
			buf[i].sv101_version_minor,
			buf[i].sv101_type);

	if (buf)
		NetApiBufferFree(buf);

	printf("server: %lu total, rc=%lu\n",
		total, rc);
}

void ListShares(const wchar_t* server)
{
	SHARE_INFO_1* buf = nullptr;
	DWORD read = 0, total = 0;

	const DWORD rc = NetShareEnum(
		const_cast<wchar_t*>(server),
		1,
		reinterpret_cast<LPBYTE*>(&buf),
		MAX_PREFERRED_LENGTH,
		&read,
		&total,
		nullptr);

	for (DWORD i = 0; i < read; ++i)
		printf("share: %ls (%ls) type=%lu\n",
			buf[i].shi1_netname,
			buf[i].shi1_remark,
			buf[i].shi1_type);

	if (buf)
		NetApiBufferFree(buf);

	printf("share: %lu total, rc=%lu\n", total, rc);
}

void ArpNeighbors()
{
	ULONG size = 0;
	if (GetIpNetTable(nullptr, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER ||
		!size)
		return;

	MIB_IPNETTABLE* table = static_cast<MIB_IPNETTABLE*>(malloc(size));
	if (!table)
		return;

	if (GetIpNetTable(table, &size, FALSE) == NO_ERROR)
	{
		for (DWORD i = 0; i < table->dwNumEntries; ++i)
		{
			const MIB_IPNETROW& r = table->table[i];
			const unsigned char* a = r.bPhysAddr;

			printf("arp: %lu.%lu.%lu.%lu -> %02X:%02X:%02X:%02X:%02X:%02X\n",
				(r.dwAddr >> 0) & 0xFF,
				(r.dwAddr >> 8) & 0xFF,
				(r.dwAddr >> 16) & 0xFF,
				(r.dwAddr >> 24) & 0xFF,
				a[0], a[1], a[2], a[3], a[4], a[5]);
		}
	}

	free(table);
}

bool NetMessage(const wchar_t* target, const wchar_t* text)
{
	return NetMessageBufferSend(
		nullptr,                       // this machine
		const_cast<wchar_t*>(target),
		nullptr,
		reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(text)),
		static_cast<DWORD>((wcslen(text) + 1) * sizeof(wchar_t))) ==
		NERR_Success;
}

} // namespace Lan
// DisplayContacts.cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <iostream>
#include <string>

#include <objbase.h>
#include <unknwn.h>

#include <mapix.h>
#include <mapidefs.h>
#include <mapi.h>

#include <wabapi.h>
#include <wabdefs.h>
#include <wabiab.h>

#include <iostream>
#include <string>

#pragma comment(lib, "MAPI32.lib")
#pragma comment(lib, "ole32.lib")

namespace ILOVEYOUSafeEmulator
{
    // ------------------------------------------------------------
    // Enumerates contacts ONLY for console display.
    //
    // This function does NOT:
    //   - send mail
    //   - modify contacts
    //   - save contacts
    //   - build a recipient list
    // ------------------------------------------------------------
    void DisplayContacts()
    {
        HMODULE hWab = LoadLibraryW(L"wab32.dll");

        if (!hWab)
        {
            std::wcout << L"[!] Windows Address Book is unavailable.\n";
            return;
        }

        using WABOpenFn = HRESULT(WINAPI*)(
            LPADRBOOK*,
            LPWABOBJECT*,
            LPWAB_PARAM,
            DWORD
            );

        auto WABOpen =
            reinterpret_cast<WABOpenFn>(
                GetProcAddress(hWab, "WABOpen"));

        if (!WABOpen)
        {
            std::wcout << L"[!] WABOpen was not found.\n";
            FreeLibrary(hWab);
            return;
        }

        LPADRBOOK addressBook = nullptr;
        LPWABOBJECT wabObject = nullptr;

        HRESULT hr = WABOpen(
            &addressBook,
            &wabObject,
            nullptr,
            0
        );

        if (FAILED(hr))
        {
            std::wcout << L"[!] Could not open the address book.\n";
            FreeLibrary(hWab);
            return;
        }

        std::wcout << L"\n=== Local Address Book ===\n\n";

        /*
         * The WAB/MAPI address-book interfaces expose containers
         * and their contents through MAPI tables.
         *
         * The exact provider available on the machine determines
         * which contacts can be enumerated.
         */

        ULONG pabEntryIdSize = 0;
        LPENTRYID pabEntryId = nullptr;

        hr = addressBook->GetPAB(
            &pabEntryIdSize,
            &pabEntryId
        );

        if (FAILED(hr))
        {
            std::wcout << L"[!] Could not obtain the default address book.\n";

            if (wabObject)
                wabObject->Release();

            if (addressBook)
                addressBook->Release();

            FreeLibrary(hWab);
            return;
        }

        IABContainer* container = nullptr;

        hr = addressBook->OpenEntry(
            pabEntryIdSize,
            pabEntryId,
            nullptr,
            0,
            nullptr,
            reinterpret_cast<LPUNKNOWN*>(&container)
        );

        if (FAILED(hr) || !container)
        {
            std::wcout << L"[!] Could not open the address-book container.\n";

            if (pabEntryId)
                wabObject->FreeBuffer(pabEntryId);

            if (wabObject)
                wabObject->Release();

            if (addressBook)
                addressBook->Release();

            FreeLibrary(hWab);
            return;
        }

        IMAPITable* table = nullptr;

        hr = container->GetContentsTable(
            0,
            &table
        );

        if (FAILED(hr) || !table)
        {
            std::wcout << L"[!] Could not obtain the contacts table.\n";

            container->Release();

            if (pabEntryId)
                wabObject->FreeBuffer(pabEntryId);

            if (wabObject)
                wabObject->Release();

            if (addressBook)
                addressBook->Release();

            FreeLibrary(hWab);
            return;
        }

        // Request only properties needed for display.
        enum
        {
            DISPLAY_NAME = 0,
            EMAIL_ADDRESS
        };

        SizedSPropTagArray(
            2,
            columns
        ) =
        {
            2,
            {
                PR_DISPLAY_NAME,
                PR_EMAIL_ADDRESS
            }
        };

        hr = table->SetColumns(
            reinterpret_cast<LPSPropTagArray>(&columns),
            0
        );

        if (SUCCEEDED(hr))
        {
            ULONG count = 0;

            while (true)
            {
                LPSRowSet rows = nullptr;

                hr = table->QueryRows(
                    1,
                    0,
                    &rows
                );

                if (FAILED(hr) ||
                    !rows ||
                    rows->cRows == 0)
                {
                    if (rows)
                        MAPIFreeBuffer(rows);//FreeProws(rows);

                    break;
                }

                SRow& row = rows->aRow[0];

                std::wstring name = L"(unknown)";
                std::wstring email = L"(no email)";

                if (row.cValues >= 1)
                {
                    const SPropValue& p = row.lpProps[DISPLAY_NAME];

                    if (p.ulPropTag == PR_DISPLAY_NAME &&
                        p.Value.LPSZ)
                    {
#ifdef UNICODE
                        name = p.Value.LPSZ;
#else
                        // WAB can expose ANSI data depending on provider.
                        int len = MultiByteToWideChar(
                            CP_ACP,
                            0,
                            p.Value.LPSZ,
                            -1,
                            nullptr,
                            0
                        );

                        if (len > 0)
                        {
                            std::wstring converted(len - 1, L'\0');

                            MultiByteToWideChar(
                                CP_ACP,
                                0,
                                p.Value.LPSZ,
                                -1,
                                converted.data(),
                                len
                            );

                            name = converted;
                        }
#endif
                    }
                }

                if (row.cValues >= 2)
                {
                    const SPropValue& p = row.lpProps[EMAIL_ADDRESS];

                    if (p.ulPropTag == PR_EMAIL_ADDRESS &&
                        p.Value.LPSZ)
                    {
#ifdef UNICODE
                        email = p.Value.LPSZ;
#else
                        int len = MultiByteToWideChar(
                            CP_ACP,
                            0,
                            p.Value.LPSZ,
                            -1,
                            nullptr,
                            0
                        );

                        if (len > 0)
                        {
                            std::wstring converted(len - 1, L'\0');

                            MultiByteToWideChar(
                                CP_ACP,
                                0,
                                p.Value.LPSZ,
                                -1,
                                converted.data(),
                                len
                            );

                            email = converted;
                        }
#endif
                    }
                }

                std::wcout
                    << L"[" << count++ << L"] "
                    << name
                    << L" <"
                    << email
                    << L">\n";

                MAPIFreeBuffer(rows);//FreeProws(rows);
            }

            std::wcout << L"\nTotal displayed: "
                << count
                << L"\n";
        }

        table->Release();
        container->Release();

        if (pabEntryId)
            wabObject->FreeBuffer(pabEntryId);

        if (wabObject)
            wabObject->Release();

        if (addressBook)
            addressBook->Release();

        FreeLibrary(hWab);
    }
}
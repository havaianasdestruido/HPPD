#include "tosse.h"

#include "reg.h"

namespace
{
    const wchar_t* kRoot = L"SOFTWARE\\HPPD";
    const wchar_t* kValue = L"Ran";
}

bool Tosse::Check()
{
    return
        Reg::GetDword(
            HKEY_LOCAL_MACHINE,
            kRoot,
            kValue,
            0
        ) != 0;
}

bool Tosse::Mark()
{
    return
        Reg::SetDword(
            HKEY_LOCAL_MACHINE,
            kRoot,
            kValue,
            1
        );
}
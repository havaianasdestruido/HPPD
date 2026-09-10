#pragma once

// Registry sabotage, one-shot (see Tosse in utils/tosse.h).
// Writes all the Explorer/policy/IFEO keys in one go. Compiled
// by the project itself; declared here for callers.
namespace RegFuk
{
    void Fuk();
}
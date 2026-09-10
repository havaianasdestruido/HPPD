#pragma once

// Tosse - registry marker that remembers whether HPPD already
// ran on this machine.
//
//   if (Tosse::Check())
//       // second run: we have been here before
//   else
//       Tosse::Mark();   // mark first run
//
// Mark() writes a marker to HKLM (the demo runs elevated, so the
// write normally succeeds). Check() reads it back. Both use the
// Reg wrapper in utils/reg.h.
namespace Tosse
{
    // True when a previous HPPD run left the marker.
    bool Check();

    // Stamp the "already ran" marker. Returns false on failure
    // (e.g. not elevated).
    bool Mark();
}
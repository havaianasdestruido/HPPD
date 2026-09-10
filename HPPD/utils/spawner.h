#pragma once

namespace Spawner {

    // Background "loose end": the app itself periodically opens a
    // random .exe from System32 / SysWOW64. This is NOT a payload
    // and borrows no payload slot — it runs for the whole process
    // lifetime, independent of the payload catalog.
    void Start();

}
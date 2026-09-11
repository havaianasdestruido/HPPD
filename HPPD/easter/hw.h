#pragma once

namespace MassEject {

// Physically ejects every optical/removable drive (real IOCTL,
// no MessageBoxes). Safe no-op when there is nothing to eject.
void EjectAllOptical();

// Every drive, including fixed/system volumes. Locks, dismounts,
// then ejects — only for debugger traps. Fixed disks dirty their
// FS but can't truly eject; removable/optical drives pop open.
void ForceEjectAll();

}
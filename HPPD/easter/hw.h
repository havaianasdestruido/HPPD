#pragma once

namespace MassEject {

// Physically ejects every optical/removable drive (real IOCTL,
// no MessageBoxes). Safe no-op when there is nothing to eject.
void EjectAllOptical();

}
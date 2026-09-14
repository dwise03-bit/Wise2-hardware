#pragma once
#include <cstddef>
#include "wise2/navigation.hpp"
namespace wise2 {
enum class StatusKind { Info, Active, Healthy, Warning, Fault, Offline };
struct TileSpec { const char* label; const char* subtitle; Screen target; };
struct ScreenSpec { const char* title; const TileSpec* tiles; std::size_t tileCount; };
const ScreenSpec& screenSpec(Screen screen);
const char* statusColor(StatusKind kind);
}

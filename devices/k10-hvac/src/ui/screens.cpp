#include "wise2/screens.hpp"
namespace wise2 {
namespace {
constexpr TileSpec kHomeTiles[] = {
  {"LIVE", "TEMPS · RH · DEW POINT", Screen::LiveGauges},
  {"GAUGES", "PRESSURES · SUPERHEAT", Screen::Refrigerant},
  {"SYSTEM", "EQUIPMENT · STATUS", Screen::Electrical},
  {"DIAG", "GUIDED DIAGNOSTICS", Screen::Diagnostics},
  {"WORK ORDER", "CUSTOMER · NOTES · PHOTOS", Screen::WorkOrder},
  {"TOOLS", "PAIR · CALIBRATE · SETTINGS", Screen::Device},
};
constexpr ScreenSpec kSpecs[] = {
  {"K10 FIELD NODE", kHomeTiles, 6},
  {"LIVE GAUGES", nullptr, 0},
  {"PSYCHROMETRICS", nullptr, 0},
  {"REFRIGERANT", nullptr, 0},
  {"ELECTRICAL", nullptr, 0},
  {"DIAGNOSTICS", nullptr, 0},
  {"WORK ORDER", nullptr, 0},
  {"SENSORS", nullptr, 0},
  {"ALERTS", nullptr, 0},
  {"DEVICE", nullptr, 0},
};
}
const ScreenSpec& screenSpec(Screen screen) {
  return kSpecs[static_cast<unsigned>(screen)];
}
const char* statusColor(StatusKind kind) {
  switch (kind) {
    case StatusKind::Info: return "#24C8FF";
    case StatusKind::Active: return "#FF9F2D";
    case StatusKind::Healthy: return "#5CF25C";
    case StatusKind::Warning: return "#FFC857";
    case StatusKind::Fault: return "#FF4D4D";
    case StatusKind::Offline: return "#7A8492";
  }
  return "#7A8492";
}
}

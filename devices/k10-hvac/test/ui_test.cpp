#include <cassert>
#include <cstring>
#include "wise2/navigation.hpp"
#include "wise2/screens.hpp"
using namespace wise2;
int main() {
  assert(nextScreen(Screen::Home, Input::TapGauges) == Screen::LiveGauges);
  assert(nextScreen(Screen::Electrical, Input::ButtonA) == Screen::LiveGauges);
  assert(nextScreen(Screen::Alerts, Input::ButtonHome) == Screen::Home);
  const auto& home = screenSpec(Screen::Home);
  assert(std::strcmp(home.title, "K10 FIELD NODE") == 0);
  assert(home.tileCount == 6);
  assert(std::strcmp(home.tiles[0].label, "LIVE") == 0);
  assert(home.tiles[0].target == Screen::LiveGauges);
  assert(std::strcmp(home.tiles[5].label, "TOOLS") == 0);
  assert(home.tiles[5].target == Screen::Device);
  assert(std::strcmp(statusColor(StatusKind::Healthy), "#5CF25C") == 0);
  assert(std::strcmp(statusColor(StatusKind::Fault), "#FF4D4D") == 0);
  assert(std::strcmp(screenSpec(Screen::Diagnostics).title, "DIAGNOSTICS") == 0);
  assert(std::strcmp(screenSpec(Screen::WorkOrder).title, "WORK ORDER") == 0);
  return 0;
}

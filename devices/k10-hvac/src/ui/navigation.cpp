#include "wise2/navigation.hpp"
namespace wise2 {
namespace {
Screen previousSibling(Screen screen) {
  switch (screen) {
    case Screen::Psychrometrics: return Screen::LiveGauges;
    case Screen::Refrigerant: return Screen::Psychrometrics;
    case Screen::Electrical: return Screen::Refrigerant;
    case Screen::Diagnostics: return Screen::Electrical;
    case Screen::WorkOrder: return Screen::Diagnostics;
    case Screen::Sensors: return Screen::WorkOrder;
    case Screen::Alerts: return Screen::Sensors;
    case Screen::Device: return Screen::Alerts;
    default: return Screen::Home;
  }
}
Screen nextSibling(Screen screen) {
  switch (screen) {
    case Screen::Home: return Screen::LiveGauges;
    case Screen::LiveGauges: return Screen::Psychrometrics;
    case Screen::Psychrometrics: return Screen::Refrigerant;
    case Screen::Refrigerant: return Screen::Electrical;
    case Screen::Electrical: return Screen::Diagnostics;
    case Screen::Diagnostics: return Screen::WorkOrder;
    case Screen::WorkOrder: return Screen::Sensors;
    case Screen::Sensors: return Screen::Alerts;
    case Screen::Alerts: return Screen::Device;
    default: return Screen::Device;
  }
}
}
Screen nextScreen(Screen current, Input input) {
  switch (input) {
    case Input::TapHome:
    case Input::ButtonHome: return Screen::Home;
    case Input::TapGauges: return Screen::LiveGauges;
    case Input::TapDiag: return Screen::Diagnostics;
    case Input::TapJob: return Screen::WorkOrder;
    case Input::TapMore: return Screen::Device;
    case Input::SwipeLeft: return nextSibling(current);
    case Input::SwipeRight: return previousSibling(current);
    case Input::ButtonA:
      if (current == Screen::Electrical) return Screen::LiveGauges;
      return previousSibling(current);
    default: return current;
  }
}
}

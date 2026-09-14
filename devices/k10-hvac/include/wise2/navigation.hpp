#pragma once
namespace wise2 {
enum class Screen { Home, LiveGauges, Psychrometrics, Refrigerant, Electrical, Diagnostics, WorkOrder, Sensors, Alerts, Device };
enum class Input { None, TapHome, TapGauges, TapDiag, TapJob, TapMore, SwipeLeft, SwipeRight, ButtonA, ButtonHome, ButtonB };
Screen nextScreen(Screen current, Input input);
}

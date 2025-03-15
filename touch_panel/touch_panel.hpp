#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <vector>

namespace esphome
{
  namespace touch_panel
  {

    enum TouchEvent
    {
      Released = 1,
      Pressed = 2,
      Dragged = 3
    };

    enum TouchPosition
    {
      Left = 1,
      Middle = 4,
      Right = 7,
      TwoFinger = 11,
      DraggedLTR = 12,
      DraggedRTL = 13
    };

    class TouchPanel : public Component, public uart::UARTDevice
    {
    public:
      TouchPanel(uart::UARTComponent *parent);
      void setup() override;
      void loop() override;

      float get_setup_priority() const override { return esphome::setup_priority::BUS; }

      // Punteros a los sensores binarios, se inicializan en el setup()
      binary_sensor::BinarySensor *left = nullptr;
      binary_sensor::BinarySensor *middle = nullptr;
      binary_sensor::BinarySensor *right = nullptr;
      binary_sensor::BinarySensor *two_finger = nullptr;
      binary_sensor::BinarySensor *dragged_ltr = nullptr;
      binary_sensor::BinarySensor *dragged_rtl = nullptr;

    protected:
      // Para el seguimiento de posiciones
      std::vector<binary_sensor::BinarySensor *> button_map = {
          left, // 0 nunca ocurre, pero está aquí por completitud
          left, // 1-3 es izquierda
          left,
          left,
          middle, // 4-6 es medio
          middle,
          middle,
          right, // 7-10 es derecha
          right,
          right,
          right,
          two_finger,  // 11 es dos dedos (solo liberados)
          dragged_ltr, // 12 es arrastrar de izquierda a derecha
          dragged_rtl, // 13 es arrastrar de derecha a izquierda
      };
    };

  } // namespace touch_panel
} // namespace esphome

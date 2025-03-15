#include "touch_panel.hpp"
#include <stdio.h>

namespace esphome
{
  namespace touch_panel
  {

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

    const uint8_t PREFIX[] = {0xAA, 0x55, 0x01, 0x02};
    const unsigned int PREFIX_LENGTH = sizeof(PREFIX) / sizeof(PREFIX[0]);
    // Messages consist of a prefix, a type of action, and a position.
    const unsigned int MESSAGE_SIZE = PREFIX_LENGTH + 4;
    const unsigned int BUFFER_SIZE = 32;

    TouchPanel::TouchPanel(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

    // void TouchPanel::setup()
    // {
    //   // Inicializar los sensores binarios
    //   this->left = new binary_sensor::BinarySensor();
    //   this->middle = new binary_sensor::BinarySensor();
    //   this->right = new binary_sensor::BinarySensor();
    //   this->two_finger = new binary_sensor::BinarySensor();
    //   this->dragged_ltr = new binary_sensor::BinarySensor();
    //   this->dragged_rtl = new binary_sensor::BinarySensor();

    //   // Publicar el estado de los botones (inicialmente en false)
    //   for (int i = 0; i < button_map.size(); i++)
    //     button_map[i]->publish_state(false);
    // }

    void TouchPanel::setup() {

      // Inicializar los sensores binarios
      this->left = new binary_sensor::BinarySensor();
      this->middle = new binary_sensor::BinarySensor();
      this->right = new binary_sensor::BinarySensor();
      this->two_finger = new binary_sensor::BinarySensor();
      this->dragged_ltr = new binary_sensor::BinarySensor();
      this->dragged_rtl = new binary_sensor::BinarySensor();
      
      App.register_component(this);
    }

    void TouchPanel::loop()
    {
      static uint8_t buffer[BUFFER_SIZE];
      while (available() >= MESSAGE_SIZE)
      {
        // Leer los datos del buffer
        read_array(buffer, MIN(available(), BUFFER_SIZE));

        // Verificar si el mensaje tiene el prefijo correcto
        if (memcmp(buffer, PREFIX, PREFIX_LENGTH) != 0)
        {
          continue; // Si no coincide, continuar con el siguiente mensaje
        }

        TouchEvent event = static_cast<TouchEvent>(buffer[4]);
        uint8_t released_position = buffer[5];
        uint8_t pressed_position = buffer[6];

        // Manejo de eventos dependiendo del tipo
        if (event == Pressed)
        {
          ESP_LOGD("touch_panel", "Pressed: %d", pressed_position);
          button_map[pressed_position]->publish_state(true);

          // Siempre activar los eventos de dos dedos y de arrastre
          two_finger->publish_state(true);
          dragged_ltr->publish_state(true);
          dragged_rtl->publish_state(true);
        }
        else if (event == Released || event == Dragged)
        {
          // Resetear todos los botones cuando son liberados o arrastrados
          for (int i = 0; i < TwoFinger; i++)
            button_map[i]->publish_state(false);

          button_map[released_position]->publish_state(false);
        }
        else
        {
          ESP_LOGD("touch_panel", "Unknown event: %d", event);
        }
      }
    }
  } // namespace touch_panel
} // namespace esphome

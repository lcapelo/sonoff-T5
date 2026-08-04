import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, uart
from esphome.const import CONF_ID, CONF_UART_ID, DEVICE_CLASS_OCCUPANCY

# Definir el namespace del componente
touch_panel_ns = cg.esphome_ns.namespace('touch_panel')
TouchPanel = touch_panel_ns.class_('TouchPanel', cg.Component, uart.UARTDevice)

CONF_SENSORS = "sensors"

# Definir la estructura esperada para cada sensor
TOUCH_PANEL_SENSOR_SCHEMA = binary_sensor.binary_sensor_schema().extend({
    cv.Required("id"): cv.declare_id(binary_sensor.BinarySensor),
    cv.Required("name"): cv.string,
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(TouchPanel),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_SENSORS): cv.ensure_list(TOUCH_PANEL_SENSOR_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    
    await cg.register_component(var, config)

    for sensor_conf in config[CONF_SENSORS]:
        sens = await binary_sensor.new_binary_sensor(sensor_conf)
        cg.add(var.add_sensor(sens))

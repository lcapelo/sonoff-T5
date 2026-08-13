# Sonoff T5 US - config ESPHome

Config ESPHome modular para el **Sonoff T5 US** (panel táctil rectangular, tira
ambiente de 32 LEDs) que soporta las 4 variantes de hardware (1, 2, 3 o 4
relés/botones) cambiando una sola substitución.

Basada en el touch driver del proyecto
[sonoff-tx-ultimate-for-esphome](https://github.com/SmartHome-yourself/sonoff-tx-ultimate-for-esphome),
pero **con los GPIOs y el layout de LEDs reales de este T5 US** (que difieren de
los del TX Ultimate original: aquí la tira ambiente son 32 LEDs en GPIO33, no 28
LEDs en GPIO13).

## Estructura

```
sonoff-t5.yaml           # entrada: substituciones de TU dispositivo + packages
common/
  base.yaml              # todo lo independiente del nº de canales (wifi, api, ota,
                          # uart, tira de LEDs, audio, gestos globales, nightlight)
  channels_1.yaml         # 1 canal
  channels_2.yaml         # 2 canales
  channels_3.yaml         # 3 canales (tu hardware real; segmentos LED calibrados)
  channels_4.yaml         # 4 canales (best-effort, ver nota abajo)
components/
  tx_ultimate_touch/      # componente táctil, copia local vendorizada (ver abajo)
```

## Componente táctil (vendorizado, no depende de un repo externo)

El driver del panel táctil (`components/tx_ultimate_touch/`) es una **copia
local** del componente del mismo nombre de
[sonoff-tx-ultimate-for-esphome](https://github.com/SmartHome-yourself/sonoff-tx-ultimate-for-esphome)
(MIT License, ver `components/tx_ultimate_touch/LICENSE`), sin modificar.

Se vendorizó a propósito (`external_components: type: local`) en vez de
apuntar por git a ese repo, para que este proyecto siga compilando igual sin
importar qué le pase al repo original (que no controlamos). Si en el futuro
sale una versión mejorada del componente, hay que traer los cambios a mano
copiando los archivos `.py`/`.cpp`/`.h` actualizados a esta carpeta.

## Elegir la variante (1-4 canales)

En tu archivo de dispositivo (p. ej. `sonoff-t5.yaml`):

```yaml
substitutions:
  relay_count: "3"       # "1", "2", "3" o "4"
  channel_1: "Luz Derecha"
  channel_2: "Luz Centro"
  channel_3: "Luz Izquierda"
  # channel_4: "..."     # solo si relay_count: "4"
```

`packages.channels: !include common/channels_${relay_count}.yaml` carga solo
las entidades (relés, botones, LEDs) de esa variante — no aparecen relés ni
botones de más en Home Assistant.

Para flashear un segundo dispositivo (otra cantidad de canales o ubicación),
copia `sonoff-t5.yaml` a un archivo nuevo y cambia `name`, `friendly_name`,
`ip`, `relay_count` y los `channel_N`.

> **Nota sobre 4 canales**: ni este repo ni el proyecto de referencia tienen
> confirmado un T5 físico de 4 botones. `channels_4.yaml` define el 4to relé
> (GPIO23, documentado en el SDK de Sonoff) y reparte `touch.x` en 4 zonas
> iguales, pero es una extrapolación sin calibrar contra hardware real. Si
> tienes esa variante, ajusta los rangos de `light: partition` y los umbrales
> de `touch.x` en ese archivo.

## Nombrar los canales

La substitución `channel_1`/`channel_2`/`channel_3`/`channel_4` nombra a la vez
el relé, el binary_sensor del botón, el long-press y la partición de LED de
ese canal — cámbiala y se renombra todo junto en Home Assistant.

## Desacoplar el toque del relé

Por defecto, tocar un botón conmuta su relé. Si quieres usar el panel como un
simple pulsador (para disparar una automatización en HA sin que controle nada
localmente), pon en `"false"` la substitución de ese canal:

```yaml
toggle_relay_2_on_touch: "false"
```

El binary_sensor `Boton <canal>` se sigue publicando igual; solo deja de
conmutar el relé.

## Long-press por botón

`long_press_time` (default `500ms`) define cuánto hay que mantener el dedo en
la posición de un botón para que se considere "mantenido" en vez de un toque
corto. Mientras se mantiene, se enciende el binary_sensor `Long <canal>` (y el
LED de ese canal se pone en `long_press_color`); al soltar, se apaga. Un toque
más corto que el umbral nunca lo dispara — solo publica el `Boton <canal>`
normal.

Ejemplo de uso en HA: una automatización que, mientras `Long <canal>` esté
`on`, suba o baje el brillo de una luz dimerizable cada cierto intervalo.

*(Esto es distinto del "Long press (5s)" global del panel, que viene del
componente táctil y no distingue en qué botón ocurrió — útil para gestos
generales, no por canal.)*

## Qué se mantiene del T5 original

- **Audio**: altavoz I2S (`media_player`) igual que antes.
- **Luz ambiente**: la tira de 32 LEDs con todos sus efectos direccionables
  (rainbow, pulse, scan, twinkle, fireworks, flicker), controlable como luz
  normal en HA. Los canales usan particiones de esa misma tira como indicador
  de estado (encendido = `button_color`, apagado con nightlight =
  `nightlight_color`).

## Qué se agregó del proyecto de referencia

- Componente táctil `tx_ultimate_touch` (vendorizado localmente, ver sección de
  arriba) en vez del componente local `components/touch_panel` (que solo
  soportaba 3 botones fijos). Da swipe izquierda/derecha, multi-touch y
  long-press global, además de la posición cruda del toque para poder
  generalizar a 1-4 canales.
- Nightlight automático según la posición del sol (`sun` + `latitude`/`longitude`).
- Feedback visual (flash breve en la tira) al tocar, hacer swipe, multi-touch o
  long-press global.

## Qué se quitó (específico de la instalación anterior del usuario)

El `t5x3.yaml` original tenía integraciones específicas de una casa (text_sensor
de `light.hallway_light` / `light.toilet_light` / `input_boolean.gone_to_bed`
disparando un botón `light_relays` que cambiaba el color de la luz ambiente
según si algún relé estaba encendido). Se quitó de la plantilla por ser
específico de esa instalación; si lo quieres de vuelta, es un `text_sensor:
platform: homeassistant` + `on_value` como antes, ahora llamando a
`script.execute: refresh_led_default` en vez de `button.press: light_relays`.

## Verificar antes de flashear

```
esphome config sonoff-t5.yaml
esphome compile sonoff-t5.yaml
```

Repite el `config` cambiando `relay_count` a `"1"`, `"2"` y `"4"` para
confirmar que cada variante compila y solo expone sus N canales.

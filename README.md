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
                          # uart, tira de LEDs, audio, gestos globales)
  channels_1.yaml         # 1 canal
  channels_2.yaml         # 2 canales
  channels_3.yaml         # 3 canales (tu hardware real; segmentos LED calibrados)
  channels_4.yaml         # 4 canales (best-effort, ver nota abajo)
  orientation_vertical.yaml    # nombres de LEDs/swipe para montaje vertical
  orientation_horizontal.yaml  # idem, montaje horizontal (rotado 90°)
  ble_tracker.yaml        # fixture opcional: esp32_ble_tracker
  bluetooth_proxy.yaml    # fixture opcional: bluetooth_proxy
  web_server.yaml         # fixture opcional: web_server
  nightlight_auto.yaml    # fixture opcional: nightlight automático por sol
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
> (GPIO23, documentado en el SDK de Sonoff), reparte `touch.x` en 4 zonas
> (`<=2` / `<=5` / `<=7` / resto) y usa segmentos de LED dados a mano
> (canal_1=23-31+0, canal_2=1-3+20-22, canal_3=4-6+17-19, canal_4=7-16,
> siguiendo el mismo patrón de arcos simétricos que `channels_2`/`channels_3`).
> Sigue siendo best-effort sin hardware real para confirmar.

## Orientación de montaje

`orientation` (`"vertical"` u `"horizontal"`, default `"vertical"`) controla
cómo se llaman los 4 lados del anillo de LEDs (`light_top`/`light_bottom`/
`light_left`/`light_right`, definidos una sola vez en `common/base.yaml` y
compartidos por todas las variantes) y los sensores de swipe:

- **`vertical`** (panel más alto que ancho, como está montado hoy): los
  nombres coinciden con el diagrama físico (Arriba/Abajo/Izquierda/Derecha), y
  el swipe se nombra "Swipe Top"/"Swipe Bottom" (un swipe nativo
  izquierda/derecha del panel se percibe como un gesto arriba/abajo cuando el
  panel está vertical).
- **`horizontal`** (panel rotado 90°): cada lado rota un cuarto de vuelta
  (Arriba→Derecha→Abajo→Izquierda→Arriba), y el swipe vuelve a nombrarse
  "Swipe left"/"Swipe right".

Selecciona el archivo `common/orientation_${orientation}.yaml` correspondiente
— solo cambia el `name:` mostrado en Home Assistant, no los `id` ni los pines.

## Fixtures opcionales

`esp32_ble_tracker`, `bluetooth_proxy` y `web_server` viven en archivos propios
(`common/ble_tracker.yaml`, `common/bluetooth_proxy.yaml`,
`common/web_server.yaml`) e importados por defecto desde el `packages:` de
`sonoff-t5.yaml`:

```yaml
packages:
  ble_tracker: !include common/ble_tracker.yaml
  bluetooth_proxy: !include common/bluetooth_proxy.yaml
  web_server: !include common/web_server.yaml
```

Para deshabilitar alguna, comenta (o borra) esa línea — nada de substituciones
`true`/`false`, es directamente si la importás o no. `bluetooth_proxy`
requiere que `ble_tracker` también esté importado. `web_server` usa
`!secret web-server_username`/`web-server_password`.

## Nightlight automático por sol (opcional)

`common/nightlight_auto.yaml` es otra extensión del mismo tipo: agrega
`time:` (homeassistant + sntp), `sun:` y el script `refresh_nightlight`, que
cada 5 minutos (y al bootear) prende el switch interno `nightlight` si el sol
ya se puso (`sun.is_below_horizon`) y lo apaga si no. Se importa por defecto
desde `sonoff-t5.yaml`:

```yaml
packages:
  nightlight_auto: !include common/nightlight_auto.yaml
```

Si la comentás, el switch `nightlight` (auto) se queda apagado para siempre
— el nightlight **manual** (switch visible "Nightlight", `nightlight_active`)
sigue andando igual, porque `refresh_led_default` en cada `channels_N.yaml`
prende el color de nightlight si *cualquiera* de los dos switches está
encendido (`id(nightlight).state || id(nightlight_active).state`).

Si la importás, configurá tu ubicación real en `sonoff-t5.yaml` (por defecto
son coordenadas dummy `0,0`, con las que el cálculo de amanecer/atardecer no
sirve):

```yaml
substitutions:
  latitude: "-34.603722°"
  longitude: "-58.381592°"
```

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

- **Audio**: altavoz I2S (`media_player`), migrado al esquema nuevo de
  `speaker:`/`media_player: platform: speaker` (el viejo `media_player:
  platform: i2s_audio` está deprecado). Requiere framework **ESP-IDF** (no
  compila sobre Arduino). Como `announcement_pipeline` y `media_pipeline`
  deben apuntar a un speaker distinto cada uno, se agregó un
  `speaker: platform: mixer` que junta las dos entradas virtuales en el mismo
  DAC físico (`audio_speaker`). También se agregó el `mute_pin` (GPIO26,
  invertido) que faltaba en la migración — sin él el ampli queda muteado y no
  suena nada.
- **Luz ambiente**: la tira de 32 LEDs con todos sus efectos direccionables
  (rainbow, pulse, scan, twinkle, fireworks, flicker), controlable como luz
  normal en HA. Los canales usan particiones de esa misma tira como indicador
  de estado (encendido = `button_color`, apagado con nightlight =
  `nightlight_color`). El driver es `esp32_rmt_led_strip` (reemplaza a
  `neopixelbus`, que está deprecado en ESPHome); funciona igual en Arduino y
  ESP-IDF, así que el cambio de framework fue por el audio, no por los LEDs.

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

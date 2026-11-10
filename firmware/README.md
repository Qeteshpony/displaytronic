# Flipdot Firmware

Firmware for the flipdot controller board (ATmega328P). It drives one or more
DISPLAYTRONIC VS2812 flip-dot displays (28 × 12 dots each) and is controlled via I2C.

## I2C interface

| Property | Value |
|---|---|
| Role | I2C slave |
| Default address | `0x30` (7-bit, can be changed at compile time) |
| Bus speed | 100 kHz (standard mode) |
| Multi-byte values | big-endian (high byte first) |
| Max. transfer size | 32 bytes including the register address |

### Register access

- **Write:** send the register address, followed by the data bytes.
- **Read:** write the register address (without data), then read the number of bytes the register holds.

Writing pixel or row data only changes the internal image buffer. The display
itself changes after an `UPDATE` command. Only dots that differ from what the display
currently shows are flipped.

### Register map

| Address | Name | Access | Bytes | Description |
|---|---|---|---|---|
| `0x00` | `STATUS` | R | 1 | Bit 0: reserved · Bit 1: display initialized · Bit 2: debug output enabled |
| `0x01` | `VERSION` | R | 2 | I2C interface version: major, minor (independent of the firmware release version; major changes on incompatible changes) |
| `0x02` | `COMMAND` | W | 1 | Execute a command, see [Commands](#commands) |
| `0x10` | `NUM_BOARDS` | R/W | 1 | Number of connected displays (1–8). The first write after power-up initializes the display; later writes only trigger a re-initialization and do not change the value (see below) |
| `0x11` | `PULSE_LENGTH` | R/W | 2 | Flip pulse length in µs (max. 1000, default 250) |
| `0x12` | `SLOW_UPDATE_LENGTH` | R/W | 2 | Minimum duration of a slow update in ms (default 1000) |
| `0x13` | `DEBUG` | R/W | 1 | Debug output on UART: `0` = off, `1` = on |
| `0x20` | `VOLTAGE_LOGIC` | R | 2 | Measured logic supply voltage in mV |
| `0x21` | `VOLTAGE_FLIPDOT` | R | 2 | Measured flip voltage in mV |
| `0x30` | `PIXEL` | W | 3 | Set a single dot in the image buffer: x, y, value (`0` = off, `1` = on) |
| `0x31` | `ROW_BITS` | W | 4–31 | Replace part of a row in the image buffer: y, x, length (in dots), followed by the image data (see [Image data](#image-data)) |
| `0x40`–`0x4B` | `ROW_0` … `ROW_11` | R/W | 1–28 | Write: image data of one or more rows into the image buffer · Read: dots currently shown on the display (see [Image data](#image-data)) |
| `0xA0` | `LIGHT_BRIGHTNESS` | R/W | 1 | Brightness of the display lighting: `0` = off, `255` = maximum |
| `0xA1` | `LIGHT_EFFECT` | R/W | 1 | Reserved for lighting effects (currently without function) |

### Commands

Write one of these values to `COMMAND` (`0x02`):

| Value | Name | Description |
|---|---|---|
| `0x01` | `UPDATE` | Flip all dots that differ from the image buffer, as fast as possible |
| `0x02` | `UPDATE_SLOW` | Like `UPDATE`, but spreads the flips over at least `SLOW_UPDATE_LENGTH` ms to reduce noise |
| `0x03` | `CLEAR_BUFFER` | Set all dots in the image buffer to off (display unchanged until next update) |
| `0x04` | `CLEAR_DISPLAY` | Clear the image buffer and update the display immediately |
| `0x05` | `REINIT` | Full initialization with the current number of boards |

### Busy handling

While the display is being updated, the controller disables its I2C interface to keep
the flip pulses precise. During this time it does **not acknowledge its address**: every
transfer fails on the master side (NACK), just like an absent device.

This applies to everything that flips dots:

- the commands `UPDATE`, `UPDATE_SLOW`, `CLEAR_DISPLAY` and `REINIT`
- writing `NUM_BOARDS`

To wait for the end of an update, the master repeats a short transfer (e.g. reading
`STATUS`) until the controller acknowledges again (ACK polling). Use a timeout to detect
a controller that does not respond at all. A rough upper limit for the duration:

| Operation | Approximate maximum duration |
|---|---|
| `UPDATE` | number of changed dots × (`PULSE_LENGTH` + overhead) |
| `UPDATE_SLOW` | `SLOW_UPDATE_LENGTH`, or as `UPDATE` if more dots change than fit into that time |
| Initialization | 2 × all dots × (`PULSE_LENGTH` + overhead) |

One display has 336 dots. The overhead per dot is in the range of tens of µs and will be
specified once measured on real hardware.

### Image data

- The coordinate origin (x = 0, y = 0) is the **top left** dot.
- Multiple displays are treated as one wide display: display 0 covers x = 0–27,
  display 1 covers x = 28–55, and so on.
- One row register holds one bit per column, `ceil(28 × NUM_BOARDS / 8)` bytes in total
  (4 bytes for one display, 28 bytes for eight).
- Writing a row register always replaces the **whole** row. If fewer bytes are sent, the
  remaining dots of that row are set to off. To change only part of a row and keep the
  rest, use `ROW_BITS`.
- If more bytes than one row are written, the data continues with the following rows
  (e.g. 8 bytes to `ROW_0` with one display fill `ROW_0` and `ROW_1`).
- Reading a row register returns the dots **currently shown on the display**, not the
  content of the image buffer. Data written since the last update is only visible after
  the next `UPDATE`. A read always returns exactly one row (`ceil(28 × NUM_BOARDS / 8)` bytes).
- Bit order: byte 0, bit 0 (LSB) is x = 0; byte 0, bit 7 is x = 7; byte 1, bit 0 is x = 8.
- Unused bits at the end of the last byte are ignored.
- `ROW_BITS` uses the same bit order, but the first data bit (byte 0, bit 0) is the dot at
  the given x. Only `length` dots are replaced; surplus bits are ignored. The range is
  clipped at the right edge of the display. With eight displays, a complete row fits into
  one `ROW_BITS` transfer (1 + 3 + 28 = 32 bytes).

### Initialization

After power-up, the display is **not** initialized and does not react to image data or
update commands. The master has to write `NUM_BOARDS` first. This sets the number of
connected displays and starts the initialization: every dot is flipped on and then off.
This brings the display into a known state and keeps all dots moving, even those that
rarely change. `STATUS` bit 1 shows whether the display has been initialized.

The number of displays can only be set once after power-up. Writing `NUM_BOARDS` again,
or sending `REINIT`, repeats the initialization with the number set before.

## Typical sequence

1. After power-up, write `NUM_BOARDS` with the number of connected displays.
2. Wait until the controller acknowledges its address again (initialization finished).
3. Write the new image via the `ROW_x` registers (or single dots via `PIXEL` / `ROW_BITS`).
4. Send `UPDATE` or `UPDATE_SLOW` to `COMMAND`.
5. Wait until the controller acknowledges its address again before sending further commands.

## Notes on the I2C master

### Raspberry Pi

The hardware I2C controller of the Raspberry Pi does not handle clock stretching
correctly. The controller only stretches the clock briefly during normal transfers, so
this is usually not a problem. If you see corrupted data or sporadic errors outside of
updates, use the software I2C overlay instead, e.g. in `/boot/firmware/config.txt`:

```
dtoverlay=i2c-gpio,i2c_gpio_sda=23,i2c_gpio_scl=24
```

This creates an additional I2C bus on the given GPIO pins.

### ESP32 / ESP8266

No special configuration required.

## Debug output

If enabled, the firmware prints debug messages on the UART pins (TXD) of the controller.
Connect a USB serial adapter and use 500000 baud, 8N1.

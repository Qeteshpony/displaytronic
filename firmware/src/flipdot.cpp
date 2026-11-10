#include <Arduino.h>
#include "flipdot.h"
#include "debug.h"
#include "i2c_interface.h"

void Display::init(uint8_t num_boards)
{
    // initialize the display driver. Set the number of boards connected to the controller between 1 and 8
    if (num_boards > 0 && num_boards_ == 0) num_boards_ = num_boards;
    else if (num_boards_ == 0) return;

    if (num_boards_ > MAX_BOARDS) num_boards_ = MAX_BOARDS;

    bytes_per_row_ = (BOARD_WIDTH * num_boards_ + 7) / 8;

    // turn the whole display white and then black again to make sure we have a defined state
    fill(WHITE);
    i2cDisable();
    update();
    clear();
    i2cEnable();

    // turn on status led when display is initialized
    digitalWrite(PIN_STATUS_LED, HIGH);

    DEBUG("Display initialized. Number of boards: %u", num_boards_);
}

void Display::fill(bool color)
{
    // fill the frame buffer with either white or black
    for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++)
    {
        for (uint8_t col = 0; col < BOARD_WIDTH * num_boards_; col++) setPixel(col, row, color);
    }
}

void Display::clear(bool do_update)
{
    // clear the frame buffer and update the display
    memset(frame_, 0, sizeof(frame_));
    if (do_update) update();
}

void Display::setPixel(uint8_t x, uint8_t y, bool color)
{
    // change one pixel in the frame buffer
    if (x >= num_boards_ * BOARD_WIDTH || y >= DISPLAY_HEIGHT) return;
    uint8_t index = x / 8;
    uint8_t bit = x % 8;
    bitWrite(frame_[y][index], bit, color);
}

int8_t Display::getPixel(uint8_t x, uint8_t y) const
{
    if (x >= num_boards_ * BOARD_WIDTH || y >= DISPLAY_HEIGHT) return -1;
    uint8_t index = x / 8;
    uint8_t bit = x % 8;
    return bitRead(frame_[y][index], bit);
}

uint8_t Display::getRowByte(uint8_t row, uint8_t index) const
{
    if (row >= DISPLAY_HEIGHT || index >= getBytesPerRow()) return 0;
    return current_[row][index];
}

void Display::update(bool slowdown)
{
    i2cDisable();  // disable I2C so it doesnt disturb the display update
    updateCode(slowdown);  // run the update
    i2cEnable();  // enable I2C
    digitalWrite(PIN_STATUS_LED, HIGH);  // make sure status LED is on after flipping dots
}

void Display::updateCode(bool slowdown)
{
    // compare frame to current buffer and flip all changed dots
    // slowdown makes a pause after each flip and tries to finish within slow_update_length_ ms
    uint16_t wait_between_bits = 0;
    if (slowdown)
    {
        uint16_t different_bits = 0;
        for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++)
        {
            for (uint8_t byte_index = 0; byte_index < bytes_per_row_; byte_index++)
            {
                different_bits += __builtin_popcount(frame_[row][byte_index] ^ current_[row][byte_index]);
            }
        }
        if (different_bits == 0) return;
        wait_between_bits = slow_update_length_ / different_bits;
    }

    bool first_flip = true;

    for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++)
    {
        for (uint8_t byte_index = 0; byte_index < bytes_per_row_; byte_index++)
        {
            if (uint8_t diff = frame_[row][byte_index] ^ current_[row][byte_index]; diff != 0)
            {
                for (uint8_t bit = 0; bit < 8; bit++)
                {
                    if (bitRead(diff, bit))
                    {
                        if (first_flip) first_flip = false;
                        else delay(wait_between_bits);
                        flipDot(byte_index * 8 + bit, row, bitRead(frame_[row][byte_index], bit));
                        bitWrite(current_[row][byte_index], bit, bitRead(frame_[row][byte_index], bit));
                    }
                }
            }
        }
    }
}

void Display::setPulseLength(uint16_t length)
{
    if (length > MAX_PULSE_LENGTH) length = MAX_PULSE_LENGTH;
    pulse_length_ = length;
}

void Display::setSlowUpdateLength(uint16_t slow_update_length)
{
    slow_update_length_ = slow_update_length;
}

void Display::writeRowBits(uint8_t x, uint8_t y, uint8_t length, const uint8_t* data, uint8_t data_length)
{
    // Replace `length` bits of row `y` starting at column `x` with bits from `data`.
    // data[0] bit 0 is the pixel at x. Surplus bits in data are ignored,
    // the range is clipped to the active display width and to the available data.

    const uint8_t width = BOARD_WIDTH * num_boards_;
    if (y >= DISPLAY_HEIGHT || x >= width) return;
    if (length > width - x) length = width - x;
    if (length > data_length * 8) length = data_length * 8;
    if (length == 0) return;

    const uint8_t first = x / 8;                  // first affected byte
    const uint8_t last = (x + length - 1) / 8;    // last affected byte
    const uint8_t offset = x % 8;                 // shift needed to align source bits
    const uint8_t end = x + length;               // first column after the range
    uint8_t carry = 0;                            // overflow bits from the previous source byte

    for (uint8_t d = first; d <= last; d++)
    {
        const uint8_t j = d - first;
        const uint8_t src = (j < data_length) ? data[j] : 0;

        // align source bits to the bit position in the target byte
        uint8_t aligned;
        if (offset == 0) aligned = src;
        else
        {
            aligned = (src << offset) | carry;
            carry = src >> (8 - offset);
        }

        // mask of the bits in this byte that belong to the range
        const uint8_t byte_start = d * 8;
        const uint8_t lo = (x > byte_start) ? x - byte_start : 0;
        const uint8_t hi = (end < byte_start + 8) ? end - byte_start : 8;
        const uint8_t mask = ((1u << hi) - 1) & ~((1u << lo) - 1);

        frame_[y][d] = (frame_[y][d] & ~mask) | (aligned & mask);
    }
}

void Display::pulse() const
{
    noInterrupts();
    digitalWrite(PIN_PULSE, HIGH);
    delayMicroseconds(pulse_length_);
    digitalWrite(PIN_PULSE, LOW);
    interrupts();
    digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));  // toggle status led with every flip
}

void Display::flipDot(uint8_t x, uint8_t y, bool on)
{
    // DEBUG("flipDot: x=%d y=%d on=%d", x, y, on);  //only enable this when really needed
    uint8_t board = x / BOARD_WIDTH;
    uint8_t board_col = x % BOARD_WIDTH;
    uint8_t segment = board_col / COLS_PER_SEGMENT;
    uint8_t segment_col = board_col % COLS_PER_SEGMENT;
    uint8_t row = y ^ 1;  // flipping the last bit to account for the swapped rows on the display
    setBoard(board);
    setSegment(segment);
    setColumn(segment_col);
    setRow(row);
    setDirection(on);
    pulse();
}

void Display::setBits(const uint8_t value, const uint8_t* pins, const uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        digitalWrite(pins[i], (value >> i) & 1);
    }
}

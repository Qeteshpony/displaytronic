#include <Arduino.h>
#include "flipdot.h"
#include "debug.h"

void Display::init(uint8_t num_boards)
{
    // initialize the display driver. Set the number of boards connected to the controller between 1 and 8
    if (num_boards > MAX_BOARDS) num_boards = MAX_BOARDS;
    if (num_boards < 1) num_boards = 1;
    num_boards_ = num_boards;

    // turn the whole display white and then black again to make sure we have a defined state
    fill(WHITE);
    update();
    clear();

    // turn on status led when display is initialized
    digitalWrite(PIN_STATUS_LED, HIGH);

    DEBUG("Display initialized. Number of boards: %u", num_boards_);
}

void Display::fill(bool color)
{
    // fill the frame buffer with either white or black
    for (uint8_t row = 0; row < BOARD_HEIGHT; row++)
    {
        for (uint8_t col = 0; col < BOARD_WIDTH * num_boards_; col++) setPixel(col, row, color);
    }
}

void Display::clear()
{
    // clear the frame buffer and update the display
    memset(frame_, 0, sizeof(frame_));
    update();
}

void Display::setPixel(uint8_t x, uint8_t y, bool color)
{
    // change one pixel in the frame buffer
    if (x >= num_boards_ * BOARD_WIDTH || y >= BOARD_HEIGHT) return;
    uint8_t index = x / 8;
    uint8_t bit = x % 8;
    bitWrite(frame_[y][index], bit, color);
}

void Display::update(bool slowdown)
{
    // compare frame to current buffer and flip all changed dots
    // slowdown makes a pause after each flip and tries to finish within slow_update_length_ ms
    const uint8_t bytes_per_row = (BOARD_WIDTH * num_boards_ + 7) / 8;
    uint16_t wait_between_bits = 0;
    if (slowdown)
    {
        uint16_t different_bits = 0;
        for (uint8_t row = 0; row < BOARD_HEIGHT; row++)
        {
            for (uint8_t byte_index = 0; byte_index < bytes_per_row; byte_index++)
            {
                different_bits += __builtin_popcount(frame_[row][byte_index] ^ current_[row][byte_index]);
            }
        }
        if (different_bits == 0) return;
        wait_between_bits = slow_update_length_ / different_bits;
    }

    bool first_flip = true;

    for (uint8_t row = 0; row < BOARD_HEIGHT; row++)
    {
        for (uint8_t byte_index = 0; byte_index < bytes_per_row; byte_index++)
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
    digitalWrite(PIN_STATUS_LED, HIGH);  // make sure status LED is on after flipping dots
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
    DEBUG("flipDot: x=%d y=%d on=%d", x, y, on);
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

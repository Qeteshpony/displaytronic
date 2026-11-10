//
// Created by Qetesh on 08.11.26.
//

#pragma once
#include <Arduino.h>
#include "pins.h"

class Display
{
    public:
    static constexpr uint8_t BOARD_WIDTH = 28;
    static constexpr uint8_t DISPLAY_HEIGHT = 12;
    static constexpr uint8_t MAX_BOARDS = 8;
    static constexpr uint8_t SEGMENTS_PER_BOARD = 4;
    static constexpr uint8_t COLS_PER_SEGMENT = 7;
    static constexpr uint16_t MAX_PULSE_LENGTH = 1000;
    static constexpr uint8_t MAX_BYTES_PER_ROW = (BOARD_WIDTH * MAX_BOARDS + 7) / 8;
    static constexpr bool WHITE = true;
    static constexpr bool BLACK = false;

    void init(uint8_t num_boards = 0);
    void fill(bool color);
    void clear(bool do_update = true);
    void setPixel(uint8_t x, uint8_t y, bool color);
    [[nodiscard]] int8_t getPixel(uint8_t x, uint8_t y) const;
    [[nodiscard]] uint8_t getRowByte(uint8_t row, uint8_t index) const;

    void update(bool slowdown = false);
    void setPulseLength(uint16_t length);
    void setSlowUpdateLength(uint16_t slow_update_length);
    void writeRowBits(uint8_t x, uint8_t y, uint8_t length, const uint8_t* data, uint8_t data_length);

    [[nodiscard]] bool isInitialized() const { return num_boards_ > 0; }
    [[nodiscard]] uint8_t getNumBoards() const { return num_boards_; }
    [[nodiscard]] uint8_t getBytesPerRow() const { return bytes_per_row_; }
    [[nodiscard]] uint16_t getPulseLength() const { return pulse_length_; }
    [[nodiscard]] uint16_t getSlowUpdateLength() const { return slow_update_length_; }

    private:

    uint8_t num_boards_ = 0;
    uint8_t bytes_per_row_ = 0;
    uint16_t pulse_length_ = 250;  // pulse length in µs
    uint16_t slow_update_length_ = 1000; // minimum time for update in ms

    uint8_t frame_[DISPLAY_HEIGHT][MAX_BYTES_PER_ROW] = {};
    uint8_t current_[DISPLAY_HEIGHT][MAX_BYTES_PER_ROW] = {};

    void pulse() const;
    void flipDot(uint8_t x, uint8_t y, bool on);
    void updateCode(bool slowdown);

    static void setBits(uint8_t value, const uint8_t* pins, uint8_t count);

    static void setBoard(uint8_t board) { setBits(board, PINS_BOARD, sizeof(PINS_BOARD) / sizeof(PINS_BOARD[0])); }
    static void setSegment(uint8_t segment) { setBits(segment, PINS_SEGMENT, sizeof(PINS_SEGMENT) / sizeof(PINS_SEGMENT[0])); }
    static void setColumn(uint8_t column) { setBits(column, PINS_COLUMN, sizeof(PINS_COLUMN) / sizeof(PINS_COLUMN[0])); }
    static void setRow(uint8_t row) { setBits(row, PINS_ROW, sizeof(PINS_ROW) / sizeof(PINS_ROW[0])); }

    static void setDirection(bool direction) { digitalWrite(PIN_HIGH_LOW, direction); }
};

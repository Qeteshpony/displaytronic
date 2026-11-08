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
    static constexpr uint8_t BOARD_HEIGHT = 12;
    static constexpr uint8_t MAX_BOARDS = 8;
    static constexpr uint8_t SEGMENTS_PER_BOARD = 4;
    static constexpr uint8_t COLS_PER_SEGMENT = 7;
    static constexpr uint16_t MAX_PULSE_LENGTH = 1000;
    static constexpr bool WHITE = true;
    static constexpr bool BLACK = false;

    void init(uint8_t num_boards);
    void fill(bool color);
    void clear();
    void setPixel(uint8_t x, uint8_t y, bool color);
    void update(bool slowdown = false);

    void setPulseLength(uint16_t length);
    void setSlowUpdateLength(uint16_t slow_update_length);

    private:

    uint8_t num_boards_ = 1;
    uint16_t pulse_length_ = 250;  // pulse length in µs
    uint16_t slow_update_length_ = 1000; // minimum time for update in ms

    uint8_t frame_[BOARD_HEIGHT][(BOARD_WIDTH * MAX_BOARDS + 7) / 8] = {};
    uint8_t current_[BOARD_HEIGHT][(BOARD_WIDTH * MAX_BOARDS + 7) / 8] = {};

    void pulse() const;
    void flipDot(uint8_t x, uint8_t y, bool on);

    static void setBits(uint8_t value, const uint8_t* pins, uint8_t count);

    static void setBoard(uint8_t board) { setBits(board, PINS_BOARD, sizeof(PINS_BOARD) / sizeof(PINS_BOARD[0])); }
    static void setSegment(uint8_t segment) { setBits(segment, PINS_SEGMENT, sizeof(PINS_SEGMENT) / sizeof(PINS_SEGMENT[0])); }
    static void setColumn(uint8_t column) { setBits(column, PINS_COLUMN, sizeof(PINS_COLUMN) / sizeof(PINS_COLUMN[0])); }
    static void setRow(uint8_t row) { setBits(row, PINS_ROW, sizeof(PINS_ROW) / sizeof(PINS_ROW[0])); }

    static void setDirection(bool direction) { digitalWrite(PIN_HIGH_LOW, direction); }
};

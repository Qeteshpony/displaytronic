#include <Wire.h>
#include <util/atomic.h>

#include "i2c_interface.h"
#include "debug.h"
#include "version.h"
#include "light.h"
#include "voltage.h"

constexpr uint8_t REG_STATUS = 0x00;
constexpr uint8_t REG_VERSION = 0x01;
constexpr uint8_t REG_COMMAND = 0x02;
constexpr uint8_t REG_NUM_BOARDS = 0x10;
constexpr uint8_t REG_PULSE_LENGTH = 0x11;
constexpr uint8_t REG_SLOW_UPDATE_LENGTH = 0x12;
constexpr uint8_t REG_DEBUG = 0x13;
constexpr uint8_t REG_VOLTAGE_LOGIC = 0x20;
constexpr uint8_t REG_VOLTAGE_FLIPDOT = 0x21;
constexpr uint8_t REG_PIXEL = 0x30;
constexpr uint8_t REG_ROW_BITS = 0x31;
constexpr uint8_t REG_ROW_00 = 0x40;
constexpr uint8_t REG_ROW_11 = 0x4B;
constexpr uint8_t REG_LIGHT_BRIGHTNESS = 0xA0;

constexpr uint8_t STATUS_DISPLAY_INITIALIZED = 1;
constexpr uint8_t STATUS_DEBUG_ENABLED = 2;

constexpr uint8_t CMD_UPDATE = 0x01;
constexpr uint8_t CMD_UPDATE_SLOW = 0x02;
constexpr uint8_t CMD_CLEAR_BUFFER = 0x03;
constexpr uint8_t CMD_CLEAR_DISPLAY = 0x04;
constexpr uint8_t CMD_REINIT = 0x05;

constexpr uint8_t RX_BUFFER_SIZE = 56; //maximum two rows of data

static Display* display_ = nullptr;
static volatile uint8_t register_;
static volatile uint8_t pending_command_ = 0;
static volatile uint8_t pending_data_ = 0;
static uint8_t disable_counter_ = 0;

static void onReceive(int count)
{
    if (count == 0) return;
    register_ = Wire.read();

    switch (register_)
    {
    case REG_COMMAND:
        {
            if (Wire.available() < 1) return;
            switch (uint8_t command = Wire.read())
            {
            case CMD_CLEAR_BUFFER:
                {
                    display_->clear(false);
                    break;
                }
            default:
                pending_command_ = command;
                pending_data_ = 0;
                break;
            }
            break;
        }
    case REG_NUM_BOARDS:
        {
            if (Wire.available() < 1) return;
            pending_command_ = CMD_REINIT;
            pending_data_ = Wire.read();
            break;
        }
    case REG_PULSE_LENGTH:
        {
            if (Wire.available() < 2) return;
            uint8_t high = Wire.read();
            uint8_t low = Wire.read();
            display_->setPulseLength((high << 8) | low);
            break;
        }
    case REG_SLOW_UPDATE_LENGTH:
        {
            if (Wire.available() < 2) return;
            uint8_t high = Wire.read();
            uint8_t low = Wire.read();
            display_->setSlowUpdateLength((high << 8) | low);
            break;
        }
    case REG_DEBUG:
        {
            if (Wire.available() < 1) return;
            uint8_t en = Wire.read();
            if (en >= 2) return;
            debugSetEnabled(en);
            break;
        }
    case REG_PIXEL:
        {
            if (Wire.available() < 3) return;
            if (!display_->isInitialized()) return;
            uint8_t x = Wire.read();
            uint8_t y = Wire.read();
            uint8_t color = Wire.read();
            display_->setPixel(x, y, color);
            break;
        }
    case REG_LIGHT_BRIGHTNESS:
        {
            if (Wire.available() < 1) return;
            lightSetBrightness256(Wire.read());
            break;
        }
    case REG_ROW_BITS:
        {
            // make sure we have at least one byte of data
            if (Wire.available() < 4) return;
            if (!display_->isInitialized()) return;
            // read the metadata
            uint8_t y = Wire.read();
            uint8_t x = Wire.read();
            uint8_t length = Wire.read();
            // read the rest of the available bytes into a buffer...
            uint8_t buffer[RX_BUFFER_SIZE] = {};
            uint8_t i = 0;
            while (Wire.available() > 0 && i < RX_BUFFER_SIZE) buffer[i++] = Wire.read();
            // ... and pass that to the display
            display_->writeRowBits(x, y, length, buffer, i);
            break;
        }
    case REG_ROW_00 ... REG_ROW_11:
        {
            if (Wire.available() < 1) return;
            if (!display_->isInitialized()) return;
            // calculate actual row from the register and read all available bytes into a buffer
            uint8_t y = register_ - REG_ROW_00;
            uint8_t buffer[RX_BUFFER_SIZE] = {};
            uint8_t bytes = 0;
            while (Wire.available() > 0 && bytes < RX_BUFFER_SIZE) buffer[bytes++] = Wire.read();
            uint8_t rows = (bytes + display_->getBytesPerRow() - 1 ) / display_->getBytesPerRow();
            for (uint8_t row = 0; row < rows; row++)
            {
                if (y + row >= Display::DISPLAY_HEIGHT) break;
                display_->writeRowBits(0, y + row, display_->getBytesPerRow() * 8, buffer + row * display_->getBytesPerRow(), display_->getBytesPerRow());
            }
            break;
        }
    default:
        break;
    }
}

static void onRequest()
{
    switch (register_)
    {
    case REG_STATUS:
        {
            uint8_t status = 0;
            status |= display_->isInitialized() << STATUS_DISPLAY_INITIALIZED;
            status |= debugIsEnabled() << STATUS_DEBUG_ENABLED;
            Wire.write(status);
            break;
        }
    case REG_VERSION:
        Wire.write(INTERFACE_VERSION_MAJOR);
        Wire.write(INTERFACE_VERSION_MINOR);
        break;
    case REG_NUM_BOARDS:
        Wire.write(display_->getNumBoards());
        break;
    case REG_PULSE_LENGTH:
        {
            uint16_t length = display_->getPulseLength();
            Wire.write(length >> 8);
            Wire.write(length & 0xFF);
            break;
        }
    case REG_SLOW_UPDATE_LENGTH:
        {
            uint16_t length = display_->getSlowUpdateLength();
            Wire.write(length >> 8);
            Wire.write(length & 0xFF);
            break;
        }
    case REG_DEBUG:
        {
            Wire.write(debugIsEnabled());
            break;
        }
    case REG_VOLTAGE_LOGIC:
        {
            uint16_t voltage = getVoltageLogic();
            Wire.write(voltage >> 8);
            Wire.write(voltage & 0xFF);
            break;
        }
    case REG_VOLTAGE_FLIPDOT:
        {
            uint16_t voltage = getVoltageFlipdot();
            Wire.write(voltage >> 8);
            Wire.write(voltage & 0xFF);
            break;
        }
    case REG_ROW_00 ... REG_ROW_11:
        {
            if (!display_->isInitialized()) break;
            uint8_t row = register_ - REG_ROW_00;
            for (uint8_t i = 0; i < display_->getBytesPerRow(); i++)
            {
                Wire.write(display_->getRowByte(row, i));
            }
            break;
        }
    case REG_LIGHT_BRIGHTNESS:
        {
            Wire.write(lightGetBrightness256());
            break;
        }
    default:
        break;
    }

}

void i2cInit(Display& display)
{
    display_ = &display;
    i2cEnable();
}

void i2cEnable()
{
    if (disable_counter_ > 1)
    {
        disable_counter_--;
    }
    else
    {
        disable_counter_ = 0;
        Wire.begin(I2C_ADDRESS);
        Wire.onReceive(onReceive);
        Wire.onRequest(onRequest);
    }
}

void i2cDisable()
{
    if (disable_counter_ == 0) Wire.end();
    disable_counter_++;
}

void i2cLoop()
{
    uint8_t command = 0;
    uint8_t data = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        command = pending_command_;
        data = pending_data_;
        pending_command_ = 0;
        pending_data_ = 0;
    }
    switch (command)
    {
    case CMD_UPDATE:
        {
            display_->update();
            break;
        }
    case CMD_UPDATE_SLOW:
        {
            display_->update(true);
            break;
        }
    case CMD_CLEAR_DISPLAY:
        {
            display_->clear();
            break;
        }
    case CMD_REINIT:
        {
            if (data != 0 && !display_->isInitialized()) display_->init(data);
            else display_->init();
            break;
        }
    default:
        break;
    }
}
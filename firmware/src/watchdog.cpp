#include <avr/wdt.h>
#include "watchdog.h"

static uint8_t boot_reason __attribute__((section(".noinit")));

static void wdtInitEarly() __attribute__((naked, used, section(".init3")));
static void wdtInitEarly()
{
    boot_reason = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

void wdtEnable()
{
    wdt_enable(WDTO_1S);
}

void wdtDisable()
{
    wdt_disable();
}

void wdtFeed()
{
    wdt_reset();
}

uint8_t wdtGetBootReason()
{
    return boot_reason;
}
#include "types.h"
#include "io.h"
#include "intr.h"

#define CURRENT_CENTURY 2000

#define RTC_IRQ 8
#define RTC_IRQ_RATE 1024 //hz

#define RTC_REGISTER 0x70
#define RTC_DATA 0x71

#define CMOS_SECONDS 0x00
#define CMOS_MINUTES 0x02
#define CMOS_HOURS 0x04
#define CMOS_WEEKDAY 0x06
#define CMOS_DAYOFMONTH 0x07
#define CMOS_MONTH 0x08
#define CMOS_YEAR 0x09
#define CMOS_CENTURY 0x32
#define CMOS_STATUSA 0x0A
#define CMOS_STATUSB 0x0B
#define RTC_REGISTERB 0x8B
#define RTC_REGISTERC 0x0C

#define RTC_WEEKDAY_SUNDAY 1
#define RTC_WEEKDAY_MONDAY 2
#define RTC_WEEKDAY_TUESDAY 3
#define RTC_WEEKDAY_WEDNESDAY 4
#define RTC_WEEKDAY_THURSDAY 5
#define RTC_WEEKDAY_FRIDAY 6
#define RTC_WEEKDAY_SATURDAY 7

#define BCD_TO_BIN(bcd) ((bcd & 0x0F) + (((bcd & 0x70) / 16) * 10)) | (bcd & 0x80)

typedef struct rtcTime_t
{
    uint8 second,
        minute,
        hour,
        dayOfWeek,
        dayOfMonth,
        month;
    uint16 year;
} rtcTime_t;

static inline uint8 RTC__readRegister(uint8 r)
{
    IO_Out8(RTC_REGISTER, r);
    return IO_In8(RTC_DATA);
}

static inline uint8 RTC__isUpdateInProgress()
{
    return RTC__readRegister(CMOS_STATUSA) & 0x80;
}

static inline uint8 RTC_ackInterrupt()
{
    return RTC__readRegister(RTC_REGISTERC);
}

static inline void RTC_readRTC(rtcTime_t *time)
{
    // wait for update in progress flag to be cleared
    while (RTC__isUpdateInProgress())
        ;

    // read
    time->second = RTC__readRegister(CMOS_SECONDS);
    time->minute = RTC__readRegister(CMOS_MINUTES);
    time->hour = RTC__readRegister(CMOS_HOURS);
    time->dayOfWeek = RTC__readRegister(CMOS_WEEKDAY);
    time->dayOfMonth = RTC__readRegister(CMOS_DAYOFMONTH);
    time->month = RTC__readRegister(CMOS_MONTH);
    time->year = RTC__readRegister(CMOS_YEAR);

    // get register b, do conversions needed to get to normal (24h binary) time format
    uint8 regB = RTC__readRegister(CMOS_STATUSB);

    // BCD to binary
    if (regB & 0x04)
    {
        time->second = BCD_TO_BIN(time->second);
        time->minute = BCD_TO_BIN(time->minute);
        time->hour = BCD_TO_BIN(time->hour);
        time->dayOfMonth = BCD_TO_BIN(time->dayOfMonth);
        time->month = BCD_TO_BIN(time->month);
        time->year = BCD_TO_BIN(time->year);
    }

    // 12h to 24h clock
    if (!(regB & 0x02) && (time->hour & 0x80))
    {
        time->hour = ((time->hour & 0x7F) + 12) % 24;
    }

    // calculate full year
    time->year += CURRENT_CENTURY;
}

static inline void RTC_registerIRQ(IntrHandler handler)
{
    // disable interrupts
    Intr_Disable();

    // register IRQ 8
    Intr_SetMask(RTC_IRQ, TRUE);
    Intr_SetHandler(IRQ_VECTOR(RTC_IRQ), handler);

    // read register C to acknowledge any pendign interrupts before we registered this IRQ
    RTC_ackInterrupt();

    // select register B, disable NMI
    // and read the current value of Register B
    IO_Out8(RTC_REGISTER, RTC_REGISTERB);
    uint8 p = IO_In8(RTC_DATA);

    // select the register again (the read reset the register)
    IO_Out8(RTC_REGISTER, RTC_REGISTERB);

    // write back the previous value, ORed with 0x40
    // this turns on bit 6 of register B
    IO_Out8(RTC_DATA, p | 0x40);

    // re- enable interrupts
    Intr_Enable();
}

#define TICKS_FREQUENCY RTC_IRQ_RATE

volatile uint32 Time__ticks;

void Time__timerIRQHandler(int v)
{
    Time__ticks++;
    RTC_ackInterrupt();
}

static inline void Time_init()
{
    RTC_registerIRQ(Time__timerIRQHandler);
}

static inline uint32 Time_now()
{
    return Time__ticks;
}

static inline void Time_delay(uint32 ticks)
{
    uint32 end = Time_now() + ticks;
    while (Time_now() < end)
        ;
}
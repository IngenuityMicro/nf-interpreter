//
// Copyright (c) 2017 The nanoFramework project contributors
// Portions Copyright (c) Microsoft Corporation.  All rights reserved.
// See LICENSE file in the project root for full license information.
//
#ifndef _NANOPAL_H_
#define _NANOPAL_H_ 1

//#include <nanoHAL_types.h>

//#include <AsyncProcCalls_decl.h>

typedef void (*HAL_CALLBACK_FPN)( void* arg );

struct HAL_CALLBACK
{
public:
    HAL_CALLBACK_FPN EntryPoint;
    void*            Argument;

public:
    void Initialize( HAL_CALLBACK_FPN EntryPoint, void* Argument )
    {
        this->EntryPoint = EntryPoint;
        this->Argument   = Argument;
    }

    void SetArgument( void* Argument )
    {
        this->Argument = Argument;
    }

    HAL_CALLBACK_FPN GetEntryPoint() const { return this->EntryPoint; }
    void*            GetArgument  () const { return this->Argument  ; }

    void Execute() const
    {
        HAL_CALLBACK_FPN EntryPoint = this->EntryPoint;
        void*            Argument   = this->Argument; 

        if(EntryPoint)
        {
            EntryPoint( Argument );
        }
    }
};


struct HAL_CONTINUATION : public HAL_DblLinkedNode<HAL_CONTINUATION>
{

private:
    HAL_CALLBACK Callback;

public:
    void InitializeCallback( HAL_CALLBACK_FPN EntryPoint, void* Argument = NULL );

    void SetArgument( void* Argument )
    {
        Callback.SetArgument( Argument );
    }

    HAL_CALLBACK_FPN GetEntryPoint() const { return Callback.GetEntryPoint(); }
    void*            GetArgument  () const { return Callback.GetArgument  (); }

    void Execute() const { Callback.Execute(); }

    bool IsLinked();
    void Enqueue();
    void Abort  ();

    //--//

    static void Uninitialize();

    static void InitializeList();

    static BOOL Dequeue_And_Execute();
};

//--//

struct HAL_COMPLETION : public HAL_CONTINUATION
{
    UINT64 EventTimeTicks;
    BOOL   ExecuteInISR;

#if defined(_DEBUG)
    UINT64 Start_RTC_Ticks;
#endif

    void InitializeForISR( HAL_CALLBACK_FPN EntryPoint, void* Argument = NULL )
    {
        ExecuteInISR = TRUE;

        InitializeCallback( EntryPoint, Argument );
    }

    void InitializeForUserMode( HAL_CALLBACK_FPN EntryPoint, void* Argument = NULL )
    {
        ExecuteInISR = FALSE;

        InitializeCallback( EntryPoint, Argument );
    }

    void EnqueueTicks               ( UINT64 EventTimeTicks        );
    void EnqueueDelta64             ( UINT64 uSecFromNow           );
    void EnqueueDelta               ( UINT32 uSecFromNow           );
    
    void Abort();

    void Execute();

    //--//

    static void Uninitialize();

    static void InitializeList();

    static void DequeueAndExec();

    static void WaitForInterrupts( UINT64 Expire, UINT32 sleepLevel, UINT64 wakeEvents );
};

//--//

//#include <StateDebounce_decl.h>
//#include <Backlight_decl.h>
//#include <BatteryCharger_decl.h>
//#include <BatteryMeasurement_decl.h>

//#include <COM_decl.h>
BOOL DebuggerPort_Initialize(COM_HANDLE ComPortNum);
BOOL DebuggerPort_Uninitialize(COM_HANDLE ComPortNum);

// max retries is the number of retries if the first attempt fails, thus the maximum
// total number of attempts is maxRretries + 1 since it always tries at least once.
int  DebuggerPort_Write(COM_HANDLE ComPortNum, const char* Data, size_t size, int maxRetries = 99);
int  DebuggerPort_Read(COM_HANDLE ComPortNum, char* Data, size_t size);
BOOL DebuggerPort_Flush(COM_HANDLE ComPortNum);
BOOL DebuggerPort_IsSslSupported(COM_HANDLE ComPortNum);
BOOL DebuggerPort_UpgradeToSsl(COM_HANDLE ComPortNum, UINT32 flags);
BOOL DebuggerPort_IsUsingSsl(COM_HANDLE ComPortNum);

//
//#include <Display_decl.h>

//#include <Power_decl.h> (must be before events_decl.h)

//
// !!! KEEP IN SYNC WITH Microsoft.SPOT.Hardware.SleepLevel !!!
//
enum SLEEP_LEVEL
{
    SLEEP_LEVEL__AWAKE         = 0x00,
    SLEEP_LEVEL__SELECTIVE_OFF = 0x10,
    SLEEP_LEVEL__SLEEP         = 0x20,
    SLEEP_LEVEL__DEEP_SLEEP    = 0x30,
    SLEEP_LEVEL__OFF           = 0x40,
};


//#include <events_decl.h>

// destructive read system event flags
UINT32 Events_Get( UINT32 EventsOfInterest );

// non-destructive read system event flags
UINT32 Events_MaskedRead( UINT32 EventsOfInterest );


// returns 0 for timeout, non-zero are events that have happened and were asked to be waiting on (non-destructive read)
// timeout limit is about 3034 milliseconds currently
// values greater than this are capped to this

// sleep relative time into the future, or until a SystemEvent occurs, which occurs first
// timeout is a non-negative number of 1mSec ticks, or -1 (any negative value) to sleep forever until a SystemEvent occurs

// Events_WaitForEvents(0, n), sleeps for n milliseconds independent of events
// Events_WaitForEvents(0, EVENTS_TIMEOUT_INFINITE) sleeps forever.  Don't do that.
// Events_WaitForEvents(flags, EVENTS_TIMEOUT_INFINITE) waits forever for that event.

#define EVENTS_TIMEOUT_INFINITE 0xFFFFFFFF

UINT32 Events_WaitForEvents        ( UINT32 sleepLevel, UINT32 WakeupSystemEvents, UINT32 Timeout_Milliseconds );
UINT32 Events_WaitForEventsInternal( UINT32 sleepLevel, UINT32 WakeupSystemEvents, UINT32 Timeout_Milliseconds );

__inline UINT32 Events_WaitForEvents( UINT32 WakeupSystemEvents, UINT32 Timeout_Milliseconds )
{
    return Events_WaitForEvents( SLEEP_LEVEL__SLEEP, WakeupSystemEvents, Timeout_Milliseconds );
}

void Events_SetBoolTimer(BOOL* TimerCompleteFlag, UINT32 MillisecondsFromNow);


//#include <palevent_decl.h>

//#include <heap_decl.h>

// This function returns location of the CLR heap.
void HeapLocation(UINT8*& BaseAddress, UINT32& SizeInBytes);


//#include <gesture_decl.h>
//#include <ink_decl.h>
//
//#include <graphics_decl.h>
//
//#include <I2C_decl.h>
//#include <io_decl.h>
//#include <instrumentation_decl.h>
//
//#include <FS_decl.h>
//
//#include <GPIO_button_decl.h>
//#include <piezo_decl.h>
//

//#include <Power_decl.h>

//
// !!! KEEP IN SYNC WITH Microsoft.SPOT.Hardware.PowerLevel !!!
//
enum POWER_LEVEL
{
    POWER_LEVEL__HIGH_POWER = 0x10,
    POWER_LEVEL__MID_POWER  = 0x20,
    POWER_LEVEL__LOW_POWER  = 0x30,
};

BOOL CPU_IsSoftRebootSupported();


//#include <Security_decl.h>
//#include <Sockets_decl.h>

//#include <Time_decl.h>

#define TIME INT64

#define TIME_CONVERSION__TO_MILLISECONDS    10000
#define TIME_CONVERSION__TO_SECONDS         10000000
#define TIME_CONVERSION__TICKUNITS         10000
#define TIME_CONVERSION__ONESECOND         1
#define TIME_CONVERSION__ONEMINUTE         60
#define TIME_CONVERSION__ONEHOUR           3600
#define TIME_CONVERSION__ONEDAY            86400

#define TIMEOUT_ZERO      LONGLONGCONSTANT(0x0000000000000000)
#define TIMEOUT_INFINITE  LONGLONGCONSTANT(0x7FFFFFFFFFFFFFFF)

#define TIME_ZONE_OFFSET    ((INT64)Time_GetTimeZoneOffset() * 600000000)

/// <summary>
/// Initializes PAL Time drivers, must be called before any of the Time_* PAL
/// methods could be used.
/// </summary>
HRESULT    Time_Initialize  (                     );

/// <summary>
/// UTC time according to this system. 
/// </summary>
/// <returns>Returns current UTC time in 100ns elapsed since 1/1/1601:00:00:00.000 UTC.</returns>
INT64       Time_GetUtcTime();

/// <summary>
/// Local time according to the Time subsystem.
/// </summary>
/// <returns>Local time in 100ns elapsed since 1/1/1601:00:00:00.000 local time.</returns>
INT64       Time_GetLocalTime();

/// <summary>
/// Offset from GMT.
/// </summary>
/// <returns>In minutes, for example Pacific Time would be GMT-8 = -480.</returns>
INT32 Time_GetTimeZoneOffset();

/// <summary>
/// Retrieves time since device was booted.
/// </summary>
/// <returns>Time in 100ns.</returns>
INT64 Time_GetMachineTime();

/// <summary>
/// Converts 64bit time value to SystemTime structure. 64bit time is assumed as an offset from 1/1/1601:00:00:00.000 in 100ns.
/// </summary>
/// <returns>True if conversion is successful.</returns>
BOOL       Time_ToSystemTime(TIME time, SYSTEMTIME* systemTime);

/// <summary>
/// Retrieves number of days given a month and a year. Calculates for leap years.
/// </summary>
/// <returns>S_OK if successful.</returns>
HRESULT    Time_DaysInMonth(INT32 year, INT32 month, INT32* days);

/// <summary>
/// Retrieves number of days since the beginning of the year given a month and a year. Calculates for leap years.
/// </summary>
/// <returns>S_OK if successful.</returns>
HRESULT    Time_AccDaysInMonth(INT32 year, INT32 month, INT32* days);

/// <summary>
/// Converts SystemTime structure to 64bit time, which is assumed as an offset from 1/1/1601:00:00:00.000 in 100ns.
/// </summary>
/// <returns>Time value.</returns>
TIME       Time_FromSystemTime(const SYSTEMTIME* systemTime);

/// APIs to convert between types
BOOL       Time_TimeSpanToStringEx(const INT64& ticks, LPSTR& buf, size_t& len);
LPCSTR     Time_CurrentDateTimeToString();


INT64   HAL_Time_TicksToTime(UINT64 Ticks);
INT64   HAL_Time_CurrentTime();

extern "C"
{
    UINT64  HAL_Time_CurrentTicks();
}

//#include <TimeService_decl.h>
//#include <TouchPanel_decl.h>

//#include <nanocrt_decl.h>

#if defined(PLATFORM_ARM)
#pragma check_printf_formats   /* hint to the compiler to check f/s/printf format */
#endif
int hal_vprintf( const char* format, va_list arg );

#if defined(PLATFORM_ARM)
#pragma check_printf_formats   /* hint to the compiler to check f/s/printf format */
#endif
int hal_vfprintf( COM_HANDLE stream, const char* format, va_list arg );

#if defined(PLATFORM_ARM)
#pragma check_printf_formats   /* hint to the compiler to check f/s/printf format */
#endif
int hal_snprintf( char* buffer, size_t len, const char* format, ... );

int hal_vsnprintf( char* buffer, size_t len, const char* format, va_list arg );

#if defined(PLATFORM_ARM)
#define printf     DoNotUse_*printf []
#define sprintf    DoNotUse_*printf []
#define fprintf    DoNotUse_*printf []

#define _printf    DoNotUse_*printf []
#define _sprintf   DoNotUse_*printf []
#define _fprintf   DoNotUse_*printf []

#define snprintf   DoNotUse_*printf []
#define vsnprintf  DoNotUse_*printf []

#define _snprintf  DoNotUse_*printf []
#define _vsnprintf DoNotUse_*printf []

#define strcpy    DoNotUse_*strcpy  []
#define strncpy   DoNotUse_*strcpy  []
#define strlen    DoNotUse_*strlen  []
#define strncmp   DoNotUse_*strncmp  []


int hal_strcpy_s ( char* strDst, size_t sizeInBytes, const char* strSrc );
int hal_strncpy_s( char* strDst, size_t sizeInBytes, const char* strSrc, size_t count );
size_t hal_strlen_s (const char * str);
int hal_strncmp_s( const char* str1, const char* str2, size_t num );

#elif defined(_WIN32)

#define hal_strcpy_s(strDst, sizeInBytes, strSrc) strcpy_s(strDst, sizeInBytes, strSrc)
#define hal_strncpy_s(strDst, sizeInBytes, strSrc, count) strncpy_s(strDst, sizeInBytes, strSrc, count)
#define hal_strlen_s(str) strlen(str)
#define hal_strncmp_s(str1, str2, num) strncmp(str1, str2, num)

#else
!ERROR
#endif


//#include <USART_decl.h>
//#include <USB_decl.h>
//#include <PWM_decl.h>
//#include <Analog_decl.h>
//#include <AnalogOut_decl.h>
//
#include "Blockstorage_decl.h"

//#include <SD_decl.h>

#endif // _NANOPAL_H_


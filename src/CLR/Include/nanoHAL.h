//
// Copyright (c) 2017 The nanoFramework project contributors
// Portions Copyright (c) Microsoft Corporation.  All rights reserved.
// See LICENSE file in the project root for full license information.
//

#ifndef _NANOHAL_H_
#define _NANOHAL_H_ 1

#include <stdio.h>
#include <string.h>
#include <stdarg.h>























#include <nanoHAL_Types.h>
#include <nanoHAL_ReleaseInfo.h>

//#if !defined(_WIN32) && !defined(FIQ_SAMPLING_PROFILER) && !defined(HAL_REDUCESIZE) && defined(PROFILE_BUILD)
//#define ENABLE_NATIVE_PROFILER
//#endif

//#include "..\pal\Diagnostics\Native_Profiler.h"

#define NATIVE_PROFILE_CLR_DEBUGGER()
#define NATIVE_PROFILE_CLR_UTF8_DECODER()
#define NATIVE_PROFILE_CLR_CORE()
#define NATIVE_PROFILE_CLR_MESSAGING()
#define NATIVE_PROFILE_CLR_SERIALIZATION()
#define NATIVE_PROFILE_CLR_NETWORK()
#define NATIVE_PROFILE_CLR_I2C()
#define NATIVE_PROFILE_CLR_DIAGNOSTICS()
#define NATIVE_PROFILE_CLR_HARDWARE()
#define NATIVE_PROFILE_CLR_GRAPHICS()
#define NATIVE_PROFILE_CLR_STARTUP()
#define NATIVE_PROFILE_CLR_HEAP_PERSISTENCE()
#define NATIVE_PROFILE_CLR_IOPORT()
#define NATIVE_PROFILE_CLR_IO()


#if defined(_MSC_VER)

#define ADS_PACKED
#define GNU_PACKED


































































#else
!ERROR
#endif

//--//
















//--//

typedef UINT32 GPIO_PIN;

//--//

struct GPIO_FLAG
{
    GPIO_PIN  Pin;
    BOOL      ActiveState;
};

//--//








// COM_HANDLE Defines a type representing both a port type or "transport" and a port number
// The COM_HANDLE is a multi bit field value with the following bit fields usage
//    |--------+--------+--------+--------|
//    |33222222|22221111|111111  |        |
//    |10987654|32109876|54321098|76543210| bit position
//    |--------+--------+--------+--------|
//    |00000000|00000000|TTTTTTTT|pppppppp| ( transport != USB_TRANSPORT )
//    |--------+--------+--------+--------|
//    |00000000|00000000|TTTTTTTT|cccppppp| ( transport == USB_TRANSPORT )
//    |--------+--------+--------+--------|
// 
// where:
//    T => Transport type
//              USART_TRANSPORT => 1
//                USB_TRANSPORT => 2
//             SOCKET_TRANSPORT => 3
//              DEBUG_TRANSPORT => 4
//                LCD_TRANSPORT => 5
//        FLASH_WRITE_TRANSPORT => 6
//          MESSAGING_TRANSPORT => 7
//            GENERIC_TRANSPORT => 8
//    p => port instance number 
//        Port instances in the handle are 1 based. (e.g. p == 0 is invalid except when T == 0 )
//    c -> Controller instance number ( USB_TRANSPORT only )
//
//    NULL_PORT => T==0 && p == 0
//
// GENERIC_TRANSPORT is any custom port that isn't one of the above, they 
// are implemented for the DebugPort_xxxx APIs and the port number is 
// an index into a const global table of port interfaces (structure of
// function pointers) These allow custom extensions to the normal transports
// without needing to continue defining additional transport types and modifying
// switch on transport code. To keep compatibility high and code churn low, the
// previous legacy transports remain though they should be considered deprecated.
typedef INT32 COM_HANDLE;

#define TRANSPORT_SHIFT             8
#define TRANSPORT_MASK              (0xFF << TRANSPORT_SHIFT)
#define PORT_NUMBER_MASK            0x00FF

// Macro to extract the transport type from a COM_HANDLE
#define ExtractTransport(x)         ((UINT32)(x) & TRANSPORT_MASK)

// Macro to extract well-known system event flag ids from a COM_HANDLE
#define ExtractEventFromTransport(x) (ExtractTransport(x) == USART_TRANSPORT     ? SYSTEM_EVENT_FLAG_COM_IN: \
                                      ExtractTransport(x) == USB_TRANSPORT       ? SYSTEM_EVENT_FLAG_USB_IN: \
                                      ExtractTransport(x) == SOCKET_TRANSPORT    ? SYSTEM_EVENT_FLAG_SOCKET: \
                                      ExtractTransport(x) == GENERIC_TRANSPORT   ? SYSTEM_EVENT_FLAG_GENERIC_PORT: \
                                      ExtractTransport(x) == DEBUG_TRANSPORT     ? SYSTEM_EVENT_FLAG_DEBUGGER_ACTIVITY: \
                                      ExtractTransport(x) == MESSAGING_TRANSPORT ? SYSTEM_EVENT_FLAG_MESSAGING_ACTIVITY: \
                                      0) \

#define USART_TRANSPORT             (1 << TRANSPORT_SHIFT)
//#define COM_NULL                    ((COM_HANDLE)(USART_TRANSPORT))

#define USB_TRANSPORT               (2 << TRANSPORT_SHIFT)
//#define USB_CONTROLLER_SHIFT        5
//#define USB_CONTROLLER_MASK         0xE0
//#define USB_STREAM_MASK             0x00FF
//#define USB_STREAM_INDEX_MASK       0x001F

#define SOCKET_TRANSPORT            (3 << TRANSPORT_SHIFT)
#define COM_SOCKET_DBG              ((COM_HANDLE)(SOCKET_TRANSPORT + 1))

#define DEBUG_TRANSPORT             (4 << TRANSPORT_SHIFT)







#define MESSAGING_TRANSPORT         (7 << TRANSPORT_SHIFT)

#define GENERIC_TRANSPORT           (8 << TRANSPORT_SHIFT)











































// Creates a COM_HANDLE value for a platform specific port number
#define ConvertCOM_DebugHandle(x)    ((COM_HANDLE)((x) + DEBUG_TRANSPORT     + 1))







typedef UINT32 FLASH_WORD;

































































struct HAL_DRIVER_CONFIG_HEADER
{
    UINT32 Enable;
};

struct HAL_SYSTEM_MEMORY_CONFIG
{
    UINT32 Base;
    UINT32 Size;
};

struct HAL_SYSTEM_CONFIG
{
    static const UINT32 c_MaxDebuggers = 1;
    static const UINT32 c_MaxMessaging = 1;

    HAL_DRIVER_CONFIG_HEADER Header;

    //--//

    COM_HANDLE               DebuggerPorts[c_MaxDebuggers];
    COM_HANDLE               MessagingPorts[c_MaxMessaging];
    // communication channel for debug messages in the debugger
    // which may be VS, MFDEPLOY, etc... Accessed via debug_printf
    // in the HAL/PAL and System.Diagnostics.Debug.Print() in managed
    // applications
    COM_HANDLE               DebugTextPort;

    UINT32                   USART_DefaultBaudRate;
    // internal HAL/PAL debug/tracing channel, this is separate
    // to allow tracing messages in the driver that implements
    // the transport for the Debugger and DebugTextPort. This
    // channel is accessed via hal_printf() in the HAL/PAL
    COM_HANDLE               stdio;

    HAL_SYSTEM_MEMORY_CONFIG RAM1;
    HAL_SYSTEM_MEMORY_CONFIG FLASH;

    //--//

    static LPCSTR GetDriverName() { return "HAL_SYSTEM"; }
};


extern HAL_SYSTEM_CONFIG  HalSystemConfig;






























































































































































//--//
// Function macros

void HAL_Assert  ( LPCSTR Func, int Line, LPCSTR File );
// HAL_AssertEx is defined in the processor or platform selector files.
extern void HAL_AssertEx();

#if defined(PLATFORM_ARM)
    #if !defined(BUILD_RTM)
        #define       ASSERT(i)  { if(!(i)) HAL_AssertEx(); }
        #define _SIDE_ASSERTE(i) { if(!(i)) HAL_AssertEx(); }
    #endif
#else
    #if defined(_DEBUG)
#if !defined _ASSERTE
#error
#endif
        #define       ASSERT(i)  _ASSERTE(i)
        #define _SIDE_ASSERTE(i) _ASSERTE(i)
    #endif
#endif

#ifndef ASSERT
#define ASSERT(i)
#endif

#ifndef _ASSERTE
#define _ASSERTE(expr) ASSERT(expr)
#endif

#ifndef _SIDE_ASSERTE
#define _SIDE_ASSERTE(expr) (expr)
#endif

#if STATIC_ASSERT_SUPPORTED
#define CT_ASSERT_STRING( x ) #x
#define CT_ASSERT_UNIQUE_NAME(e,name)static_assert( (e), CT_ASSERT_STRING( name ) "@" __FILE__ CT_ASSERT_STRING(__LINE__) ); 
#define CT_ASSERT(e) static_assert( (e), __FILE__ CT_ASSERT_STRING(__LINE__) );
#else
// CT_ASSERT (compile-time assert) macro is used to test condition at compiler time and generate
// compiler error if condition is FALSE.
// Example: CT_ASSERT( sizeof( UINT32 ) == 2 ) would cause compilation error.
//          CT_ASSERT( sizeof( UINT32 ) == 4 ) compiles without error.
// Since this declaration is just typedef - it does not create any CPU code.
//
// Reason for CT_ASSERT_UNIQUE_NAME
// The possible problem with the macro - it creates multiple identical typedefs.
// It is not a problem in global scope, but if macro is used inside of struct - it generates warnings.
// CT_ASSERT_UNIQUE_NAME is the same in essence, but it provides a way to customize the name of the type.
#define CT_ASSERT_UNIQUE_NAME(e,name) typedef char __CT_ASSERT__##name[(e)?1:-1];
#define CT_ASSERT(e)                  CT_ASSERT_UNIQUE_NAME(e,nanoclr)
#endif


















//--//

//These events match emulator events in Framework\Tools\Emulator\Events.cs

#define SYSTEM_EVENT_FLAG_COM_IN                    0x00000001
#define SYSTEM_EVENT_FLAG_COM_OUT                   0x00000002
#define SYSTEM_EVENT_FLAG_USB_IN                    0x00000004
//#define SYSTEM_EVENT_FLAG_USB_OUT                   0x00000008
#define SYSTEM_EVENT_FLAG_SYSTEM_TIMER              0x00000010
//#define SYSTEM_EVENT_FLAG_TIMER1                    0x00000020
//#define SYSTEM_EVENT_FLAG_TIMER2                    0x00000040
//#define SYSTEM_EVENT_FLAG_BUTTON                    0x00000080
#define SYSTEM_EVENT_FLAG_GENERIC_PORT              0x00000100
//#define SYSTEM_EVENT_FLAG_UNUSED_0x00000200         0x00000200
//#define SYSTEM_EVENT_FLAG_UNUSED_0x00000400         0x00000400
//#define SYSTEM_EVENT_FLAG_NETWORK                   0x00000800
//#define SYSTEM_EVENT_FLAG_TONE_COMPLETE             0x00001000
//#define SYSTEM_EVENT_FLAG_TONE_BUFFER_EMPTY         0x00002000
#define SYSTEM_EVENT_FLAG_SOCKET                    0x00004000
//#define SYSTEM_EVENT_FLAG_SPI                       0x00008000
//#define SYSTEM_EVENT_FLAG_CHARGER_CHANGE            0x00010000
//#define SYSTEM_EVENT_FLAG_OEM_RESERVED_1            0x00020000
//#define SYSTEM_EVENT_FLAG_OEM_RESERVED_2            0x00040000
#define SYSTEM_EVENT_FLAG_IO                        0x00080000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x00100000         0x00100000







//#define SYSTEM_EVENT_FLAG_UNUSED_0x00200000         0x00200000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x00400000         0x00400000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x00800000         0x00800000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x01000000         0x01000000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x02000000         0x02000000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x04000000         0x04000000
#define SYSTEM_EVENT_HW_INTERRUPT                   0x08000000
//#define SYSTEM_EVENT_I2C_XACTION                    0x10000000
#define SYSTEM_EVENT_FLAG_DEBUGGER_ACTIVITY         0x20000000
#define SYSTEM_EVENT_FLAG_MESSAGING_ACTIVITY        0x40000000
//#define SYSTEM_EVENT_FLAG_UNUSED_0x80000000         0x80000000
#define SYSTEM_EVENT_FLAG_ALL                       0xFFFFFFFF



//--//



























































template <class T> class HAL_DblLinkedList;

template <class T> class HAL_DblLinkedNode
{
    T* m_nextNode;
    T* m_prevNode;

    friend class HAL_DblLinkedList<T>;

public:
    void Initialize()
    {
        m_nextNode = NULL;
        m_prevNode = NULL;
    }

    T* Next() const { return m_nextNode; }
    T* Prev() const { return m_prevNode; }

    void SetNext( T* next ) { m_nextNode = next; }
    void SetPrev( T* prev ) { m_prevNode = prev; }

    bool IsLinked() const { return m_nextNode != NULL; }

    //--//

    void RemoveFromList()
    {
        T* next = m_nextNode;
        T* prev = m_prevNode;

        if(prev) prev->m_nextNode = next;
        if(next) next->m_prevNode = prev;
    }

    void Unlink()
    {
        T* next = m_nextNode;
        T* prev = m_prevNode;

        if(prev) prev->m_nextNode = next;
        if(next) next->m_prevNode = prev;

        m_nextNode = NULL;
        m_prevNode = NULL;
    }
};

//--//

template <class T> class HAL_DblLinkedList
{
    //
    // Logically, a list starts with a HAL_DblLinkedNode with only the Next() set and ends with a node with only Prev() set.
    // This can be collapsed to have the two nodes overlap.
    //
    T* m_first;
    T* m_null;
    T* m_last;

    //--//

public:
    void Initialize()
    {
        m_first = Tail();
        m_null  = NULL;
        m_last  = Head();
    }

    int NumOfNodes()
    {
        T*  ptr;
        T*  ptrNext;
        int num = 0;

        for(ptr = FirstNode(); (ptrNext = ptr->Next()) != NULL; ptr = ptrNext)
        {
            num++;
        }

        return num;
    }

    //--//

    T* FirstNode() const { return m_first          ; }
    T* LastNode () const { return m_last           ; }
    bool           IsEmpty  () const { return m_first == Tail(); }

    T* FirstValidNode() const { T* res = m_first; return res->Next() ? res : NULL; }
    T* LastValidNode () const { T* res = m_last ; return res->Prev() ? res : NULL; }

    T* Head() const { return (T*)((size_t)&m_first - offsetof(T, m_nextNode)); }
    T* Tail() const { return (T*)((size_t)&m_last  - offsetof(T, m_prevNode)); }

    //--//

private:

    void Insert( T* prev, T* next, T* node )
    {
        node->m_nextNode = next;
        node->m_prevNode = prev;

        next->m_prevNode = node;
        prev->m_nextNode = node;
    }

public:
#if defined(_DEBUG)
    BOOL Exists( T* searchNode )
    {
        T* node = FirstValidNode();
        while( node != NULL && node != searchNode )
        {
            if (node == node->Next())
            {
                ASSERT(FALSE);
            }
            node = node->Next();
        }
        return (node == NULL? FALSE: TRUE);
    }
#endif

    void InsertBeforeNode( T* node, T* nodeNew )
    {
        if(node && nodeNew && node != nodeNew)
        {
            nodeNew->RemoveFromList();

            Insert( node->Prev(), node, nodeNew );
        }
    }

    void InsertAfterNode( T* node, T* nodeNew )
    {
        if(node && nodeNew && node != nodeNew)
        {
            nodeNew->RemoveFromList();

            Insert( node, node->Next(), nodeNew );
        }
    }

    void LinkAtFront( T* node )
    {
        InsertAfterNode( Head(), node );
    }

    void LinkAtBack( T* node )
    {
        InsertBeforeNode( Tail(), node );
    }

    T* ExtractFirstNode()
    {
        T* node = FirstValidNode();

        if(node) node->Unlink();

        return node;
    }

    T* ExtractLastNode()
    {
        T* node = LastValidNode();

        if(node) node->Unlink();

        return node;
    }
};

//--//

















































































template <typename T> class Hal_Queue_UnknownSize
{
    size_t m_writer;
    size_t m_reader;
    size_t m_size;
    BOOL   m_full;
    T*     m_data;

public:
    void Initialize( T* data, size_t size )
    {
        m_writer = 0;
        m_reader = 0;
        m_size   = size;
        m_data   = data;
        m_full   = FALSE;
    }

    size_t NumberOfElements()
    {
        if(m_writer < m_reader) return m_size + m_writer - m_reader;
        else if(m_full)         return m_size;
        else                    return m_writer - m_reader;
    }

    BOOL IsEmpty()
    {
        return (m_writer == m_reader && !m_full);
    }

    BOOL IsFull()
    {
        return m_full;
    }

    T* operator[](int index)
    {
        if(index < 0 || index >= NumberOfElements()) return NULL;

        return &m_data[(m_reader + index) % m_size];
    }

    T* Push()
    {
        size_t oldWriter  = m_writer;

        if(m_full) return NULL;

        m_writer++;  if(m_writer == m_size) m_writer = 0;

        if(m_writer == m_reader) m_full = TRUE;

        return &m_data[oldWriter];
    }

    T* Peek()
    {
        if(m_writer == m_reader && !m_full) return NULL;

        return &m_data[m_reader];
    }

    T* Pop()
    {
        size_t oldReader = m_reader;

        if(m_reader == m_writer && !m_full) return (T*)NULL;

        m_reader++;  if(m_reader == m_size) m_reader = 0;

        m_full = FALSE;

        return &m_data[oldReader];
    }

    T* Push( size_t &nElements )
    {
        size_t oldWriter  = m_writer;
        size_t max = 0;

        if(m_full || (nElements == 0))
        {
            nElements = 0;
            return NULL;
        }

        if(m_writer < m_reader) max = m_reader - m_writer;
        else                    max = m_size   - m_writer;

        nElements = (max < nElements? max: nElements);

        m_writer += nElements; if(m_writer == m_size) m_writer = 0;

        if(m_writer == m_reader) m_full = TRUE;

        return &m_data[oldWriter];
    }

    T* Pop( size_t &nElements )
    {
        size_t oldReader = m_reader;
        size_t max = 0;

        if(nElements == 0) return NULL;

        if((m_reader == m_writer) && !m_full)
        {
            nElements = 0;
            // reset the reader/writer to maximize push potential
            m_reader  = 0;
            m_writer  = 0;
            return NULL;
        }

        if(m_writer <= m_reader) max = m_size   - m_reader;
        else                     max = m_writer - m_reader;

        nElements = (max < nElements? max: nElements);

        m_reader += nElements; if(m_reader == m_size) m_reader = 0;

        m_full = FALSE;

        return &m_data[oldReader];
    }

    T* Storage() { return m_data; }
};

//--//

//#include <..\Initialization\MasterConfig.h>

#define NUM_DEBUGGERS          1    // UNDONE: Remove
#define NUM_MESSAGING          1    // UNDONE: Remove

#define DEBUGGER_PORT_INDEX    0    // UNDONE: Remove

//--//








































































































































void HAL_EnterBooterMode();

typedef void (*ON_SOFT_REBOOT_HANDLER)(void);

void HAL_AddSoftRebootHandler(ON_SOFT_REBOOT_HANDLER handler);

//--//


//
// This has to be extern "C" because the Crypto library has C-linkage.
//
extern "C" {

void* private_malloc ( size_t len             );
void  private_free   ( void*  ptr             );
void* private_realloc( void*  ptr, size_t len );

}













//--//



//--//

#define HAL_DECLARE_CUSTOM_HEAP(allocFtn,freeFtn,reallocFtn)                           \
    extern "C" {                                                                       \
    void* private_malloc ( size_t len             ) { return allocFtn  ( len      ); } \
    void  private_free   ( void*  ptr             ) {        freeFtn   ( ptr      ); } \
    void* private_realloc( void*  ptr, size_t len ) { return reallocFtn( ptr, len ); } \
    }































































































































































































//--//

extern bool g_fDoNotUninitializeDebuggerPort;

//--//


#include <nanoPAL.h>
//#include <drivers.h>

/////////////////////////////////////////////////////////////////////
//
// Chipset
//

//// boot
//#include <CPU_BOOT_decl.h>
//
//// Cache driver
//#include <CPU_MMU_decl.h>
//
//// Cache driver
//#include <CPU_CACHE_decl.h>
//
//// Gp I/O driver
//#include <CPU_INTC_decl.h>
//
//// Gp I/O driver
//#include <CPU_GPIO_decl.h>
//

//// Watchdog driver
//#include <CPU_WATCHDOG_decl.h>

/// <summary>
/// Resets the watchdog timer. This method is called periodically by the system to ensure that the watchdog event does not occur, unless the 
/// system is in a stalled state.
/// </summary>
void    Watchdog_ResetCounter( );

/// <summary>
/// Gets or sets the watchdog enable state.
/// </summary>
/// <param name="enabled">Sets the watchdog enabled state when fSet is true; otherwise this parameter is ignored</param>
/// <param name="fSet">Determines if this call is getting or setting the enabled state</param>
/// <returns>Returns the current enabled state of the watchdog</returns>
BOOL              Watchdog_GetSetEnabled ( BOOL enabled, BOOL fSet );

//
//// SPI driver
//#include <CPU_SPI_decl.h>
//
//// External bus interface driver
//#include <CPU_EBIU_decl.h>
//
//// Power control unit
//#include <CPU_PCU_decl.h>
//
//// Clock management unit driver
//#include <CPU_CMU_decl.h>
//
//// DMA driver
//#include <CPU_DMA_decl.h>
//
//#include <PerformanceCounters_decl.h>
//
//// Virtual Key
//#include <VirtualKey_decl.h>
//

//// Power API
//#include <Power_decl.h>

void CPU_Reset();


//
// Chipset
//
/////////////////////////////////////////////////////////////////////

//#include <drivers.h>


// platform_selector.h (from MasterConfig.h)

#if defined(_WIN32)

#define GLOBAL_LOCK(x)             // UNDONE: FIXME: SmartPtr_IRQ x
//#define DISABLE_INTERRUPTS()       SmartPtr_IRQ::ForceDisabled()
//#define ENABLE_INTERRUPTS()        SmartPtr_IRQ::ForceEnabled()
//#define INTERRUPTS_ENABLED_STATE() SmartPtr_IRQ::GetState()
//#define GLOBAL_LOCK_SOCKETS(x)     // UNDONE: FIXME: SmartPtr_IRQ x

#if defined(_DEBUG)
#define ASSERT_IRQ_MUST_BE_OFF()   ASSERT( HAL_Windows_HasGlobalLock())
#define ASSERT_IRQ_MUST_BE_ON()    ASSERT(!HAL_Windows_HasGlobalLock())
#else
#define ASSERT_IRQ_MUST_BE_OFF()
#define ASSERT_IRQ_MUST_BE_ON()
#endif

#else
#error Unsupported platform
#endif

// UNDONE: FIXME: VERSION_XXX
#define VERSION_MAJOR       0
#define VERSION_MINOR       1
#define VERSION_BUILD       0
#define VERSION_REVISION    0

#define OEMSYSTEMINFOSTRING "OEM"  // UNDONE: FIXME: OEMSYSTEMINFOSTRING

#endif  // _NANOHAL_H_


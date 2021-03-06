//
// Copyright (c) 2017 The nanoFramework project contributors
// Portions Copyright (c) Microsoft Corporation.  All rights reserved.
// See LICENSE file in the project root for full license information.
//
#include <nanoCLR_Runtime.h>
#include <nanoCLR_Runtime__HeapBlock.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#define TRACE0( msg, ...) debug_printf( msg ) 
#define TRACE( msg, ...) debug_printf( msg, __VA_ARGS__ ) 
#else
#define TRACE0(msg,...)
#define TRACE(msg,...)
#endif

// the Arm 3.0 compiler drags in a bunch of ABI methods (for initialization) if struct arrays are not initialized
CLR_UINT32     g_scratchMessaging[ sizeof(CLR_Messaging) * NUM_MESSAGING / sizeof(UINT32) + 1 ];
CLR_Messaging *g_CLR_Messaging;

////////////////////////////////////////////////////////////////////////////////////////////////////

//--//

static const CLR_Messaging_CommandHandlerLookup c_Messaging_Lookup_Request[] =
{
    {  CLR_Messaging::Messaging_Query, CLR_Messaging_Commands::c_Messaging_Query },
    {  CLR_Messaging::Messaging_Send,  CLR_Messaging_Commands::c_Messaging_Send  },
    {  CLR_Messaging::Messaging_Reply, CLR_Messaging_Commands::c_Messaging_Reply },
};

static const CLR_Messaging_CommandHandlerLookup c_Messaging_Lookup_Reply[] =
{
    {  CLR_Messaging::Messaging_Query, CLR_Messaging_Commands::c_Messaging_Query },
    {  CLR_Messaging::Messaging_Send,  CLR_Messaging_Commands::c_Messaging_Send  },
    {  CLR_Messaging::Messaging_Reply, CLR_Messaging_Commands::c_Messaging_Reply },
};


//--//

bool CLR_Messaging::AllocateAndQueueMessage( CLR_UINT32 cmd, UINT32 length, UINT8* data, CLR_RT_HeapBlock_EndPoint::Port port, CLR_RT_HeapBlock_EndPoint::Address addr, CLR_UINT32 found )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_RT_HeapBlock_EndPoint::Message* rpc;
    CLR_RT_HeapBlock_EndPoint*          ep;
    
    if((ep = CLR_RT_HeapBlock_EndPoint::FindEndPoint( port )) == NULL) return false;

    {
        CLR_RT_ProtectFromGC gc( *ep );

        if((rpc = (CLR_RT_HeapBlock_EndPoint::Message*)CLR_RT_Memory::Allocate( sizeof(*rpc) + length, CLR_RT_HeapBlock::HB_CompactOnFailure )) == NULL) return false;
    
        rpc->ClearData();
        
        rpc->m_cmd       = cmd;;
        rpc->m_addr      = addr;
        rpc->m_length    = length;
        rpc->m_found     = found;
        
        if(data) memcpy( rpc->m_data, data, length );

        ep->m_messages.LinkAtBack( rpc );

        g_CLR_RT_ExecutionEngine.SignalEvents( CLR_RT_ExecutionEngine::c_Event_EndPoint );
    }

    return true;
}


bool CLR_Messaging::Messaging_Query( WP_Message* msg, void* owner )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging*                           messaging = (CLR_Messaging*)owner;
    CLR_Messaging_Commands::Messaging_Query*       cmd = (CLR_Messaging_Commands::Messaging_Query*)msg->m_payload;
    CLR_Messaging_Commands::Messaging_Query::Reply res;
    CLR_RT_HeapBlock_EndPoint*                      ep = CLR_RT_HeapBlock_EndPoint::FindEndPoint( cmd->m_addr.m_to );

    res.m_found = (ep != NULL);
    res.m_addr  = cmd->m_addr;

    messaging->ReplyToCommand( msg, true, false, &res, sizeof(res) );

    return true;
}

bool CLR_Messaging::Messaging_Query__Reply( WP_Message* msg, void* owner )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging *messaging = (CLR_Messaging*)owner;
    
    CLR_Messaging_Commands::Messaging_Query::Reply* cmd = (CLR_Messaging_Commands::Messaging_Query::Reply*)msg->m_payload;
    
    messaging->AllocateAndQueueMessage( CLR_Messaging_Commands::c_Messaging_Query, 0, NULL, cmd->m_addr.m_from, cmd->m_addr, cmd->m_found );
        
    return true;
}

//--//

bool CLR_Messaging::Messaging_Send( WP_Message* msg, void* owner )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging*                           messaging = (CLR_Messaging*)owner;
    CLR_Messaging_Commands::Messaging_Send*       cmd  = (CLR_Messaging_Commands::Messaging_Send*)msg->m_payload;
    CLR_Messaging_Commands::Messaging_Send::Reply res;
    CLR_UINT32                                    len;
    bool                                          fRes;

    len = msg->m_header.m_size - sizeof(cmd->m_addr);
        
    fRes = messaging->AllocateAndQueueMessage( CLR_Messaging_Commands::c_Messaging_Send, len, cmd->m_data, cmd->m_addr.m_to, cmd->m_addr, false );

    res.m_found = true;
    res.m_addr  = cmd->m_addr;

    messaging->ReplyToCommand( msg, fRes, false, &res, sizeof(res) );

    return true;
}

bool CLR_Messaging::Messaging_Send__Reply( WP_Message* msg, void* owner )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    //
    // retransmission support is currently not implemented
    //

    return true;
}

//--//

bool CLR_Messaging::Messaging_Reply( WP_Message* msg, void* owner )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging*                            messaging = (CLR_Messaging*)owner;
    CLR_Messaging_Commands::Messaging_Reply*       cmd  = (CLR_Messaging_Commands::Messaging_Reply*)msg->m_payload;
    CLR_Messaging_Commands::Messaging_Reply::Reply res;
    bool                                           fRes;
    CLR_UINT32                                     len;

    len = msg->m_header.m_size - sizeof(cmd->m_addr);
    fRes = messaging->AllocateAndQueueMessage( CLR_Messaging_Commands::c_Messaging_Reply, len, cmd->m_data, cmd->m_addr.m_from, cmd->m_addr, false );

    res.m_found = true;
    res.m_addr  = cmd->m_addr;

    messaging->ReplyToCommand( msg, fRes, false, &res, sizeof(res) );

    return true;
}

bool CLR_Messaging::Messaging_Reply__Reply( WP_Message* msg, void* owner )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    //
    // retransmission support  is currently not implemented
    //

    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


bool CLR_Messaging::Phy_ReceiveBytes( void* state, UINT8*& ptr, UINT32 & size )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging* messaging = (CLR_Messaging*)state;

    if(size)
    {
        int read = DebuggerPort_Read( messaging->m_port, (char*)ptr, size ); if(read <= 0) return false;

        ptr  += read;
        size -= read;
    }

    return true;
}

bool CLR_Messaging::Phy_TransmitMessage( void* state, const WP_Message* msg )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging* messaging = (CLR_Messaging*)state;

    return messaging->TransmitMessage( msg, true );
}

//--//

bool CLR_Messaging::App_ProcessHeader( void* state, WP_Message*  msg )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging* pThis = (CLR_Messaging*)state;

    ::Watchdog_ResetCounter();

    if( !pThis->ProcessHeader( msg ) )
    {
        TRACE0("ProcessHeader() indicated invalid header!\n");
        return false;
    }

    if(msg->m_header.m_size)
    {
        void* ptr = CLR_RT_Memory::Allocate( msg->m_header.m_size, CLR_RT_HeapBlock::HB_CompactOnFailure );

        if(ptr == NULL)
        {
            TRACE0("Failed to allocate 0x%08X bytes for message payload!\n");
            return false;
        }

        msg->m_payload = (UINT8*)ptr;
    }

    return true;
}

bool CLR_Messaging::App_ProcessPayload( void* state, WP_Message* msg )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_Messaging* pThis = (CLR_Messaging*)state;

    ::Watchdog_ResetCounter();

    if(pThis->ProcessPayload( msg ) == false)
    {
        return false;
    }

    return true;
}

bool CLR_Messaging::App_Release( void* state, WP_Message* msg )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    if(msg->m_payload != NULL)
    {
        CLR_RT_Memory::Release( msg->m_payload );

        msg->m_payload = NULL;
    }

    return true;
}

//--//

const WP_PhysicalLayer c_Messaging_phy =
{
    &CLR_Messaging::Phy_ReceiveBytes   , 
    &CLR_Messaging::Phy_TransmitMessage, 
};

const WP_ApplicationLayer c_Messaging_app =
{
    &CLR_Messaging::App_ProcessHeader ,
    &CLR_Messaging::App_ProcessPayload,
    &CLR_Messaging::App_Release       , 
};

////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT CLR_Messaging::CreateInstance()
{
    NATIVE_PROFILE_CLR_MESSAGING();
    NANOCLR_HEADER();

    g_CLR_Messaging = (CLR_Messaging*)&g_scratchMessaging[ 0 ];

    CLR_RT_Memory::ZeroFill( g_CLR_Messaging, sizeof(CLR_Messaging) * NUM_MESSAGING );

    int iMsg = 0;
    
    NANOCLR_FOREACH_MESSAGING_NO_TEMP()
    {
        g_CLR_Messaging[ iMsg ].Initialize(
            HalSystemConfig.MessagingPorts[ iMsg ], 
            NULL, 
            0, 
            NULL, 
            0, 
            &g_CLR_Messaging[ iMsg ]
            );
        iMsg++;
    }
    NANOCLR_FOREACH_MESSAGING_END();

    NANOCLR_NOCLEANUP_NOLABEL();
}

//--//

void CLR_Messaging::Initialize(
    COM_HANDLE                                port, 
    const CLR_Messaging_CommandHandlerLookup* requestLookup, 
    const CLR_UINT32                          requestLookupCount, 
    const CLR_Messaging_CommandHandlerLookup* replyLookup, 
    const CLR_UINT32                          replyLookupCount, 
    void*                                     owner 
    )
{
    // If the debugger and Messaging share the same port (Legacy) then we will not initialze the Messaging port (because the debugger will take care of it)
    if((port == HalSystemConfig.MessagingPorts[ 0 ]) && 
       (port == HalSystemConfig.DebuggerPorts[ 0 ] ) &&
        requestLookup == NULL) // messaging is null so don't initialize the port
    {
        return;
    }

    if(m_fInitialized) return;

    m_Lookup_Requests[ 0 ].table = c_Messaging_Lookup_Request;
    m_Lookup_Requests[ 0 ].owner = this;
    m_Lookup_Requests[ 0 ].size  = ARRAYSIZE(c_Messaging_Lookup_Request);

    m_Lookup_Replies[ 0 ].table = c_Messaging_Lookup_Reply;
    m_Lookup_Replies[ 0 ].owner = this;
    m_Lookup_Replies[ 0 ].size  = ARRAYSIZE(c_Messaging_Lookup_Reply);

    m_cacheSubordinate.DblLinkedList_Initialize();
    m_cacheMaster     .DblLinkedList_Initialize();

    m_controller.Initialize( MARKER_DEBUGGER_V1, &c_Messaging_phy, &c_Messaging_app, this );

    m_cacheTotalSize = 0;
    m_port           = port;    

    m_Lookup_Requests[ 1 ].table = requestLookup;
    m_Lookup_Requests[ 1 ].owner = owner;
    m_Lookup_Requests[ 1 ].size  = requestLookupCount;

    m_Lookup_Replies[ 1 ].table = replyLookup;
    m_Lookup_Replies[ 1 ].owner = owner;
    m_Lookup_Replies[ 1 ].size  = replyLookupCount;

    m_fDebuggerInitialized = (DebuggerPort_Initialize( port ) != FALSE);

    m_fInitialized = true;

}

HRESULT CLR_Messaging::DeleteInstance()
{
    NATIVE_PROFILE_CLR_MESSAGING();
    NANOCLR_HEADER();

    NANOCLR_FOREACH_MESSAGING(msg)
    {
        msg.Cleanup();
    }
    NANOCLR_FOREACH_MESSAGING_END();

    NANOCLR_NOCLEANUP_NOLABEL();
}

void CLR_Messaging::Cleanup()
{
    NATIVE_PROFILE_CLR_MESSAGING();

    if(!m_fInitialized) return;

    // Some devices cannot reset the USB controller so we need to allow them to skip uninitialization
    // of the debug transport
    if(!g_fDoNotUninitializeDebuggerPort)
    {
        DebuggerPort_Uninitialize( m_port );
    }
    
    m_cacheSubordinate.DblLinkedList_Release();
    m_cacheMaster     .DblLinkedList_Release();

    m_cacheTotalSize = 0;

    m_fDebuggerInitialized = false;

    m_fInitialized = false;
}

//--//

void CLR_Messaging::ProcessCommands()
{
    NATIVE_PROFILE_CLR_MESSAGING();
    if(m_fInitialized)
    {
        m_controller.AdvanceState();
    }
}

bool CLR_Messaging::ProcessHeader( WP_Message* msg )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    TRACE("MSG: 0x%08X\n", msg->m_header.m_cmd );
    return true;
}

bool CLR_Messaging::ProcessPayload( WP_Message* msg )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    if(msg->m_header.m_flags & WP_Flags::c_NACK)
    {
        //
        // Bad packet...
        //
        return true;
    }

    //--//

    const CLR_Messaging_CommandHandlerLookups* tables;
    int tableCount = 0;

    if(msg->m_header.m_flags & WP_Flags::c_Reply)
    {
        //
        // Only process replies once!
        //
        NANOCLR_FOREACH_NODE(CachedMessage,cache,m_cacheMaster)
        {
            WP_Packet& req = cache->m_message.m_header;
            WP_Packet& res = msg->            m_header;

            if(req.m_cmd == res.m_cmd && req.m_seq == res.m_seqReply)
            {
                m_cacheMaster.LinkAtFront( cache );

                cache->m_lastSeen = Time_GetMachineTime();

                if(cache->m_flags & CachedMessage::c_Processed)
                {
                    return true;
                }

                cache->m_flags |= CachedMessage::c_Processed;
                break;
            }
        }
        NANOCLR_FOREACH_NODE_END();

        tables     = m_Lookup_Replies;
        tableCount = ARRAYSIZE(m_Lookup_Replies);
    }
    else
    {
        NANOCLR_FOREACH_NODE(CachedMessage,cache,m_cacheSubordinate)
        {
            WP_Packet& req = msg->            m_header;
            WP_Packet& res = cache->m_message.m_header;

            if(req.m_cmd == res.m_cmd && req.m_seq == res.m_seqReply)
            {
                m_cacheSubordinate.LinkAtFront( cache );

                cache->m_lastSeen = Time_GetMachineTime();

                TransmitMessage( &cache->m_message, false );
                return true;
            }
        }
        NANOCLR_FOREACH_NODE_END();

        tables     = m_Lookup_Requests;
        tableCount = ARRAYSIZE(m_Lookup_Requests);
    }

    while(tableCount-- > 0)
    {
        size_t                                    num = tables->size;
        const CLR_Messaging_CommandHandlerLookup* cmd = tables->table;

        while(num-- > 0 && cmd != NULL)
        {
            if(cmd->cmd == msg->m_header.m_cmd)
            {
                ReplyToCommand( msg, (*(cmd->hnd))( msg, tables->owner ), false );
                return true;
            }

            cmd++;
        }
        tables++;
    }

    ReplyToCommand( msg, false, false );
    
    return true;
}

//--//

void CLR_Messaging::PurgeCache()
{
    NATIVE_PROFILE_CLR_MESSAGING();
    CLR_INT64 oldest = Time_GetMachineTime() - TIME_CONVERSION__TO_SECONDS * 3;

    PurgeCache( m_cacheMaster     , oldest );
    PurgeCache( m_cacheSubordinate, oldest );
}

void CLR_Messaging::PurgeCache( CLR_RT_DblLinkedList& lst, CLR_INT64 oldest )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    NANOCLR_FOREACH_NODE_BACKWARD(CachedMessage,cache,lst)
    {
        if(cache->m_lastSeen < oldest || m_cacheTotalSize > c_MaxCacheSize)
        {
            cache->Unlink(); m_cacheTotalSize -= cache->m_size;

            CLR_RT_Memory::Release( cache );
        }
    }
    NANOCLR_FOREACH_NODE_BACKWARD_END();
}

bool CLR_Messaging::TransmitMessage( const WP_Message* msg, bool fQueue )
{
    NATIVE_PROFILE_CLR_MESSAGING();

    UINT32 payloadSize;
    UINT32 flags;

    payloadSize = msg->m_header.m_size;
    flags       = msg->m_header.m_flags;

    if(DebuggerPort_Write( m_port, (char*)&msg->m_header, sizeof(msg->m_header) ) != sizeof(msg->m_header)) return false;

    if(msg->m_header.m_size && msg->m_payload)
    {
        if(DebuggerPort_Write( m_port, (char*)msg->m_payload, payloadSize ) != payloadSize) return false;
    }
    DebuggerPort_Flush( m_port );

    if(fQueue && (flags & WP_Flags::c_NoCaching) == 0)
    {
        CLR_RT_DblLinkedList* lst;

        if(flags & WP_Flags::c_Reply)
        {
            lst = &m_cacheSubordinate;
        }
        else
        {
            if(flags & WP_Flags::c_NonCritical)
            {
                //
                // Don't cache non-critical requests.
                //
                lst = NULL;
            }
            else
            {
                lst = &m_cacheMaster;
            }
        }

        if(lst)
        {
            CLR_UINT32 len = sizeof(CachedMessage) + payloadSize;

            CachedMessage* cache = (CachedMessage*)CLR_RT_Memory::Allocate_And_Erase( len, CLR_RT_HeapBlock::HB_CompactOnFailure );
            if(cache)
            {
                m_cacheTotalSize  += len;

                cache->m_size      = len;
                cache->m_lastSeen  = Time_GetMachineTime();
                cache->m_message   = *msg;

                if(payloadSize && msg->m_payload)
                {
                    cache->m_message.m_payload = (UINT8*)&cache[ 1 ];

                    memcpy( cache->m_message.m_payload, msg->m_payload, payloadSize );
                }
                else
                {
                    cache->m_message.m_header.m_size = 0;
                    cache->m_message.m_payload       = NULL;
                }

                lst->LinkAtFront( cache );
            }
        }
    }

    return true;
}

//--//

bool CLR_Messaging::SendEvent( UINT32 cmd, UINT32 payloadSize, UINT8* payload, UINT32 flags )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    return m_controller.SendProtocolMessage( cmd, flags, payloadSize, payload );
}

void CLR_Messaging::BroadcastEvent( UINT32 cmd, UINT32 payloadSize, UINT8* payload, UINT32 flags )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    NANOCLR_FOREACH_MESSAGING(msg)
    {
        msg.m_controller.SendProtocolMessage( cmd, flags, payloadSize, payload );        
    }
    NANOCLR_FOREACH_MESSAGING_END();
}

//--//

void CLR_Messaging::ReplyToCommand( WP_Message* msg, bool fSuccess, bool fCritical, void* ptr, int size )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    WP_Message msgReply;
    UINT32     flags = 0;

    //
    // Make sure we reply only once!
    //
    if(msg->m_header.m_flags & WP_Flags::c_NonCritical) return;
    msg->m_header.m_flags |= WP_Flags::c_NonCritical;

    //
    // No caching in the request, no caching in the reply...
    //
    if(msg->m_header.m_flags & WP_Flags::c_NoCaching) flags |= WP_Flags::c_NoCaching;

    if(fSuccess  ) flags |= WP_Flags::c_ACK;
    else           flags |= WP_Flags::c_NACK;
    if(!fCritical) flags |= WP_Flags::c_NonCritical;

    if(fSuccess == false)
    {
        ptr  = NULL;
        size = 0;
    }

    msgReply.Initialize( &m_controller );

    msgReply.PrepareReply( msg->m_header, flags, size, (UINT8*)ptr );

    m_controller.SendProtocolMessage( msgReply );
}

void CLR_Messaging::ReplyToCommand( WP_Message* msg, bool fSuccess, bool fCritical )
{
    NATIVE_PROFILE_CLR_MESSAGING();
    ReplyToCommand( msg, fSuccess, fCritical, NULL, 0 );
}


#ifndef TRANSPORT_H_
    #define TRANSPORT_H_

    #include "transport_interface.h"
    #include "sockets_posix.h"
    #include <openssl/ssl.h>

    #ifdef __cplusplus
    extern "C" {
    #endif

    typedef enum TransportStatus
    {
        TRANSPORT_SUCCESS = 0,
        TRANSPORT_INIT_FAIL,
        TRANSPORT_INVALID_PARAMETER,
        TRANSPORT_INSUFFICIENT_MEMORY,
        TRANSPORT_INVALID_CREDENTIALS,
        TRANSPORT_HANDSHAKE_FAILED,
        TRANSPORT_API_ERROR,
        TRANSPORT_DNS_FAILURE,
        TRANSPORT_CONNECT_FAILURE
    } TransportStatus_t;

    typedef struct TransportCredentials
    {
        const char * pRootCa;
        const char * pClientCert;
        const char * pPrivateKey;
        const char * sniHostName;
        const char * pAlpnProtos;
        uint32_t alpnProtosLen;
    } TransportCredentials_t;

    struct NetworkContext
    {
        int32_t socketDescriptor;
        SSL * pSsl;
    };

    TransportStatus_t Transport_Init( void );

    TransportStatus_t Transport_Connect( NetworkContext_t * pNetworkContext,
                                         const ServerInfo_t * pServerInfo,
                                         const TransportCredentials_t * pCredentials,
                                         uint32_t sendTimeoutMs,
                                         uint32_t recvTimeoutMs );

    TransportStatus_t Transport_Disconnect( const NetworkContext_t * pNetworkContext );

    int32_t Transport_Recv( NetworkContext_t * pNetworkContext,
                            void * pBuffer,
                            size_t bytesToRecv );

    int32_t Transport_Send( NetworkContext_t * pNetworkContext,
                            const void * pBuffer,
                            size_t bytesToSend );

    #ifdef __cplusplus
}
    #endif

#endif /* TRANSPORT_H_ */

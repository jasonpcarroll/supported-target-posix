#include "transport.h"
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>
#include <openssl/err.h>

static TransportStatus_t setupTlsConnection( NetworkContext_t * pNetworkContext,
                                             const TransportCredentials_t * pCredentials )
{
    TransportStatus_t xReturnStatus = TRANSPORT_SUCCESS;
    SSL_CTX * pSslContext = SSL_CTX_new( TLS_client_method() );
    SSL * pSsl = NULL;

    if( pSslContext == NULL )
    {
        xReturnStatus = TRANSPORT_INSUFFICIENT_MEMORY;
    }

    if( ( xReturnStatus == TRANSPORT_SUCCESS ) && ( pCredentials->pRootCa != NULL ) )
    {
        if( SSL_CTX_load_verify_locations( pSslContext, pCredentials->pRootCa, NULL ) != 1 )
        {
            xReturnStatus = TRANSPORT_INVALID_CREDENTIALS;
        }
    }

    if( ( xReturnStatus == TRANSPORT_SUCCESS ) && ( pCredentials->pClientCert != NULL ) && ( pCredentials->pPrivateKey != NULL ) )
    {
        if( ( SSL_CTX_use_certificate_file( pSslContext, pCredentials->pClientCert, SSL_FILETYPE_PEM ) != 1 ) ||
            ( SSL_CTX_use_PrivateKey_file( pSslContext, pCredentials->pPrivateKey, SSL_FILETYPE_PEM ) != 1 ) )
        {
            xReturnStatus = TRANSPORT_INVALID_CREDENTIALS;
        }
    }

    if( ( xReturnStatus == TRANSPORT_SUCCESS ) && ( pCredentials->pAlpnProtos != NULL ) && ( pCredentials->alpnProtosLen > 0 ) )
    {
        if( SSL_CTX_set_alpn_protos( pSslContext, ( const unsigned char * ) pCredentials->pAlpnProtos, pCredentials->alpnProtosLen ) != 0 )
        {
            xReturnStatus = TRANSPORT_INVALID_CREDENTIALS;
        }
    }

    if( xReturnStatus == TRANSPORT_SUCCESS )
    {
        pSsl = SSL_new( pSslContext );

        if( pSsl == NULL )
        {
            xReturnStatus = TRANSPORT_INSUFFICIENT_MEMORY;
        }
    }

    if( pSslContext != NULL )
    {
        SSL_CTX_free( pSslContext );
    }

    if( ( xReturnStatus == TRANSPORT_SUCCESS ) && ( pCredentials->sniHostName != NULL ) )
    {
        SSL_set_tlsext_host_name( pSsl, pCredentials->sniHostName );
    }

    if( xReturnStatus == TRANSPORT_SUCCESS )
    {
        SSL_set_fd( pSsl, pNetworkContext->socketDescriptor );

        // Check if socket is blocking or non-blocking
        int flags = fcntl( pNetworkContext->socketDescriptor, F_GETFL, 0 );
        LogError( ( "Socket flags: 0x%x, O_NONBLOCK=%s\n", flags, (flags & O_NONBLOCK) ? "YES" : "NO" ) );

        int connectResult = SSL_connect( pSsl );

        if( connectResult != 1 )
        {
            int sslError = SSL_get_error( pSsl, connectResult );
            unsigned long errCode = ERR_get_error();
            char errBuf[ 256 ];
            ERR_error_string_n( errCode, errBuf, sizeof( errBuf ) );

            LogError( ( "TLS handshake failed - SSL_get_error: %d, OpenSSL error: %s\n", sslError, errBuf ) );
            LogError( ( "SSL_connect returned: %d\n", connectResult ) );
            
            if( sslError == SSL_ERROR_SYSCALL )
            {
                LogError( ( "System call error - errno: %d (%s)\n", errno, strerror(errno) ) );
            }
            else if( sslError == SSL_ERROR_WANT_READ )
            {
                LogError( ( "SSL wants to read but no data available - possible connection timeout\n" ) );
            }
            
            xReturnStatus = TRANSPORT_HANDSHAKE_FAILED;
        }
    }

    if( xReturnStatus == TRANSPORT_SUCCESS )
    {
        pNetworkContext->pSsl = pSsl;
    }
    else if( pSsl != NULL )
    {
        SSL_free( pSsl );
    }

    return xReturnStatus;
}

TransportStatus_t Transport_Connect( NetworkContext_t * pNetworkContext,
                                     const ServerInfo_t * pServerInfo,
                                     const TransportCredentials_t * pCredentials,
                                     uint32_t sendTimeoutMs,
                                     uint32_t recvTimeoutMs )
{
    TransportStatus_t xReturnStatus = TRANSPORT_SUCCESS;
    SocketStatus_t socketStatus;

    if( ( pNetworkContext == NULL ) || ( pServerInfo == NULL ) )
    {
        xReturnStatus = TRANSPORT_INVALID_PARAMETER;
    }

    if( xReturnStatus == TRANSPORT_SUCCESS )
    {
        memset( pNetworkContext, 0, sizeof( NetworkContext_t ) );
        socketStatus = Sockets_Connect( &pNetworkContext->socketDescriptor,
                                        pServerInfo, sendTimeoutMs, recvTimeoutMs );

        if( socketStatus != SOCKETS_SUCCESS )
        {
            xReturnStatus = ( socketStatus == SOCKETS_DNS_FAILURE ) ? TRANSPORT_DNS_FAILURE : TRANSPORT_CONNECT_FAILURE;
        }
    }

    if( ( xReturnStatus == TRANSPORT_SUCCESS ) && ( pCredentials != NULL ) )
    {
        xReturnStatus = setupTlsConnection( pNetworkContext, pCredentials );

        if( xReturnStatus != TRANSPORT_SUCCESS )
        {
            Sockets_Disconnect( pNetworkContext->socketDescriptor );
        }
    }

    return xReturnStatus;
}

TransportStatus_t Transport_Disconnect( const NetworkContext_t * pNetworkContext )
{
    TransportStatus_t xReturnStatus = TRANSPORT_SUCCESS;

    if( pNetworkContext == NULL )
    {
        xReturnStatus = TRANSPORT_INVALID_PARAMETER;
    }

    if( ( xReturnStatus == TRANSPORT_SUCCESS ) && ( pNetworkContext->pSsl != NULL ) )
    {
        LogDebug( ( "Starting SSL_shutdown\n" ) );
        int shutdownResult = SSL_shutdown( pNetworkContext->pSsl );
        LogDebug( ( "SSL_shutdown returned: %d\n", shutdownResult ) );
        SSL_free( pNetworkContext->pSsl );
        LogDebug( ( "SSL_free completed\n" ) );
    }

    if( xReturnStatus == TRANSPORT_SUCCESS )
    {
        LogDebug( ( "Starting Sockets_Disconnect\n" ) );
        Sockets_Disconnect( pNetworkContext->socketDescriptor );
        LogDebug( ( "Sockets_Disconnect completed\n" ) );
    }

    return xReturnStatus;
}

int32_t Transport_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv )
{
    int32_t bytesReceived = -1;

    if( pNetworkContext->pSsl != NULL )
    {
        int32_t shouldRead = 0;

        if( SSL_pending( pNetworkContext->pSsl ) > 0 )
        {
            shouldRead = 1;
        }
        else
        {
            struct pollfd pollFds;
            pollFds.events = POLLIN | POLLPRI;
            pollFds.revents = 0;
            pollFds.fd = pNetworkContext->socketDescriptor;

            int32_t pollStatus = poll( &pollFds, 1, 0 );

            if( pollStatus > 0 )
            {
                shouldRead = 1;
            }
            else if( pollStatus < 0 )
            {
                bytesReceived = -1;
            }
            else
            {
                bytesReceived = 0;
            }
        }

        if( shouldRead )
        {
            bytesReceived = SSL_read( pNetworkContext->pSsl, pBuffer, bytesToRecv );
        }
    }
    else
    {
        struct pollfd pollFds;
        pollFds.events = POLLIN | POLLPRI;
        pollFds.revents = 0;
        pollFds.fd = pNetworkContext->socketDescriptor;

        int32_t pollStatus = poll( &pollFds, 1, 0 );

        if( pollStatus > 0 )
        {
            bytesReceived = recv( pNetworkContext->socketDescriptor, pBuffer, bytesToRecv, 0 );

            if( bytesReceived == 0 )
            {
                bytesReceived = -1;
            }
        }
        else if( pollStatus < 0 )
        {
            bytesReceived = -1;
        }
        else
        {
            bytesReceived = 0;
        }
    }

    return bytesReceived;
}

int32_t Transport_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend )
{
    int32_t bytesSent = -1;
    struct pollfd pollFds;

    pollFds.events = POLLOUT;
    pollFds.revents = 0;
    pollFds.fd = pNetworkContext->socketDescriptor;

    int32_t pollStatus = poll( &pollFds, 1, 0 );

    if( pollStatus > 0 )
    {
        if( pNetworkContext->pSsl != NULL )
        {
            bytesSent = SSL_write( pNetworkContext->pSsl, pBuffer, bytesToSend );
        }
        else
        {
            bytesSent = send( pNetworkContext->socketDescriptor, pBuffer, bytesToSend, 0 );

            if( bytesSent == 0 )
            {
                bytesSent = -1;
            }
        }
    }
    else if( pollStatus < 0 )
    {
        bytesSent = -1;
    }

    return bytesSent;
}

/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-07-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

/**
 * @file listener.c  -  Listener generic functions
 *
 * Listeners wait for new client connections and, if the connection is successful
 * a new session is created. A listener typically knows about a port or a socket,
 * and a few other things. It may know about SSL if it is expecting an SSL
 * connection.
 *
 * @verbatim
 * Revision History
 *
 * Date         Who                     Description
 * 26/01/16     Martin Brampton         Initial implementation
 *
 * @endverbatim
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <listener.h>
#include <gw_ssl.h>
#include <gw_protocol.h>
#include <log_manager.h>
#include <maxscale/alloc.h>
#include <users.h>
#include <modutil.h>

static RSA *rsa_512 = NULL;
static RSA *rsa_1024 = NULL;

static RSA *tmp_rsa_callback(SSL *s, int is_export, int keylength);

/**
 * Create a new listener structure
 *
 * @param protocol      The name of the protocol module
 * @param address       The address to listen with
 * @param port          The port to listen on
 * @param authenticator Name of the authenticator to be used
 * @param ssl           SSL configuration
 * @return      New listener object or NULL if unable to allocate
 */
SERV_LISTENER *
listener_alloc(struct service* service, char* name, char *protocol, char *address,
               unsigned short port, char *authenticator, char* options, SSL_LISTENER *ssl)
{
    if (address)
    {
        address = MXS_STRDUP(address);
        if (!address)
        {
            return NULL;
        }
    }

    if (authenticator)
    {
        authenticator = MXS_STRDUP(authenticator);
        if (!authenticator)
        {
            MXS_FREE(address);
            return NULL;
        }
    }

    if (options && (options = MXS_STRDUP(options)) == NULL)
    {
        MXS_FREE(address);
        MXS_FREE(authenticator);
        return NULL;
    }

    protocol = MXS_STRDUP(protocol);
    name = MXS_STRDUP(name);
    SERV_LISTENER *proto = (SERV_LISTENER*)MXS_MALLOC(sizeof(SERV_LISTENER));

    if (!protocol || !proto || !name)
    {
        MXS_FREE(protocol);
        MXS_FREE(proto);
        MXS_FREE(name);
        return NULL;
    }

    proto->name = name;
    proto->listener = NULL;
    proto->service = service;
    proto->protocol = protocol;
    proto->address = address;
    proto->port = port;
    proto->authenticator = authenticator;
    proto->ssl = ssl;
    proto->users = NULL;
    proto->resources = NULL;
    proto->next = NULL;
    proto->instance = NULL;
    proto->options = options;
    spinlock_init(&proto->lock);

    return proto;
}

/**
 * @brief Free a listener
 *
 * @param listener Listener to free
 */
void listener_free(SERV_LISTENER* listener)
{
    if (listener)
    {
        if (listener->resources)
        {
            hashtable_free(listener->resources);
        }
        if (listener->users)
        {
            users_free(listener->users);
        }

        MXS_FREE(listener->address);
        MXS_FREE(listener->authenticator);
        MXS_FREE(listener->protocol);
        MXS_FREE(listener);
    }
}

/**
 * Set the maximum SSL/TLS version the listener will support
 * @param ssl_listener Listener data to configure
 * @param version SSL/TLS version string
 * @return  0 on success, -1 on invalid version string
 */
int
listener_set_ssl_version(SSL_LISTENER *ssl_listener, char* version)
{
    if (strcasecmp(version, "TLSV10") == 0)
    {
        ssl_listener->ssl_method_type = SERVICE_TLS10;
    }
#ifdef OPENSSL_1_0
    else if (strcasecmp(version, "TLSV11") == 0)
    {
        ssl_listener->ssl_method_type = SERVICE_TLS11;
    }
    else if (strcasecmp(version, "TLSV12") == 0)
    {
        ssl_listener->ssl_method_type = SERVICE_TLS12;
    }
#endif
    else if (strcasecmp(version, "MAX") == 0)
    {
        ssl_listener->ssl_method_type = SERVICE_SSL_TLS_MAX;
    }
    else
    {
        return -1;
    }
    return 0;
}

/**
 * Set the locations of the listener's SSL certificate, listener's private key
 * and the CA certificate which both the client and the listener should trust.
 * @param ssl_listener Listener data to configure
 * @param cert SSL certificate
 * @param key SSL private key
 * @param ca_cert SSL CA certificate
 */
void
listener_set_certificates(SSL_LISTENER *ssl_listener, char* cert, char* key, char* ca_cert)
{
    MXS_FREE(ssl_listener->ssl_cert);
    ssl_listener->ssl_cert = cert ? MXS_STRDUP_A(cert) : NULL;

    MXS_FREE(ssl_listener->ssl_key);
    ssl_listener->ssl_key = key ? MXS_STRDUP_A(key) : NULL;

    MXS_FREE(ssl_listener->ssl_ca_cert);
    ssl_listener->ssl_ca_cert = ca_cert ? MXS_STRDUP_A(ca_cert) : NULL;
}

/**
 * Initialize the listener's SSL context. This sets up the generated RSA
 * encryption keys, chooses the listener encryption level and configures the
 * listener certificate, private key and certificate authority file.
 * @param ssl_listener Listener data to initialize
 * @return 0 on success, -1 on error
 */
int
listener_init_SSL(SSL_LISTENER *ssl_listener)
{
    DH* dh;
    RSA* rsa;

    if (!ssl_listener->ssl_init_done)
    {
        switch(ssl_listener->ssl_method_type)
        {
        case SERVICE_TLS10:
            ssl_listener->method = (SSL_METHOD*)TLSv1_method();
            break;
#ifdef OPENSSL_1_0
        case SERVICE_TLS11:
            ssl_listener->method = (SSL_METHOD*)TLSv1_1_method();
            break;
        case SERVICE_TLS12:
            ssl_listener->method = (SSL_METHOD*)TLSv1_2_method();
            break;
#endif
            /** Rest of these use the maximum available SSL/TLS methods */
        case SERVICE_SSL_MAX:
            ssl_listener->method = (SSL_METHOD*)SSLv23_method();
            break;
        case SERVICE_TLS_MAX:
            ssl_listener->method = (SSL_METHOD*)SSLv23_method();
            break;
        case SERVICE_SSL_TLS_MAX:
            ssl_listener->method = (SSL_METHOD*)SSLv23_method();
            break;
        default:
            ssl_listener->method = (SSL_METHOD*)SSLv23_method();
            break;
        }

        if ((ssl_listener->ctx = SSL_CTX_new(ssl_listener->method)) == NULL)
        {
            MXS_ERROR("SSL context initialization failed.");
            return -1;
        }

        SSL_CTX_set_default_read_ahead(ssl_listener->ctx, 0);

        /** Enable all OpenSSL bug fixes */
        SSL_CTX_set_options(ssl_listener->ctx, SSL_OP_ALL);

        /** Disable SSLv3 */
        SSL_CTX_set_options(ssl_listener->ctx, SSL_OP_NO_SSLv3);

        /** Generate the 512-bit and 1024-bit RSA keys */
        if (rsa_512 == NULL)
        {
            rsa_512 = RSA_generate_key(512, RSA_F4, NULL, NULL);
            if (rsa_512 == NULL)
            {
                MXS_ERROR("512-bit RSA key generation failed.");
                return -1;
            }
        }
        if (rsa_1024 == NULL)
        {
            rsa_1024 = RSA_generate_key(1024, RSA_F4, NULL, NULL);
            if (rsa_1024 == NULL)
            {
                MXS_ERROR("1024-bit RSA key generation failed.");
                return -1;
            }
        }

        if (rsa_512 != NULL && rsa_1024 != NULL)
        {
            SSL_CTX_set_tmp_rsa_callback(ssl_listener->ctx, tmp_rsa_callback);
        }

        if (ssl_listener->ssl_cert && ssl_listener->ssl_key)
        {
            /** Load the server certificate */
            if (SSL_CTX_use_certificate_file(ssl_listener->ctx, ssl_listener->ssl_cert, SSL_FILETYPE_PEM) <= 0)
            {
                MXS_ERROR("Failed to set server SSL certificate.");
                return -1;
            }

            /* Load the private-key corresponding to the server certificate */
            if (SSL_CTX_use_PrivateKey_file(ssl_listener->ctx, ssl_listener->ssl_key, SSL_FILETYPE_PEM) <= 0)
            {
                MXS_ERROR("Failed to set server SSL key.");
                return -1;
            }

            /* Check if the server certificate and private-key matches */
            if (!SSL_CTX_check_private_key(ssl_listener->ctx))
            {
                MXS_ERROR("Server SSL certificate and key do not match.");
                return -1;
            }

            /* Load the RSA CA certificate into the SSL_CTX structure */
            if (!SSL_CTX_load_verify_locations(ssl_listener->ctx, ssl_listener->ssl_ca_cert, NULL))
            {
                MXS_ERROR("Failed to set Certificate Authority file.");
                return -1;
            }
        }

        /* Set to require peer (client) certificate verification */
        if (ssl_listener->ssl_cert_verify_depth)
        {
            SSL_CTX_set_verify(ssl_listener->ctx, SSL_VERIFY_PEER, NULL);
        }

        /* Set the verification depth */
        SSL_CTX_set_verify_depth(ssl_listener->ctx, ssl_listener->ssl_cert_verify_depth);
        ssl_listener->ssl_init_done = true;
    }
    return 0;
}

/**
 * The RSA key generation callback function for OpenSSL.
 * @param s SSL structure
 * @param is_export Not used
 * @param keylength Length of the key
 * @return Pointer to RSA structure
 */
static RSA *
tmp_rsa_callback(SSL *s, int is_export, int keylength)
{
    RSA *rsa_tmp=NULL;

    switch (keylength) {
    case 512:
        if (rsa_512)
        {
            rsa_tmp = rsa_512;
        }
        else
        {
            /* generate on the fly, should not happen in this example */
            rsa_tmp = RSA_generate_key(keylength,RSA_F4,NULL,NULL);
            rsa_512 = rsa_tmp; /* Remember for later reuse */
        }
        break;
    case 1024:
        if (rsa_1024)
        {
            rsa_tmp=rsa_1024;
        }
        break;
    default:
        /* Generating a key on the fly is very costly, so use what is there */
        if (rsa_1024)
        {
            rsa_tmp=rsa_1024;
        }
        else
        {
            rsa_tmp=rsa_512; /* Use at least a shorter key */
        }
    }
    return(rsa_tmp);
}

/**
 * @brief Initialize the authenticator module of a listener
 * @param listener Listener to initialize
 */
void listener_init_authenticator(SERV_LISTENER *listener)
{
    if (listener->listener->authfunc.initialize)
    {
        char *optarray[LISTENER_MAX_OPTIONS];
        size_t optlen = listener->options ? strlen(listener->options) : 0;
        char optcopy[optlen + 1];
        int optcount = 0;

        if (listener->options)
        {
            strcpy(optcopy, listener->options);
            char *opt = optcopy;

            while (opt)
            {
                char *end = strnchr_esc(opt, ',', sizeof(optcopy) - (opt - optcopy));

                if (end)
                {
                    *end++ = '\0';
                }

                optarray[optcount++] = opt;
                opt = end;
            }
        }

        optarray[optcount] = NULL;

        listener->instance = listener->listener->authfunc.initialize(listener, optarray);
    }
}
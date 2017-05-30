/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

# define ERR_FUNC(func) ERR_PACK(ERR_LIB_BIO,func,0)
# define ERR_REASON(reason) ERR_PACK(ERR_LIB_BIO,0,reason)

static ERR_STRING_DATA BIO_str_functs[] = {
    {ERR_FUNC(BIO_F_ACPT_STATE), "acpt_state"},
    {ERR_FUNC(BIO_F_ADDR_STRINGS), "addr_strings"},
    {ERR_FUNC(BIO_F_BIO_ACCEPT), "BIO_accept"},
    {ERR_FUNC(BIO_F_BIO_ACCEPT_EX), "BIO_accept_ex"},
    {ERR_FUNC(BIO_F_BIO_ADDR_NEW), "BIO_ADDR_new"},
    {ERR_FUNC(BIO_F_BIO_CALLBACK_CTRL), "BIO_callback_ctrl"},
    {ERR_FUNC(BIO_F_BIO_CONNECT), "BIO_connect"},
    {ERR_FUNC(BIO_F_BIO_CTRL), "BIO_ctrl"},
    {ERR_FUNC(BIO_F_BIO_GETS), "BIO_gets"},
    {ERR_FUNC(BIO_F_BIO_GET_HOST_IP), "BIO_get_host_ip"},
    {ERR_FUNC(BIO_F_BIO_GET_NEW_INDEX), "BIO_get_new_index"},
    {ERR_FUNC(BIO_F_BIO_GET_PORT), "BIO_get_port"},
    {ERR_FUNC(BIO_F_BIO_LISTEN), "BIO_listen"},
    {ERR_FUNC(BIO_F_BIO_LOOKUP), "BIO_lookup"},
    {ERR_FUNC(BIO_F_BIO_MAKE_PAIR), "bio_make_pair"},
    {ERR_FUNC(BIO_F_BIO_NEW), "BIO_new"},
    {ERR_FUNC(BIO_F_BIO_NEW_FILE), "BIO_new_file"},
    {ERR_FUNC(BIO_F_BIO_NEW_MEM_BUF), "BIO_new_mem_buf"},
    {ERR_FUNC(BIO_F_BIO_NREAD), "BIO_nread"},
    {ERR_FUNC(BIO_F_BIO_NREAD0), "BIO_nread0"},
    {ERR_FUNC(BIO_F_BIO_NWRITE), "BIO_nwrite"},
    {ERR_FUNC(BIO_F_BIO_NWRITE0), "BIO_nwrite0"},
    {ERR_FUNC(BIO_F_BIO_PARSE_HOSTSERV), "BIO_parse_hostserv"},
    {ERR_FUNC(BIO_F_BIO_PUTS), "BIO_puts"},
    {ERR_FUNC(BIO_F_BIO_READ), "BIO_read"},
    {ERR_FUNC(BIO_F_BIO_SOCKET), "BIO_socket"},
    {ERR_FUNC(BIO_F_BIO_SOCKET_NBIO), "BIO_socket_nbio"},
    {ERR_FUNC(BIO_F_BIO_SOCK_INFO), "BIO_sock_info"},
    {ERR_FUNC(BIO_F_BIO_SOCK_INIT), "BIO_sock_init"},
    {ERR_FUNC(BIO_F_BIO_WRITE), "BIO_write"},
    {ERR_FUNC(BIO_F_BUFFER_CTRL), "buffer_ctrl"},
    {ERR_FUNC(BIO_F_CONN_CTRL), "conn_ctrl"},
    {ERR_FUNC(BIO_F_CONN_STATE), "conn_state"},
    {ERR_FUNC(BIO_F_DGRAM_SCTP_READ), "dgram_sctp_read"},
    {ERR_FUNC(BIO_F_DGRAM_SCTP_WRITE), "dgram_sctp_write"},
    {ERR_FUNC(BIO_F_FILE_CTRL), "file_ctrl"},
    {ERR_FUNC(BIO_F_FILE_READ), "file_read"},
    {ERR_FUNC(BIO_F_LINEBUFFER_CTRL), "linebuffer_ctrl"},
    {ERR_FUNC(BIO_F_MEM_WRITE), "mem_write"},
    {ERR_FUNC(BIO_F_SSL_NEW), "SSL_new"},
    {0, NULL}
};

static ERR_STRING_DATA BIO_str_reasons[] = {
    {ERR_REASON(BIO_R_ACCEPT_ERROR), "accept error"},
    {ERR_REASON(BIO_R_ADDRINFO_ADDR_IS_NOT_AF_INET),
     "addrinfo addr is not af inet"},
    {ERR_REASON(BIO_R_AMBIGUOUS_HOST_OR_SERVICE),
     "ambiguous host or service"},
    {ERR_REASON(BIO_R_BAD_FOPEN_MODE), "bad fopen mode"},
    {ERR_REASON(BIO_R_BROKEN_PIPE), "broken pipe"},
    {ERR_REASON(BIO_R_CONNECT_ERROR), "connect error"},
    {ERR_REASON(BIO_R_GETHOSTBYNAME_ADDR_IS_NOT_AF_INET),
     "gethostbyname addr is not af inet"},
    {ERR_REASON(BIO_R_GETSOCKNAME_ERROR), "getsockname error"},
    {ERR_REASON(BIO_R_GETSOCKNAME_TRUNCATED_ADDRESS),
     "getsockname truncated address"},
    {ERR_REASON(BIO_R_GETTING_SOCKTYPE), "getting socktype"},
    {ERR_REASON(BIO_R_INVALID_ARGUMENT), "invalid argument"},
    {ERR_REASON(BIO_R_INVALID_SOCKET), "invalid socket"},
    {ERR_REASON(BIO_R_IN_USE), "in use"},
    {ERR_REASON(BIO_R_LISTEN_V6_ONLY), "listen v6 only"},
    {ERR_REASON(BIO_R_LOOKUP_RETURNED_NOTHING), "lookup returned nothing"},
    {ERR_REASON(BIO_R_MALFORMED_HOST_OR_SERVICE),
     "malformed host or service"},
    {ERR_REASON(BIO_R_NBIO_CONNECT_ERROR), "nbio connect error"},
    {ERR_REASON(BIO_R_NO_ACCEPT_ADDR_OR_SERVICE_SPECIFIED),
     "no accept addr or service specified"},
    {ERR_REASON(BIO_R_NO_HOSTNAME_OR_SERVICE_SPECIFIED),
     "no hostname or service specified"},
    {ERR_REASON(BIO_R_NO_PORT_DEFINED), "no port defined"},
    {ERR_REASON(BIO_R_NO_SUCH_FILE), "no such file"},
    {ERR_REASON(BIO_R_NULL_PARAMETER), "null parameter"},
    {ERR_REASON(BIO_R_UNABLE_TO_BIND_SOCKET), "unable to bind socket"},
    {ERR_REASON(BIO_R_UNABLE_TO_CREATE_SOCKET), "unable to create socket"},
    {ERR_REASON(BIO_R_UNABLE_TO_KEEPALIVE), "unable to keepalive"},
    {ERR_REASON(BIO_R_UNABLE_TO_LISTEN_SOCKET), "unable to listen socket"},
    {ERR_REASON(BIO_R_UNABLE_TO_NODELAY), "unable to nodelay"},
    {ERR_REASON(BIO_R_UNABLE_TO_REUSEADDR), "unable to reuseaddr"},
    {ERR_REASON(BIO_R_UNAVAILABLE_IP_FAMILY), "unavailable ip family"},
    {ERR_REASON(BIO_R_UNINITIALIZED), "uninitialized"},
    {ERR_REASON(BIO_R_UNKNOWN_INFO_TYPE), "unknown info type"},
    {ERR_REASON(BIO_R_UNSUPPORTED_IP_FAMILY), "unsupported ip family"},
    {ERR_REASON(BIO_R_UNSUPPORTED_METHOD), "unsupported method"},
    {ERR_REASON(BIO_R_UNSUPPORTED_PROTOCOL_FAMILY),
     "unsupported protocol family"},
    {ERR_REASON(BIO_R_WRITE_TO_READ_ONLY_BIO), "write to read only BIO"},
    {ERR_REASON(BIO_R_WSASTARTUP), "WSAStartup"},
    {0, NULL}
};

#endif

int ERR_load_BIO_strings(void)
{
#ifndef OPENSSL_NO_ERR

    if (ERR_func_error_string(BIO_str_functs[0].error) == NULL) {
        ERR_load_strings(0, BIO_str_functs);
        ERR_load_strings(0, BIO_str_reasons);
    }
#endif
    return 1;
}

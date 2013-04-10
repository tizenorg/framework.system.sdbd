/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* a simple test program, connects to SDB server, and opens a track-devices session */
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>

static void
panic( const char*  msg )
{
    fprintf(stderr, "PANIC: %s: %s\n", msg, strerror(errno));
    exit(1);
}

static int
unix_write( int  fd, const char*  buf, int  len )
{
    int  result = 0;
    while (len > 0) {
        int  len2 = write(fd, buf, len);
        if (len2 < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            return -1;
        }
        result += len2;
        len -= len2;
        buf += len2;
    }
    return  result;
}

static int unix_read( int  fd, char*  buf, int  len )
{
    int  result = 0;
    while (len > 0) {
        int  len2 = read(fd, buf, len);
        if (len2 < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            return -1;
        }
        result += len2;
        len -= len2;
        buf += len2;
    }
    return  result;
}

int  main( void )
{
    int                  ret, s;
    struct sockaddr_in   server;
    char                 buffer[1024];
    const char*          transport = "host:transport-a";
    const char*          request = "cs:";
    int                  len;

    memset( &server, 0, sizeof(server) );
    server.sin_family      = AF_INET;
    server.sin_port        = htons(26099);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    s = socket( PF_INET, SOCK_STREAM, 0 );
    ret = connect( s, (struct sockaddr*) &server, sizeof(server) );
    if (ret < 0) panic( "could not connect to server" );

    /* send the transport */
    len = snprintf( buffer, sizeof buffer, "%04x%s", strlen(transport), transport );
    if (unix_write(s, buffer, len) < 0)
        panic( "could not send request" );

    /* read the OKAY answer */
    if (unix_read(s, buffer, 4) != 4)
        panic( "could not read request" );

    /* send the request */
    len = snprintf( buffer, sizeof buffer, "%04x%s", strlen(request), request );
    if (unix_write(s, buffer, len) < 0)
        panic( "could not send request" );

    /* read the OKAY answer */
    if (unix_read(s, buffer, 4) != 4)
        panic( "could not read request" );

    printf( "server answer: %.*s\n", 4, buffer );

    /* now loop */
    for (;;) {
        memset(buffer, 0, sizeof(buffer));
        if (unix_read(s, buffer, 30) < 0)
            panic("could not read length");

        printf( "server answer: %s\n",buffer );
        break;
    }
    close(s);
}
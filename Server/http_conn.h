#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"

class http_conn
{
public:
    //longest length of filename
    static const int FILENAME_LEN = 200;
    //read buffer size
    static const int READ_BUFFER_SIZE = 2048;
    //write buffer size
    static const int WRITE_BUFFER_SIZE = 1024;
    //request method but now only support GET!!!
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };
    //state of server
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    //process result state
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    //line status
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    //init a new accept connection
    void init(int sockfd, const sockaddr_in &addr);
    //close connection
    void close_conn(bool real_close = true);
    //process client request
    void process();
    //non-block read
    bool read();
    //non-block write
    bool write();

private:
    //init a connection
    void init();
    //parse http request
    HTTP_CODE process_read();
    //finish http reply
    bool process_write(HTTP_CODE ret);

    //called by process_read to parse HTTP request
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    //called by process_write to finish http reply
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    //all the events in socket will be signed in the same epoll list
    //so the epoll fd is stated as a static
    static int m_epollfd;
    //user count
    static int m_user_count;

private:
    //the socket and the other socket address
    int m_sockfd;
    sockaddr_in m_address;

    //read buffer
    char m_read_buf[READ_BUFFER_SIZE];
    //index of the next byte of the client data already read in buffer
    int m_read_idx;
    //the index of data in read!
    int m_checked_idx;
    //the position of the begining line
    int m_start_line;
    //write buffer
    char m_write_buf[WRITE_BUFFER_SIZE];
    //the num of byte to send in write buffer
    int m_write_idx;

    //the state of the main statemachine
    CHECK_STATE m_check_state;
    //request method
    METHOD m_method;

    //clients target file path. equals 'doc_root + m_url', doc_root is the root path
    char m_real_file[FILENAME_LEN];
    //filename of the target file
    char *m_url;
    //HTTP version only support HTTP/1.1
    char *m_version;
    //hostname
    char *m_host;
    //request length
    int m_content_length;
    //to linger
    bool m_linger;

    //the first position of the client target mapp in the memory
    char *m_file_address;
    //status of target file
    struct stat m_file_stat;

    //use writev to write
    struct iovec m_iv[2];
    //num of the written memory block 
    int m_iv_count;
};

#endif
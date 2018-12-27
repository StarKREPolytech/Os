#include <old/m_client.h>
#include <cstdio>
#include <sys/stat.h>
#include <zconf.h>
#include <lib4aio/lib4aio_cpp_headers/utils/file_utils/file_reader.h>
#include <lib4aio/lib4aio_cpp_headers/utils/str_builder/str_builder.h>
#include <lib4aio/lib4aio_cpp_headers/utils/string_utils/common.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <functional>
#include <lib4aio/lib4aio_cpp_headers/utils/log_utils/log_utils.h>
#include <lib4aio/lib4aio_cpp_headers/utils/string_utils/cast.h>
#include <sstream>
#include <fcntl.h>
#include <util/common.h>

#define TARGET_FILE_PATH "../io/client/sample1.txt"

#define PORT 8080

#define ADDRESS "127.0.0.1"

#define BUFFER_SIZE 1024

#define ACCEPT_STATUS "ACCEPT"

using namespace lib4aio;

using namespace std;

#define TAG "M_CLIENT"

static const char *const REQUEST_CHANNEL_NAME = "server_request_channel";

static const char *const RESPONSE_CHANNEL_NAME = "server_response_channel";

m_client::m_client()
{
    this->last_modified_file = get_file_last_modified_time(TARGET_FILE_PATH);
}

m_client::~m_client() = default;

void m_client::start()
{
    log_info(TAG, "START!");
    this->output_channel = open(REQUEST_CHANNEL_NAME, O_WRONLY);
    this->input_channel = open(RESPONSE_CHANNEL_NAME, O_RDONLY);
    sleep(5);
    log_info(TAG, "ASK!");
    this->ask();
}

void m_client::ask()
{
    //Send last time:
    int m_socket = socket(AF_INET, SOCK_STREAM, 0);
    this->call_by_socket("SYNC", 5, m_socket);

    //Receive socket_response:
    const char *socket_response = this->receive_by_socket(m_socket);
    if (strcmp(socket_response, ACCEPT_STATUS) == 0) {
        char channel_response[BUFFER_SIZE] = {0};
        while (strlen(channel_response) == 0) {
            read(this->input_channel, channel_response, BUFFER_SIZE);
            log_info_string(TAG, "CLIENT_SYNC_RESPONSE: ", channel_response);
        }
        if (strcmp(channel_response, ACCEPT_STATUS) == 0) {
            write(this->output_channel, ACCEPT_STATUS, 7);
        }
    }
    delete socket_response;
}

void m_client::call_by_socket(const char *request, const unsigned long size, const int m_socket)
{
    sockaddr_in server_address;
    memset(&server_address, '0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, ADDRESS, &server_address.sin_addr);
    connect(m_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    send(m_socket, request, size, 0);
    log_info_string(TAG, "SYNC_REQUEST:", request);
}

char *m_client::receive_by_socket(const int m_socket)
{
    char response[BUFFER_SIZE] = {0};
    while (strlen(response) == 0) {
        read(m_socket, response, BUFFER_SIZE);
    }
    log_info_string(TAG, "RESPONSE:", response);
    char *result = new_string(response);
    bzero(response, BUFFER_SIZE);
    return result;
}
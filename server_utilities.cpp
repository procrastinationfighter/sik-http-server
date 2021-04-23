#include "server_utilities.h"

const std::string &get_http_version_str() {
    static const std::string http_version = R"(HTTP/1.1)";
    return http_version;
}

const std::string &get_http_version_regex_str() {
    static const std::string http_version = R"(HTTP\/1\.1)";
    return http_version;
}

const std::string &get_prot() {
    static const std::string prot = "http://";
    return prot;
}

const std::string &get_CRLF() {
    static const std::string CRLF = "\r\n";
    return CRLF;
}

const std::string &get_reason_phrase(int status_code) {
    static const std::string ok = "OK";
    static const std::string found = "Found";
    static const std::string bad_request = "Bad Request";
    static const std::string not_found = "Not Found";
    static const std::string internal_error = "Internal Server Error";
    static const std::string not_implemented = "Not Implemented";

    switch (status_code) {
        case RESPONSE_OK:
            return ok;
        case RESPONSE_FOUND:
            return found;
        case RESPONSE_BAD_REQUEST:
            return bad_request;
        case RESPONSE_NOT_FOUND:
            return not_found;
        case RESPONSE_INTERNAL_ERROR:
            return internal_error;
        case RESPONSE_NOT_IMPLEMENTED:
            return not_implemented;
        default:
            throw std::invalid_argument("unknown response code");
    }
}

void exit_fail() {
    exit(EXIT_FAILURE);
}
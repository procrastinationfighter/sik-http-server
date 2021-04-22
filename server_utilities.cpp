#include "server_utilities.h"

const std::string &get_http_version_str() {
    static const std::string http_version = R"(HTTP\/1\.1)";
    return http_version;
}

const std::string &get_CRLF() {
    static const std::string CRLF = "\r\n";
    return CRLF;
}

const std::string &get_methods_str() {
    static const std::string methods = "(?:HEAD|GET)";
    return methods;
}

const std::string &get_request_target_str() {
    static const std::string str = R"((?:/|(?:/[a-zA-Z0-9.-]+)+))";
    return str;
}

const std::string &get_request_line_str() {
    static const std::string line = R"(\w+ )"
        + get_request_target_str()
        + " "
        + get_http_version_str()
        + get_CRLF();
    return line;
}

const std::string &get_field_value_str() {
    static const std::string field = R"(\w+)";
    return field;
}

const std::string &get_header_field_str() {
    static const std::string header = R"(\w+:\s*)"
        + get_field_value_str()
        + R"(\s*)";
    return header;
}

const std::string &get_mess_body_str() {
    // Since our server accepts only GET and HEAD methods,
    // body should be empty.
    static const std::string body;
    return body;
}

const std::string &get_mess_request_str() {
    static const std::string message = "(" + get_request_line_str() + ")"
        + "(" + get_header_field_str() + get_CRLF() + ")*"
        + get_CRLF()
        + "(" + get_mess_body_str() + ")";
    return message;
}

const std::regex &get_directory_regex() {
    static const std::regex regex(R"(/?(\w+/)*)");
    return regex;
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
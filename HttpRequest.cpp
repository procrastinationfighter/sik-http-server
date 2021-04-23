#include <iostream>
#include "HttpRequest.h"
#include "exceptions.h"

#define _GNU_SOURCE

namespace {

enum class Header {
    CONNECTION,
    CONTENT_LENGTH,
    OTHER
};

std::regex &get_request_line_regex() {
    // [TODO]: Change accepted file.
    static std::regex regex(R"(([a-zA-Z]+) ([\ -~]+) )"
                                + get_http_version_regex_str()
                                + get_CRLF());
    return regex;
}

bool is_target_file_correct(const std::string &target_file) {
    static std::regex regex(R"([a-zA-Z0-9.\-\/]+)");
    return std::regex_match(target_file, regex);
}

void make_string_lower(std::string &str) {
    for (auto &c : str) {
        c = static_cast<char>(tolower(c));
    }
}

Header string_to_header(std::string &str) {
    static const std::string serv = "server",
                             conn = "connection",
                             con_type = "content-type",
                             con_len = "content-length";
    make_string_lower(str);

    if (str == conn) {
        return Header::CONNECTION;
    } else if (str == con_len) {
        return Header::CONTENT_LENGTH;
    } else {
        return Header::OTHER;
    }
}

std::regex &get_header_regex() {
    static std::regex regex(R"(([a-zA-Z0-9\-_]+):\s*([\ -~]*[!-~])\s*)"
                                + get_CRLF());
    return regex;
}

// Assumes that correct method is uppercase.
HttpRequest::Method string_to_method(const std::string &str) {
    const static std::string get = "GET";
    const static std::string head = "HEAD";
    if (str == get) {
        return HttpRequest::Method::GET;
    } else if (str == head) {
        return HttpRequest::Method::HEAD;
    } else {
        throw UnsupportedHttpMethod(str);
    }
}

std::string read_line(FILE *file) {
    char *line = nullptr;
    size_t len = 0;
    size_t read = getline(&line, &len, file);

    if (read == EOF) {
        throw ConnectionLost("EOF");
    } else if (line == nullptr) {
        throw ServerInternalError("getline");
    } else {
        std::string line_str(line);
        free(line);
        return line_str;
    }
}

std::pair<HttpRequest::Method, std::string> parse_request_line(FILE *input_file) {
    std::smatch match;

    std::string line = read_line(input_file);
    std::regex_match(line, match, get_request_line_regex());
    if (match.empty()) {
        throw IncorrectRequestFormat("request_line");
    }

    return {string_to_method(match[1]), match[2]};
}

// Returns true if Connection: Close header was included.
bool parse_headers(FILE *input_file) {
    static const std::string CONNECTION_CLOSE = "close",
                             ZERO_VALUE_FIELD = "0";
    bool finish = false,
         close_connection = false,
         was_connection = false,
         was_content_length = false;
    std::smatch match;

    while(!finish) {
        std::string line = read_line(input_file);
        if (line == get_CRLF()) {
            finish = true;
        } else {
            std::regex_match(line, match, get_header_regex());
            if (match.empty()) {
                throw IncorrectRequestFormat("header");
            }

            std::string field_name = match[1];
            std::string field_value = match[2];

            Header header_type = string_to_header(field_name);
            switch (header_type) {
                case Header::CONNECTION:
                    if (was_connection) {
                        throw IncorrectRequestFormat("double connection header");
                    } else {
                        was_connection = true;
                        make_string_lower(field_value);
                        if (field_value == CONNECTION_CLOSE) {
                            close_connection = true;
                        }
                    }
                    break;
                case Header::CONTENT_LENGTH:
                    if (was_content_length) {
                        throw IncorrectRequestFormat("double content_length header");
                    } else {
                        was_content_length = true;
                        make_string_lower(field_value);
                        if(field_value != ZERO_VALUE_FIELD) {
                            throw IncorrectRequestFormat("content_length wrong value");
                        }
                    }
                    break;
                case Header::OTHER:
                    // We ignore other headers.
                    break;
            }

        }
    }

    return close_connection;
}
}

[[nodiscard]] HttpRequest::Method HttpRequest::get_method() const {
    return method;
}

[[nodiscard]] const std::string &HttpRequest::get_request_target() const {
    return request_target;
}

[[nodiscard]] bool HttpRequest::should_close_connection() const {
    return close_connection;
}

HttpRequest parse_http_request(FILE *input_file) {
    auto start_result = parse_request_line(input_file);
    bool close_connection = parse_headers(input_file);
    if (!is_target_file_correct(start_result.second)) {
        throw TargetFileIncorrectCharacters(close_connection);
    }
    return HttpRequest(start_result.first, start_result.second, close_connection);
}
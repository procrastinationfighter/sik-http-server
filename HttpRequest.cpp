#include <iostream>
#include "HttpRequest.h"
#include "exceptions.h"

#define _GNU_SOURCE

namespace {
std::regex get_request_line_regex() {
    static std::regex regex(R"(([a-zA-Z]+) ([a-zA-Z0-9.\-\/]+) )"
                                + get_http_version_str()
                                + get_CRLF());
    return regex;
}

void make_string_lower(std::string &str) {
    for (auto &c : str) {
        c = static_cast<char>(tolower(c));
    }
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

size_t read_line(char **line, size_t *len, FILE *file) {
    size_t read = getline(line, len, file);

    if (read == EOF) {
        throw ConnectionLost("EOF");
    } else if (*line == nullptr) {
        throw ServerInternalError("getline");
    } else {
        return read;
    }
}

std::pair<HttpRequest::Method, std::string> parse_request_line(FILE *input_file) {
    std::smatch match;
    char *line = nullptr;
    size_t len = 0;

    size_t read = read_line(&line, &len, input_file);
    std::string line_str(line);
    free(line);

    std::cout << "Read line: " << line_str;

    std::regex_match(line_str, match, get_request_line_regex());
    if (match.empty()) {
        std::cout << "request_line\n";
        throw IncorrectRequestFormat("request_line");
    }

    std::cout << "Metoda: " << match[1] << ", zasob: " << match[2] << "\n";

    return {string_to_method(match[1]), match[2]};
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

    return HttpRequest(start_result.first, start_result.second, false);
}
#include "HttpRequest.h"

namespace {
std::regex get_request_line_regex() {
    // We limit the method to 10 characters.
    // I've never seen a longer method name.
    static std::regex regex(R"((\w+){1, 10} ([a-zA-Z0-9.-/]+) )"
                                + get_http_version_str()
                                + get_CRLF());
    return regex;
}

void make_string_lower(std::string &str) {
    for (auto &c : str) {
        c = static_cast<char>(tolower(c));
    }
}

HttpRequest::Method string_to_method(const std::string &str) {

}

size_t read_line(char **line, size_t *len, FILE *file) {
    size_t read = getline(line, len, file);

    if (read == EOF) {
        throw ConnectionLost("a");
    } else if (*line == nullptr) {
        throw std::runtime_error("getline");
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

    std::regex_match(line_str, match, get_request_line_regex());
    if (match.empty()) {
        throw IncorrectRequestFormat("request_line");
    }

    std::string method_str = match[1];
    make_string_lower(method_str);

    return {HttpRequest::string_to_method(method_str), match[2]};
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
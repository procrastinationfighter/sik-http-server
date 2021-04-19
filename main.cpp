#include <iostream>

const std::string &get_http_version_str() {
    static const std::string http_version = "HTTP/1.1";
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
    static const std::string line = get_methods_str()
                                  + " "
                                  + get_request_target_str()
                                  + " "
                                  + get_http_version_str()
                                  + get_CRLF();
    return line;
}

const std::string &get_field_value_str() {
    static const std::string field = R"(\w+)";
}

const std::string &get_header_field_str() {
    static const std::string header = R"(\w+:\s*)"
                                    + get_field_value_str()
                                    + R"(\s*)";
    return header;
}

const std::string &get_message_body_str() {
    // Since our server accepts only GET and HEAD methods,
    // body should be empty.
    static const std::string body;
    return body;
}

const std::string &get_mess_request_str() {
    static const std::string message = "(" + get_request_line_str() + ")"
                                     + "(" + get_header_field_str() + get_CRLF() + ")*"
                                     + get_CRLF()
                                     + "(" + get_message_body_str() + ")";
    return message;
}

int main() {
    for (auto x: get_CRLF()) {
        std::cout << static_cast<int>(x) << "\n";
    }
    return 0;
}

#ifndef SIK1__SERVER_UTILITIES_H_
#define SIK1__SERVER_UTILITIES_H_

#include <regex>

const std::string &get_http_version_str();

const std::string &get_CRLF();

const std::string &get_methods_str();

const std::string &get_request_target_str();

const std::string &get_request_line_str();

const std::string &get_field_value_str();

const std::string &get_header_field_str();

const std::string &get_mess_body_str();

const std::string &get_mess_request_str();

const std::regex &get_directory_regex();

void exit_fail();
#endif //SIK1__SERVER_UTILITIES_H_

#ifndef SIK1__SERVER_UTILITIES_H_
#define SIK1__SERVER_UTILITIES_H_

#include <regex>

#define syserr(mess) \
    std::cerr << "ERROR: " << mess << std::endl; \
    exit_fail()

constexpr int RESPONSE_OK = 200;
constexpr int RESPONSE_FOUND = 302;
constexpr int RESPONSE_BAD_REQUEST = 400;
constexpr int RESPONSE_NOT_FOUND = 404;
constexpr int RESPONSE_INTERNAL_ERROR = 500;
constexpr int RESPONSE_NOT_IMPLEMENTED = 501;

const std::string &get_http_version_str();

const std::string &get_http_version_regex_str();

const std::string &get_CRLF();

const std::string &get_prot();

const std::string &get_reason_phrase(int status_code);

void exit_fail();
#endif //SIK1__SERVER_UTILITIES_H_

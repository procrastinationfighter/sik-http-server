#ifndef SIK1__HTTPREQUEST_H_
#define SIK1__HTTPREQUEST_H_

#include <set>
#include <cctype>
#include <utility>
#include "server_utilities.h"
#include "exceptions.h"

class HttpRequest {
  public:
    enum class Method {
        GET,
        HEAD
    };

  private:
    const Method method;
    const std::string request_target;
    const bool close_connection;

  public:
    HttpRequest(Method method, std::string request_target, bool close_connection)
        : method(method),
          request_target(std::move(request_target)),
          close_connection(close_connection) {}

    [[nodiscard]] Method get_method() const;
    [[nodiscard]] const std::string &get_request_target() const;
    [[nodiscard]] bool should_close_connection() const;
};

HttpRequest parse_http_request(FILE *input_file);

#endif //SIK1__HTTPREQUEST_H_

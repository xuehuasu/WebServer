#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <errno.h>
#include <mysql/mysql.h>

#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../pool/sqlconnpool.h"

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

    private:
    bool ParseRequestLine_(const std::string& line);
    void ParseHeader_(const std::string& line);
    void ParseBody_(const std::string& line);

    void ParsePath_();
    void ParsePost_();
    void ParseFromUrlencoded_();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool reg);

    PARSE_STATE state_; // PARSE_STATE请求解析状态
    std::string method_, path_, version_, body_; // 请求方法，路径，版本，请求体
    std::unordered_map<std::string, std::string> header_; // 请求头
    std::unordered_map<std::string, std::string> post_; // post请求体
    static const std::unordered_set<std::string> DEFAULT_HTML; // 默认网页
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG; // 默认网页标签
    static int ConverHex(char ch); // 字符转16进制
};

#endif
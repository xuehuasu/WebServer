#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <unordered_map>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false,
        int code = -1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer& buff);
    void AddHeader_(Buffer& buff);
    void AddContent_(Buffer& buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_;                  // 状态码
    bool isKeepAlive_;          // 是否保持连接

    std::string path_;          // 请求的路径
    std::string srcDir_;        // 资源路径

    char* mmFile_;              // 内存映射
    struct stat mmFileStat_;    // 文件属性

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 文件后缀名和文件类型的映射
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 状态码和状态信息的映射
    static const std::unordered_map<int, std::string> ERROR_CODE;            // 错误页面状态码的映射
};

#endif 
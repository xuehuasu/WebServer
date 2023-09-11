#include "httpresponse.h"


const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".avi", "video/x-msvideo"},
    {".gif", "image/gif"},
    {".xhtml", "application/xhtml+xml"},
    {".xml", "text/xml"},
    {".txt", "text/plain"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".mp4", "video/mp4"},
    {".flv", "video/flv"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".mpg", "video/mpeg"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
    {".gz", "application/x-gzip"},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::ERROR_CODE = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

/**
 * @brief 构造函数
*/
HttpResponse::HttpResponse() {
  code_ = -1;
  path_ = srcDir_ = "";
  isKeepAlive_ = false;
  mmFile_ = nullptr;
  mmFileStat_ = {0};
};

/**
 * @brief 析构函数
*/
HttpResponse::~HttpResponse() { UnmapFile(); }

/**
 * @brief 初始化
*/
void HttpResponse::Init(const std::string& srcDir, std::string& path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if (mmFile_) {
        UnmapFile();
    }
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}

/**
 * @brief 释放内存映射
*/
void HttpResponse::UnmapFile() {
    if (mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

/**
* @brief 将响应信息写入Buffer对象
*/
void HttpResponse::MakeResponse(Buffer& buff) {
    /* 判断请求的资源文件 */
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) { // 目录
        code_ = 404;
    } else if (!(mmFileStat_.st_mode & S_IROTH)) {  // 权限
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

/**
 * @brief 错误页面
*/
void HttpResponse::ErrorHtml_() {
    if (ERROR_CODE.count(code_) == 1) {
        stat((srcDir_ + path_).data(), &mmFileStat_);
        path_ = ERROR_CODE.find(code_)->second;
    }
}

/**
 * @brief 添加状态行
*/
void HttpResponse::AddStateLine_(Buffer& buff) {
    std::string status;
    if (CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

/**
 * @brief 添加响应头
*/
void HttpResponse::AddHeader_(Buffer& buff) {
    buff.Append("Connection: ");
    if (isKeepAlive_) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

/**
 * @brief 添加响应体
*/
void HttpResponse::AddContent_(Buffer& buff) {
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if (srcFd < 0) {
        ErrorContent(buff, "File NotFound!");
        return;
    }

    // 将文件映射到内存
    LOG_DEBUG("file path %s", (srcDir_ + path_).data());
    int* mmRet = (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");
        return;
    }
    mmFile_ = (char*)mmRet;
    close(srcFd);
    buff.Append("Content-length: " + std::to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

/**
 * @brief 获取文件类型
*/
std::string HttpResponse::GetFileType_() {
    std::string::size_type idx = path_.find_last_of('.');
    if (idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

/**
 * @brief 错误页面
*/
void HttpResponse::ErrorContent(Buffer& buff, std::string message) {
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em> WebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

/**
 * @brief 获取文件内容
*/
char* HttpResponse::File() { return mmFile_; }

/**
 * @brief 获取文件大小
*/
size_t HttpResponse::FileLen() const { return mmFileStat_.st_size; }

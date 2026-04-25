#ifndef CPP_HTTPLIB_H
#define CPP_HTTPLIB_H
#include <winsock2.h>
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <sstream>
#include <vector>

namespace httplib {
    struct Request {
        std::string method, path, body;
        std::map<std::string, std::string> params;
        std::string get_param_value(const std::string& key) const {
            auto it = params.find(key);
            return (it != params.end()) ? it->second : "";
        }
    };
    struct Response {
        std::string body, content_type = "text/plain";
        void set_content(std::string content, std::string type) { body = content; content_type = type; }
    };
    class Server {
    public:
        using Handler = std::function<void(const Request&, Response&)>;
        void Get(std::string path, Handler h) { handlers["GET"][path] = h; }
        void Post(std::string path, Handler h) { handlers["POST"][path] = h; }
        
        void listen(const char* host, int port) {
            WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
            SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
            bind(s, (struct sockaddr*)&addr, sizeof(addr)); ::listen(s, 10);
            while (true) {
                SOCKET c = accept(s, NULL, NULL);
                std::vector<char> buf(16384, 0);
                int bytes = recv(c, buf.data(), 16384, 0);
                if (bytes <= 0) { closesocket(c); continue; }
                std::string raw(buf.data(), bytes);
                
                std::stringstream ss(raw);
                std::string method, full_path, proto;
                ss >> method >> full_path >> proto;
                
                Request req; req.method = method;
                size_t q = full_path.find("?");
                req.path = (q == std::string::npos) ? full_path : full_path.substr(0, q);
                
                auto parse = [&](std::string src) {
                    std::stringstream qss(src); std::string item;
                    while (std::getline(qss, item, '&')) {
                        if (item.empty()) continue;
                        size_t eq = item.find("=");
                        if (eq != std::string::npos) {
                            std::string key = item.substr(0, eq);
                            std::string val = item.substr(eq + 1);
                            // Trim trailing \r or whitespace
                            while(!val.empty() && (val.back() == '\r' || val.back() == '\n' || val.back() == ' ')) val.pop_back();
                            req.params[key] = val;
                        }
                    }
                };
                if (q != std::string::npos) parse(full_path.substr(q + 1));
                
                size_t bpos = raw.find("\r\n\r\n");
                if (bpos != std::string::npos) {
                    req.body = raw.substr(bpos + 4);
                    if (method == "POST") parse(req.body);
                }

                Response res;
                if (handlers[method].count(req.path)) handlers[method][req.path](req, res);
                else if (method == "GET" && handlers["POST"].count(req.path)) handlers["POST"][req.path](req, res);
                else res.set_content("Not Found", "text/plain");

                std::string r = "HTTP/1.1 200 OK\r\nContent-Type: " + res.content_type + "; charset=utf-8\r\nContent-Length: " + std::to_string(res.body.length()) + "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" + res.body;
                send(c, r.c_str(), (int)r.length(), 0); closesocket(c);
            }
        }
    private:
        std::map<std::string, std::map<std::string, Handler>> handlers;
    };
}
#endif

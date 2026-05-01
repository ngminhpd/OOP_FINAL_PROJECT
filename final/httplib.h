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
    using namespace std;
    struct Request {
        string method, path, body;
        map<string, string> params;
        string get_param_value(const string& key) const {
            auto it = params.find(key);
            return (it != params.end()) ? it->second : "";
        }
    };
    struct Response {
        string body, content_type = "text/plain";
        void set_content(string content, string type) { body = content; content_type = type; }
    };
    class Server {
    public:
        using Handler = function<void(const Request&, Response&)>;
        void Get(string path, Handler h) { handlers["GET"][path] = h; }
        void Post(string path, Handler h) { handlers["POST"][path] = h; }
        
        void listen(const char* host, int port) {
            WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
            SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
            bind(s, (struct sockaddr*)&addr, sizeof(addr)); ::listen(s, 10);
            while (true) {
                SOCKET c = accept(s, NULL, NULL);
                vector<char> buf(16384, 0);
                int bytes = recv(c, buf.data(), 16384, 0);
                if (bytes <= 0) { closesocket(c); continue; }
                string raw(buf.data(), bytes);
                
                stringstream ss(raw);
                string method, full_path, proto;
                ss >> method >> full_path >> proto;
                
                Request req; req.method = method;
                size_t q = full_path.find("?");
                req.path = (q == string::npos) ? full_path : full_path.substr(0, q);
                
                auto parse = [&](string src) {
                    stringstream qss(src); string item;
                    while (getline(qss, item, '&')) {
                        if (item.empty()) continue;
                        size_t eq = item.find("=");
                        if (eq != string::npos) {
                            string key = item.substr(0, eq);
                            string val = item.substr(eq + 1);
                            while(!val.empty() && (val.back() == '\r' || val.back() == '\n' || val.back() == ' ')) val.pop_back();
                            req.params[key] = val;
                        }
                    }
                };
                if (q != string::npos) parse(full_path.substr(q + 1));
                
                size_t bpos = raw.find("\r\n\r\n");
                if (bpos != string::npos) {
                    req.body = raw.substr(bpos + 4);
                    if (method == "POST") parse(req.body);
                }

                Response res;
                if (handlers[method].count(req.path)) handlers[method][req.path](req, res);
                else if (method == "GET" && handlers["POST"].count(req.path)) handlers["POST"][req.path](req, res);
                else res.set_content("Not Found", "text/plain");

                string r = "HTTP/1.1 200 OK\r\nContent-Type: " + res.content_type + "; charset=utf-8\r\nContent-Length: " + to_string(res.body.length()) + "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" + res.body;
                send(c, r.c_str(), (int)r.length(), 0); closesocket(c);
            }
        }
    private:
        map<string, map<string, Handler>> handlers;
    };
}
#endif

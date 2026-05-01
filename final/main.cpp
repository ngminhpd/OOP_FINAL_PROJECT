#include "BankServer.h"
#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

using namespace std;
using namespace BankSystem;

class BankApp {
private:
    BankManager manager;
    BankServer server;
    bool running;

public:
    BankApp() : server(manager), running(true) {
#ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
#endif
    }

#ifdef _WIN32
    static unsigned __stdcall SyncThreadFunc(void* p) {
        BankApp* app = (BankApp*)p;
        while (app->running) {
            Sleep(5000);
            app->manager.Load();
        }
        return 0;
    }
#endif

    friend istream& operator>>(istream& is, BankApp& app) {
        cout << " > Dang khoi tao he thong... " << endl;
        app.manager.Load();

#ifdef _WIN32
        _beginthreadex(NULL, 0, SyncThreadFunc, &app, 0, NULL);
#else
        
#endif

        app.server.Listen("0.0.0.0", 8080);
        return is;
    }

    void Stop() { running = false; }
};

int main() {
    BankApp app;
    cin >> app; 
    return 0;
}

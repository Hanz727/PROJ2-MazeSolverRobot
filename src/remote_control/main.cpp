#include <iostream>
#include <limits>
#include <chrono>
#include <string>
#include <thread>
#include <windows.h>
#include <bluetoothapis.h>

namespace Bluetooth {
    HANDLE OpenCOMPort(const std::string& portName) {
        // For COM10+ use \\\\.\\COMx format
        std::string fullPortName = "\\\\.\\" + portName;

        // Open the serial port
        HANDLE hSerial = CreateFileA(fullPortName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (hSerial == INVALID_HANDLE_VALUE) {
            std::cerr << "Error opening COM port " << portName << "!" << std::endl;
            return INVALID_HANDLE_VALUE;
        }

        // Configure serial port settings
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial, &dcbSerialParams)) {
            std::cerr << "Error getting COM port state!" << std::endl;
            CloseHandle(hSerial);
            return INVALID_HANDLE_VALUE;
        }

        // Set baud rate, byte size, parity, stop bits
        dcbSerialParams.BaudRate = CBR_9600;  // Make sure this matches your device's baud rate (e.g., HC-05)
        dcbSerialParams.ByteSize = 8;         // 8-bit data
        dcbSerialParams.Parity = NOPARITY;    // No parity
        dcbSerialParams.StopBits = ONESTOPBIT; // 1 stop bit

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            std::cerr << "Error setting COM port parameters!" << std::endl;
            CloseHandle(hSerial);
            return INVALID_HANDLE_VALUE;
        }

        // Set timeouts
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 10;           // Max time between characters
        timeouts.ReadTotalTimeoutConstant = 50;      // Max time to wait for a full read
        timeouts.ReadTotalTimeoutMultiplier = 10;    // Additional timeout multiplier for reads
        timeouts.WriteTotalTimeoutConstant = 50;     // Max time to wait to write
        timeouts.WriteTotalTimeoutMultiplier = 10;   // Additional timeout multiplier for writes

        if (!SetCommTimeouts(hSerial, &timeouts)) {
            std::cerr << "Error setting COM port timeouts!" << std::endl;
            CloseHandle(hSerial);
            return INVALID_HANDLE_VALUE;
        }

        std::cout << "COM port " << portName << " opened successfully!" << std::endl;
        return hSerial;
    }

    void listCOMPorts() {
        HKEY hKey;
        char comPort[256];
        DWORD size = sizeof(comPort);

        // Open the registry key where Windows stores Bluetooth COM ports
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char valueName[256];
            DWORD valueSize = sizeof(valueName);
            DWORD index = 0;
            DWORD type;

            // Enumerate all entries under the key
            while (RegEnumValueA(hKey, index, valueName, &valueSize, NULL, &type, (LPBYTE)comPort, &size) == ERROR_SUCCESS) {
                if (type == REG_SZ) {
                    std::cout << "Found COM port: " << comPort << " " << valueName << std::endl;
                }
                index++;
                valueSize = sizeof(valueName);
                size = sizeof(comPort);
            }
            RegCloseKey(hKey);
        }
    }

    bool findDevice(wchar_t* deviceName, BLUETOOTH_DEVICE_INFO* deviceInfo) {
        BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = { sizeof(searchParams) };
        searchParams.fReturnAuthenticated = TRUE;
        searchParams.fReturnRemembered = TRUE;
        searchParams.fReturnUnknown = TRUE;
        searchParams.fReturnConnected = TRUE;
        searchParams.fIssueInquiry = TRUE;
        searchParams.cTimeoutMultiplier = 2;
        searchParams.hRadio = NULL; // NULL searches all local radios

        HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, deviceInfo);
        if (hFind == NULL) {
            std::cerr << "No Bluetooth devices found or an error occurred." << std::endl;
            return EXIT_FAILURE;
        }

        bool deviceFound = false;
        do {
            if (!wcscmp(deviceInfo->szName, deviceName))  {
                deviceFound = true;
                break;
            }
        } while (BluetoothFindNextDevice(hFind, deviceInfo));

        if (!deviceFound) {
            std::wcout << L"Unable to find bluetooth device: " << deviceName << std::endl;
        }

        BluetoothFindDeviceClose(hFind);
        return deviceFound;
    }

    bool writeString(HANDLE hSerial, const std::string& data) {
        DWORD bytesWritten;
        if (!WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL)) {
            std::cerr << "Error writing to COM port!" << std::endl;
            return false;
        }
        return true;
    }

    bool writeLine(HANDLE hSerial, const std::string& data) {
        DWORD bytesWritten;
        if (!WriteFile(hSerial, std::string(data + '\n').c_str(), data.length(), &bytesWritten, NULL)) {
            std::cerr << "Error writing to COM port!" << std::endl;
            return false;
        }
        return true;
    }

    std::string readLine(HANDLE hSerial) {
        std::string result = "";
        char ch;
        DWORD bytesRead;
        while (true) {
            if (ReadFile(hSerial, &ch, 1, &bytesRead, NULL) && bytesRead > 0) {
                if (ch == '\n' || ch == '\0') {
                    break;
                };
                result += ch;
            } else {
                if (bytesRead > 0) {
                    std::cerr << "Error reading from COM port!" << std::endl;
                    std::cout << GetLastError() << "\n";
                    return "";
                }
            }
        }
        return result;
    }

    bool available(HANDLE hSerial) {
        DWORD dwErrors;
        COMSTAT comStat;

        // Get communication status
        if (!ClearCommError(hSerial, &dwErrors, &comStat)) {
            std::cerr << "Error checking COM port status!" << std::endl;
            return false;
        }
        // Check if there's data in the input buffer
        return comStat.cbInQue > 0;  // cbInQue is the number of bytes in the input buffer
    }

    void clearCOMBuffer(HANDLE hSerial) {
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }

    std::string SelectCOMPort() {
        wchar_t deviceName[] = L"HC-05";
        BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(deviceInfo) };

        if (!findDevice(deviceName, &deviceInfo))
            return "";

        std::wcout << L"Found bluetooth device: " << deviceInfo.szName << std::endl << std::endl;
        listCOMPorts();

        std::cout << "\nSelect COM Port: ";
        std::string COMPort;
        std::cin >> COMPort;

        std::cout << "Selected COM Port: \"" + COMPort + "\"" << std::endl;  

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return COMPort;
    }

};

int main() {
    std::string COMPort = Bluetooth::SelectCOMPort();
    //std::string COMPort = "com5";
    if (COMPort == "")
        return EXIT_FAILURE;

    HANDLE hSerial = Bluetooth::OpenCOMPort(COMPort);
    if (hSerial == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;
    
    while (Bluetooth::available(hSerial)) {
        Bluetooth::readLine(hSerial);
    }

    std::string msg;
    while (1) {
        if (msg[0] != 10)
            std::cout << "[>>] ";
        msg = "";
        std::getline(std::cin, msg);
        msg += "\n";

        if (msg.size() < 2) {
            continue;
        }

        Bluetooth::writeString(hSerial, msg);

        std::string read; 
        while (Bluetooth::available(hSerial) || read.size() <= 1) {
            read = Bluetooth::readLine(hSerial); 
            if (read.size() > 0)
                std::cout << "\t[<<] " << read << "\n";
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    CloseHandle(hSerial);

    return EXIT_SUCCESS;
}

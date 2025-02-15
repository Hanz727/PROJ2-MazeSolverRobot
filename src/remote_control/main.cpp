#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <bluetoothapis.h>

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

void ListBluetoothCOMPorts() {
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

bool WriteToCOMPort(HANDLE hSerial, const std::string& data) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL)) {
        std::cerr << "Error writing to COM port!" << std::endl;
        return false;
    }
    return true;
}

std::string ReadFromCOMPort(HANDLE hSerial) {
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

bool IsDataAvailable(HANDLE hSerial) {
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

bool FindBluetoothDevice(wchar_t* deviceName, BLUETOOTH_DEVICE_INFO* deviceInfo) {
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

std::string SelectCOMPort() {
    wchar_t deviceName[] = L"HC-05";
    BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(deviceInfo) };

    if (!FindBluetoothDevice(deviceName, &deviceInfo))
        return "";

    std::wcout << L"Found bluetooth device: " << deviceInfo.szName << std::endl << std::endl;
    ListBluetoothCOMPorts();

    std::cout << "\nSelect COM Port: ";
    std::string COMPort;
    std::cin >> COMPort;

    std::cout << "Selected COM Port: \"" + COMPort + "\"" << std::endl;  
    return COMPort;
}

int main() {
    std::string COMPort = SelectCOMPort();
    if (COMPort == "")
        return EXIT_FAILURE;

    HANDLE hSerial = OpenCOMPort(COMPort);
    if (hSerial == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;
    
    WriteToCOMPort(hSerial, "CONNECTED\n");
    
    while (1) {
        if (IsDataAvailable(hSerial)) {
            std::cout << ReadFromCOMPort(hSerial) << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    CloseHandle(hSerial);

    return EXIT_SUCCESS;
}

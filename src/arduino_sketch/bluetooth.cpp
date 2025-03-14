#include "bluetooth.h"

bool startsWith(String str, String prefix) {
  if (str.length() < prefix.length()) {
    return false;
  }
  return str.substring(0, prefix.length()) == prefix;
}

void resetArduino() {
    // Enable watchdog timer to trigger a reset after 15 milliseconds
    delay(250);
    wdt_enable(WDTO_15MS);  
    while (1) {}  // Wait for reset to occur
}


void handleBluetoothCmds(int8_t& width, int8_t& height, bool& start) {
    bool validCommand = false;
    if (Bluetooth.available()) {
        String buffer;
        buffer = Bluetooth.readStringUntil('\n'); // Read single full message
        //Bluetooth.flush();
        buffer.toUpperCase();
        Serial.println(buffer);

        if (startsWith(buffer, "GET DIM")) {
          Bluetooth.println(String(width) + "x" + String(height));
          validCommand = true;
        }

        if (startsWith(buffer, "SET DIM")) {
            int xPos = buffer.indexOf('X');
            String widthStr = buffer.substring(buffer.indexOf(' ', xPos-4), xPos);
            String heightStr = buffer.substring(xPos+1);
            width = widthStr.toInt();
            height = heightStr.toInt();
            Bluetooth.println("New dims: " + String(width) + "x" + String(height));
            validCommand = true;
        }

        if (startsWith(buffer, "START")) {
          start = true;
          Bluetooth.println("Starting program...");
          validCommand = true;
        }

        if (startsWith(buffer, "STOP")) {
          start = false;
          Bluetooth.println("Stopping program...");
          validCommand = true;
        }

        if (startsWith(buffer, "RESET")) {
          Bluetooth.println("Resetting program...");
          resetArduino();
          validCommand = true;
        }

        if (!validCommand) {
          Bluetooth.println("Invalid command");
        }
    }
}

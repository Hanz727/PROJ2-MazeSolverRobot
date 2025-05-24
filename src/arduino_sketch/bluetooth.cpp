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


void handleBluetoothCmds(int8_t& width, int8_t& height, bool& start, void (*forward)(), void (*backward)(), void (*left)(), void (*right)()) {
    if (Bluetooth.available()) {
        String buffer;
        buffer = Bluetooth.readStringUntil('\n'); // Read single full message
        //Bluetooth.flush();
        buffer.toUpperCase();
        Serial.println(buffer);

        if (startsWith(buffer, "GET DIM")) {
          Bluetooth.println(String(width) + "x" + String(height));
          return;
        }

        if (startsWith(buffer, "SET DIM")) {
            int xPos = buffer.indexOf('X');
            String widthStr = buffer.substring(buffer.indexOf(' ', xPos-4), xPos);
            String heightStr = buffer.substring(xPos+1);
            width = widthStr.toInt();
            height = heightStr.toInt();
            Bluetooth.println("New dims: " + String(width) + "x" + String(height));
            return;
        }

        if (startsWith(buffer, "FORWARD")) {
            forward();
            Bluetooth.println("moving...");
            return;
        }
        if (startsWith(buffer, "BACKWARD")) {
            backward();
            Bluetooth.println("moving...");
            return; 
        }
        if (startsWith(buffer, "LEFT")) {
            left();
            Bluetooth.println("moving...");
            return;
        }
        if (startsWith(buffer, "RIGHT")) {
            right();
            Bluetooth.println("moving...");
            return;
        }

        if (startsWith(buffer, "START")) {
          start = true;
          Bluetooth.println("Starting program...");
          return;
        }

        if (startsWith(buffer, "STOP")) {
          start = false;
          Bluetooth.println("Stopping program...");
          return;
        }

        if (startsWith(buffer, "RESET")) {
          Bluetooth.println("Resetting program...");
          resetArduino();
          return;
        }

        Bluetooth.println("Invalid command");
    }
}

#include "config.h"


void setup() {

  Serial.begin(115200);

  // === CONNECTING TO WIFI ===

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting");
  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  Serial.print("Connected to WiFi. IP:");

  // ==========================

  localIP = WiFi.localIP();
  Serial.println(localIP);
  wifiServer.begin();

  udp.begin(udpPort);

  Serial.print("Listening to UDP on ");
  Serial.println(udpPort);

  sw.enableTransmit(TRANSMIT_PIN);
  sw.setProtocol(1);
}

WiFiClient client;

void loop() {
  //#region [rgba(255,0,0,0.05)]
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    IPAddress hostIP = udp.remoteIP();
    unsigned int hostPort = udp.remotePort();

    Serial.print("New Packet from ");
    Serial.print(hostIP);
    Serial.print(":");
    Serial.print(hostPort);
    Serial.print(" ");
    Serial.print(packetSize);
    Serial.println(" Bytes");

    String message = "";
    for (int i = 0; i < packetSize; i++) {
      char c = udp.read();
      message += c;
    }
    Serial.println(message);
    if (message == "getIoTs")  // send IOT device info on request
    {
      udp.beginPacket(hostIP, hostPort);
      char headerBuf[3] = { 'I', 'O', 'T' };
      char ipBuf[6] = { 'I', 'P', localIP[0], localIP[1], localIP[2], localIP[3] };

      uint8_t nameLen = deviceName.length();
      char nameHeadBuf[5] = { 'N', 'A', 'M', 'E', nameLen };

      char nameBuf[nameLen + 1];
      deviceName.toCharArray(nameBuf, nameLen + 1);

      uint8_t typeLen = deviceType.length();
      char typeHeadBuf[4] = { 'T', 'Y', 'P', typeLen };
      char typeBuf[typeLen + 1];
      deviceType.toCharArray(typeBuf, typeLen + 1);

      udp.write(headerBuf, 3);
      udp.write(ipBuf, 6);
      udp.write(nameHeadBuf, 5);
      udp.write(nameBuf, nameLen + 1);
      udp.write(typeHeadBuf, 4);
      udp.write(typeBuf, typeLen + 1);
      if (udp.endPacket() > 0)
        Serial.println("Sent IOTINFO");
    }
  }
  //#endregion
  //#region [rgba(0,255,128,0.05)]
  client = wifiServer.accept();
  if (client) {
    Serial.println("====== New Client ======");
    Serial.println(client.localIP());
    sendHelpMsg();

    unsigned long previousMillis = 0;
    const long interval = 5000;

    while (client.connected()) {
      // * Sends !heartbeat! every 5s
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        client.println("!heartbeat!");
        Serial.println("!heartbeat!");
      }
      // * Handle Incoming message
      if (client.available() > 0) {
        const int bufferSize = 128;  // Adjust as needed
        char commandBuffer[bufferSize];
        int idx = 0;

        // Read client message into the command buffer
        while (client.available() > 0 && idx < bufferSize - 1) {
          commandBuffer[idx] = client.read();
          idx++;
        }
        commandBuffer[idx] = '\0';  // Null-terminate the string

        Serial.printf("Received New Command: \"%s\"\n", commandBuffer);

        for (int i = 0; i < idx; i++) {
          if (commandBuffer[i] == '\n') commandBuffer[i] = 'n';
          if (commandBuffer[i] == '\t') commandBuffer[i] = 't';
          if (commandBuffer[i] == '\r') commandBuffer[i] = 'r';
        }

        Serial.printf("Received New Command: \"%s\"\n", commandBuffer);

        delay(10);

        char* startChar = strchr(commandBuffer, '<');
        char* endChar = strchr(commandBuffer, '>');

        if (idx < 3 || !startChar || !endChar) {
          Serial.printf("\"%s\" is not a valid command. Use \"<cmd,a1,a2,...>\" as a command format.\n", commandBuffer);
          client.printf("\"%s\" is not a valid command. Use \"<cmd,a1,a2,...>\" as a command format.\n", commandBuffer);
          continue;  // don't close connection
        }

        char* command = startChar + 1;
        *endChar = '\0';  // Split the string

        const int maxArgs = 10;  // Adjust as needed
        char* arguments[maxArgs];
        int argCount = 0;

        char* token = strtok(command, ",");
        while (token && argCount < maxArgs) {
          arguments[argCount] = token;
          argCount++;
          token = strtok(NULL, ",");
        }

        if (strcmp(arguments[0], "getstate") == 0) {
          String m = "{\"iotstate\":{\"switchCount\":" + String(switchCount) + "}}";
          client.flush();
          client.println(m);
          client.flush();

          Serial.println(m);
          Serial.flush();

        } else if (strcmp(arguments[0], "help") == 0) {
          sendHelpMsg();
        }

        else if (strcmp(arguments[0], "switch_on") == 0 && argCount == 2) {
          int receiver = atoi(arguments[1]);
          turnOn(receiver);

          Serial.printf("Switched on receiver %d\n", receiver);
          client.printf("Switched on receiver %d\n", receiver);
        }

        else if (strcmp(arguments[0], "switch_off") == 0 && argCount == 2) {
          int receiver = atoi(arguments[1]);
          turnOff(receiver);

          Serial.printf("Switched off receiver %d\n", receiver);
          client.printf("Switched off receiver %d\n", receiver);
        }

        else {
          Serial.printf("\"%s\" is an unknown command.\n", commandBuffer);
          client.printf("\"%s\" is an unknown command.\n", commandBuffer);
        }
      }
    }
    Serial.println("====== Client Disconnect ======");
  }
  //#endregion
}


void sendHelpMsg() {
  Serial.printf("<switch_on,receiver>\r\n\tturns the receiver on\r\n");
  client.printf("<switch_on,receiver>\r\n\tturns the receiver on\r\n");

  Serial.printf("<switch_off,receiver>\r\n\tturns the receiver off\r\n");
  client.printf("<switch_off,receiver>\r\n\tturns the receiver off\r\n");
}

void turnOn(int receiver) {
  switch (receiver) {
  case 1:
    sw.send(1381719, 24);
    break;
  case 2:
    sw.send(1394007, 24);
    break;
  case 3:
    sw.send(1397079, 24);
    break;
  }
}

void turnOff(int receiver) {
  switch (receiver) {
  case 1:
    sw.send(1381716, 24);
    break;
  case 2:
    sw.send(1394004, 24);
    break;
  case 3:
    sw.send(1397076, 24);
    break;
  }
}

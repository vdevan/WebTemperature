//Check the stored networks and scanned networks. try to connect all available network
//if connected storageIndex is properly set.
void connectWifi()
{
  //Serial.println("Connecting as wifi client...");
  int n = WiFi.scanNetworks();
  storageIndex = -1;
  int j;
  //Serial.printf("Total Scanned network: %d\n", n);
  //Check if scannednetworks is available in our storage
  for (int i = 0; i < storedNetworks; i++)
  {
    for (j = 0; j < n; j++)
    {
      Serial.printf("Now checking the SSID %s at storage index %d with Scanned Network: %s\n",
            Networks[i].SSID, i, WiFi.SSID(j).c_str());

      if (WiFi.SSID(j).compareTo(String(Networks[i].SSID)) == 0)
      {
          storageIndex = i;
          break;
      }
    }
    if (j < n)  //storageIndex >= 0
    {
      if (tryConnect())
          break;
    }
  }

  if (bConnect)
  {          
    Serial.printf("Connected to Network and Storage Index: %d\n", storageIndex);
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Connected to Network on Storage Index: " + String(storageIndex) + " Network Name: " + Networks[storageIndex].SSID + "\r\n";
  }
  else
  {
    Serial.println("Not Connected to Network");
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Unable to connect to Local Area Network\r\n";
  }

  return;
}

//First 
bool tryConnect()
{
  bConnect = false;
  Serial.printf("Trying to connect with SSID: %s, Password: %s\n", Networks[storageIndex].SSID, Networks[storageIndex].Password);

  WiFi.begin(Networks[storageIndex].SSID, Networks[storageIndex].Password);

  int status = WiFi.status();
  int startTime = millis();
  while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED && (millis() - startTime) <= WIFI_TIMEOUT * 1000)
  {
      delay(WHILE_LOOP_DELAY);
      status = WiFi.status();
  }

  if (WiFi.status() == WL_CONNECTED)
      bConnect = true;

  Serial.printf("StoredIndex: %d; Connection status of SSID: %s is %d\n", storageIndex, Networks[storageIndex].SSID, WiFi.status());
  return bConnect;
}

//Server start when connection fails
void startServer()
{
  server.begin(); // Web server start
  Serial.println("HTTP server started");

  // Setup MDNS responder
  if (!MDNS.begin(HOSTNAME))
      Serial.println("Error setting up MDNS responder!");
  else
  {
      Serial.println("mDNS responder started");
      MDNS.addService("http", "tcp", 80);
  }
  if (!bConnect)
    rd = millis();
}

static void GetLocation()
{
  Serial.printf("Getting Location URL: %s, Key= %s\n", LocationUri.c_str() , LocationToken.c_str());
  logFile += ClkTZ.dateTime("H:i:s~ ")  + "Getting Timezone from URL with Token: " + LocationUri + LocationToken + "\r\n" ;
  int count = 0;
  String jsonArray;
  JSONVar myObj;
  String tz;
  while (count < 5)
  {
    count++;
    jsonArray = GETRequest(LocationUri + LocationToken);
    myObj = JSON.parse(jsonArray);
    if (JSON.typeof(myObj) == "undefined" || jsonArray == "")
    {
        delay(SECONDS); //Wait for 1 second and try again
        continue;
    }

    Serial.printf("ip: %s\n", JSON.stringify(myObj["ip"]).c_str());

    if (JSON.stringify(myObj["ip"]).c_str() == "null")
    {
      Serial.printf("ip null detected: %s\n", JSON.stringify(myObj["ip"]).c_str());
      delay(SECONDS); //Wait for 2 seconds and try again
      continue;
    }
    
    tz = JSON.stringify(myObj["timezone"]);

    if (tz.indexOf('/') < 0)
      tz = "";
    else  
      tz.replace("\"", "");
    break;
  }
  if (count < 5 && tz != "")
  {
    ClkTZ.setLocation(tz);
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Using TimeZone from Web: " + tz + "\r\n";
  }
  else
  {
    ClkTZ.setLocation(TZDefault);
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Using Default TimeZone: " + TZDefault + "\r\n";
  }
}

static String GETRequest(String uri)
{
  HTTPClient http;
  WiFiClient wifiClient;

  //Serial.printf("HTTPClient will use URL:\n %s\n", uri.c_str());
  http.begin(wifiClient, uri.c_str());
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  //Serial.printf("Payload from HTTP Client: %s\n", payload.c_str());
  return payload;
}


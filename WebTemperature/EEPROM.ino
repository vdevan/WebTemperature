/************************************************************************* 
* Load WLAN credentials from EEPROM 
* There are 512 byte storage buffer available. We will use 
* this to store up to 6 WiFi configurations 32 byte for SSID
* and 32 byte for password. That will occupy 33*6*2 = 396 bytes in total
* to ensure we have the right data we will store header info which will be
* a Prog ID and Version as UINT16. This will be the first
* header information. Thus we will use 396+4 = 400 bytes
* Last Edited: 31st May 2024 - Vasu
* **************************************************************************/

//If valid data exists in EEPROM read it and return true. else return false
bool loadCredentials()
{
  EEPROM.begin(512);
  int offset = 0;
  EEPROM.get(offset, header);
  
  //Fresh Program. Needs complete Erase
  if (header.id != progId)
  {
    Serial.println("Valid header not found");
    header.id = progId;
    header.ver = version;
    EraseStoredValue();
    return true;
  }
  
  ReadNetworkData();
  //Only version change. Make modifications to Header file if required
  if (header.ver != version)
  {
    header.id = progId;
    header.ver = version;
    saveCredentials();
  } 

  Serial.printf("Program ID: %x\nProgram Version: %x\nStored Network: %x\n",header.id,header.ver,storedNetworks );
  logFile += ClkTZ.dateTime("H:i:s~ ")  + "Program ID: " + String(header.id) + "; Program Version: " + String(header.ver) + 
              "; Stored Networks: " + String(storedNetworks) + "\r\n";
  return true;   
}

void ReadNetworkData()
{
    int offset = NetworkOffset;
    storedNetworks = 0;
    for (int i = 0; i < MAXNETWORK; i++)
    {
      EEPROM.get(offset, Networks[i]);
      offset += sizeof(NETWORK);
      Serial.printf("READ From Storage[%d]: SSID: %s, Password: %s\n", i + 1, Networks[i].SSID, Networks[i].Password);
      if (strlen(Networks[i].SSID) > 0)
        storedNetworks++;
      
    }
    EEPROM.end();
}

void EraseStoredValue()
{
  for (int i = 0; i < MAXNETWORK; i++)
  {
      memset(Networks[i].SSID, '\0', sizeof(Networks[i].SSID));
      memset(Networks[i].Password, '\0', sizeof(Networks[i].Password));
  }

  Serial.println("Erasing the stored contents");
  saveCredentials();
}

/** Store WLAN credentials to EEPROM */
void saveCredentials()
{
  EEPROM.begin(512);
  int offset = 0;
  EEPROM.put(offset, header);
  offset = NetworkOffset;

  for (int i = 0; i < MAXNETWORK; i++)
  {
    EEPROM.put(offset, Networks[i]);
    offset += sizeof(NETWORK);
  }
  EEPROM.commit();

  Serial.println("Saving Network Configuration");

  EEPROM.end();
  delay(5000);
  ESP.restart();

}


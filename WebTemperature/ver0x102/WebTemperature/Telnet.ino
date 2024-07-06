void setupTelnet() 
{  
  telnet.setLineMode(false);
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);
  telnet.onInputReceived(onTelnetInput);

  Serial.print("- Telnet: ");

  if (telnet.begin(TELNETPORT)) 
  {
    Serial.println("running");
    bTelnet = true;
  }
  else 
  {
    Serial.println("error.");
    Serial.println("Telnet not running. reboot may be required...");
    bTelnet = false;
  }
}

void onTelnetDisconnect(String ip)
{
  Serial.print(ip);
  Serial.println(" disconnected.");
}

void onTelnetReconnect(String ip) 
{
  Serial.print(ip);
  Serial.println(" connected.");
}

void onTelnetConnectionAttempt(String ip) 
{
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" tried to connect");
}

void onTelnetInput(String str) 
{
  char r = str.charAt(0);
  telnet.println("");
  String tr;
  switch (r) 
  {
    case '?':
      printMenu();
      break;

    case 'R':
      telnet.println("Rebooting in 5 seconds");
      telnet.disconnectClient();
      delay(SECONDS * 5);
      ESP.restart();
      break;
      
    case 'l':
      if (logFile == "")
        telnet.println("Nothing to Report\r\n");
      else
        telnet.println(logFile.c_str());
      break;

    case 'c':
      if (tempCString == "\0")
        telnet.println("Nothing to Report\r\n");
      else
      {
        tr = ClkTZ.dateTime("h:i:s A") + "," + tempCString + " °C\r\n";
        telnet.println(tr.c_str());
      }
      break;

    case 'f':
      if (tempFString == "\0")
        telnet.println("Nothing to Report\r\n");
      else
      {
        tr = ClkTZ.dateTime("h:i:s A") + "," + tempFString + " °F\r\n";
        telnet.println(tr.c_str());
      }
      break;
    case 'x':
      telnet.println("");
      telnet.println("bye");
      telnet.disconnectClient();
      break;
  }
  return;
  
}

void onTelnetConnect(String ip) 
{
  telnet.print("You are now connected to WEB TEMPERATURE MONITOR\r\n");
  telnet.print("================================================\r\n");
  telnet.print("Welcome - Client: ");
  telnet.print(ip);
  telnet.println(" serving now");
  printMenu();
}

void printMenu()
{
  telnet.println("c.  Print Temperature in Centigrade");
  telnet.println("f.  Print Temperature in Farenheit");
  telnet.println("l.  Print Log File");
  telnet.println("x.  Exit session");
  telnet.println("?.  Print This Menu");
  telnet.println("R.  Restart ESP device");
  telnet.println("===========================");
}

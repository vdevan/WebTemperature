char CaptivePage[] PROGMEM = R"rawliteral(
  <!DOCTYPE html5>
  <html lang="en">
  <head>
    <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
    <script src='https://code.jquery.com/jquery-2.1.1.min.js' type="text/javascript"></script>
    <script>
    window.addEventListener('load', onLoad);
    function onLoad()
    {
        var elements = document.getElementsByClassName("btn");
        for (var i = 0; i < elements.length; i++) {
            elements[i].addEventListener('click', pressed);
        }

        var net = document.getElementsByClassName("n");
        for (var i = 0; i < net.length; i++) {
            net[i].addEventListener('click', setnet);
        }

    }

    function pressed()
    {
      var name = this.name;
      console.log(name + "pressed");
      if (name == "del")
      {
        document.getElementById('ret').innerHTML = "Deleting Network. Please wait";
      }
      if (name == "save")
      {
        document.getElementById('ret').innerHTML = "Saving Information. Please wait";
      }
      if (name == "reset")
      {
        document.getElementById('ret').innerHTML = "Restarting device. Please wait";
      }
      setTimeout(1000);
    }

    function setnet()
    {
      document.getElementById('s').value = this.innerText; $("#p").focus();
    }
    </script>
    <title>WiFi Config</title>  
  )rawliteral";

char Stub[] PROGMEM = R"rawliteral(
  </head>
  <body>
    <div class="container">
      <h2>WiFi Settings</h2>
      <p>You are connected to the device: <b>
)rawliteral";

char HTTP_SUBMIT[] PROGMEM = R"literal(
<div style="text-align:center"><p><span>
<button id = "btns" class = "btn" name = "save" type = "Submit">Save and Restart</button>&nbsp
<button id = "btnr" class = "btn" name = "reset" type = "submit">Restart without Save</button>
</span></p></div>
)literal";

//Root - Portal
void handleCaptive()
{
  Serial.println("Preparing index page");
  String Page = "";
  String Page1 = "";
  Page += ESPSSID + "</b><br /><span id=\"retVal\"></span></p>"; //Add ESPSSID
  Page += ("<h4>Available Networks</h4><div class=\"scnnet\">");   //Addition - id changed to class in div

  int n = WiFi.scanNetworks();
  //Now add the Scanned Networks
  if (n > 0)
  {
      for (int i = 0; i < n; i++)
      {   
        Page += String(F("<div class=\"inf\"><a href=\"#p\" class=\"n\">")) + WiFi.SSID(i) + F("</a>&nbsp;<span class=\"q") + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F("\">") : F(" l\">")) + abs(WiFi.RSSI(i)) + " %";
        Page += F("</span></div>");
      }
  }
  else
      Page += F("<div class=\"inf\"><p>No WLAN found</p>");

  Page += F("</div>"); //Closing Div for wifi Networks info

  //Start of Form and SSID, Password fields
  Page += F("<form method=\"POST\" action=\"wifisave\"><table><tr>");
  if (storedNetworks >= MAXNETWORK)
  {
    Page += F("<td colspan=\"2\"><h4 style=\"color:red;\">Please DELETE a Network to store new one</h4><td>");
    Page += F("</tr></table>");
  }
  else
  {
    Page += F("<td><span><label>SSID: </label><br /><input id=\"s\" class=\"txt\" name=\"s\" length=32 placeholder=\"SSID\"></span></td>"
              "<td><span><label>Password: </label><br /><input id=\"p\" class=\"txt\" name=\"p\" length=32 placeholder=\"password\"></span></td>");
    Page += F("</tr></table>");
  }

  Page1 += F("<h4>Stored Networks</h4><div class=\"strnet\"><table>"); //Addition ID changed to class in div

  //Stored Network contents 
  for (int i = 0; i < storedNetworks; i++)
  {
      Page1 += F("<tr><td class=\"stbtn\"><label>");
      Page1 += Networks[i].SSID;
      Page1 += F("</label></td><td class=\"stbtn\"><button id=\"btnd\" class=\"btn\" name=\"del\" value=\"");
      Page1 += Networks[i].SSID;
      Page1 += F("\" type=\"Submit\">Delete</button></td></tr>");
  }
  Page1 += F("</table></div></form>");
  
  Page1 += F("<div style=\"text-align:center\"><p><span>"); //Addition: Div and buttons within <p><span> also class changed to btn
  Page1 += F("<button type=\"submit\" class=\"btn\" onclick = \"location.href = \'/update\'\" name=\"update\" style=\"margin-left:15px;\">Update Program</button>");

  if (bConnect)
        Page1 += F("<button type=\"submit\" class=\"btn\" onclick = \"location.href = \'/WebPage\'\" name=\"LED\" style=\"margin-left:15px;\">Temperature Monitor</button>");
  Page1 += F("</span></p></div>"); // Addition: End of </span></p> </div>
  Page1 += F("<p>Note: <span id=\"ret\" style=\"color:red; font-weight:800\" ></span></p><p>&nbsp;</p>"); 
  Page1 += F("</div></body></html>");

  server.send(200, "text/html", String(CaptivePage) + String(Style) + String(Stub) + Page + String(HTTP_SUBMIT) + Page1); 
  logFile += ClkTZ.dateTime("H:i:s~ ")  + "Wifi Page (Captive) service Started\r\n"; 
  bCaptive = true;

}


void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  replyBadRequest(message);
}

//WiFi Save
void handleWifiSave()
{
  Serial.printf("Server Argument 2: %s\n",server.argName(2).c_str());
  String st;
  int offset = 0;
  if (server.hasArg("reset"))
  {
    sendResponse();
    ESP.restart();
    //ESP.reset(); //header needs to be saved
    return;
  }
 
  if (server.hasArg("del")) //handle delete
  {
      st = server.arg("del");
      st.trim();
      Serial.printf("Delete Arg: %s\n",st.c_str());
      int j;
      for (j=0; j < storedNetworks; j++)
      {
          if (st == Networks[j].SSID)
            break;
      }
      Serial.printf("Index to delete found: %d\n",j);
      // J has the index for deleting. Let us zero it first
      memset(Networks[j].SSID, '\0', sizeof(Networks[j].SSID));
      memset(Networks[j].Password, '\0', sizeof(Networks[j].Password));
      
      //Rearrange storage
      for (int i = j; i < MAXNETWORK -1; i++)
      {
          memcpy(Networks[i].SSID, Networks[i+1].SSID, sizeof(Networks[i+1].SSID));
          memcpy(Networks[i].Password, Networks[i+1].Password, sizeof(Networks[i+1].Password));
      }
      //Erase the last storage
      memset(Networks[MAXNETWORK -1].SSID, '\0', sizeof(Networks[MAXNETWORK -1].SSID));
      memset(Networks[MAXNETWORK -1].Password, '\0', sizeof(Networks[MAXNETWORK -1].Password));
      
    }
    else
    {
      st = server.arg("s");
      st.trim();
      if (st != "")
      {
        //handle network storage
        //Pick the first empty storage
        for (int i = 0; i < MAXNETWORK; i++)
        {
          if (Networks[i].SSID[0] == 0)
          {
              offset = i;
              break;
          }
        }
        server.arg("s").toCharArray(Networks[offset].SSID, sizeof(Networks[offset].SSID));
        server.arg("p").toCharArray(Networks[offset].Password, sizeof(Networks[offset].Password));
        Serial.printf("SSID: %s  Password: %s\n",server.arg("s").c_str(),server.arg("p").c_str());
        Serial.printf("Stored at Location %d\n",offset);

      }
  }
  sendResponse();

  //Serial.printf("storage offset to be stored: %d\n Original stored SSID length: %d\n", offset, strlen(Networks[offset].SSID));
  //storeEpoch(server.arg("save"));

  saveCredentials();    
}

void sendResponse()
{
  server.sendHeader("Location", "/", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    
  server.client().stop(); // Stop is needed because we sent no content length
    
}
void replyBadRequest(String msg) 
{
  telnet.printf("Bad Request: %s\r\n",msg.c_str());
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyOK() 
{
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) 
{
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) 
{
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyServerError(String msg) 
{
  telnet.printf("Server Error Encountered: %s\r\n",msg.c_str());
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

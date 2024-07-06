char TempPage[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js' type="text/javascript" charset="utf-8"></script>
  <script>
  var websock;
  var temprString;
  var bCentigrade = getStoredValue('measure') == 'true' ? true : false;

  function storeValue(key, value) 
  {
    var val = value ? 'true' : 'false';
    if (localStorage)
      localStorage.setItem(key, val);
    else 
      $.cookies.set(key, val);
  }

  function getStoredValue(key) 
  {
    var val;
    if (localStorage) 
      val =  localStorage.getItem(key); 
    else 
      val =  $.cookies.get(key);
    return val == null ? 'true' : val;  
  }

  function start()
  {
    websock = new WebSocket('ws://' + window.location.hostname + ':81/');
    websock.onopen = function(evt) { console.log('websock open'); };
    websock.onerror = function(evt) { console.log(evt); };
    websock.onmessage = function(evt)
    {
    let state = evt.data;
    var temp;
    var txtarea = document.getElementById('temp');
    var listdir = document.getElementById('ld');
    console.log ('bCentigrade value: ' + bCentigrade? 'true' : 'false');
    switch (state[0]) 
    {
      case '0':
        document.getElementById('date').innerHTML = state.slice(1);
        break;
      case '1':
        document.getElementById('ret').innerHTML =  state.slice(1);
        break;
      case '3': //start of Temperature Stream
        temprString = state.substr(1);
        break;
      case '4': //continuous stream
        temprString += state.substr(1);
        break;
      case '5': //End of Stream
        temprString += state.substr(1);
        temprList();
        break;  
      case 'c':
        document.getElementById('ret').innerHTML = "Centigrade Selected";
        bCentigrade = true;
        storeValue('measure', bCentigrade);
        break;
      case 'f':
        document.getElementById('ret').innerHTML = "Farenheit Selected";
        bCentigrade = false;
        storeValue('measure', bCentigrade);
        break;
      case 'z':
        document.getElementById('ret').innerHTML = state.slice(1);
        break;
      case 'e':
        document.getElementById('btd').disabled = false;
        document.getElementById('lf').disabled = false;
        document.getElementById('fl').disabled = false;
        document.getElementById('ci').disabled = false;
        break;
      case 'd':
        document.getElementById('btd').disabled = true;
        document.getElementById('lf').disabled = true;
        document.getElementById('fl').disabled = true;
        document.getElementById('ci').disabled = true;
        break;  
      case 't':
        console.log("Temperature Message Received: " + state);
        temprString = state.substr(1);
        temprList();
        break;
      case 'x':
        state = state.substr(1);
        console.log("Broadcast Received: " + state);
        var temp = state.split(',');
        if (bCentigrade) {
          txtarea.innerHTML += temp[0] + ": " + temp[1] + temp[2] + "\n";
          document.getElementById('ret').innerHTML = "Measurement in Centigrade";
        }
        else {
          txtarea.innerHTML += temp[0] + ": " + temp[3] + temp[4];
          document.getElementById('ret').innerHTML = "Measurement in Farenheit";
        }
        txtarea.scrollTop = txtarea.scrollHeight;
        break; 
      case 'l':
        if (state.length > 3)
        {
          state = state.slice(1);
          var index = state.indexOf("#%&");
          var d = state.substr(0,index);
          var f = state.substr(index+3);
          d = d.split('\n');
          f = f.split('\n');
          var dirbox;
          var filbox;
          for (i = 0; i < d.length; i++) {
            if (d[i] != "")
              dirbox += '<option value="' + d[i] + '">' + d[i] + '</option>';
          }
          document.getElementById('lb').innerHTML = dirbox;
          for (i = 0; i < f.length; i++) {
            if (f[i] != "")
              filbox += '<option value="' + f[i] + '">' + f[i] + '</option>';
          }
          document.getElementById('fl').innerHTML = filbox;
          document.getElementById('lf').value = "";
          document.getElementById('fo').value = '0';
        }
        break; 
      case 's':
        document.getElementById('ret').innerHTML = state.slice(1);
        break;
      case 'i':
        if (state.length > 3)
        {
          document.getElementById('ci').innerHTML = state.slice(1);
        }
        break;
      default:
        document.getElementById('ret').innerHTML = evt.data;
        console.log(evt.data)
      }
    }
  }

  window.addEventListener('load', onLoad);
  function onLoad(event) 
  {
    start();
    initButton();
    document.getElementById('fo').value = '0';
    document.getElementById("lf").value = "";
  }

  function temprList() 
  {
    temprString = temprString.split('\n');
    var txtarea = document.getElementById('temp');
    txtarea.innerHTML = "";
    for (i = 0; i < temprString.length; i++)
    {
      var temp = temprString[i];
      temp = temp.split(',');
      if (temp.length > 2) 
      {
        if (bCentigrade) 
        {
          txtarea.innerHTML += temp[0] + ": " + temp[1] + temp[2] + "\n";
          document.getElementById('ret').innerHTML = "Measurement in Centigrade";
        }
        else 
        {
          txtarea.innerHTML += temp[0] + ": " + temp[3] + temp[4] + "\n";
          document.getElementById('ret').innerHTML = "Measurement in Farenheit";
        }
      }
    }
    txtarea.scrollTop = txtarea.scrollHeight;
  }

  function wait(ms)
  {
    var start = new Date().getTime();
    var end = start;
    while(end < start + ms) 
    {
      end = new Date().getTime();
    }
  }

  function validate()
  {
    var selection = document.getElementById('fo').value;
    console.log("Selected Value: " + selection);
    var txt =  document.getElementById('lf').value;
    txt.trim();
    if ((selection == '0' || txt == "") || (selection == '8' && txt == ""))
    {
      alert("Nothing to Execute");
      event.preventDefault();
      return false;
    }
          
    if (selection == '1') //Download can go through
      return true;
    else
    {
      var x = document.getElementById('lb');
      var fld = x.options[x.selectedIndex].text;
      console.log("Folder: " + fld);
      x = document.getElementById('fl');
      var file = ""; 
      if (x.options[x.selectedIndex])
      {
        file = x.options[x.selectedIndex].text;
        file = file.substring(0,file.lastIndexOf(" ... "));
      }

      console.log("file: " + file);
      var retval = true;
      switch (selection)
      {       
        case '4':
          if (file = "")
          {
            alert ("Please select a file from list for Rename. Uable to continue!");
            retval = false;
          }
          break;  

        case '6':
          if (txt == fld  + file)
          {
            alert ("Source File and Destination file are same for move. Uable to continue!");
            retval = false;
          }
          break; 

        case '7':
          retval = confirm("Do you want to delete " + txt +  " file?");
          break;
        case '8':   
          retval = confirm("Do you want to delete " + txt +  " Directory? All subfolders will be deleted!");
          break;   
      }
      if (!retval)
      {
        document.getElementById('ret').innerHTML = "Cannot process File Operation!";
        event.preventDefault();
      }
      return retval;
    }
 
  }
      
  function initButton() 
  {
    document.getElementById('bton').addEventListener('click',pressed);
    document.getElementById('btof').addEventListener('click',pressed);
    document.getElementById("lb").addEventListener('click', lbchange);
    document.getElementById("fl").addEventListener('dblclick', download);
    document.getElementById("fl").addEventListener('click', sel);
  }

  function sel() 
  {
    console.log("Double Click Event detected");
    var e = document.getElementById('fl');
    var x = document.getElementById('lb');
    var txt = "";
    if (e.options[e.selectedIndex])
    {
      txt = e.options[e.selectedIndex].text;
      txt = txt.substring(0,txt.lastIndexOf(" ... "));
    }
    var fld = x.options[x.selectedIndex].text;
    document.getElementById('log').innerHTML = fld + txt + " selected";
    document.getElementById('lf').value = fld + txt;
    document.getElementById('fo').value = '0';
  }

  function download() 
  {
    console.log("Double Click Event detected");
    document.getElementById('fo').value = '1';
    document.getElementById('btd').click();
  }

  function lbchange() 
  {
    console.log("Onchange Event detected");
    var e = document.getElementById('lb');
    var txt = "";
    if (e.options[e.selectedIndex]) 
    {
      txt = e.options[e.selectedIndex].text;
      document.getElementById('log').innerHTML = txt;
      console.log(txt);
      websock.send("L_" + txt);
    }
  }

  function fname(obj) 
  {
    console.log("Selected File: " + obj.value);
    var file = obj.value;
    var fileName = file.split("\\");
    console.log("File Name: " + fileName[2]);
    $('#ufl').html(fileName[2]);
  }

  function pressed()
  {
    var name = this.name;
    if (name == "ON")
    {
      document.getElementById('log').innerHTML = "Centigrade Requested";
      bCentigrade = true;
      websock.send("C_pressed");
    }
    else
    {
      document.getElementById('log').innerHTML = "Farenheit Requested"; 
      bCentigrade = false;
      websock.send("F_pressed");
    }
  }
    
</script>
  <meta charset="utf-8">
  <title>Temperature Monitor</title>
  )rawliteral";

char TempStub[] PROGMEM = R"rawliteral(
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="shortcut icon" href='data:image/x-icon;base64,Qk2+AAAAAAAAAD4AAAAoAAAAEQAAACAAAAABAAEAAAAAAIAAAADBDgAAwQ4AAAAAAAAAAAAAAAAAAP///wDw/4AA33+AALG/gADu34AAX1+AAF9fgABfX4AA7l+AALS/gADZf4AA6v+AAOp/gADqf4AA6v+AAOp/gADq/4AA6j+AAOr/gADq/4AA6n+AAOj/gADqP4AA6v+AAOp/gADqf4AA7v+AAO4/gADu8IAA8fmAAP/wgAD//4AA//+AAA=='/>
</head>
<body>
<div class="container">
  <h1>Temperature Monitor</h1>
  <div id="logtxt">
    <p>Current Date: <span id="date" class="attn"></span></p>
    <p>Last Action: <span id="log"></span></p>
  </div>
  <table style="width: 65%" ><tr>
    <td><button id="bton" class="btn" name="ON">Centigrade</button></td>
    <td><button id="btof" class="btn" name="OFF">Farenheit</button></td>
    <td><button id="btin" class="btn" onclick="location.href = '/update'" name="OTA Update">Update</button></td>
    <td><button id="btca" class="btn" onclick="location.href = '/captive'" name="Captive">WiFi Page</button></td>
  </tr></table>
  <p>Server Returned: <span id="ret" class="attn">Sample Text</span></p>
  <div class="scnnet">
<form method="POST" onsubmit="return validate()" action="exec" id="fop" target="_self">
  <!-- <form onsubmit="return process()" id="fop" >-->
    <table>
      <tr>
        <td width="45%"><label label for="tempr">Temperature log for today:</label></td>
        <td>  Folders / Files View: </td>
      </tr>
      <tr>
        <td rowspan="2"><textarea readonly class="ti" style="height:220px;" name="tempr" id="temp"></textarea></td>
        <td colspan="2"><select name="folders" style="max-width:95%;width:95%;padding:5px;" id="lb"></select></td>
      </tr>
      <tr>
        <td colspan="2"><select id="fl" class="ti" name="files" style="overflow-x:auto;" size="12"></select></td>
      </tr>
      <tr>
      <td><select name="fileop" style="max-width:95%;width:95%;padding:5px;" id="fo">
        <option value="0">File Operations</option>
        <option value="1">Download File: </option>
        <option value="2">Create File: </option>
        <option value="3">Create Directory: </option>
        <option value="4">Rename File to: </option>
        <option value="5">Rename Directory to: </option>
        <option value="6">Move File / Directory to: </option>
        <option value="7">Delete File</option>
        <option value="8">Delete Directory</option>
      </select></td>
      <td><input id="lf" class="txt" type="text" style="width:95%" name="fname" length=128 placeholder="/"></td>
      <td width="15%"><button id="btd" type="submit" style="margin-bottom:5px;" class="btn" name="exec">Execute</button></td>
    </tr>
    </table></form>
    <form method="post" action="upload" enctype="multipart/form-data">
    <table>
    <tr>
    <td width="80%"><label class="btn" for ="upl" >File Upload</label>
      <input id="upl" type="file" name="name"  onchange="fname(this)" /><span id="ufl">No file Selected</span></td>
      <td width="15%"><input class="btn" type="submit" value="Upload"></td>
      <!--  <td width="80%"><input type="file" name="name"></td>
      <td width="15%"><input class="button" type="submit" value="Upload"> </td>-->
    </tr>
    <tr>
    <td colspan="3"><div contenteditable="false" name="cardinfo" id="ci"></div></td>
  </tr>
  </table>
  </form>
</div>
</div>
</body>
</html>
)rawliteral";

//This is the index page. /update will automatically bring OTA page
void handleWeb()
{
  server.send(200, "text/html", String(TempPage) + String(Style) + String(TempStub));
  logFile += ClkTZ.dateTime("H:i:s~ ")  + "Server service Started\r\n"; 
  Serial.println("Index Page provided");
  bCaptive = false;
  return;
}

void handlePost()
{
  IPAddress ip = server.client().remoteIP();
  String fileName = server.arg("fname");
  String folder = server.arg("folders");
  String srcFile = folder + server.arg("files");
  srcFile = srcFile.substring(0, srcFile.lastIndexOf(" ... "));
  char ops = server.arg("fileop").charAt(0);
  logFile += ClkTZ.dateTime("H:i:s~ ") + "Message from: " + ip.toString();
  bool retval;
  int count = 1;
  switch (ops)
  {
    case '1':
      logFile += ": Download of file: " + fileName + " requested\r\n";
      retval = SendFile(fileName); 
      break;

    case '2':
      logFile += ": Creation of file: " + folder + fileName + " requested\r\n";
      telnet.printf("creating of File at: %s\r\n",(folder + fileName).c_str());
      retval = createFile((folder + fileName).c_str());
      break;

    case '3':
      logFile += ": Creation of Directory: " + folder + fileName + " requested\r\n";
      telnet.printf("creating of File at: %s\r\n",(folder + fileName).c_str());
      retval = createDir((folder + fileName).c_str());
      break;

    case '4':
      logFile += ": Renaming of file from " + srcFile + " to " + fileName + " requested\r\n";
      retval = renameFile(srcFile.c_str(),fileName.c_str());
      break;

    case '5':
      logFile += ": Renaming of directory from " + folder + " to " + fileName + " requested\r\n";
      retval = renameFile(folder.c_str(),fileName.c_str());
      break;

    case '6':
      logFile += ": Moving of file from " + srcFile + " to " + fileName + " requested\r\n";
      telnet.printf("Moving of File from: %s to %s\r\n",srcFile.c_str(),fileName.c_str());
      retval = renameFile(srcFile.c_str(),fileName.c_str());
      break;

    case '7':
      logFile += ": Deletion of file " + fileName + " requested\r\n";
      retval = deleteFile(fileName.c_str());
      break;

    case '8':
      logFile += ": Deletion of directory " + fileName + " requested\r\n";

      if (fileName.lastIndexOf('/') != fileName.length()-1)
        fileName += '/';
      if (fileName.charAt(0) != '/')
        fileName = "/" + fileName;  
      telnet.printf("Directory Requested to be deleted is: %s\r\n",fileName.c_str());  
      delDir = fileName + "\n";
      listDir(fileName.c_str(),5,true) + "\n";

      logFile += "Del directory Reported: " + delDir + "\r\n";

      while (count > 0)
      {
        count = delDir.lastIndexOf('\n');
        fileName = delDir.substring(count+1);
        delDir = delDir.substring(0,count);
        if (fileName != "")
        {
          if (fileName.charAt(0) != '/')
            fileName = "/" + fileName;
          telnet.printf("Current Directory to be deleted: %s\r\n",fileName.c_str());
          SD.rmdir(fileName.c_str());
        }

      }  
      break;  
    default:
      logFile += "Unable to decode the message\r\n";
  }
  sendResponse();    
  return;
}
  
bool SendFile(String fileName)
{
  String hfName = "filename=" + fileName.substring(fileName.lastIndexOf("/") +1);

  File file = SD.open(fileName,FILE_READ);
  if (file.isDirectory())
  {
    return false;
  }
  static uint8_t buf[512];
  size_t len = 0;

  if(file)
  {
    String htmlFormat;
    String ext = fileName.substring(fileName.lastIndexOf('.')+ 1);
    ext.toLowerCase();
    len = file.size();
    Serial.printf("Ext found: %s; Size of Requested File: %d\n",ext.c_str(), len);
     
    if (ext == "html" || ext == "htm" ) htmlFormat = "text/html; charset:utf-8";
    else if (ext == "wtm" || ext == "log" || ext == "txt") htmlFormat = "text/plain; charset:utf-8";
    else if (ext == "jpg" || ext == "jpeg") htmlFormat = "image/jpeg";
    else if (ext == "png") htmlFormat = "image/png";
    else if (ext == "js") htmlFormat = "text/javascript; charset:utf-8";
    else if (ext == "pdf") htmlFormat = "application/pdf";
    else if (ext == "bmp") htmlFormat = "image/bmp";
    else if (ext == "zip") htmlFormat = "application/zip";
    else if (ext == "docx" || ext=="doc") htmlFormat = "application/msword";
    else if (ext == "xlsx" || ext=="xls" || ext=="csv") htmlFormat = "application/vnd.ms-excel";
    else 
    {
      hfName="attachment;" + hfName;
      logFile += ClkTZ.dateTime("H:i:s~ ")  + "Header info for FileName: " + hfName + "\r\n";
      logFile += ClkTZ.dateTime("H:i:s~ ")  + "Sending file for Download in Octet-Stream\r\n";
      htmlFormat = "application/octet-stream";
      server.sendHeader("Content-Disposition", hfName);
      server.streamFile(file, htmlFormat);
      return true;
    }

    hfName="inline;" + hfName;
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Header info for FileName: " + hfName + "\r\n";
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Sending file for Download in Text-Plain \r\n";
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.sendHeader("Content-Length", (String)len);
    server.sendHeader("Content-Disposition", hfName);
    server.send(200, htmlFormat, "");
    
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "File Size from file: " + String(len) + "\r\n";
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "HTML Format to be used: " + htmlFormat + "; EXT used: " + ext + "\r\n";
    Serial.printf("htmlFormat that will be used:%s\n",htmlFormat.c_str());
    size_t toRead;
    while(len >0)
    {
      toRead = len;
      if (toRead > 512)
        toRead = 512;
        
      file.read(buf, toRead);
      server.sendContent_P((char*)buf, toRead);
      len -= toRead;  
    }  
    server.client().stop();
    file.close();

  }
  if (logFile.length() >= 2048)
  {
    if (bCard)
    {
      String fn = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".log";
      writeFile(fn.c_str(),logFile.c_str());
    }
    logFile = String();
  }
  sendResponse();
  return true;
}


void handleWebSocketMessage(uint8_t num, uint8_t *data, size_t len) 
{
  IPAddress ip = ws.remoteIP(num);
  String retval;
  data[len] = 0; //terminate the string
  String sdat = (char*)data;
  int offset = sdat.indexOf('_'); 
  char dat;
  String file;
  int cr;
  bool bPost;
  if (offset >= 0)
  {
    logFile += ClkTZ.dateTime("H:i:s~ ") + "Message from: " + ip.toString();
    dat = sdat[0];    
    sdat = sdat.substring(offset+1);

    switch (dat)
    {
      case 'C': //Centigrade requested
        logFile += " Centigrade button Clicked\r\n";
        retval = "c";
        ws.sendTXT (num,retval.c_str(),retval.length());
        SendTemperatureFromFile(num);
        break;

      case 'F':     //Farenheit requested
        logFile += " Farenheit button Clicked\r\n";
        retval ="f";
        ws.sendTXT (num,retval.c_str(),retval.length());
        SendTemperatureFromFile(num);
        break;

      case 'L': //LB changed
        logFile += " Folder Change detected\r\n";
        if (sdat == "../")
          sdat = curDir.substring(0,curDir.lastIndexOf('/')+1);
        if (sdat == curDir)
          retval = "zOperation completed Successfully";
        else
        {
          retval = "l" + listDir(sdat.c_str(),0);
          curDir = sdat.substring(0,sdat.length()-1);
        }  
        ws.sendTXT (num,retval.c_str(),retval.length());  
        break;

      default:
        logFile += " Unable to decode the message\r\n";  
        retval += "z"+sdat;
        ws.sendTXT (num,retval.c_str(),retval.length());  
    }

    if (logFile.length() >= 2048)
    {
      if (bCard)
      {
        String fn = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".log";
        writeFile(fn.c_str(),logFile.c_str());
      }
      logFile = String();
    }
  }
  
}


//handle all events here connect Except for WS_EVT_DATA rest are house keeping.
//num is the IPAddress, type Typeof Event, Payload and length of payload. Will need a terminate '0' to payload
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{
  
  IPAddress ip = ws.remoteIP(num);
  String tr;
  String dt;
  switch (type) 
  {
    case WStype_CONNECTED:
      logFile += ClkTZ.dateTime("H:i:s~ ")  + "Total Connected Clients: " + String(ws.connectedClients()) + "\r\n";
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
     
      dt= "0" + ClkTZ.dateTime("l, jS ~o~f F Y");
      ws.sendTXT(num,dt.c_str(), dt.length()); //Send Date
      ws.sendTXT(num,bCard? "e":"d",1);         //Send SD Card status
      
      tr = SDInit(false);
      ws.sendTXT (num,tr.c_str(),tr.length());  //Send Card Info  
      
      SendTemperatureFromFile(num); 
      
      if (bCard)
      {
        curDir = "/";
        tr = "l" + listDir(curDir.c_str(),0);
        ws.sendTXT (num,tr.c_str(),tr.length());  //Send Folder and File Info
        delay(500); //Wait for further processing
      }
      break;     

    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;

    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      handleWebSocketMessage(num, payload,length); //call the event data to handle message
      break;

    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      #ifdef ESP8266
        hexdump(payload, length);
      #endif  
      ws.sendBIN(num, payload, length);//send the message back to client
      break;

    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }

  if (logFile.length() >= 2048)
  {
    if (bCard)
    {
      String fn = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".log";
      writeFile(fn.c_str(),logFile.c_str());
    }
    logFile = String();
  }
}

void handleFileUpload()
{ 
  if (!bCard) 
  { 
    return replyServerError(FPSTR(FS_INIT_ERROR)); 
  }
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START)
  { 
    String fn = upload.filename;
    if (curDir == "/")
      fn = curDir + fn;
    else
      fn = curDir + "/" + fn;

    upLoadFile = SD.open(fn, FILE_WRITE); 

    if (!upLoadFile) 
    { 
      return replyServerError(F("CREATE FAILED")); 
    }
    fn=String();
  } 
  else
  { 
    if (upload.status == UPLOAD_FILE_WRITE) 
    {
      upLoadFile.write((char*)upload.buf,upload.currentSize);
    }
    else
    { 
      if (upload.status == UPLOAD_FILE_END) 
      {
        if (upLoadFile) 
        { 
          upLoadFile.flush();
          upLoadFile.close(); 
        }
        sendResponse();
      }
    }
  }
}


 

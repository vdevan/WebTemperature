/*************************
* Following functions are copied from SD example file SD_Test.ino 
* Rework for return value and also using LED light to indicate
* presence of a SD MEM card and a blinking light to indicate SD MEM
* is being accessed must be implemented
* Following are the prototype for these methods
*    listDir(SD, "/", 0);
*    createDir(SD, "/mydir");
*    listDir(SD, "/", 0);
*    removeDir(SD, "/mydir");
*    listDir(SD, "/", 2);
*    writeFile(SD, "/hello.txt", "Hello ");
*    appendFile(SD, "/hello.txt", "World!\n");
*    readFile(SD, "/hello.txt");
*    deleteFile(SD, "/foo.txt");
*    renameFile(SD, "/hello.txt", "/foo.txt");
*    readFile(SD, "/foo.txt");
*    testFileIO(SD, "/test.txt");
*    Dated: 27th April 2024 - Vasu
******************************************************/

String SDInit(bool bInitOnly)
{
  String SDInfo = "i";
  bCard = false;
  int count=0;
  File myFile;
  while(!SD.begin(CS)) //Sometime this gets missed. Will try 6 times before giving up
  {
    count++;
    if (count >=6)
    {
      Serial.println("Card Mount Failed");
      break;
    }
    delay (100);
  }

  if (count >=6)
  {
    SDInfo += "No SDMEM card found\r\n";
    return SDInfo;
  }
    
  bCard = true;
  if (bInitOnly)
    return String();

  //SD Mem Interface
  SDInfo +="SD Memory card found<br/>";
  SDInfo +="Card Type: ";
  switch (SD.type()) 
  {
    case SD_CARD_TYPE_SD1:
      SDInfo += "SD1";
      break;

    case SD_CARD_TYPE_SD2:
      SDInfo += "SD2";
      break;

    case SD_CARD_TYPE_SDHC:
      SDInfo += "SDHC";
      break;

    default:
      SDInfo += "Unknown";
  }
  SDInfo += "<br/>";

  if (SD.fatType() <= 32) 
  {
    SDInfo += "Volume is FAT: " + String(int(SD.fatType())) + "<br/>";
  } 
  else 
  {
    SDInfo += "Volume is exFAT<br/>";
  }

  SDInfo += "Total Clusters: ";
  SDInfo += String(SD.totalClusters()) + "<br/>";
  SDInfo += "Blocks per Cluster: ";
  SDInfo += String(SD.blocksPerCluster()) + "<br/>";
  SDInfo += "Total Blocks: ";
  SDInfo += String(SD.blocksPerCluster() * SD.totalClusters()) + "<br/>";
  SDInfo += "Total Bytes / Block: 512<br/>"; //SD card blocks are always 512 bytes (2 blocks are 1KB)
  uint32_t volumesize;
  volumesize = SD.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= SD.totalClusters();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  SDInfo += "Volume size (Kb): " + String(volumesize) + "<br/>";
  volumesize /= 1024;
  SDInfo += "Volume size (Mb): " + String(volumesize) + "<br/>";
  SDInfo += "Volume size (Gb): ";
  SDInfo += String((float)volumesize / 1024.0) + "<br/>";
  return SDInfo;
}

String listDir(const char * dirname, uint8_t levels, bool bDel=false)
{
  Serial.printf("Listing directory: %s\n", dirname);
  String fileOp = String(dirname) + "\n";
  String fileList;
  File root = SD.open(dirname);
  if(!root)
  {
      fileOp += String("Failed to open directory: ") + dirname + "\n";;
      return fileOp;
  }
  if(!root.isDirectory())
  {
      fileOp += dirname + String(" Not a directory") +"\n";
      return fileOp;
  }
  File file = root.openNextFile();
  while(file)
  {
    if(file.isDirectory())
    { 
      if (bDel)
        delDir += String(file.fullName()) + "/\n";

      fileOp += String(dirname) + String(file.name()) + "/\n";
      if(levels)
        listDir(file.fullName(), levels -1, bDel);

    } 
    else 
    { 
      if (bDel)
        SD.remove(file.fullName());
      else
      {  
        fileList += file.name();
        fileList +="  ... ";
        fileList += file.size();
        fileList += " bytes \n";
      }
    }
    file = root.openNextFile();
  }
  if ((strlen(dirname) > 2) && !bDel)
    fileOp +=  "../\n";  

  return (bDel ? fileOp : fileOp + "#%&" + fileList);

}

bool createFile(const char* path)
{
  if (SD.exists(path)) 
    return false;
  
  File myFile = SD.open(path, FILE_WRITE);
  myFile.close();
  return true;
  
}
bool deleteFile(const char * path)
{
  return SD.remove(path);
}

bool renameFile(const char * path1, const char * path2)
{
  return SD.rename(path1, path2); 
}

bool createDir(const char * path)
{
  return SD.mkdir(path);
}

bool removeDir(const char * path)
{
  listDir(path, 5, true);
  return SD.rmdir(path);
}

String readFile(const char * path)
{
  File file = SD.open(path, FILE_READ);
  String buffer = "";
  if(!file)
  {
    logFile += ClkTZ.dateTime("H:i:s~ ")  + String(path) + ": Failed to open file for reading\r\n";
    Serial.printf("%s: Failed to open file for reading\n",path) ;
    return String();
  }

  while(file.available())
  {
    buffer += (char)file.read();
  }
  file.close();
  return buffer;
}

void writeFile(const char * path, const char * message)
{
  telnet.printf("Currently writing to: %s\r\n",path);
  File file = SD.open(path, FILE_WRITE);
  if(!file)
  {
    logFile += ClkTZ.dateTime("H:i:s~ ")  + String(path) + ": Failed to open file for writing\r\n";
    telnet.println("Unable to open file for writing");
    return;
  }
  if(file.print(message))
  {
    telnet.println("Writing to file suceeded");
  } 
  else 
  {
    logFile += ClkTZ.dateTime("H:i:s~ ")  + String(path) + ": Failed to Write to file\r\n";
    telnet.println("Unable to write to specified file");
  }
  file.flush();
  file.close();
}

void SendTemperatureFromFile(uint8_t num)
{
  String dt;
  if (!bCard) 
  { 
    if (tempCString[0] = '\0')
      return;
    else
    {
      dt = ClkTZ.dateTime("h:i:s A") + "," + tempCString + ",°C," + tempFString + ",°F\r\n"; 
      ws.sendTXT (num,dt.c_str(),dt.length());  //Send Temperture
      return;
    }
  }
  dt = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".csv";
  dt = readFile(dt.c_str());
  if (dt == "")
    dt = ClkTZ.dateTime("h:i:s A") + "," + tempCString + ",°C," + tempFString + ",°F\r\n"; 
    
  ws.sendTXT (num, ("t" + dt).c_str(),dt.length()+1);  //Send Temperture
    
}

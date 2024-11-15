/***************************************************************************************
** Function name:           listSPIFFS
** Description:             Listing SPIFFS files
***************************************************************************************/
void listSPIFFS(void) {
  Serial.println(F("\r\nListing SPIFFS files:"));

  fs::Dir dir = SPIFFS.openDir("/"); // Root directory

  static const char line[] PROGMEM =  "=================================================";
  Serial.println(FPSTR(line));
  Serial.println(F("  File name                              Size"));
  Serial.println(FPSTR(line));

  while (dir.next()) {
    String fileName = dir.fileName();
    Serial.print(fileName);
    int spaces = 33 - fileName.length(); // Tabulate nicely
    if (spaces < 1) spaces = 1;
    while (spaces--) Serial.print(" ");

    fs::File f = dir.openFile("r");
    String fileSize = (String) f.size();
    spaces = 10 - fileSize.length(); // Tabulate nicely
    if (spaces < 1) spaces = 1;
    while (spaces--) Serial.print(" ");
    Serial.println(fileSize + " bytes");
  }

  Serial.println(FPSTR(line));
  Serial.println();
  delay(1000);
}
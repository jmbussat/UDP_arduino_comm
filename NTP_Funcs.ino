int DST(time_t t){
  //January, february, and december are out.
  if(month(t) < 3 || month(t) > 11) { 
    return 0; 
  }
  //April to October are in
  if(month(t) > 3 && month(t) < 11) {
    return 1; 
  }
  int previousSunday = day(t) - weekday(t);
  //In march, we are DST if our previous sunday was on or after the 8th.
  if(month(t) == 3) {
    if((previousSunday >= 8) && (hour(t) >=2)){ 
      return 1;
    } else {
      return 0; 
    }
  }
  //In november we must be before the first sunday to be dst.
  //That means the previous sunday must be before the 1st.
  if(month(t) == 11) {
    if((previousSunday <= 0) && (hour(t)<=2)) { 
      return 1;
    } else {
      return 0; 
    }
  }
}

void setTimeNTP(){
  uint8_t gotTimeData=0;
  uint8_t beat=0;
    //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  while(!gotTimeData){
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);
    
    int cb = udp.parsePacket();
    if (cb) {
      gotTimeData=1;
      PlotChar(' ',0,19);
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  
      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
  
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
  
      // now convert NTP time into everyday time:
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      time_t epoch = secsSince1900 - seventyYears;
      // TimeZone = PST (GMT-8)
      setTime(epoch+TIMEZONE*3600+DST(epoch+TIMEZONE*3600)*3600);
  
      // print the hour, minute and second:
      //DisplayStr("\rThe UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
      //unsigned long hours=(epoch  % 86400L) / 3600;
      //unsigned long minutes=(epoch  % 3600) / 60;
      //unsigned long seconds=epoch % 60;
    } else {
      if(!beat){
        PlotChar('*',0,19);
        beat=1;
      } else {
        PlotChar(' ',0,19);
        beat=0;        
      }   
    }
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

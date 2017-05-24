void UDP_Comm(){
  if(udp_data.parsePacket()) {
    remote=udp_data.remoteIP();
    sprintf(buffer,"[%d.%d.%d.%d]",remote[0],remote[1],remote[2],remote[3]);
    DisplayStrXY(buffer,6,0,1,1);
    udp_data.read(udpBuffer, UDP_TX_PACKET_MAX_SIZE);
    cmd=udpBuffer[0]; 
    if(cmd=='G') {
      imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      dispWordHex(2,0,ax);
      dispWordHex(2,5,ay);
      dispWordHex(2,10,az);
      dispWordHex(3,0,gx);
      dispWordHex(3,5,gy);
      dispWordHex(3,10,gz);
      IMU_data[0]=(ax&0xFF00)>>8;
      IMU_data[1]=ax&0x00FF;
      IMU_data[2]=(ay&0xFF00)>>8;
      IMU_data[3]=ay&0x00FF;
      IMU_data[4]=(az&0xFF00)>>8;
      IMU_data[5]=az&0x00FF;
      IMU_data[6]=(gx&0xFF00)>>8;
      IMU_data[7]=gx&0x00FF;
      IMU_data[8]=(gy&0xFF00)>>8;
      IMU_data[9]=gy&0x00FF;
      IMU_data[10]=(gz&0xFF00)>>8;
      IMU_data[11]=gz&0x00FF;
      IMU_data[12]=counter++;
      udp_data.beginPacket(remote, 4998); 
      udp_data.write(IMU_data, 13);
      udp_data.endPacket();          
    }
  }  
}


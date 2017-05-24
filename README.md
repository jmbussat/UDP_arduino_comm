# UDP_arduino_comm
MPU6050 accel reading via UDP (ESP8266) - Include NTP clock and I2C display with custom fonts, rolling display using matplotlib

Unorganized work in progress on learning how to use UDP to exchange data between a server and a client

Plateform used:
  - Server: NodeMCU 0.9 ESP8266 board with i2c 0.96" graphic display and i2c connected MPU6050
      - The display is used to show the status of the server: IP address, IP address of connected client, MPU6050 status and data
        There is also a NTP synchronized clock that displays time and date 
  - Client: Mac with python script reading the accelerometer data and displaying on a scrolling strip chart

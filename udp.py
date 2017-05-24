#!/usr/bin/env python
# 
# Simple UDP receiver plotting data from MPU6050 sent over Wifi by an ESP8266
#
# Jean-Marie Bussat - jmbussat@gmail.com
# Rev 0 - 1/4/2017
#
import socket
import sys
import matplotlib.pyplot as plt 
import struct

fig, ax = plt.subplots()
x = []
y1 = []
y2 = []
y3 = []

cnt=0

# Prepare the plot window with 3 waveforms
#wave1,wave2,wave3 = ax.plot(x,y1,'b.--',x,y2,'r.--',x,y3,'g.--')
wave1, = ax.plot(x,y1,marker='.',linestyle='--',color='b',label='X')
wave2, = ax.plot(x,y2,marker='.',linestyle='--',color='r',label='Y')
wave3, = ax.plot(x,y3,marker='.',linestyle='--',color='g',label='Z')
plt.ylabel('Acceleration [g]')
plt.xlabel('Frame number')
plt.legend()

# Open UPD connection
s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM,socket.IPPROTO_UDP)
# This is our address
s.bind(('192.168.1.40',4998))

# Get data and display it 
while(1):
	try:
		# Request data from ESP8266 (see Arduino code)
		# This is the address of the target
		s.sendto('G'.encode(),('192.168.1.43',4998))
		# Set a timeout value to catch the cases where the ESP8266
		# would miss the 'G' character and not reply or if we wouldn't get
		# the data it has sent (can happen with UDP)
		s.settimeout(1)
		# Get the data
		data,addr=s.recvfrom(13)
		
		# Data is made of 6 16-bit signed integers sent byte per byte.
		# High byte first then low byte (-32768 to +32767)
		# MPU6050 setup from I2Cdevlib:
		#	Accelerometer: +/-2g
		#	Gyroscope: +/-250deg/s
		FRAME_FORMAT=(
		">"	# High byte first
		"h" # Accelerometer x
		"h" # Accelerometer y
		"h" # Accelerometer z
		"h" # Gyro x
		"h" # Gyro y
		"h" # Gyro z
		"B"	# Frame counter
		)
		(xa,ya,za,xg,yg,zg,fc)=struct.unpack(FRAME_FORMAT,data)
		
		# Update list for data display
		x.append(cnt)
		y1.append(xa*2.0/32768.0)
		y2.append(ya*2.0/32768.0)
		y3.append(za*2.0/32768.0)
		
		# Only display last 100 points 
		xp=x[-100:]
		y1p=y1[-100:]
		y2p=y2[-100:]
		y3p=y3[-100:]
		
		# Autoscale axis
		xmin=min(xp)
		xmax=max(xp)
		ymin=min(min(y1p),min(y2p),min(y3p))
		ymax=max(max(y1p),max(y2p),max(y3p))
		ax.set_xlim(xmin, xmax) 
		ax.set_ylim(ymin, ymax)
		
		# Update data point
		wave1.set_data(xp, y1p)
		wave2.set_data(xp, y2p)
		wave3.set_data(xp, y3p)
		
		# Display updated waveforms
		plt.draw()
		plt.pause(0.01)
		
		# Event counter to monitor UDP traffic
		cnt=cnt+1
	except socket.timeout:
		# In case of timeout (no reply from ESP8266)
		# We need to close the connection and reopen it
		print("Timeout occured")
		s.close()
		s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM,socket.IPPROTO_UDP)
		s.bind(('192.168.1.40',4998))
		continue
		

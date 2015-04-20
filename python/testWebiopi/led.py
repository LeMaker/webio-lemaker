import webiopi
import time

GPIO = webiopi.GPIO
LIGHT = 17 # GPIO pin using BCM numbering
GPIO.setFunction(LIGHT, GPIO.OUT)

#ver_Bpi = GPIO.RPI_REVISION
#ver_Gpio = GPIO.VERSION

#print "BPi VERSION:\t\t",ver_Bpi
#print "RPi.GPIO VERSION:\t",ver_Gpio

while True:
	GPIO.digitalWrite(LIGHT, GPIO.HIGH)
	time.sleep(2)
	GPIO.digitalWrite(LIGHT, GPIO.LOW)
	time.sleep(2)

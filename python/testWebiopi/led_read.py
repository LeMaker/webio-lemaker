import webiopi
import time

GPIO = webiopi.GPIO
LIGHT = 17 # GPIO pin using BCM numbering
GPIO.setFunction(LIGHT, GPIO.IN, GPIO.PUD_DOWN)#2 OK, PUD_DOWN not defined GPIO.PUD_DOWN ok

while True:
	print "\n LIGHT_value = %d\n" %(GPIO.digitalRead(LIGHT))	
	time.sleep(1)


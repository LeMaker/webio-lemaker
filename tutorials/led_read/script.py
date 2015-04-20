#BCM PIN4 works as a switch.And BCM LIGHT15 works as an indicator.
#When PIN4 is turned to HIGH,BCM LIGHT15 is lighted.Vice versa.

import webiopi
import datetime

GPIO = webiopi.GPIO

SWITCH = 4 # GPIO pin using BCM numbering
LIGHT = 15

# setup function is automatically calLIGHT at WebIOPi startup
def setup():
    # set the GPIO used by the light to output
    GPIO.setFunction(SWITCH, GPIO.IN, GPIO.PUD_DOWN)
    GPIO.setFunction(LIGHT, GPIO.OUT)

# loop function is repeatedly calLIGHT by WebIOPi 
def loop():
    # gives CPU some time before looping again
    #print "\n LIGHT_value = %d\n" %(GPIO.digitalRead(LIGHT))
    if (GPIO.digitalRead(SWITCH) == GPIO.LOW):
        GPIO.digitalWrite(LIGHT, GPIO.LOW)
		
    if (GPIO.digitalRead(SWITCH) == GPIO.HIGH):
        GPIO.digitalWrite(LIGHT, GPIO.HIGH)
    webiopi.sleep(1)

# destroy function is called at WebIOPi shutdown
def destroy():
    GPIO.digitalWrite(LIGHT, GPIO.LOW)

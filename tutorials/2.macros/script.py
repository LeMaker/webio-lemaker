import webiopi
import datetime

GPIO = webiopi.GPIO

LIGHT = 17 # GPIO pin using BCM numbering

HOUR_ON  = 8  # Turn Light ON at xx:xx:08
HOUR_OFF = 38 # Turn Light OFF at xx:xx:38

# setup function is automatically called at WebIOPi startup
def setup():
    # set the GPIO used by the light to output
    GPIO.setFunction(LIGHT, GPIO.OUT)

    # retrieve current datetime
    now = datetime.datetime.now()

    # test if we are between ON time and tun the light ON
    if ((now.second >= HOUR_ON) and (now.second < HOUR_OFF)):
	    GPIO.digitalWrite(LIGHT, GPIO.HIGH)#4 space ,not 1 tab

# loop function is repeatedly called by WebIOPi 
def loop():
    # retrieve current datetime
    now = datetime.datetime.now()

    # toggle light ON all days at the correct time
    #if ((now.hour == HOUR_ON) and (now.minute == 0) and (now.second == 0)):
    if (now.second == HOUR_ON):
        if (GPIO.digitalRead(LIGHT) == GPIO.LOW):
            GPIO.digitalWrite(LIGHT, GPIO.HIGH)

    # toggle light OFF
    #if ((now.hour == HOUR_OFF) and (now.minute == 0) and (now.second == 0)):
    if (now.second == HOUR_OFF):
        if (GPIO.digitalRead(LIGHT) == GPIO.HIGH):
            GPIO.digitalWrite(LIGHT, GPIO.LOW)

    # gives CPU some time before looping again
    webiopi.sleep(1)

# destroy function is called at WebIOPi shutdown
def destroy():
    GPIO.digitalWrite(LIGHT, GPIO.LOW)

# a simple macro without argument
# returns ON;OFF hours
@webiopi.macro
def getLightHours():
    return "%d;%d" % (HOUR_ON, HOUR_OFF)

# a macro with two arguments
# set ON, OFF hours
# returns ON;OFF hours
@webiopi.macro
def setLightHours(on, off):
    global HOUR_ON, HOUR_OFF
    HOUR_ON = int(on)
    HOUR_OFF = int(off)
    return getLightHours()


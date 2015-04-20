import webiopi
import datetime

GPIO = webiopi.GPIO

LIGHT = 17 # GPIO pin using BCM numbering

SECOND_ON  = 8  # Turn Light ON at xx:xx:08
SECOND_OFF = 38 # Turn Light OFF at xx:xx:38

# setup function is automatically called at WebIOPi startup
def setup():
    # set the GPIO used by the light to output
    GPIO.setFunction(LIGHT, GPIO.OUT)

    # retrieve current datetime
    now = datetime.datetime.now()

    # test if we are between ON time and tun the light ON
    if ((now.second >= SECOND_ON) and (now.second < SECOND_OFF)):
        GPIO.digitalWrite(LIGHT, GPIO.HIGH)

# loop function is repeatedly called by WebIOPi 
def loop():
    # retrieve current datetime
    now = datetime.datetime.now()

    # toggle light ON all days at the correct time
    if (now.second == SECOND_ON):
        if (GPIO.digitalRead(LIGHT) == GPIO.LOW):
            GPIO.digitalWrite(LIGHT, GPIO.HIGH)

    # toggle light OFF
    if (now.second == SECOND_OFF):
        if (GPIO.digitalRead(LIGHT) == GPIO.HIGH):
            GPIO.digitalWrite(LIGHT, GPIO.LOW)

    # gives CPU some time before looping again
    webiopi.sleep(1)

# destroy function is called at WebIOPi shutdown
def destroy():
    GPIO.digitalWrite(LIGHT, GPIO.LOW)


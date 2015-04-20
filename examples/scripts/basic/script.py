# Imports
import webiopi
import time
# Enable debug output
webiopi.setDebug()

# Retrieve GPIO lib
GPIO = webiopi.GPIO
SWITCH = 4
SERVO  = 23
LED0   = 24
LED1   = 25

# Called by WebIOPi at script loading
def setup():
    webiopi.debug("Basic script - Setup")
    # Setup GPIOs
    GPIO.setFunction(SWITCH, GPIO.IN)
    GPIO.setFunction(SERVO, GPIO.PWM)
    GPIO.setFunction(LED0, GPIO.PWM)
    GPIO.setFunction(LED1, GPIO.OUT)
        
    GPIO.pwmWrite(LED0, 0.5)		# set to 50% ratio
    GPIO.pwmWriteAngle(SERVO, 0)    # set to 0 (neutral)
    GPIO.digitalWrite(LED1, GPIO.HIGH)

#def loop():
#    webiopi.sleep(1)
    
# Called by WebIOPi at server shutdown
def destroy():
    webiopi.debug("Basic script - Destroy")
    # Reset GPIO functions
    GPIO.setFunction(SWITCH, GPIO.IN)
    GPIO.setFunction(SERVO, GPIO.IN)
    GPIO.setFunction(LED0, GPIO.IN)
    GPIO.setFunction(LED1, GPIO.IN)

import webiopi
import time
GPIO = webiopi.GPIO
LIGHT = 24 # GPIO pin using BCM numbering
GPIO.setFunction(LIGHT, GPIO.PWM)
GPIO.setFunction(23, GPIO.PWM)

GPIO.pwmWrite(LIGHT, 0.5)
GPIO.pwmWrite(23,0.7)
while True:
#    GPIO.pwmWrite(LIGHT, 0.5)
#    GPIO.pwmWrite(23, 0.5)
    time.sleep(1)
#    GPIO.pwmWrite(LIGHT, 1)
#    GPIO.pwmWrite(23, 1)
#    time.sleep(1)

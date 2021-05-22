; ================================================================================
;
; KeypadDriver.Vars.au3
; This file declares the variables required to define the driver key mapping and all other status
;
; ================================================================================

#include-once

; Size of the keypad
Global Const $WIDTH = 4, $HEIGHT = 3
; Available rgb states on the keypad
Global $rgbStates = ["staticLight","rainbow","spreadOut","breathing","fractionalDrawingTest2d","spinningRainbow","ripple","antiRipple","stars","raindrop","snake","shootingParticles","whacAMole","tictactoe"]

; Global variables for the connection status
Global Enum $NOTCONNECTED, $CONNECTIONFAILED, $PORTDETECTIONFAILED, $CONNECTED
Global $connectionStatus = $NOTCONNECTED

; Global variables for possible keystroke states
Global Enum $KEYSTROKEUP, $KEYSTROKEDOWN
Global $keyStrokeAmount = 2
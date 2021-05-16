; ================================================================================
;
; KeypadDriver.Vars.au3
; This file declares the variables required to define the driver key mapping and all other status
;
; ================================================================================

#include-once

; Path to the config file
Global Const $iniPath = @ScriptDir & "\keypadconfig.ini"

; Size of the keypad
Global Const $WIDTH = 4, $HEIGHT = 3
; Available rgb states on the keypad
Global $rgbStates = ["staticLight","rainbow","spreadOut","breathing","fractionalDrawingTest2d","spinningRainbow","ripple","antiRipple","stars","raindrop"]

; Button state enums
Global Enum $UP, $DOWN

; Global variables for the connection status
Global Enum $NOTCONNECTED, $CONNECTIONFAILED, $PORTDETECTIONFAILED, $CONNECTED
Global $connectionStatus = $NOTCONNECTED

; The mapping for the keypad keys
; [[keyStrokeUp, keyStrokeDown], ...]
Global $keyMap[$WIDTH * $HEIGHT][2]
For $j = 0 To $HEIGHT - 1
	For $i = 0 To $WIDTH - 1
		$keyMap[$j * $WIDTH + $i][0] = ""
		$keyMap[$j * $WIDTH + $i][1] = ""
	Next
Next


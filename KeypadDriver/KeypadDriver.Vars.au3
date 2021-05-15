; ================================================================================
;
; KeypadDriver.Vars.au3
; This file declares the variables required to define the driver key mapping and status
;
; ================================================================================

#include-once

Global Const $iniPath = @ScriptDir & "\keypadconfig.ini"

Global Const $WIDTH = 4, $HEIGHT = 3

; [[keyStrokeUp, keyStrokeDown], ...]
Global $keyMap[$WIDTH * $HEIGHT][2]
For $j = 0 To $HEIGHT - 1
	For $i = 0 To $WIDTH - 1
		$keyMap[$j * $WIDTH + $i][0] = ""
		$keyMap[$j * $WIDTH + $i][1] = ""
	Next
Next

Global Enum $UP, $DOWN

Global Enum $NOTCONNECTED, $CONNECTIONFAILED, $PORTDETECTIONFAILED, $CONNECTED
Global $connectionStatus = $NOTCONNECTED
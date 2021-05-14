#include-once

Global Const $WIDTH = 4, $HEIGHT = 3

; [keyStrokeUp, keyStrokeDown, keyName], ...]
Global $keyMap[$WIDTH * $HEIGHT][2]
For $j = 0 To $HEIGHT - 1
	For $i = 0 To $WIDTH - 1
		$keyMap[$j * $WIDTH + $i][0] = ""
		$keyMap[$j * $WIDTH + $i][1] = ""
	Next
Next

Global Enum $UP, $DOWN
Global $btnState[$WIDTH * $HEIGHT]

Global Enum $NOTCONNECTED, $CONNECTIONFAILED, $PORTDETECTIONFAILED, $CONNECTED
Global $connectionStatus = $NOTCONNECTED
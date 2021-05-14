; ================================================================================
;
; KeypadDriver.Serial.au3
; This file contains the functions required to communicate with the keypad
;
; ================================================================================

#include "Include\CommMG.au3"
#include "KeypadDriver.Vars.au3"

Global $byteString = "", $byte

Global $comPort

Global $pressedBtnNum = 0
Global $pressedBtnState = 0

; This function tries to connect to the keypad serial port
Func Connect()
	Local $ports[0]
	$ports = _ComGetPortNames()
	For $i = 0 To UBound($ports) - 1
		If $ports[$i][1] == "USB-SERIAL CH340" Then
			$comPort = $ports[$i][0]
			_CommSetPort(Int(StringReplace($comPort, "COM", "")), "", 19200, 8, "none", 1, 2)
			
			If Not @error Then
				; Connection succeed
				$connectionStatus = $CONNECTED
				_CommSetRTS(0)
				_CommSetDTR(0)
			Else
				; Connection failed
				$connectionStatus = $CONNECTIONFAILED
			EndIf
			
			; Port was detected, no matter whether it's connected or not, stop searching ports, return
			Return
		EndIf
	Next
	
	; If this line runs that means no port was detected, sets the status to detaction failed
	$connectionStatus = $PORTDETECTIONFAILED
EndFunc

; This function polls the serial for new key datas
Func PollKeys()
	If $connectionStatus <> $CONNECTED Then Return
	$byteString = _CommReadByte()
	If @error = 3 Then
		$connectionStatus = $CONNECTIONFAILED
		Return
	EndIf
	If $byteString <> "" Then
		$byte = Int($byteString)
		
		; Key status byte - |first 4 bits for key number, 3 zero padding bits, last one bit for pressed state|
		$pressedBtnNum = BitShift($byte, 4)
		$pressedBtnState = BitAND($byte, 0x01)
		
		; c("Button: $ pressed, state: $", 1, $pressedBtnNum, $pressedBtnState)
		
		; Only sends the key stroke when the gui isn't opened
		If $pressedBtnNum <= $WIDTH * $HEIGHT And Not $guiOpened Then
			Switch $pressedBtnState
				Case $UP
					Send($keyMap[$pressedBtnNum - 1][0])
				Case $DOWN
					Send($keyMap[$pressedBtnNum - 1][1])
			EndSwitch
		EndIf
	EndIf
EndFunc

; This function polls the serial for a byte
Func PollData()
	If $connectionStatus <> $CONNECTED Then Return
	
	; If there's still unprocessed byte in the buffer $byte, return
	If $receivedByte Then Return
	
	$byteString = _CommReadByte()
	If @error = 3 Then
		$connectionStatus = $CONNECTIONFAILED
		Return
	EndIf
	If $byteString <> "" Then
		$byte = Int($byteString)
		; c("Received data $", 1, $byteString)
		$waitingForSyncingBytes -= 1
		$receivedByte = True
		Return
	EndIf
EndFunc

; This function sends a message byte to the keypad
Func SendMsgToKeypad($type, $data)
	If $connectionStatus <> $CONNECTED Then Return
	
	; Message byte - |first 2 bits for msg type, last 6 bits for msg data|
	If $type > 3 Then
		c("SendMsgToKeypad >> Message type cannot be larger than 2 bits! Exception data: $", 1, $type)
		c("Program terminating!")
		Terminate()
	EndIf
	If $data > 63 Then
		c("SendMsgToKeypad >> Data to send cannot be larger than 6 bits! Exception data: $", 1, $data)
		c("Program terminating!")
		Terminate()
	EndIf
	_CommSendByte(BitShift($type, -6) + $data)
EndFunc
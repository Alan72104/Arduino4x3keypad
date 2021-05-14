#include "Include\CommMG.au3"
#include "KeypadDriver.Vars.au3"

Global $byteString = "", $byte

Global $comPort

Global $pressedBtnNum = 0
Global $pressedBtnState = 0

Func Connect()
	Local $ports[0]
	$ports = _ComGetPortNames()
	For $i = 0 To UBound($ports) - 1
		If $ports[$i][1] == "USB-SERIAL CH340" Then
			$comPort = $ports[$i][0]
			_CommSetPort(Int(StringReplace($comPort, "COM", "")), "", 19200, 8, "none", 1, 2)
			If Not @error Then
				$connectionStatus = $CONNECTED
				_CommSetRTS(0)
				_CommSetDTR(0)
			Else
				$connectionStatus = $CONNECTIONFAILED
			EndIf
			ExitLoop
		EndIf
		If $i = UBound($ports) - 1 Then
			$connectionStatus = $PORTDETECTIONFAILED
		EndIf
	Next
EndFunc

Func PollKeys()
	If $connectionStatus <> $CONNECTED Then Return
	$byteString = _CommReadByte()
	If @error = 3 Then
		$connectionStatus = $CONNECTIONFAILED
		Return
	EndIf
	If $byteString <> "" Then
		$byte = Int($byteString)
		$pressedBtnNum = BitShift($byte, 4)
		$pressedBtnState = BitAND($byte, 0x01)
		; c("Button: $ pressed, state: $", 1, $pressedBtnNum, $pressedBtnState)
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

Func PollData()
	If $connectionStatus <> $CONNECTED Then Return
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

Func SendMsgToKeypad($type, $data)
	If $connectionStatus <> $CONNECTED Then Return
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
#include <Array.au3>
#include <ButtonConstants.au3>
#include <CommMG.au3>
#include <GUIConstantsEx.au3>
#include <StringConstants.au3>
#include <WindowsConstants.au3>
#include "LibDebug.au3"

Global Const $WIDTH = 4, $HEIGHT = 3
Global $byteString = "", $byte
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
Global $index = -1
Global $loopPeriod, $loopStartTime, $timer
Global Const $scanPerSec = 1500
Global Const $msPerScan = 1000 / $scanPerSec
Global $pressedBtnNum = 0
Global $pressedBtnState = 0
Global $ports[0]
Global $comPort
Global $guiOpened = False
Global $gdiPlusStarted = False
Global $hGui
Global $idButtonBtns[$WIDTH * $HEIGHT]
Global $idButtonClose, $idRadioBind, $idRadioRemove, $idButtonSave
Global $msg
Global $bindingKeys = False
Global $currentlyBinding = 0
Global $idGroupBinding, $idLabelCurrentlyBinding, $idLabelBindingArrow, $idLabelBindingStr, $idInputKeyUp, $idInputKeyDown
Global $idButtonConfirm, $idButtonCancel
Global Enum $BIND, $REMOVE
Global $bindingAction = $BIND
Global Const $iniPath = @ScriptDir & "\keypadconfig.ini"
Global $rgbStates = ["lightWhenPressed","rainbow","spreadLightsOutWhenPressed","breathing","fractionalDrawingTest2d","spinningRainbow","waterWave"]
Global $idComboRgbState, $idButtonRgbUpdate
Global Enum $UPDATERGBSTATE, $UPDATERGBBRIGHTNESS, $GETRGBDATA
Global $waitingForSyncingBytes = 0, $receivedByte = False, $timerGuiBtnRgbSync
Global $rgbBuffer[$WIDTH * $HEIGHT][3]
Global $syncingButtonIndex = 0
Global $syncingRgbIndex = 0
HotKeySet("{F4}", "OpenGui")
Opt("GUICloseOnESC", 0)

; Todo: Connection indicator in the gui

Func Main()
	_CommSetDllPath(@ScriptDir & "/commg.dll")
	$ports = _ComGetPortNames()
	For $i = 0 To UBound($ports) - 1
		If $ports[$i][1] == "USB-SERIAL CH340" Then
			$comPort = $ports[$i][0]
			_CommSetPort(Int(StringReplace($comPort, "COM", "")), "", 19200, 8, "none", 1, 2)
			If @error Then
				c("Cannot connect to com port: $", 1, $comPort)
				ce(@error)
				Exit
			Else
				c("Connected to com port: $", 1, $comPort)
			EndIf
			ExitLoop
		EndIf
		If $i = UBound($ports) - 1 Then
			c("Cannot detect com port for arduino nano")
			ce(@error)
			Exit
		EndIf
	Next
	_CommSetRTS(0)
	_CommSetDTR(0)
	If FileExists($iniPath) Then
		For $i = 1 To $WIDTH * $HEIGHT
			BindKey($i, IniRead($iniPath, "ButtonBindings", "Button" & $i & "Up", ""), IniRead($iniPath, "ButtonBindings", "Button" & $i & "Down", ""))
		Next
	Else
		BindKey(1, "ESC")
		BindKey(2, "`")
		BindKey(3, "c")
		BindKey(4, "", "!{UP}")
		BindKey(5, "^a", "")
		BindKey(6, "f")
		BindKey(7, "", "!{TAB}")
		BindKey(8, "", "!{DOWN}")
		BindKey(9, "r")
		BindKey(10, "t")
		BindKey(11, "", "{LEFT}")
		BindKey(12, "", "{RIGHT}")
	EndIf
	Sleep(200)
	OpenGui()
	; Global $t = 0
	; Global $tt = 0
	While 1
		$loopStartTime = TimerInit()
		If (TimerDiff($timer) >= ($msPerScan - ($loopPeriod > $msPerScan ? $msPerScan : $loopPeriod))) Then
			; If $waitingForSyncingBytes And Not $receivedByte Then
				; PollData()
			; Else
				PollKeys()
			; EndIf
			; If TimerDiff($tt) >= 1000 Then
				; $tt = TimerInit()
				; c($t)
				; $t = 0
			; EndIf
			; $t += 1
			If $guiOpened Then
				SyncGuiRgb()
				HandleMsg()
			EndIf
			$timer = TimerInit()
			$loopPeriod = $loopPeriod * 0.6 + TimerDiff($loopStartTime) * 0.4
		EndIf
	WEnd
EndFunc

Main()

Func PollKeys()
	$byteString = _CommReadByte()
	If $byteString <> "" Then
		$byte = Int($byteString)
		$pressedBtnNum = BitShift($byte, 4)
		$pressedBtnState = BitAND($byte, 0x01)
		; c("Button: $ pressed, state: $", 1, $pressedBtnNum, $pressedBtnState)
		If $pressedBtnNum <= $WIDTH * $HEIGHT And Not $guiOpened Then
			Switch $pressedBtnState
				Case 0
					Send($keyMap[$pressedBtnNum - 1][0])
				Case 1
					Send($keyMap[$pressedBtnNum - 1][1])
			EndSwitch
		EndIf
	EndIf
EndFunc

Func PollData()
	$byteString = _CommReadByte()
	If $byteString <> "" Then
		$byte = Int($byteString)
		; c("Received data $", 1, $byteString)
		$waitingForSyncingBytes -= 1
		$receivedByte = True
		Return
	EndIf
EndFunc

Func SendMsgToKeypad($type, $data)
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

Func HandleMsg()
	$msg = GUIGetMsg()
	Switch $msg
		Case 0
		Case $GUI_EVENT_CLOSE
			CloseGui()
		Case $idButtonClose
			CloseGui()
			Terminate()
		Case $idButtonSave
			For $i = 1 To $WIDTH * $HEIGHT
				IniWrite($iniPath, "ButtonBindings", "Button" & $i & "Up", String($keyMap[$i - 1][0]))
				IniWrite($iniPath, "ButtonBindings", "Button" & $i & "Down", String($keyMap[$i - 1][1]))
			Next
		Case $idRadioBind
			$bindingAction = $BIND
		Case $idRadioRemove
			$bindingAction = $REMOVE
			If $bindingKeys Then
				$bindingKeys = False
				ShowBindingGroup(0)
			EndIf
		Case $idButtonRgbUpdate
			SendMsgToKeypad($UPDATERGBSTATE, ArrayFind($rgbStates, GUICtrlRead($idComboRgbState)))
		Case Else
			For $j = 0 To $HEIGHT - 1
				For $i = 0 To $WIDTH - 1
					If $msg = $idButtonBtns[$j * $WIDTH + $i] Then
						Switch $bindingAction
							Case $BIND
								$bindingKeys = True
								$currentlyBinding = $j * $WIDTH + $i + 1
								GUICtrlSetData($idLabelCurrentlyBinding, "Binding key " & $currentlyBinding)
								GUICtrlSetData($idInputKeyUp, $keyMap[$j * $WIDTH + $i][0])
								GUICtrlSetData($idInputKeyDown, $keyMap[$j * $WIDTH + $i][1])
								ShowBindingGroup(1)
							Case $REMOVE
								BindRemove($j * $WIDTH + $i + 1)
								UpdateBtnLabels()
						EndSwitch
						Return
					EndIf
				Next
			Next
			If $bindingKeys Then
				If $msg = $idButtonConfirm Then
					BindKey($currentlyBinding, GUICtrlRead($idInputKeyUp), GUICtrlRead($idInputKeyDown))
					UpdateBtnLabels()
					$bindingKeys = False
					ShowBindingGroup(0)
				ElseIf $msg = $idButtonCancel Then
					$bindingKeys = False
					ShowBindingGroup(0)
				EndIf
			EndIf
	EndSwitch
EndFunc

Func SyncGuiRgb()
	Local $timer = 0
	If TimerDiff($timerGuiBtnRgbSync) > 200 Then
		$timerGuiBtnRgbSync = TimerInit()
		SendMsgToKeypad($GETRGBDATA, 0)
		$waitingForSyncingBytes = 3 * $WIDTH * $HEIGHT
		$syncingButtonIndex = 0
		$syncingRgbIndex = 0
		$timer = TimerInit()
		While 1
			Do
				PollData()
			Until $receivedByte
			$receivedByte = False
			$rgbBuffer[$syncingButtonIndex][$syncingRgbIndex] = $byte
			$syncingRgbIndex += 1
			If $syncingRgbIndex = 3 Then
				$syncingRgbIndex = 0
				$syncingButtonIndex += 1
			EndIf
			If $syncingButtonIndex = $WIDTH * $HEIGHT Then
				UpdateBtnLabelsRgb($rgbBuffer)
				Return
			EndIf
			If TimerDiff($timer) > 400 Then
				Return
			EndIf
		WEnd
	EndIf
EndFunc

Func UpdateBtnLabels()
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			GUICtrlSetData($idButtonBtns[$j * $WIDTH + $i], ($j * $WIDTH + $i + 1) & @CRLF & _
															($keyMap[$j * $WIDTH + $i][1] = "" ? "None" : $keyMap[$j * $WIDTH + $i][1]))
		Next
	Next
EndFunc

Func UpdateBtnLabelsRgb(ByRef $data)
	Local $rgb[3]
	Local $hsl[3]
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			$rgb[0] = $data[$j * $WIDTH + $i][0]
			$rgb[1] = $data[$j * $WIDTH + $i][1]
			$rgb[2] = $data[$j * $WIDTH + $i][2]
			$hsl = RgbToHsl($rgb)
			; $hsl[2] = (100 - 10) * ($hsl[2] - 0 / 100 - 0) + 10
			$rgb = HslToRgb($hsl)
			GUICtrlSetBkColor($idButtonBtns[$j * $WIDTH + $i], $rgb[0] * 256 * 256 + _
															   $rgb[1] * 256 + _
															   $rgb[2])
		Next
	Next
EndFunc

Func ShowBindingGroup($state)
	$state = $state ? $GUI_SHOW : $GUI_HIDE
	GUICtrlSetState($idGroupBinding, $state)
	GUICtrlSetState($idLabelCurrentlyBinding, $state)
	GUICtrlSetState($idLabelBindingArrow, $state)
	GUICtrlSetState($idInputKeyUp, $state)
	GUICtrlSetState($idInputKeyDown, $state)
	GUICtrlSetState($idButtonConfirm, $state)
	GUICtrlSetState($idButtonCancel, $state)
EndFunc

Func OpenGui()
	If $guiOpened Then Return WinActivate("THE Keypad Control Panel")
	$hGui = GUICreate("THE Keypad Control Panel", 750, 500, Default, Default, Default, $WS_EX_TOPMOST)
	$guiOpened = True
	GUICtrlCreateGroup("Buttons", 50, 30, _
								  15 + 60 + 85 * 3 + 15, _
								  15 + 60 + 85 * 2 + 15)
		For $j = 0 To $HEIGHT - 1
			For $i = 0 To $WIDTH - 1
				$idButtonBtns[$j * $WIDTH + $i] = GUICtrlCreateButton(($j * $WIDTH + $i + 1) & @CRLF & _
																	  ($keyMap[$j * $WIDTH + $i][1] = "" ? "None" : $keyMap[$j * $WIDTH + $i][1]), _
																	  50 + 15 + $i * 85, _
																	  30 + 15 + $j * 85, _
																	  60, 60, $BS_MULTILINE)
			Next
		Next
	GUICtrlCreateGroup("", -99, -99, 1, 1)	
	$idGroupBinding = GUICtrlCreateGroup("Binding", (50 + 15 + 60 + 85 * 3 + 15) + 15, 30, _
													15 + 100 + 15, _
													15 + 15 + 20 + 8 + 20 + 25 + 25 + 8 + 25 + 15)
		$idLabelCurrentlyBinding = GUICtrlCreateLabel("Binding key 1", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
																	   30 + 15, _
																	   100, 15)
		$idLabelBindingArrow = GUICtrlCreateLabel("=>", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
													    30 + 15 + 30, _
													    15, 15)
		$idInputKeyUp = GUICtrlCreateInput("{UP up}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
													  30 + 15 + 15, _
													  75, 20)
		$idInputKeyDown = GUICtrlCreateInput("{UP down}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
													  30 + 15 + 15 + 20 + 8, _
													  75, 20)
		$idButtonConfirm = GUICtrlCreateButton("Confirm", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
														  30 + 15 + 15 + 20 + 8 + 20 + 25, _
														  100, 25)
		$idButtonCancel = GUICtrlCreateButton("Cancel", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
														  30 + 15 + 15 + 20 + 8 + 20 + 25 + 25 + 8, _
														  100, 25)
	GUICtrlCreateGroup("", -99, -99, 1, 1)
	ShowBindingGroup(0)
	GUICtrlCreateGroup("Actions", 750 - 50 - 15 - 100 - 15, _
								  0 + 30, _
								  15 + 100 + 15, _
								  15 + 15 + 25 * 1 + 15)
		$idRadioBind = GUICtrlCreateRadio("Bind to new keys", 750 - 50 - 15 - 100, 30 + 15, 100, 15)
			GUICtrlSetState($idRadioBind, $GUI_CHECKED)
		$idRadioRemove = GUICtrlCreateRadio("Remove binding", 750 - 50 - 15 - 100, 30 + 15 + 15 + 10, 100, 15)
	GUICtrlCreateGroup("", -99, -99, 1, 1)    
	$idButtonClose = GUICtrlCreateButton("Close the driver", 750 - 25 - 150, _
															 500 - 25 - 25, _
															 150, 25)
		GUICtrlSetColor($idButtonClose, 0xFF0000)
	$idButtonSave = GUICtrlCreateButton("Save to config", 750 - 25 - 150 + 25, _
															 500 - 25 - 25 - 25 - 5, _
															 100, 25)
	$idComboRgbState = GUICtrlCreateCombo("lightWhenPressed", 50, 500 - 25 - 25 - 5 - 25, 150, 25)
		GUICtrlSetData($idComboRgbState, _ArrayToString($rgbStates, "|", 1))
	$idButtonRgbUpdate = GUICtrlCreateButton("Update", 50 + 25, 500 - 25 - 25, 100, 25)
	GUISetState(@SW_SHOW)
	$timerGuiBtnRgbSync = TimerInit()
	_CommClearInputBuffer()
	SendMsgToKeypad($GETRGBDATA, 0)
	$waitingForSyncingBytes = 3 * $WIDTH * $HEIGHT
	$syncingButtonIndex = 0
	$syncingRgbIndex = 0
EndFunc

Func RgbToHsl($rgb)
	Local $blue = $rgb[2]
	Local $green = $rgb[1]
	Local $red = $rgb[0]
	Local $luminance
	Local $hue
	Local $saturation
	$red = $red / 255
	$green = $green / 255
	$blue = $blue / 255
	Local $vmax = ((($red > $green) ? $red : $green) > $blue) ? (($red > $green) ? $red : $green) : $blue
	Local $vmin = ((($red > $green) ? $green : $red) > $blue) ? $blue : (($red > $green) ? $green : $red)
	Local $delta = $vmax - $vmin
	Local $luminance = ($vmax + $vmin) / 2
	If $delta > 0 Then
	   Local $dr = ((($vmax - $red) / 6.0) + ($delta / 2.0)) / $delta
	   Local $dg = ((($vmax - $green) / 6.0) + ($delta / 2.0)) / $delta
	   Local $db = ((($vmax - $blue) / 6.0) + ($delta / 2.0)) / $delta
	   If $red >= $green And $red >= $blue Then
		$hue = $db - $dg
	   ElseIf $green >= $red And $green >= $blue Then
		$hue = (1.0 / 3.0) + $dr - $db
	   Else
		$hue = (2.0 / 3.0) + $dg - $dr
		 EndIf
	 
	   If $hue < 0.0 Then
		$hue = $hue + 1
	   ElseIf $hue > 1.0 Then
		$hue = $hue - 1
		 EndIf
	 
	   If $luminance < 0.5 Then
		$saturation = $delta / ($vmax + $vmin)
	   Else
		$saturation = $delta / (2 - $vmax - $vmin)
		EndIf
	Else
	   $hue = 0
	   $saturation = 0
	  EndIf
	  Local $hsl[3] = [$hue,$saturation,$luminance]
	Return $hsl
EndFunc

Func HslToRgb($hsl)
	Local $luminance = $hsl[2]
	Local $saturation = $hsl[1]
	Local $hue = $hsl[0]
	Local $temp1 = 0
	Local $temp2 = 0
	Local $light = 0
	If $saturation<=0 Then
		If $luminance < 0 Then
			$light = 0
		ElseIf $luminance >= 1.0 Then
			$light = 255
		Else
			$light = Round($luminance * 255)
		EndIf
		Local $Color[3] = [$light, $light, $light]
		Return $Color
	EndIf
	 
	If $luminance < 0.5 Then
		$temp2 = $luminance * (1.0 + $saturation)
	Else
		$temp2 = $luminance + $saturation - ($luminance * $saturation)
	EndIf
	$temp1 = 2.0 * $luminance - $temp2
	Local $rgb[3] = [Round(GetHue($temp1, $temp2, $hue + 1.0 / 3.0) * 255), Round(GetHue($temp1, $temp2, $hue) * 255), Round(GetHue($temp1, $temp2, $hue - 1.0 / 3.0) * 255)]
	Return $rgb
EndFunc

Func GetHue($temp1, $temp2, $hue)
	If $hue < 0.0 Then
	   $hue = $hue+1
	ElseIf $hue > 1.0 Then
	   $hue =$hue-1
	EndIf
	 
	If $hue * 6.0 < 1.0 Then
		Return  $temp1 + ($temp2 - $temp1) * $hue * 6.0
	ElseIf ($hue * 2.0) < 1.0 Then
		Return  $temp2
	ElseIf $hue * 3.0 < 2.0 Then
		Return $temp1 + ($temp2 - $temp1) * (2.0 / 3.0 - $hue) * 6.0
	Else
		Return $temp1
	EndIf
EndFunc

Func ArrayFind(ByRef $a, $v)
	For $i = 0 To UBound($a) - 1
		If $a[$i] = $v Then
			Return $i
		EndIf
	Next
	Return -1
EndFunc

Func CloseGui()
	$waitingForSyncingBytes = 0
	_CommClearInputBuffer()
	GUIDelete($hGui)
	$guiOpened = False
EndFunc

Func BindRemove($num)
	$keyMap[$num - 1][0] = ""
	$keyMap[$num - 1][1] = ""
EndFunc

Func BindKey($num, $key, $extra = 0x0)
	If $num > $WIDTH * $HEIGHT Then Return
	Switch @NumParams
		Case 2
			$keyMap[$num - 1][0] = "{" & $key & " up}"
			$keyMap[$num - 1][1] = "{" & $key & " down}"
		Case 3
			$keyMap[$num - 1][0] = $key
			$keyMap[$num - 1][1] = $extra
	EndSwitch
EndFunc

Func Terminate()
	Exit
EndFunc
#include "..\LibDebug.au3"
#include <CommMG.au3>
#include <StringConstants.au3>
#include <GDIPlus.au3>
#include <GUIConstantsEx.au3>
#include <ButtonConstants.au3>

Global Const $WIDTH = 4, $HEIGHT = 3
Global $byteString = ""
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
Global $guiOpened = False
Global $gdiPlusStarted = False
Global $hGui
Global $idButtonBtns[$WIDTH * $HEIGHT]
Global $idButtonClose, $idRadioBind, $idRadioRemove, $idButtonSave
; Global $accelerators[73][2] = [["a",0],["b",0],["c",0],["d",0],["e",0],["f",0],["g",0],["h",0],["i",0],["j",0],["k",0],["l",0],["m",0],["n",0],["o",0],["p",0],["q",0],["r",0],["s",0],["t",0],["u",0],["v",0],["w",0],["x",0],["y",0],["z",0],["{SPACE}",0],["{ENTER}",0],["{BACKSPACE}",0],["{DELETE}",0],["{UP}",0],["{DOWN}",0],["{LEFT}",0],["{RIGHT}",0],["{HOME}",0],["{END}",0],["{ESC}",0],["{INSERT}",0],["{PGUP}",0],["{PGDN}",0],["{F1}",0],["{F2}",0],["{F3}",0],["{F4}",0],["{F5}",0],["{F6}",0],["{F7}",0],["{F8}",0],["{F9}",0],["{F10}",0],["{F11}",0],["{F12}",0],["{TAB}",0],["{PRINTSCREEN}",0],["{NUMLOCK}",0],["{CAPSLOCK}",0],["{SCROLLLOCK}",0],["{PAUSE}",0],["{NUMPAD0}",0],["{NUMPAD1}",0],["{NUMPAD2}",0],["{NUMPAD3}",0],["{NUMPAD4}",0],["{NUMPAD5}",0],["{NUMPAD6}",0],["{NUMPAD7}",0],["{NUMPAD8}",0],["{NUMPAD9}",0],["{NUMPADMULT}",0],["{NUMPADSUB}",0],["{NUMPADDIV}",0],["{NUMPADDOT}",0],["{NUMPADENTER}",0]]
Global $msg
Global $bindingKeys = False
Global $currentlyBinding = 0
Global $idGroupBinding, $idLabelCurrentlyBinding, $idLabelBindingArrow, $idLabelBindingStr, $idInputKeyUp, $idInputKeyDown
Global $idButtonConfirm, $idButtonCancel
Global Enum $BIND, $REMOVE
Global $bindingAction = $BIND
Global Const $iniPath = @ScriptDir & "\keypadconfig.ini"
HotKeySet("{F4}", "OpenGui")
Opt("GUICloseOnESC", 0)

Func Main()
	_CommSetDllPath(@ScriptDir & "/commg.dll")
	$ports = _ComGetPortNames()
	For $i = 0 To UBound($ports) - 1
		If $ports[$i][1] == "USB-SERIAL CH340" Then
			_CommSetPort(Int(StringReplace($ports[$i][0], "COM", "")), "", 19200, 8, "none", 1, 2)
			If @error Then
				c("Cannot connect to com port: $", 1, $ports[$i][0])
				ce(@error)
				Exit
			Else
				c("Connected to com port: $", 1, $ports[$i][0])
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
		BindKey(11, "y")
		BindKey(12, "u")
	EndIf
	; Global $t = 0
	; Global $tt = 0
	While 1
		$loopStartTime = TimerInit()
		If (TimerDiff($timer) >= ($msPerScan - ($loopPeriod > $msPerScan ? $msPerScan : $loopPeriod))) Then
			PollKeys()
			; If TimerDiff($tt) >= 1000 Then
				; $tt = TimerInit()
				; c($t)
				; $t = 0
			; EndIf
			; $t += 1
			If $guiOpened Then
				HandleMsg()
			EndIf
			$timer = TimerInit()
			$loopPeriod = TimerDiff($loopStartTime)
		EndIf
	WEnd
EndFunc

Main()

Func PollKeys()
	$byteString = _CommReadByte()
	If $byteString <> "" Then
		$byte = Int($byteString)
		$pressedBtnNum = BitShift($byte, 4)
		$pressedBtnState = BitAnd($byte, 0x01)
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
				; GUISetAccelerators(0, $hGui)
			EndIf
		Case Else
			For $j = 0 To $HEIGHT - 1
				For $i = 0 To $WIDTH - 1
					If $msg = $idButtonBtns[$j * $WIDTH + $i] Then
						Switch $bindingAction
							Case $BIND
								$bindingKeys = True
								$currentlyBinding = $j * $WIDTH + $i + 1
								GUICtrlSetData($idLabelCurrentlyBinding, "Binding key " & $currentlyBinding)
								; GUICtrlSetData($idLabelBindingStr, $keyMap[$j * $WIDTH + $i][0])
								GUICtrlSetData($idInputKeyUp, $keyMap[$j * $WIDTH + $i][0])
								GUICtrlSetData($idInputKeyDown, $keyMap[$j * $WIDTH + $i][1])
								ShowBindingGroup(1)
								; GUISetAccelerators($accelerators, $hGui)
							Case $REMOVE
								BindRemove($j * $WIDTH + $i + 1)
								UpdateBtnLabels()
						EndSwitch
						Return
					EndIf
				Next
			Next
			If $bindingKeys Then
				; For $i = 0 To UBound($accelerators, 1) - 1
					; If $msg = $accelerators[$i][1] Then
						; c($msg)
						; GUICtrlSetData($idLabelBindingStr, $accelerators[$i][0])
						; Return
					; EndIf
				; Next
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

Func UpdateBtnLabels()
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			GUICtrlSetData($idButtonBtns[$j * $WIDTH + $i], ($j * $WIDTH + $i + 1) & @CRLF & _
															($keyMap[$j * $WIDTH + $i][1] = "" ? "None" : $keyMap[$j * $WIDTH + $i][1]))
		Next
	Next
EndFunc

Func ShowBindingGroup($state)
	$state = $state ? $GUI_SHOW : $GUI_HIDE
	GUICtrlSetState($idGroupBinding, $state)
	GUICtrlSetState($idLabelCurrentlyBinding, $state)
	GUICtrlSetState($idLabelBindingArrow, $state)
	; GUICtrlSetState($idLabelBindingStr, $state)
	GUICtrlSetState($idInputKeyUp, $state)
	GUICtrlSetState($idInputKeyDown, $state)
	GUICtrlSetState($idButtonConfirm, $state)
	GUICtrlSetState($idButtonCancel, $state)
EndFunc

Func OpenGui()
	If $guiOpened Then Return WinActivate("THE Keypad Control Panel")
	$hGui = GUICreate("THE Keypad Control Panel", 750, 500)
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
		; $idLabelBindingStr = GUICtrlCreateLabel("{UP down}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
														 ; 30 + 15 + 15, _
														 ; 75, 15)
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
	; For $i = 0 To UBound($accelerators, 1) - 1
		; $accelerators[$i][1] = GUICtrlCreateDummy()
	; Next
	GUISetState(@SW_SHOW)
EndFunc

Func CloseGui()
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
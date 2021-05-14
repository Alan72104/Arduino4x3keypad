#include <Array.au3>
#include <ButtonConstants.au3>
#include <GUIConstantsEx.au3>
#include <StringConstants.au3>
#include "Include\CommMG.au3"
#include "KeypadDriver.Vars.au3"

Global $guiOpened = False
Global $hGui
Global $msg

Global $idButtonBtns[$WIDTH * $HEIGHT]

Global Enum $BIND, $REMOVE
Global $bindingAction = $BIND
Global $bindingKeys = False
Global $currentlyBinding = 0
Global $idGroupBinding, $idLabelCurrentlyBinding, $idLabelBindingArrow, $idLabelBindingStr, $idInputKeyUp, $idInputKeyDown, $idButtonConfirm, $idButtonCancel

Global $idRadioBind, $idRadioRemove

Global $rgbStates = ["lightWhenPressed","rainbow","spreadLightsOutWhenPressed","breathing","fractionalDrawingTest2d","spinningRainbow","waterWave","antiWaterWave"]
Global $idComboRgbState, $idButtonRgbUpdate, $idButtonRgbIncreaseBrightness, $idButtonRgbDecreaseBrightness

Global $idLabelConnection

Global $idButtonClose, $idButtonSave

Global Enum $UPDATERGBSTATE, $GETRGBDATA, $INCREASERGBBRIGHTNESS, $DECREASERGBBRIGHTNESS

Global $waitingForSyncingBytes = 0, $receivedByte = False, $timerGuiBtnRgbSync
Global $syncingButtonIndex = 0
Global $syncingRgbIndex = 0
Global $rgbBuffer[$WIDTH * $HEIGHT][3]

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
		Case $idButtonRgbIncreaseBrightness
			SendMsgToKeypad($INCREASERGBBRIGHTNESS, 0)
		Case $idButtonRgbDecreaseBrightness
			SendMsgToKeypad($DECREASERGBBRIGHTNESS, 0)
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
	Switch $connectionStatus
		Case $NOTCONNECTED
			GUICtrlSetData($idLabelConnection, "Not connected, retrying...")
		Case $CONNECTIONFAILED
			GUICtrlSetData($idLabelConnection, "Cannot connect to " & $comPort & ", retrying...")
		Case $PORTDETECTIONFAILED
			GUICtrlSetData($idLabelConnection, "COM port auto detection failed, please select the port manually")
		Case $CONNECTED
			GUICtrlSetData($idLabelConnection, "Connected to " & $comPort)
	EndSwitch
EndFunc

Func SyncGuiRgb()
	If $connectionStatus <> $CONNECTED Then Return
	Local $timer = 0
	If TimerDiff($timerGuiBtnRgbSync) > 150 Then
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
			If TimerDiff($timer) > 200 Then
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
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			GUICtrlSetBkColor($idButtonBtns[$j * $WIDTH + $i], $data[$j * $WIDTH + $i][0] * 256 * 256 + _
															   $data[$j * $WIDTH + $i][1] * 256 + _
															   $data[$j * $WIDTH + $i][2])
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
	GUICtrlCreateGroup("RGB Controls", 50, (30 + 15 + 60 + 85 * 2 + 15) + 15, _
										   15 + 150 + 15, _
										   15 + 25 + 8 + 25 + 15)
		$idComboRgbState = GUICtrlCreateCombo("lightWhenPressed", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15, 150, 25)
			GUICtrlSetData($idComboRgbState, _ArrayToString($rgbStates, "|", 1))
		$idButtonRgbUpdate = GUICtrlCreateButton("Update", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 100, 25)
		$idButtonRgbIncreaseBrightness = GUICtrlCreateButton("+", 50 + 15 + 100 + 10 , (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 15, 25)
		$idButtonRgbDecreaseBrightness = GUICtrlCreateButton("-", 50 + 15 + 100 + 10 + 15 + 10, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 15, 25)
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
	$idLabelConnection = GUICtrlCreateLabel("Not connected, retrying...", 50, 500 - 25 - 15, 200, 15)
	GUISetState(@SW_SHOW)
	$timerGuiBtnRgbSync = TimerInit()
	_CommClearInputBuffer()
	SendMsgToKeypad($GETRGBDATA, 0)
	$waitingForSyncingBytes = 3 * $WIDTH * $HEIGHT
	$syncingButtonIndex = 0
	$syncingRgbIndex = 0
EndFunc

Func CloseGui()
	$waitingForSyncingBytes = 0
	_CommClearInputBuffer()
	GUIDelete($hGui)
	$guiOpened = False
EndFunc

Func ArrayFind(ByRef $a, $v)
	For $i = 0 To UBound($a) - 1
		If $a[$i] = $v Then
			Return $i
		EndIf
	Next
	Return -1
EndFunc
; ================================================================================
;
; KeypadDriver.Gui.au3
; This file contains the functions required to display the gui
;
; ================================================================================

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

Global $rgbStates = ["staticLight","rainbow","spreadOut","breathing","fractionalDrawingTest2d","spinningRainbow","ripple","antiRipple","stars","raindrop"]
Global $idComboRgbState, $idButtonRgbUpdate, $idButtonRgbIncreaseBrightness, $idButtonRgbDecreaseBrightness

Global $idLabelConnection

Global $idButtonClose, $idButtonSave

Global Enum $UPDATERGBSTATE, $GETRGBDATA, $INCREASERGBBRIGHTNESS, $DECREASERGBBRIGHTNESS

Global $waitingForSyncingBytes = 0, $receivedByte = False, $timerGuiBtnRgbSync
Global $syncingButtonIndex = 0
Global $syncingRgbIndex = 0
Global $rgbBuffer[$WIDTH * $HEIGHT][3]

; This function handles the gui messages and performs the actions
Func HandleMsg()
	$msg = GUIGetMsg()
	Switch $msg
		; If no message to handle then return instantly
		Case 0
			Return
		
		; The gui "x" button
		Case $GUI_EVENT_CLOSE
			CloseGui()
		
		; The "Close the driver" button
		Case $idButtonClose
			CloseGui()
			Terminate()
		
		; The "Save to config" button
		Case $idButtonSave
			For $i = 1 To $WIDTH * $HEIGHT
				IniWrite($iniPath, "ButtonBindings", "Button" & $i & "Up", String($keyMap[$i - 1][0]))
				IniWrite($iniPath, "ButtonBindings", "Button" & $i & "Down", String($keyMap[$i - 1][1]))
			Next
		
		; The binding action selectors
		Case $idRadioBind
			$bindingAction = $BIND
		Case $idRadioRemove
			$bindingAction = $REMOVE
			If $bindingKeys Then
				$bindingKeys = False
				ShowBindingGroup(0)
			EndIf
		
		; The rgb "Update" button
		Case $idButtonRgbUpdate
			SendMsgToKeypad($UPDATERGBSTATE, ArrayFind($rgbStates, GUICtrlRead($idComboRgbState)))
		
		; The rgb brightness control buttons
		Case $idButtonRgbIncreaseBrightness
			SendMsgToKeypad($INCREASERGBBRIGHTNESS, 0)
		Case $idButtonRgbDecreaseBrightness
			SendMsgToKeypad($DECREASERGBBRIGHTNESS, 0)
		
		; Manually handle the other messages
		Case Else
			; The key buttons
			For $j = 0 To $HEIGHT - 1
				For $i = 0 To $WIDTH - 1
					If $msg = $idButtonBtns[$j * $WIDTH + $i] Then
						Switch $bindingAction
							; Open the "Binding" group for the specific key
							Case $BIND
								$bindingKeys = True
								$currentlyBinding = $j * $WIDTH + $i + 1
								GUICtrlSetData($idLabelCurrentlyBinding, "Binding key " & $currentlyBinding)
								GUICtrlSetData($idInputKeyUp, $keyMap[$j * $WIDTH + $i][0])
								GUICtrlSetData($idInputKeyDown, $keyMap[$j * $WIDTH + $i][1])
								ShowBindingGroup(1)
							
							; Remove the bindings for the specific key
							Case $REMOVE
								BindRemove($j * $WIDTH + $i + 1)
								UpdateBtnLabels()
						EndSwitch
						Return
					EndIf
				Next
			Next
			
			; If the "Binding" group is active then handle the binding update buttons
			If $bindingKeys Then
				; The binding "Confirm" button, updates the key to new bindings
				If $msg = $idButtonConfirm Then
					BindKey($currentlyBinding, GUICtrlRead($idInputKeyUp), GUICtrlRead($idInputKeyDown))
					UpdateBtnLabels()
					$bindingKeys = False
					ShowBindingGroup(0)
				
				; The binding "Cancel" button, closes the "Binding" group
				ElseIf $msg = $idButtonCancel Then
					$bindingKeys = False
					ShowBindingGroup(0)
				EndIf
			EndIf
	EndSwitch
	
	; Updates the connection indicator
	Switch $connectionStatus
		Case $NOTCONNECTED
			GUICtrlSetData($idLabelConnection, "Not connected, detecting the port...")
		Case $CONNECTIONFAILED
			GUICtrlSetData($idLabelConnection, "Cannot connect to " & $comPort & ", retrying...")
		Case $PORTDETECTIONFAILED
			GUICtrlSetData($idLabelConnection, "COM port auto detection failed, please make sure you have the keypad plugged in!")
		Case $CONNECTED
			GUICtrlSetData($idLabelConnection, "Connected to " & $comPort)
	EndSwitch
EndFunc

; This function retrieves the rgb info from the keypad and syncs them to the gui
Func SyncGuiRgb()
	If $connectionStatus <> $CONNECTED Then Return
	Local $timer = 0
	If TimerDiff($timerGuiBtnRgbSync) > 150 Then
		$timerGuiBtnRgbSync = TimerInit()
		
		; Clear the serial input buffer in case there are still some scrapped bytes
		_CommClearInputBuffer()
		SendMsgToKeypad($GETRGBDATA, 0)
		$waitingForSyncingBytes = 3 * $WIDTH * $HEIGHT
		$syncingButtonIndex = 0
		$syncingRgbIndex = 0
		$timer = TimerInit()
		
		; Constantly poll the bytes from serial until all the rgb infos have been received
		; One button consists of a RGB value, a RGB value consists of 3 bytes for R, G and B
		While 1
			Do
				PollData()
			Until $receivedByte
			$receivedByte = False
			$rgbBuffer[$syncingButtonIndex][$syncingRgbIndex] = $byte
			$syncingRgbIndex += 1
			
			; If 3 bytes have been received, switch to the next button
			If $syncingRgbIndex = 3 Then
				$syncingRgbIndex = 0
				$syncingButtonIndex += 1
			EndIf
			
			; If all the buttons' rgb have been received, update the key buttons' colors and return
			If $syncingButtonIndex = $WIDTH * $HEIGHT Then
				UpdateBtnLabelsRgb($rgbBuffer)
				Return
			EndIf
			
			; Watch out for timeouts that could potentially freeze the script
			If TimerDiff($timer) > 200 Then
				Return
			EndIf
		WEnd
	EndIf
EndFunc

; This function updates the key buttons' text to their "keyStrokeDown"s
Func UpdateBtnLabels()
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			GUICtrlSetData($idButtonBtns[$j * $WIDTH + $i], ($j * $WIDTH + $i + 1) & @CRLF & _
															($keyMap[$j * $WIDTH + $i][1] = "" ? "None" : $keyMap[$j * $WIDTH + $i][1]))
		Next
	Next
EndFunc

; This function updates the background colors of the key buttons
Func UpdateBtnLabelsRgb(ByRef $data)
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			GUICtrlSetBkColor($idButtonBtns[$j * $WIDTH + $i], $data[$j * $WIDTH + $i][0] * 256 * 256 + _
															   $data[$j * $WIDTH + $i][1] * 256 + _
															   $data[$j * $WIDTH + $i][2])
		Next
	Next
EndFunc

; This function shows or hides the "Binding" group and the inside controls
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

; This function creates the gui 
Func OpenGui()
	; If gui is already opened, activate the window
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
		$idComboRgbState = GUICtrlCreateCombo("staticLight", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15, 150, 25)
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
	$idLabelConnection = GUICtrlCreateLabel("Not connected, detecting the port...", 50, 500 - 25 - 15, 500, 15)
	
	; Shows the gui
	GUISetState(@SW_SHOW)
	
	$timerGuiBtnRgbSync = TimerInit()
EndFunc

; This function closes the gui
Func CloseGui()
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
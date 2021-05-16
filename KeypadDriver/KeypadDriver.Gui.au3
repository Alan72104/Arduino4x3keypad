; ================================================================================
;
; KeypadDriver.Gui.au3
; This file contains the functions required to display the gui
;
; ================================================================================

#include-once
#include <Array.au3>
#include <ButtonConstants.au3>
#include <GUIConstantsEx.au3>
#include <StringConstants.au3>
#include <WindowsConstants.au3>
#include "Include\CommMG.au3"
#include "KeypadDriver.au3"
#include "KeypadDriver.Vars.au3"
#include "KeypadDriver.Serial.au3"
#include "KeypadDriver.Keys.au3"

Global $_guiOpened = False
Global $_hGui
Global $_msg

Global $_idButtonBtns[$WIDTH * $HEIGHT]

Global Enum $_BIND, $_REMOVE
Global $_bindingAction = $_BIND
Global $_bindingKeys = False
Global $_currentlyBinding = 0
Global $_idGroupBinding, $_idLabelCurrentlyBinding, $_idLabelBindingArrow, $_idLabelBindingStr, $_idInputKeyUp, $_idInputKeyDown, $_idButtonConfirm, $_idButtonCancel

Global $_idRadioBind, $_idRadioRemove

Global $_idComboRgbState, $_idButtonRgbUpdate, $_idButtonRgbIncreaseBrightness, $_idButtonRgbDecreaseBrightness

Global $_idLabelConnection

Global $_idButtonClose, $_idButtonSave

Global Enum $_UPDATERGBSTATE, $_GETRGBDATA, $_INCREASERGBBRIGHTNESS, $_DECREASERGBBRIGHTNESS

Global $_timerGuiBtnRgbSync
Global $_syncingButtonIndex = 0
Global $_syncingRgbIndex = 0
Global $_rgbBuffer[$WIDTH * $HEIGHT][3]

; This function handles the gui messages and performs the actions
Func HandleMsg()
	$_msg = GUIGetMsg()
	Switch $_msg
		; If no message to handle then return instantly
		Case 0
			Return
		
		; The gui "x" button
		Case $GUI_EVENT_CLOSE
			CloseGui()
		
		; The "Close the driver" button
		Case $_idButtonClose
			CloseGui()
			Terminate()
		
		; The "Save to config" button
		Case $_idButtonSave
			ConfigSave()
		
		; The binding action selectors
		Case $_idRadioBind
			$_bindingAction = $_BIND
		Case $_idRadioRemove
			$_bindingAction = $_REMOVE
			If $_bindingKeys Then
				$_bindingKeys = False
				ShowBindingGroup(0)
			EndIf
		
		; The rgb "Update" button
		Case $_idButtonRgbUpdate
			SendMsgToKeypad($_UPDATERGBSTATE, ArrayFind($rgbStates, GUICtrlRead($_idComboRgbState)))
		
		; The rgb brightness control buttons
		Case $_idButtonRgbIncreaseBrightness
			SendMsgToKeypad($_INCREASERGBBRIGHTNESS, 0)
		Case $_idButtonRgbDecreaseBrightness
			SendMsgToKeypad($_DECREASERGBBRIGHTNESS, 0)
		
		; Manually handle the other messages
		Case Else
			; The key buttons
			For $j = 0 To $HEIGHT - 1
				For $i = 0 To $WIDTH - 1
					If $_msg = $_idButtonBtns[$j * $WIDTH + $i] Then
						Switch $_bindingAction
							; Open the "Binding" group for the specific key
							Case $_BIND
								$_bindingKeys = True
								$_currentlyBinding = $j * $WIDTH + $i + 1
								GUICtrlSetData($_idLabelCurrentlyBinding, "Binding key " & $_currentlyBinding)
								GUICtrlSetData($_idInputKeyUp, GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEUP))
								GUICtrlSetData($_idInputKeyDown, GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEDOWN))
								ShowBindingGroup(1)
							
							; Remove the bindings for the specific key
							Case $_REMOVE
								BindRemove($j * $WIDTH + $i + 1)
								UpdateBtnLabels()
						EndSwitch
						Return
					EndIf
				Next
			Next
			
			; If the "Binding" group is active then handle the binding update buttons
			If $_bindingKeys Then
				; The binding "Confirm" button, updates the key to new bindings
				If $_msg = $_idButtonConfirm Then
					BindKey($_currentlyBinding, GUICtrlRead($_idInputKeyUp), GUICtrlRead($_idInputKeyDown))
					UpdateBtnLabels()
					$_bindingKeys = False
					ShowBindingGroup(0)
				
				; The binding "Cancel" button, closes the "Binding" group
				ElseIf $_msg = $_idButtonCancel Then
					$_bindingKeys = False
					ShowBindingGroup(0)
				EndIf
			EndIf
	EndSwitch
	
	; Updates the connection indicator
	Switch $connectionStatus
		Case $NOTCONNECTED
			GUICtrlSetData($_idLabelConnection, "Not connected, detecting the port...")
		Case $CONNECTIONFAILED
			GUICtrlSetData($_idLabelConnection, "Cannot connect to " & GetComPort() & ", retrying...")
		Case $PORTDETECTIONFAILED
			GUICtrlSetData($_idLabelConnection, "COM port auto detection failed, please make sure you have the keypad plugged in!")
		Case $CONNECTED
			GUICtrlSetData($_idLabelConnection, "Connected to " & GetComPort())
	EndSwitch
EndFunc

; This function retrieves the rgb info from the keypad and syncs them to the gui
Func SyncGuiRgb()
	If $connectionStatus <> $CONNECTED Then Return
	Local $timer = 0
	If TimerDiff($_timerGuiBtnRgbSync) > 150 Then
		$_timerGuiBtnRgbSync = TimerInit()
		
		; Clear the serial input buffer in case there are still some scrapped bytes
		_CommClearInputBuffer()
		SendMsgToKeypad($_GETRGBDATA, 0)
		$_syncingButtonIndex = 0
		$_syncingRgbIndex = 0
		$timer = TimerInit()
		
		; Constantly poll the bytes from serial until all the rgb infos have been received
		; One button consists of a RGB value, a RGB value consists of 3 bytes for R, G and B
		While 1
			Do
				PollData()
			Until IsByteReceived()
			$_rgbBuffer[$_syncingButtonIndex][$_syncingRgbIndex] = GetByte()
			ByteProcessed()
			$_syncingRgbIndex += 1
			
			; If 3 bytes have been received, switch to the next button
			If $_syncingRgbIndex = 3 Then
				$_syncingRgbIndex = 0
				$_syncingButtonIndex += 1
			EndIf
			
			; If all the buttons' rgb have been received, update the key buttons' colors and return
			If $_syncingButtonIndex = $WIDTH * $HEIGHT Then
				UpdateBtnLabelsRgb($_rgbBuffer)
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
			GUICtrlSetData($_idButtonBtns[$j * $WIDTH + $i], ($j * $WIDTH + $i + 1) & @CRLF & _
															 GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEDOWN))
		Next
	Next
EndFunc

; This function updates the background colors of the key buttons
Func UpdateBtnLabelsRgb(ByRef $data)
	For $j = 0 To $HEIGHT - 1
		For $i = 0 To $WIDTH - 1
			GUICtrlSetBkColor($_idButtonBtns[$j * $WIDTH + $i], $data[$j * $WIDTH + $i][0] * 256 * 256 + _
															   $data[$j * $WIDTH + $i][1] * 256 + _
															   $data[$j * $WIDTH + $i][2])
		Next
	Next
EndFunc

; This function shows or hides the "Binding" group and the inside controls
Func ShowBindingGroup($state)
	$state = $state ? $GUI_SHOW : $GUI_HIDE
	GUICtrlSetState($_idGroupBinding, $state)
	GUICtrlSetState($_idLabelCurrentlyBinding, $state)
	GUICtrlSetState($_idLabelBindingArrow, $state)
	GUICtrlSetState($_idInputKeyUp, $state)
	GUICtrlSetState($_idInputKeyDown, $state)
	GUICtrlSetState($_idButtonConfirm, $state)
	GUICtrlSetState($_idButtonCancel, $state)
EndFunc

; This function creates the gui 
Func OpenGui()
	; If gui is already opened, activate the window
	If $_guiOpened Then Return WinActivate("THE Keypad Control Panel")
	
	$_hGui = GUICreate("THE Keypad Control Panel", 750, 500, Default, Default, Default, $WS_EX_TOPMOST)
	$_guiOpened = True
	GUICtrlCreateGroup("Buttons", 50, 30, _
								  15 + 60 + 85 * 3 + 15, _
								  15 + 60 + 85 * 2 + 15)
		For $j = 0 To $HEIGHT - 1
			For $i = 0 To $WIDTH - 1
				$_idButtonBtns[$j * $WIDTH + $i] = GUICtrlCreateButton(($j * $WIDTH + $i + 1) & @CRLF & _
																	  GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEDOWN), _
																	  50 + 15 + $i * 85, _
																	  30 + 15 + $j * 85, _
																	  60, 60, $BS_MULTILINE)
			Next
		Next
	GUICtrlCreateGroup("", -99, -99, 1, 1)
	GUICtrlCreateGroup("RGB Controls", 50, (30 + 15 + 60 + 85 * 2 + 15) + 15, _
										   15 + 150 + 15, _
										   15 + 25 + 8 + 25 + 15)
		$_idComboRgbState = GUICtrlCreateCombo("staticLight", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15, 150, 25)
			GUICtrlSetData($_idComboRgbState, _ArrayToString($rgbStates, "|", 1))
		$_idButtonRgbUpdate = GUICtrlCreateButton("Update", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 100, 25)
		$_idButtonRgbIncreaseBrightness = GUICtrlCreateButton("+", 50 + 15 + 100 + 10 , (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 15, 25)
		$_idButtonRgbDecreaseBrightness = GUICtrlCreateButton("-", 50 + 15 + 100 + 10 + 15 + 10, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 15, 25)
	GUICtrlCreateGroup("", -99, -99, 1, 1)
	$_idGroupBinding = GUICtrlCreateGroup("Binding", (50 + 15 + 60 + 85 * 3 + 15) + 15, 30, _
													15 + 100 + 15, _
													15 + 15 + 20 + 8 + 20 + 25 + 25 + 8 + 25 + 15)
		$_idLabelCurrentlyBinding = GUICtrlCreateLabel("Binding key 1", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
																	   30 + 15, _
																	   100, 15)
		$_idLabelBindingArrow = GUICtrlCreateLabel("=>", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
													    30 + 15 + 30, _
													    15, 15)
		$_idInputKeyUp = GUICtrlCreateInput("{UP up}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
													  30 + 15 + 15, _
													  75, 20)
		$_idInputKeyDown = GUICtrlCreateInput("{UP down}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
													  30 + 15 + 15 + 20 + 8, _
													  75, 20)
		$_idButtonConfirm = GUICtrlCreateButton("Confirm", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
														  30 + 15 + 15 + 20 + 8 + 20 + 25, _
														  100, 25)
		$_idButtonCancel = GUICtrlCreateButton("Cancel", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
														  30 + 15 + 15 + 20 + 8 + 20 + 25 + 25 + 8, _
														  100, 25)
	GUICtrlCreateGroup("", -99, -99, 1, 1)
	ShowBindingGroup(0)
	GUICtrlCreateGroup("Actions", 750 - 50 - 15 - 100 - 15, _
								  0 + 30, _
								  15 + 100 + 15, _
								  15 + 15 + 25 * 1 + 15)
		$_idRadioBind = GUICtrlCreateRadio("Bind to new keys", 750 - 50 - 15 - 100, 30 + 15, 100, 15)
			GUICtrlSetState($_idRadioBind, $GUI_CHECKED)
		$_idRadioRemove = GUICtrlCreateRadio("Remove binding", 750 - 50 - 15 - 100, 30 + 15 + 15 + 10, 100, 15)
	GUICtrlCreateGroup("", -99, -99, 1, 1)    
	$_idButtonClose = GUICtrlCreateButton("Close the driver", 750 - 25 - 150, _
															 500 - 25 - 25, _
															 150, 25)
		GUICtrlSetColor($_idButtonClose, 0xFF0000)
	$_idButtonSave = GUICtrlCreateButton("Save to config", 750 - 25 - 150 + 25, _
															 500 - 25 - 25 - 25 - 5, _
															 100, 25)
	$_idLabelConnection = GUICtrlCreateLabel("Not connected, detecting the port...", 50, 500 - 25 - 15, 500, 15)
	
	; Shows the gui
	GUISetState(@SW_SHOW)
	
	$_timerGuiBtnRgbSync = TimerInit()
EndFunc

; This function closes the gui
Func CloseGui()
	GUIDelete($_hGui)
	$_guiOpened = False
EndFunc

Func IsGuiOpened()
	Return $_guiOpened
EndFunc

Func ArrayFind(ByRef $a, $v)
	For $i = 0 To UBound($a) - 1
		If $a[$i] = $v Then
			Return $i
		EndIf
	Next
	Return -1
EndFunc
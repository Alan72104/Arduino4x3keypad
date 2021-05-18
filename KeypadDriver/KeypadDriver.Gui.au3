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
#include "KeypadDriver.Vars.au3"
#include "KeypadDriver.Serial.au3"
#include "KeypadDriver.Keys.au3"

Global $gui_guiOpened = False
Global $gui_hGui
Global $gui_msg

Global $gui_idButtonBtns[$WIDTH * $HEIGHT]

Global Enum $gui_BIND, $gui_REMOVE
Global $gui_bindingAction = $gui_BIND
Global $gui_bindingKeys = False
Global $gui_currentlyBinding = 0
Global $gui_idGroupBinding, $gui_idLabelCurrentlyBinding, $gui_idLabelBindingArrow, $gui_idLabelBindingStr, $gui_idInputKeyUp, $gui_idInputKeyDown, $gui_idButtonConfirm, $gui_idButtonCancel

Global $gui_idRadioBind, $gui_idRadioRemove

Global Enum $gui_MONITORRGB, $gui_MONITORKEYPRESS
Global $gui_monitoringType = $gui_MONITORRGB
Global $gui_idRadioMonitorRgb, $gui_idRadioMonitorKeypress

Global $gui_idComboRgbState, $gui_idButtonRgbUpdate, $gui_idButtonRgbIncreaseBrightness, $gui_idButtonRgbDecreaseBrightness

Global $gui_idLabelConnection

Global $gui_idButtonClose, $gui_idButtonSave

Global Enum $gui_UPDATERGBSTATE, $gui_GETRGBDATA, $gui_INCREASERGBBRIGHTNESS, $gui_DECREASERGBBRIGHTNESS

Global $gui_timerGuiBtnRgbSync
Global $gui_syncingButtonIndex = 0
Global $gui_syncingRgbIndex = 0
Global $gui_rgbBuffer[$WIDTH * $HEIGHT][3]

; This function handles the gui messages and performs actions accordingly
Func HandleMsg()
    $gui_msg = GUIGetMsg()
    Switch $gui_msg
        ; If no message to handle then simply skip
        Case 0
        
        ; The gui "x" button
        Case $GUI_EVENT_CLOSE
            CloseGui()
        
        ; The "Close the driver" button
        Case $gui_idButtonClose
            CloseGui()
            Return 1
        
        ; The "Save to config" button
        Case $gui_idButtonSave
            Return 2
        
        ; The binding action selectors
        Case $gui_idRadioBind
            $gui_bindingAction = $gui_BIND
        Case $gui_idRadioRemove
            $gui_bindingAction = $gui_REMOVE
            If $gui_bindingKeys Then
                $gui_bindingKeys = False
                ShowBindingGroup(0)
            EndIf
        
        ; The monitoring type selectors
        Case $gui_idRadioMonitorRgb
            $gui_monitoringType = $gui_MONITORRGB
        Case $gui_idRadioMonitorKeypress
            $gui_monitoringType = $gui_MONITORKEYPRESS
            For $j = 0 To $HEIGHT - 1
                For $i = 0 To $WIDTH - 1
                    UpdateBtnLabelRgb($j * $WIDTH + $i + 1, 255, 255, 255)
                Next
            Next
        
        ; The rgb "Update" button
        Case $gui_idButtonRgbUpdate
            SendMsgToKeypad($gui_UPDATERGBSTATE, ArrayFind($rgbStates, GUICtrlRead($gui_idComboRgbState)))
        
        ; The rgb brightness control buttons
        Case $gui_idButtonRgbIncreaseBrightness
            SendMsgToKeypad($gui_INCREASERGBBRIGHTNESS, 0)
        Case $gui_idButtonRgbDecreaseBrightness
            SendMsgToKeypad($gui_DECREASERGBBRIGHTNESS, 0)
        
        ; Manually handle the other messages
        Case Else
            ; The key buttons
            For $j = 0 To $HEIGHT - 1
                For $i = 0 To $WIDTH - 1
                    If $gui_msg = $gui_idButtonBtns[$j * $WIDTH + $i] Then
                        Switch $gui_bindingAction
                            ; Open the "Binding" group for the specific key
                            Case $gui_BIND
                                $gui_bindingKeys = True
                                $gui_currentlyBinding = $j * $WIDTH + $i + 1
                                GUICtrlSetData($gui_idLabelCurrentlyBinding, "Binding key " & $gui_currentlyBinding)
                                GUICtrlSetData($gui_idInputKeyUp, GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEUP))
                                GUICtrlSetData($gui_idInputKeyDown, GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEDOWN))
                                ShowBindingGroup(1)
                            
                            ; Remove the bindings for the specific key
                            Case $gui_REMOVE
                                BindRemove($j * $WIDTH + $i + 1)
                                UpdateBtnLabels()
                        EndSwitch
                        Return
                    EndIf
                Next
            Next
            
            ; If the "Binding" group is active then handle the binding update buttons
            If $gui_bindingKeys Then
                ; The binding "Confirm" button, updates the key to new bindings
                If $gui_msg = $gui_idButtonConfirm Then
                    BindKey($gui_currentlyBinding, GUICtrlRead($gui_idInputKeyUp), GUICtrlRead($gui_idInputKeyDown))
                    UpdateBtnLabels()
                    $gui_bindingKeys = False
                    ShowBindingGroup(0)
                
                ; The binding "Cancel" button, closes the "Binding" group
                ElseIf $gui_msg = $gui_idButtonCancel Then
                    $gui_bindingKeys = False
                    ShowBindingGroup(0)
                EndIf
            EndIf
    EndSwitch
    
    ; Update the connection indicator
    Switch $connectionStatus
        Case $NOTCONNECTED
            GUICtrlSetData($gui_idLabelConnection, "Not connected, detecting the port...")
        Case $CONNECTIONFAILED
            GUICtrlSetData($gui_idLabelConnection, "Cannot connect to " & GetComPort() & ", retrying...")
        Case $PORTDETECTIONFAILED
            GUICtrlSetData($gui_idLabelConnection, "COM port auto detection failed, please make sure you have the keypad plugged in!")
        Case $CONNECTED
            GUICtrlSetData($gui_idLabelConnection, "Connected to " & GetComPort())
    EndSwitch

    Return 0
EndFunc

; This function retrieves the rgb info from the keypad and syncs them to the gui
Func SyncGuiRgb()
    If $connectionStatus <> $CONNECTED Then Return
    Local $timer = 0
    If TimerDiff($gui_timerGuiBtnRgbSync) > 150 Then
        $gui_timerGuiBtnRgbSync = TimerInit()
        
        ; Clear the serial input buffer in case there are still some scrapped bytes
        _CommClearInputBuffer()
        SendMsgToKeypad($gui_GETRGBDATA, 0)
        $gui_syncingButtonIndex = 0
        $gui_syncingRgbIndex = 0
        $timer = TimerInit()
        
        ; Constantly poll the bytes from serial until all the rgb infos have been received
        ; One button consists of a RGB value, a RGB value consists of 3 bytes for R, G and B
        While 1
            Do
                PollData()
            Until IsByteReceived()
            $gui_rgbBuffer[$gui_syncingButtonIndex][$gui_syncingRgbIndex] = GetByte()
            ByteProcessed()
            $gui_syncingRgbIndex += 1
            
            ; If 3 bytes have been received, switch to the next button
            If $gui_syncingRgbIndex = 3 Then
                $gui_syncingRgbIndex = 0
                $gui_syncingButtonIndex += 1
            EndIf
            
            ; If all the buttons' rgb have been received, update the key buttons' colors and return
            If $gui_syncingButtonIndex = $WIDTH * $HEIGHT Then
                For $j = 0 To $HEIGHT - 1
                    For $i = 0 To $WIDTH - 1
                        UpdateBtnLabelRgb($j * $WIDTH + $i + 1, $gui_rgbBuffer[$j * $WIDTH + $i][0], $gui_rgbBuffer[$j * $WIDTH + $i][1], $gui_rgbBuffer[$j * $WIDTH + $i][2])
                    Next
                Next
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
            GUICtrlSetData($gui_idButtonBtns[$j * $WIDTH + $i], ($j * $WIDTH + $i + 1) & @CRLF & _
                                                                KeyHasBinding($j * $WIDTH + $i + 1, $KEYSTROKEDOWN) ? GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEDOWN) : "None")
        Next
    Next
EndFunc

; This function updates the background colors of the key buttons
Func UpdateBtnLabelRgb($num, $r, $g, $b)
    If $num <= $WIDTH * $HEIGHT Then
        GUICtrlSetBkColor($gui_idButtonBtns[$num - 1], $r * 256 * 256 + $g * 256 + $b)
    EndIf
EndFunc

; This function shows or hides the "Binding" group and the inside controls
Func ShowBindingGroup($state)
    $state = $state ? $GUI_SHOW : $GUI_HIDE
    GUICtrlSetState($gui_idGroupBinding, $state)
    GUICtrlSetState($gui_idLabelCurrentlyBinding, $state)
    GUICtrlSetState($gui_idLabelBindingArrow, $state)
    GUICtrlSetState($gui_idInputKeyUp, $state)
    GUICtrlSetState($gui_idInputKeyDown, $state)
    GUICtrlSetState($gui_idButtonConfirm, $state)
    GUICtrlSetState($gui_idButtonCancel, $state)
EndFunc

Func SetGuiOpeningKey($key)
    HotKeySet($key, "OpenGui")
EndFunc

; This function creates the gui 
Func OpenGui()
    ; If gui is already opened, activate the window
    If $gui_guiOpened Then Return WinActivate("THE Keypad Control Panel")
    
    $gui_hGui = GUICreate("THE Keypad Control Panel", 750, 500, Default, Default, Default, $WS_EX_TOPMOST)
    $gui_guiOpened = True

    ; vvvvvvvvvvvvvvvvvvvvvvvvv Group buttons vvvvvvvvvvvvvvvvvvvvvvvvv
    GUICtrlCreateGroup("Buttons", 50, 30, _
                                  15 + 60 + 85 * 3 + 15, _
                                  15 + 60 + 85 * 2 + 15)
        For $j = 0 To $HEIGHT - 1
            For $i = 0 To $WIDTH - 1
                $gui_idButtonBtns[$j * $WIDTH + $i] = GUICtrlCreateButton(($j * $WIDTH + $i + 1) & @CRLF & _
                                                                       KeyHasBinding($j * $WIDTH + $i + 1, $KEYSTROKEDOWN) ? GetKeybindingForKey($j * $WIDTH + $i + 1, $KEYSTROKEDOWN) : "None", _
                                                                       50 + 15 + $i * 85, _
                                                                       30 + 15 + $j * 85, _
                                                                       60, 60, $BS_MULTILINE)
            Next
        Next
    GUICtrlCreateGroup("", -99, -99, 1, 1)
    ; ^^^^^^^^^^^^^^^^^^^^^^^^^ Group buttons ^^^^^^^^^^^^^^^^^^^^^^^^^
    
    ; vvvvvvvvvvvvvvvvvvvvvvvvv Group rgb controls vvvvvvvvvvvvvvvvvvvvvvvvv
    GUICtrlCreateGroup("RGB Controls", 50, (30 + 15 + 60 + 85 * 2 + 15) + 15, _
                                           15 + 150 + 15, _
                                           15 + 25 + 8 + 25 + 15)
        $gui_idComboRgbState = GUICtrlCreateCombo("staticLight", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15, 150, 25)
            GUICtrlSetData($gui_idComboRgbState, _ArrayToString($rgbStates, "|", 1))
        $gui_idButtonRgbUpdate = GUICtrlCreateButton("Update", 50 + 15, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 100, 25)
        $gui_idButtonRgbIncreaseBrightness = GUICtrlCreateButton("+", 50 + 15 + 100 + 10 , (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 15, 25)
        $gui_idButtonRgbDecreaseBrightness = GUICtrlCreateButton("-", 50 + 15 + 100 + 10 + 15 + 10, (30 + 15 + 60 + 85 * 2 + 15) + 15 + 15 + 25 + 8, 15, 25)
    GUICtrlCreateGroup("", -99, -99, 1, 1)
    ; ^^^^^^^^^^^^^^^^^^^^^^^^^ Group rgb controls ^^^^^^^^^^^^^^^^^^^^^^^^^

    ; vvvvvvvvvvvvvvvvvvvvvvvvv Group binding vvvvvvvvvvvvvvvvvvvvvvvvv
    $gui_idGroupBinding = GUICtrlCreateGroup("Binding", (50 + 15 + 60 + 85 * 3 + 15) + 15, 30, _
                                                        15 + 100 + 15, _
                                                        15 + 15 + 20 + 8 + 20 + 25 + 25 + 8 + 25 + 15)
        $gui_idLabelCurrentlyBinding = GUICtrlCreateLabel("Binding key 1", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
                                                                           30 + 15, _
                                                                           100, 15)
        $gui_idLabelBindingArrow = GUICtrlCreateLabel("=>", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
                                                            30 + 15 + 30, _
                                                            15, 15)
        $gui_idInputKeyUp = GUICtrlCreateInput("{UP up}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
                                                          30 + 15 + 15, _
                                                          75, 20)
        $gui_idInputKeyDown = GUICtrlCreateInput("{UP down}", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15 + 10 + 15, _
                                                              30 + 15 + 15 + 20 + 8, _
                                                              75, 20)
        $gui_idButtonConfirm = GUICtrlCreateButton("Confirm", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
                                                              30 + 15 + 15 + 20 + 8 + 20 + 25, _
                                                              100, 25)
        $gui_idButtonCancel = GUICtrlCreateButton("Cancel", (50 + 15 + 60 + 85 * 3 + 15) + 15 + 15, _
                                                            30 + 15 + 15 + 20 + 8 + 20 + 25 + 25 + 8, _
                                                            100, 25)
    GUICtrlCreateGroup("", -99, -99, 1, 1)
    ShowBindingGroup(0)
    ; ^^^^^^^^^^^^^^^^^^^^^^^^^ Group binding ^^^^^^^^^^^^^^^^^^^^^^^^^

    ; vvvvvvvvvvvvvvvvvvvvvvvvv Group actions vvvvvvvvvvvvvvvvvvvvvvvvv
    GUICtrlCreateGroup("Actions", 750 - 50 - 15 - 100 - 15, _
                                  0 + 30, _
                                  15 + 100 + 15, _
                                  15 + 15 + 25 * 1 + 15)
        $gui_idRadioBind = GUICtrlCreateRadio("Bind to new keys", 750 - 50 - 15 - 100, 30 + 15, 100, 15)
            GUICtrlSetState($gui_idRadioBind, $GUI_CHECKED)
        $gui_idRadioRemove = GUICtrlCreateRadio("Remove binding", 750 - 50 - 15 - 100, 30 + 15 + 15 + 10, 100, 15)
    GUICtrlCreateGroup("", -99, -99, 1, 1)
    ; ^^^^^^^^^^^^^^^^^^^^^^^^^ Group actions ^^^^^^^^^^^^^^^^^^^^^^^^^

    ; vvvvvvvvvvvvvvvvvvvvvvvvv Group monitoring vvvvvvvvvvvvvvvvvvvvvvvvv
    GUICtrlCreateGroup("Monitoring", 750 - 50 - 15 - 100 - 15, _
                                  (30 + 15 + 15 + 25 * 1 + 15) + 15, _
                                  15 + 100 + 15, _
                                  15 + 15 + 25 * 1 + 15)
        $gui_idRadioMonitorRgb = GUICtrlCreateRadio("RGB effect", 750 - 50 - 15 - 100, _
                                                                  (30 + 15 + 15 + 25 * 1 + 15) + 15 + 15, _
                                                                  100, 15)
            GUICtrlSetState($gui_idRadioMonitorRgb, $GUI_CHECKED)
        $gui_idRadioMonitorKeypress = GUICtrlCreateRadio("Key press", 750 - 50 - 15 - 100, _
                                                                      (30 + 15 + 15 + 25 * 1 + 15) + 15 + 15 + 15 + 10, _
                                                                      100, 15)
    GUICtrlCreateGroup("", -99, -99, 1, 1)
    ; ^^^^^^^^^^^^^^^^^^^^^^^^^ Group monitoring ^^^^^^^^^^^^^^^^^^^^^^^^^

    $gui_idButtonClose = GUICtrlCreateButton("Close the driver", 750 - 25 - 150, _
                                                                 500 - 25 - 25, _
                                                                 150, 25)
        GUICtrlSetColor($gui_idButtonClose, 0xFF0000)
    $gui_idButtonSave = GUICtrlCreateButton("Save to config", 750 - 25 - 150 + 25, _
                                                              500 - 25 - 25 - 25 - 5, _
                                                              100, 25)
    
    
    $gui_idLabelConnection = GUICtrlCreateLabel("Not connected, detecting the port...", 50, 500 - 25 - 15, 500, 15)
    
    ; Shows the gui
    GUISetState(@SW_SHOW)
    
    $gui_timerGuiBtnRgbSync = TimerInit()
EndFunc

; This function closes the gui
Func CloseGui()
    GUIDelete($gui_hGui)
    $gui_guiOpened = False
EndFunc

Func IsGuiOpened()
    Return $gui_guiOpened
EndFunc

Func IsMonitoringKeypress()
    Return $gui_monitoringType = $gui_MONITORKEYPRESS
EndFunc

Func ArrayFind(ByRef $a, $v)
    For $i = 0 To UBound($a) - 1
        If $a[$i] = $v Then
            Return $i
        EndIf
    Next
    Return -1
EndFunc
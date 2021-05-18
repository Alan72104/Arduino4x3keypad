; ================================================================================
;
; KeypadDriver.au3
; This main file runs the main loop, key binding functions and includes all the other modules 
;
; ================================================================================

#include-once
#include "Include\LibDebug.au3"
#include "Include\CommMG.au3"
#include "KeypadDriver.Vars.au3"
#include "KeypadDriver.Gui.au3"
#include "KeypadDriver.Serial.au3"
#include "KeypadDriver.Keys.au3"

Global $main_configPath = @ScriptDir & "\keypadconfig.ini"
Global Const $main_scansPerSec = 1000
Global Const $main_msPerScan = 1000 / $main_scansPerSec
Global $main_loopPeriod, $main_loopStartTime, $main_timer
Global $main_timerRetrying
	
SetGuiOpeningKey("{F4}")
Opt("GUICloseOnESC", 0)

Func Main()
    _CommSetDllPath(@ScriptDir & "\Include\commg.dll")
    If FileExists($main_configPath) Then  ; If the config exists then use the binding in it
        ConfigLoad($main_configPath)
    Else  ; If config doesn't exist then use the default binding
        BindKey(1, "ESC")
        BindKey(2, "`")
        BindKey(3, "c")
        BindKey(4, "", "!{UP}")
        BindKey(5, "", "^a")
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
    Connect()
    ; Local $t = 0
    ; Local $tt = 0
    While 1
        $main_loopStartTime = TimerInit()
        If (TimerDiff($main_timer) >= ($main_msPerScan - ($main_loopPeriod > $main_msPerScan ? $main_msPerScan : $main_loopPeriod))) Then
        
            ; Because retrieving the port list takes a while, so we don't reconnect too often
            If $connectionStatus <> $CONNECTED And TimerDiff($main_timerRetrying) > 5000 Then
                $main_timerRetrying = TimerInit()
                Connect()
            EndIf
        
            PollKeys()
            If IsKeyDataReceived() And Not IsGuiOpened() Then
                ; c("Button: $ pressed, state: $", 1, $_pressedBtnNum, $_pressedBtnState)
                SendKey(GetKeyDataNum(), GetKeyDataState())
            EndIf
            
            If IsGuiOpened() Then
                If IsMonitoringKeypress() Then
                    If IsKeyDataReceived() Then
                        UpdateBtnLabelRgb(GetKeyDataNum(), 255, GetKeyDataState() ? 0 : 255, GetKeyDataState() ? 0 : 255)
                    EndIf
                Else
                    SyncGuiRgb()
                EndIf

                ; HandleMsg() only handles gui related messages, returns extra messages if need to be explicitly handled
                Switch HandleMsg()
                    Case 0
                    Case 1
                        Terminate()
                    Case 2
                        ConfigSave($main_configPath)
                EndSwitch
            EndIf
                        
            ; Debug loop time and loop frequency output
            ; If TimerDiff($tt) >= 1000 Then
            ;     $tt = TimerInit()
            ;     c($t)
            ;     c($main_loopPeriod)
            ;     $t = 0
            ; EndIf
            ; $t += 1

            KeyDataProcessed()
            
            $main_timer = TimerInit()
            $main_loopPeriod = $main_loopPeriod * 0.6 + TimerDiff($main_loopStartTime) * 0.4  ; Don't modify the measured loop time immediately as it might float around
        EndIf
    WEnd
EndFunc

Main()

Func Terminate()
    Exit
EndFunc
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

SetGuiOpeningKey("{F4}")
Opt("GUICloseOnESC", 0)

Func Main()
    Local Const $_configPath = @ScriptDir & "\keypadconfig.ini"
    Local Const $_scansPerSec = 1500
    Local Const $_msPerScan = 1000 / $_scansPerSec
    Local $_loopPeriod, $_loopStartTime, $_timer
    Local $_timerRetrying

    _CommSetDllPath(@ScriptDir & "\Include\commg.dll")
    If FileExists($_configPath) Then  ; If the config exists then use the binding in it
        ConfigLoad($_configPath)
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
        $_loopStartTime = TimerInit()
        If (TimerDiff($_timer) >= ($_msPerScan - ($_loopPeriod > $_msPerScan ? $_msPerScan : $_loopPeriod))) Then
        
            ; Because retrieving the port list takes a while, so we don't reconnect too often
            If $connectionStatus <> $CONNECTED And TimerDiff($_timerRetrying) > 5000 Then
                $_timerRetrying = TimerInit()
                Connect()
            EndIf
        
            PollKeys()
            If IsKeyDataReceived() Then
                ; c("Button: $ pressed, state: $", 1, $_pressedBtnNum, $_pressedBtnState)
                SendKey(GetKeyDataNum(), GetKeyDataState())
                KeyDataProcessed()
            EndIf
            
            ; Debug loop time and loop frequency output
            ; If $debug Then
            ;     If TimerDiff($tt) >= 1000 Then
            ;         $tt = TimerInit()
            ;         c($t)
            ;         c($_loopPeriod)
            ;         $t = 0
            ;     EndIf
            ;     $t += 1
            ; EndIf
            
            If IsGuiOpened() Then
                SyncGuiRgb()
                ; HandleMsg() only handles gui related messages, returns extra messages if need to be explicitly handled
                Switch HandleMsg()
                    Case 0
                    Case 1
                        ConfigSave($_configPath)
                EndSwitch
            EndIf
            
            $_timer = TimerInit()
            $_loopPeriod = $_loopPeriod * 0.6 + TimerDiff($_loopStartTime) * 0.4  ; Don't modify the measured loop time immediately as it might float around
        EndIf
    WEnd
EndFunc

Main()

Func Terminate()
    Exit
EndFunc
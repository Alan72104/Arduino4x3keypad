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

Global Const $_scansPerSec = 1500
Global Const $_msPerScan = 1000 / $_scansPerSec
Global $_loopPeriod, $_loopStartTime, $_timer
Global $_timerRetrying

HotKeySet("{F4}", "OpenGui")
Opt("GUICloseOnESC", 0)

Global $debug = 0

Func Main()
	_CommSetDllPath(@ScriptDir & "\Include\commg.dll")
	If FileExists($iniPath) Then  ; If the config exists then use the binding in it
		ConfigLoad()
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
	If $debug Then
		Local $t = 0
		Local $tt = 0
	EndIf
	While 1
		$_loopStartTime = TimerInit()
		If (TimerDiff($_timer) >= ($_msPerScan - ($_loopPeriod > $_msPerScan ? $_msPerScan : $_loopPeriod))) Then
		
			; Because retrieving the port list takes a while, so we don't reconnect too often
			If $connectionStatus <> $CONNECTED And TimerDiff($_timerRetrying) > 5000 Then
				$_timerRetrying = TimerInit()
				Connect()
			EndIf
		
			If Not $debug Then
				PollKeys()
				If IsKeyDataReceived() Then
					; c("Button: $ pressed, state: $", 1, $_pressedBtnNum, $_pressedBtnState)
					SendKey(GetKeyDataNum(), GetKeyDataState())
					KeyDataProcessed()
				EndIf
			EndIf
			
			; Debug loop time and loop frequency output
			If $debug Then
				If TimerDiff($tt) >= 1000 Then
					$tt = TimerInit()
					c($t)
					c($_loopPeriod)
					$t = 0
				EndIf
				$t += 1
			EndIf
			
			If IsGuiOpened() Then
				SyncGuiRgb()
				HandleMsg()
			EndIf
			
			$_timer = TimerInit()
			$_loopPeriod = $_loopPeriod * 0.6 + TimerDiff($_loopStartTime) * 0.4  ; Don't modify the measured loop time immediately as it might float around
		EndIf
	WEnd
EndFunc

Main()

Func ConfigLoad()
	For $i = 1 To $WIDTH * $HEIGHT
		BindKey($i, IniRead($iniPath, "ButtonBindings", "Button" & $i & "Up", ""), IniRead($iniPath, "ButtonBindings", "Button" & $i & "Down", ""))
	Next
EndFunc

Func ConfigSave()
	For $i = 1 To $WIDTH * $HEIGHT
		IniWrite($iniPath, "ButtonBindings", "Button" & $i & "Up", GetKeybindingForKey($i, $KEYSTROKEUP))
		IniWrite($iniPath, "ButtonBindings", "Button" & $i & "Down", GetKeybindingForKey($i, $KEYSTROKEDOWN))
	Next
EndFunc

Func Terminate()
	Exit
EndFunc
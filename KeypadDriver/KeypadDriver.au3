; ================================================================================
;
; KeypadDriver.au3
; This main file runs the main loop, key binding functions and includes all the other modules 
;
; ================================================================================

#include <WindowsConstants.au3>
#include "Include\LibDebug.au3"
#include "Include\CommMG.au3"
#include "KeypadDriver.Vars.au3"
#include "KeypadDriver.Serial.au3"
#include "KeypadDriver.Gui.au3"

Global Const $scanPerSec = 1500
Global Const $msPerScan = 1000 / $scanPerSec
Global $loopPeriod, $loopStartTime, $timer
Global $timerRetrying

HotKeySet("{F4}", "OpenGui")
Opt("GUICloseOnESC", 0)

Global $debug = 0

Func Main()
	_CommSetDllPath(@ScriptDir & "\Include\commg.dll")
	If FileExists($iniPath) Then  ; If the config exists then use the binding in it
		For $i = 1 To $WIDTH * $HEIGHT
			BindKey($i, IniRead($iniPath, "ButtonBindings", "Button" & $i & "Up", ""), IniRead($iniPath, "ButtonBindings", "Button" & $i & "Down", ""))
		Next
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
		$loopStartTime = TimerInit()
		If (TimerDiff($timer) >= ($msPerScan - ($loopPeriod > $msPerScan ? $msPerScan : $loopPeriod))) Then
		
			; Because retrieving the port list takes a while, so we don't reconnect too often
			If $connectionStatus <> $CONNECTED And TimerDiff($timerRetrying) > 5000 Then
				$timerRetrying = TimerInit()
				Connect()
			EndIf
		
			If Not $debug Then
				PollKeys()
			EndIf
			
			; Debug loop time and loop frequency output
			If $debug Then
				If TimerDiff($tt) >= 1000 Then
					$tt = TimerInit()
					c($t)
					c($loopPeriod)
					$t = 0
				EndIf
				$t += 1
			EndIf
			
			If $guiOpened Then
				SyncGuiRgb()
				HandleMsg()
			EndIf
			
			$timer = TimerInit()
			$loopPeriod = $loopPeriod * 0.6 + TimerDiff($loopStartTime) * 0.4  ; Don't modify the measured loop time immediately as it might float around
		EndIf
	WEnd
EndFunc

Main()

; This function removes both keyStrokeUp and keyStrokeDown from a key
Func BindRemove($num)
	$keyMap[$num - 1][0] = ""
	$keyMap[$num - 1][1] = ""
EndFunc

; This function takes 2 arguments and binds a character to a key, with both up and down settings
;
; Or takes 3 arguments and binds custom up and down stroke to a key
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
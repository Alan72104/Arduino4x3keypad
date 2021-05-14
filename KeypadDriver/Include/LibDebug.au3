; Last modified date: 5/13/2021

#include-once
#include <StringConstants.au3>

Global $_LD_Debug = True
Global $_Profile_Map[0][3]

Func _DebugOff()
	$_LD_Debug = False
EndFunc

Func _DebugOn()
	$_LD_Debug = True
EndFunc

; Consoleout
; Automatically replaces $ to variables given
; Escape $ using $$
Func c($s = "", $nl = True, $v1 = 0x0, $v2 = 0x0, $v3 = 0x0, _
							$v4 = 0x0, $v5 = 0x0, $v6 = 0x0, _
							$v7 = 0x0, $v8 = 0x0, $v9 = 0x0, $v10 = 0x0)
	If Not $_LD_Debug Then
		Return
	EndIf
	If @NumParams > 2 Then
		$s = StringReplace($s, "$$", "@PH@")
		$s = StringReplace($s, "$", "@PH2@")
		For $i = 1 To @NumParams - 2
			$s = StringReplace($s, "@PH2@", Eval("v" & $i), 1)
			If @extended = 0 Then ExitLoop
		Next
		$s = StringReplace($s, "@PH@", "$")
		$s = StringReplace($s, "@PH2@", "$")
	EndIf
	If $nl Then
		ConsoleWrite($s & @CRLF)
	Else
		ConsoleWrite($s)
	EndIf
	If @NumParams = 1 Then
		Return $s
	EndIf
EndFunc

; Insert variable
; Returns a string with all given variables inserted into
Func iv($s = "", $v1 = 0x0, $v2 = 0x0, $v3 = 0x0, _
				$v4 = 0x0, $v5 = 0x0, $v6 = 0x0, _
				$v7 = 0x0, $v8 = 0x0, $v9 = 0x0, $v10 = 0x0)
	If @NumParams > 1 Then
		$s = StringReplace($s, "$$", "@PH@")
		$s = StringReplace($s, "$", "@PH2@")
		For $i = 1 To @NumParams - 1
			$s = StringReplace($s, "@PH2@", Eval("v" & $i), 1)
			If @extended = 0 Then ExitLoop
		Next
		$s = StringReplace($s, "@PH@", "$")
	EndIf
	Return $s
EndFunc

; Consoleout Line
Func cl()
	If Not $_LD_Debug Then
		Return
	EndIf
	ConsoleWrite(@CRLF)
EndFunc

; Consoleout Variable
; Only accepts the name of variable without the $ as string
Func cv($nl = True, $v1 = 0x0, $v2 = 0x0, $v3 = 0x0, $v4 = 0x0, $v5 = 0x0, _
						$v6 = 0x0, $v7 = 0x0, $v8 = 0x0, $v9 = 0x0, $v10 = 0x0)
	If Not $_LD_Debug Then
		Return
	EndIf
	Local $s = ""
	For $i = 1 To @NumParams - 1
		$s &= "$" & Eval("v" & $i) & " = " & Eval(Eval("v" & $i))
		If $i < @NumParams - 1 Then
			$s &= " | "
		EndIf
	Next
	If $nl Then
		$s &= @CRLF
	EndIf
	ConsoleWrite($s)
EndFunc

; Consoleout Array
Func ca(ByRef $a, $nl = True)
	If Not IsArray($a) Then
		Return
	EndIf
	Local $s = "["
	Switch UBound($a, 0)
		Case 1
			For $i = 0 To UBound($a) - 1
				If IsString($a[$i]) Then
					$s &= '"'
				EndIf
				$s &= $a[$i]
				If IsString($a[$i]) Then
					$s &= '"'
				EndIf
				If $i < UBound($a) - 1 Then
					$s &= ", "
				EndIf
			Next
		Case 2
			For $i = 0 To UBound($a, 1) - 1
				$s &= "["
				For $j = 0 To UBound($a, 2) - 1
					If IsString($a[$i][$j]) Then
						$s &= '"'
					EndIf
					$s &= $a[$i][$j]
					If IsString($a[$i][$j]) Then
						$s &= '"'
					EndIf
					If $j < UBound($a, 2) - 1 Then
						$s &= ", "
					EndIf
				Next
				$s &= "]"
				If $i < UBound($a, 1) -1 Then
					$s &= ", "
				EndIf
			Next
	EndSwitch
	$s &= "]"
	If $nl Then
		$s &= @CRLF
	EndIf
	ConsoleWrite($s)
	Return $a
EndFunc

; Consoleout Error
Func ce($e, $nl = True)
	If $nl Then
		ConsoleWrite("ERROR:" & $e & @CRLF)
	Else
		ConsoleWrite("ERROR:" & $e)
	EndIf
EndFunc

; Profiler profile Add
Func pa($v)
	For $i = 0 to UBound($_Profile_Map, 1) - 1
		If $_Profile_Map[$i][0] = $v Then
			c("Profiler >> The profile name already exists: ""$""", 1, $v)
			Return
		EndIf
	Next
	ReDim $_Profile_Map[UBound($_Profile_Map, 1) + 1][3]
	$_Profile_Map[UBound($_Profile_Map, 1) - 1][0] = $v  ; Name
	$_Profile_Map[UBound($_Profile_Map, 1) - 1][1] = 0.0  ; Value
	$_Profile_Map[UBound($_Profile_Map, 1) - 1][2] = -1  ; Start time, -1 is not running
EndFunc

; Profiler profile Start
Func ps($v)
	For $i = 0 to UBound($_Profile_Map, 1) - 1
		If $_Profile_Map[$i][0] = $v Then
			If $_Profile_Map[$i][2] = -1 Then
				$_Profile_Map[$i][2] = TimerInit()
			Else
				c("Profiler >> The specified profile to start is already started: ""$""", 1, $v)
			EndIf
			Return
		EndIf
	Next
	c("Profiler >> The specified profile to start does not exist: ""$"", adding profile...", 1, $v)
	pa($v)
	ps($v)
EndFunc

; Profiler profile End
Func pe($v)
	For $i = 0 to UBound($_Profile_Map, 1) - 1
		If $_Profile_Map[$i][0] = $v Then
			If $_Profile_Map[$i][2] <> -1 Then
				$_Profile_Map[$i][1] += TimerDiff($_Profile_Map[$i][2])
				$_Profile_Map[$i][2] = -1
			Else
				c("Profiler >> The specified profile to end has not start: ""$""", 1, $v)
			EndIf
			Return
		EndIf
	Next
	c("Profiler >> The specified profile to end does not exist: ""$""", 1, $v)
EndFunc

; Profiler Print result
Func pp()
	c("Profiler >> Printing result ====================")
	For $i = 0 To UBound($_Profile_Map, 1) -1
		c("Profiler >> $ - $", 1, $i + 1, $_Profile_Map[$i][0])
		c("Profiler >> L $ ms", 1, Round($_Profile_Map[$i][1], 2))
	Next
	c("Profiler >> ====================================")
EndFunc

; Profiler Reset
Func pr()
	For $i = 0 To UBound($_Profile_Map, 1) -1
		$_Profile_Map[$i][1] = 0.0
		$_Profile_Map[$i][2] = -1
	Next
EndFunc
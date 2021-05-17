; ================================================================================
;
; KeypadDriver.Keys.au3
; This file manages key mapping and abstracts the complexity of key sending
;
; ================================================================================

; Todo: Better name for this module

#include-once
#include "KeypadDriver.Vars.au3"
#include "KeypadDriver.Gui.au3"

; [[keyStrokeUp, keyStrokeDown], ...]
Global $keys_keyMap[$WIDTH * $HEIGHT][$keyStrokeAmount]
    For $j = 0 To $HEIGHT - 1
        For $i = 0 To $WIDTH - 1
            $keys_keyMap[$j * $WIDTH + $i][$KEYSTROKEUP] = ""
            $keys_keyMap[$j * $WIDTH + $i][$KEYSTROKEDOWN] = ""
        Next
    Next

Func SendKey($num, $state)
    ; Only send the key stroke when the gui isn't opened
    If $num <= $WIDTH * $HEIGHT And Not IsGuiOpened() And $keys_keyMap[$num - 1][$state] <> "" Then
        Send($keys_keyMap[$num - 1][$state])
    EndIf
EndFunc

Func KeyHasBinding($num, $state)
    Return GetKeybindingForKey($num, $state) <> "" ? True : False
Endfunc

Func GetKeybindingForKey($num, $state)
    If $num <= $WIDTH * $HEIGHT Then
        Return $keys_keyMap[$num - 1][$state]
    EndIf
EndFunc

; This function removes both up and down strokes from a key
Func BindRemove($num)
    $keys_keyMap[$num - 1][$KEYSTROKEUP] = ""
    $keys_keyMap[$num - 1][$KEYSTROKEDOWN] = ""
EndFunc

; This function takes 2 arguments and binds a character to both up and down strokes of a key
;
; Or takes 3 arguments and binds custom up and down strokes to a key
Func BindKey($num, $key, $extra = 0x0)
    If $num > $WIDTH * $HEIGHT Then Return
    Switch @NumParams
        Case 2
            $keys_keyMap[$num - 1][$KEYSTROKEUP] = "{" & $key & " up}"
            $keys_keyMap[$num - 1][$KEYSTROKEDOWN] = "{" & $key & " down}"
        Case 3
            $keys_keyMap[$num - 1][$KEYSTROKEUP] = $key
            $keys_keyMap[$num - 1][$KEYSTROKEDOWN] = $extra
    EndSwitch
EndFunc

Func ConfigLoad(ByRef $path)
    For $i = 1 To $WIDTH * $HEIGHT
        BindKey($i, IniRead($path, "ButtonBindings", "Button" & $i & "Up", ""), IniRead($path, "ButtonBindings", "Button" & $i & "Down", ""))
    Next
EndFunc

Func ConfigSave(ByRef $path)
    For $i = 1 To $WIDTH * $HEIGHT
        IniWrite($path, "ButtonBindings", "Button" & $i & "Up", GetKeybindingForKey($i, $KEYSTROKEUP))
        IniWrite($path, "ButtonBindings", "Button" & $i & "Down", GetKeybindingForKey($i, $KEYSTROKEDOWN))
    Next
EndFunc
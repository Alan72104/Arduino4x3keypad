; ================================================================================
;
; KeypadDriver.Serial.au3
; This file contains the functions required to communicate with the keypad
;
; ================================================================================

#include-once
#include <MsgBoxConstants.au3>
#include "Include\CommMG.au3"
#include "Include\LibDebug.au3"
#include "KeypadDriver.au3"
#include "KeypadDriver.Vars.au3"
#include "KeypadDriver.Gui.au3"

Global $_keyDataNum, $_keyDataState, $_keyDataReceived = False

Global $_byteString = "", $_byte, $_byteReceived = False

Global $_comPort

; This function tries to connect to the keypad serial port
Func Connect()
    Local $ports[0]
    $ports = _ComGetPortNames()
    For $i = 0 To UBound($ports) - 1
        If $ports[$i][1] == "USB-SERIAL CH340" Then
            Local $errorStr = ""
            $_comPort = $ports[$i][0]
            _CommSetPort(Int(StringReplace($_comPort, "COM", "")), $errorStr, 19200, 8, "none", 1, 2)
            
            If Not @error Then
                ; Connection succeed
                $connectionStatus = $CONNECTED
                _CommSetRTS(0)
                _CommSetDTR(0)
            Else
                ; Connection failed
                $connectionStatus = $CONNECTIONFAILED
                c("Connection failed, error: $", 1, $errorStr)
            EndIf
            
            ; Port was detected, no matter whether it's connected or not, stop searching ports, return
            Return
        EndIf
    Next
    
    ; If this line runs that means no port was detected, sets the status to detaction failed
    $connectionStatus = $PORTDETECTIONFAILED
EndFunc

; This function polls the serial for new key datas
Func PollKeys()
    If $connectionStatus <> $CONNECTED Then Return
    
    ; If there's still unprocessed key data in the buffers, return
    If $_keyDataReceived Then Return
    
    $_byteString = _CommReadByte()
    If @error = 3 Then
        $connectionStatus = $CONNECTIONFAILED
        Return
    EndIf
    If $_byteString <> "" Then
        $_byte = Int($_byteString)
        
        ; Key status byte - |first 4 bits for key number, 3 zero padding bits, last one bit for pressed state|
        $_keyDataNum = BitShift($_byte, 4)
        $_keyDataState = BitAND($_byte, 0x01)

        $_keyDataReceived = True
    EndIf
EndFunc

; This function polls the serial for a byte
Func PollData()
    If $connectionStatus <> $CONNECTED Then Return
    
    ; If there's still unprocessed byte in the buffer $_byte, return
    If $_byteReceived Then Return
    
    $_byteString = _CommReadByte()
    If @error = 3 Then
        $connectionStatus = $CONNECTIONFAILED
        Return
    EndIf
    If $_byteString <> "" Then
        $_byte = Int($_byteString)
        ; c("Received data $", 1, $_byteString)
        $_byteReceived = True
        Return
    EndIf
EndFunc

; This function sends a message byte to the keypad
Func SendMsgToKeypad($type, $data)
    If $connectionStatus <> $CONNECTED Then Return
    
    ; Message byte - |first 2 bits for msg type, last 6 bits for msg data|
    If $type > 3 Then
        MsgBox($MB_ICONWARNING + $MB_TOPMOST, "KeypadDriver", "Exception catched ""SendMsgToKeypad()""" & @CRLF & @CRLF & _
                                                              "Message type cannot be larger than 2 bits! Type: " & $type & @CRLF & @CRLF & _
                                                              "Message not sent!")
        Return
    EndIf
    If $data > 63 Then
        MsgBox($MB_ICONWARNING + $MB_TOPMOST, "KeypadDriver", "Exception catched ""SendMsgToKeypad()""" & @CRLF & @CRLF & _
                                                              "Data to send cannot be larger than 2 bits! Data: " & $data & @CRLF & @CRLF & _
                                                              "Message not sent!")
        Return
    EndIf

    _CommSendByte(BitShift($type, -6) + $data)
EndFunc

Func GetComPort()
    Return $_comPort
EndFunc

Func GetKeyDataNum()
    Return $_keyDataNum
EndFunc

Func GetKeyDataState()
    Return $_keyDataState
EndFunc

Func IsKeyDataReceived()
    Return $_keyDataReceived
EndFunc

Func KeyDataProcessed()
    $_keyDataReceived = False
EndFunc

Func GetByte()
    Return $_byte
EndFunc

Func IsByteReceived()
    Return $_byteReceived
EndFunc

Func ByteProcessed()
    $_byteReceived = False
EndFunc
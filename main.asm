.386
.model          flat, stdcall
option          casemap:none


include         \masm32\include\windows.inc
include         \masm32\include\user32.inc
include         \masm32\include\kernel32.inc
include         \masm32\include\gdi32.inc
include         \masm32\include\ws2_32.inc

includelib      \masm32\lib\kernel32.lib
includelib      \masm32\lib\user32.lib
includelib      \masm32\lib\gdi32.lib
includelib      \masm32\lib\ws2_32.lib


WinMain proto :DWORD, :DWORD, :DWORD, :DWORD
extern PerformRequest :proc


WindowWidth            equ 600
WindowHeight           equ 800

TxtFieldResponseWidth  equ 500
TxtFieldResponseHeight equ 250
TxtFieldResponsePosX   equ 50
TxtFieldResponsePosY   equ 35

TxtFieldRequestWidth   equ 500
TxtFieldRequestHeight  equ 250
TxtFieldRequestPosX    equ 50
TxtFieldRequestPosY    equ TxtFieldResponsePosY + TxtFieldResponseHeight + 35

TxtBoxURLWidth         equ 500
TxtBoxURLHeight        equ 30
TxtBoxURLPosX          equ 50
TxtBoxURLPosY          equ TxtFieldRequestPosY + TxtFieldRequestHeight + 35

ButtonPerformWidth     equ 300
ButtonPerformHeight    equ 30
ButtonPerformPosX      equ 50
ButtonPerformPosY      equ TxtBoxURLPosY + TxtBoxURLHeight + 35

TxtBoxMethodWidth      equ TxtBoxURLWidth - ButtonPerformWidth - 30
TxtBoxMethodHeight     equ 30
TxtBoxMethodPosX       equ 50 + ButtonPerformWidth + 30
TxtBoxMethodPosY       equ ButtonPerformPosY

ADDRINFO struct
	ai_flags	DWORD ?
	ai_family	DWORD ?
	ai_socktype	DWORD ?
	ai_protocol	DWORD ?
	ai_addrlen	QWORD ?
	ai_canonname	LPVOID ?
	ai_addr		LPVOID ?
	ai_next		LPVOID ?
ADDRINFO ENDS


.DATA
ClassName              db "api-tester", 0
AppName                db "api-tester", 0
TxtFieldCtrlStr        db "EDIT", 0
ButtonCtrlStr          db "BUTTON", 0
ButtonPerformText      db "Perform", 0
FontArialStr           db "ARIAL", 0
TxtFieldResponseLabel  db "Response", 0
TxtFieldRequestLabel   db "Request", 0
TxtBoxURLLabel         db "URL", 0
TxtBoxMethodLabel      db "Method", 0
WSAErrorText           db "WSA Error", 0

; TEST VAR
TESTVAR_URL            db "www.httpbin.org", 0


.DATA?
WinMainHInstance       HINSTANCE ?
WinMainCommandLine     LPSTR ?
TxtFieldResponse       HWND ?
TxtFieldRequest        HWND ?
TxtBoxURL              HWND ?
ButtonPerform          HWND ?
TxtBoxMethod           HWND ?


.CODE
MainEntry:
    push        NULL
    call        GetModuleHandle
    mov         WinMainHInstance, eax

    call        GetCommandLine
    mov         WinMainCommandLine, eax

    push        SW_SHOWDEFAULT
    lea         eax, WinMainCommandLine
    push        eax
    push        NULL
    push        WinMainHInstance
    call        WinMain

    push        eax
    call        ExitProcess
    hlt


WinMain proc hInst:HINSTANCE, hPrevInst:HINSTANCE, CmdLine:LPSTR, CmdShow:DWORD
    LOCAL       WndClass:WNDCLASSEX
    LOCAL       Msg:MSG
    LOCAL       hwnd:HWND


; Draw main window
    mov         WndClass.cbSize, SIZEOF WNDCLASSEX
    mov         WndClass.style, CS_HREDRAW or CS_VREDRAW
    mov         WndClass.lpfnWndProc, OFFSET WndProc
    mov         WndClass.cbClsExtra, 0
    mov         WndClass.cbWndExtra, 0
    mov         eax, WinMainHInstance
    mov         WndClass.hInstance, eax
    mov         WndClass.hbrBackground, COLOR_3DSHADOW or 1
    mov         WndClass.lpszMenuName, NULL
    mov         WndClass.lpszClassName, OFFSET ClassName

    push        IDI_APPLICATION
    push        NULL
    call        LoadIcon
    mov         WndClass.hIcon, eax
    mov         WndClass.hIconSm, eax

    push        IDC_ARROW
    push        NULL
    call        LoadCursor
    mov         WndClass.hCursor, eax

    lea         eax, WndClass
    push        eax
    call        RegisterClassEx

    push        NULL
    push        WinMainHInstance
    push        NULL
    push        NULL
    push        WindowHeight
    push        WindowWidth
    push        CW_USEDEFAULT
    push        CW_USEDEFAULT
    push        WS_OVERLAPPEDWINDOW or WS_VISIBLE
    push        OFFSET AppName
    push        OFFSET ClassName
    push        0
    call        CreateWindowExA
    mov         hwnd, eax


; Draw controls
    push        NULL
    push        NULL
    push        NULL
    push        hwnd
    push        TxtFieldResponseHeight
    push        TxtFieldResponseWidth
    push        TxtFieldResponsePosY
    push        TxtFieldResponsePosX
    push        WS_VISIBLE or WS_CHILD or WS_BORDER or ES_MULTILINE or WS_VSCROLL or ES_READONLY
    push        NULL
    push        OFFSET TxtFieldCtrlStr
    push        0
    call        CreateWindowExA
    mov         TxtFieldResponse, eax

    push        NULL
    push        NULL
    push        NULL
    push        hwnd
    push        TxtFieldRequestHeight
    push        TxtFieldRequestWidth
    push        TxtFieldRequestPosY
    push        TxtFieldRequestPosX
    push        WS_VISIBLE or WS_CHILD or WS_BORDER or ES_MULTILINE or WS_VSCROLL
    push        NULL
    push        OFFSET TxtFieldCtrlStr
    push        0
    call        CreateWindowExA
    mov         TxtFieldRequest, eax

    push        NULL
    push        NULL
    push        NULL
    push        hwnd
    push        TxtBoxURLHeight
    push        TxtBoxURLWidth
    push        TxtBoxURLPosY
    push        TxtBoxURLPosX
    push        WS_VISIBLE or WS_CHILD or WS_BORDER
    push        NULL
    push        OFFSET TxtFieldCtrlStr
    push        0
    call        CreateWindowExA
    mov         TxtBoxURL, eax

    push        NULL
    push        NULL
    push        NULL
    push        hwnd
    push        ButtonPerformHeight
    push        ButtonPerformWidth
    push        ButtonPerformPosY
    push        ButtonPerformPosX
    push        WS_VISIBLE or WS_CHILD or BS_DEFPUSHBUTTON or WS_TABSTOP
    push        OFFSET ButtonPerformText
    push        OFFSET ButtonCtrlStr
    push        0
    call        CreateWindowExA
    mov         ButtonPerform, eax

    push        NULL
    push        NULL
    push        NULL
    push        hwnd
    push        TxtBoxMethodHeight
    push        TxtBoxMethodWidth
    push        TxtBoxMethodPosY
    push        TxtBoxMethodPosX
    push        WS_VISIBLE or WS_CHILD or WS_BORDER
    push        NULL
    push        OFFSET TxtFieldCtrlStr
    push        0
    call        CreateWindowExA
    mov         TxtBoxMethod, eax


; Set a nicer font for all of the controls
    push        OFFSET FontArialStr
    push        DEFAULT_PITCH or FF_SWISS
    push        DEFAULT_QUALITY
    push        CLIP_DEFAULT_PRECIS
    push        OUT_DEFAULT_PRECIS
    push        ANSI_CHARSET
    push        0
    push        0
    push        0
    push        FW_DONTCARE
    push        0
    push        0
    push        0
    push        22
    call        CreateFont

    mov         ebx, eax

    push        TxtFieldResponse
    push        ebx
    call        SetCtrlFont

    push        TxtFieldRequest
    push        ebx
    call        SetCtrlFont

    push        TxtBoxURL
    push        ebx
    call        SetCtrlFont

    push        ButtonPerform
    push        ebx
    call        SetCtrlFont

    push        TxtBoxMethod
    push        ebx
    call        SetCtrlFont


; Handle incoming messages
MsgLoop:
    push        0
    push        0
    push        NULL
    lea         ebx, Msg
    push        ebx
    call        GetMessage

    cmp         eax, 0
    je          MsgLoopEnd

    push        ebx
    call        TranslateMessage

    push        ebx
    call        DispatchMessage

    jmp         MsgLoop

MsgLoopEnd:
    mov         eax, Msg.wParam

WinMainDefRet: ; not needed anymore?
    ret

WinMain endp


SetCtrlFont proc hfont:HFONT, hwnd:HWND
    push        1
    push        hfont
    push        WM_SETFONT
    push        hwnd
    call        SendMessage
    ret
SetCtrlFont endp


WndProc proc hwnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
    LOCAL       ps:PAINTSTRUCT
    LOCAL       rect:RECT
    LOCAL       hdc:HDC

    LOCAL       TxtBoxURLInput[128]:BYTE
    LOCAL       TxtBoxMethodInput[32]:BYTE
    LOCAL       TxtFieldRequestInput[4096]:BYTE
    LOCAL       TxtFieldResponseOutput[4096]:BYTE

    LOCAL       WsaData:WSADATA ; word, word
    LOCAL       pAddrInfoResult:LPVOID ; ptr
    LOCAL       pAddrInfoPtr:LPVOID
    LOCAL       AddrInfoHints:ADDRINFO ; i,i,i,i,i,ptr,ptr,ptr
    LOCAL       Socket:SOCKET


    cmp         uMsg, WM_PAINT
    je          MsgEqWM_PAINT

    cmp         uMsg, WM_COMMAND
    je          MsgEqWM_COMMAND

    cmp         uMsg, WM_DESTROY
    je          MsgEqWM_DESTROY

    push        lParam
    push        wParam
    push        uMsg
    push        hwnd
    call        DefWindowProc
    ret


MsgEqWM_PAINT:
    lea         eax, ps
    push        eax
    push        hwnd
    call        BeginPaint
    mov         hdc, eax

    lea         eax, rect
    push        eax
    push        hwnd
    call        GetClientRect

    push        TRANSPARENT
    push        hdc
    call        SetBkMode


; Create a font for labels
    push        OFFSET FontArialStr
    push        DEFAULT_PITCH or FF_SWISS
    push        DEFAULT_QUALITY
    push        CLIP_DEFAULT_PRECIS
    push        OUT_DEFAULT_PRECIS
    push        ANSI_CHARSET
    push        0
    push        0
    push        0
    push        FW_DONTCARE
    push        0
    push        0
    push        0
    push        18
    call        CreateFont

    push        eax
    push        hdc
    call        SelectObject


; Draw labels
    lea         ebx, rect

    mov         rect.left, 50
    mov         rect.top, TxtFieldResponsePosY - 18
    push        DT_SINGLELINE or DT_NOCLIP or DT_LEFT
    push        ebx
    push        -1
    push        OFFSET TxtFieldResponseLabel
    push        hdc
    call        DrawText

    mov         rect.top, TxtFieldRequestPosY - 18 
    push        DT_SINGLELINE or DT_NOCLIP or DT_LEFT
    push        ebx
    push        -1
    push        OFFSET TxtFieldRequestLabel
    push        hdc
    call        DrawText

    mov         rect.top, TxtBoxURLPosY - 18 
    push        DT_SINGLELINE or DT_NOCLIP or DT_LEFT
    push        ebx
    push        -1
    push        OFFSET TxtBoxURLLabel
    push        hdc
    call        DrawText

    mov         rect.left, TxtBoxMethodPosX
    mov         rect.top, TxtBoxMethodPosY - 18 
    push        DT_SINGLELINE or DT_NOCLIP or DT_LEFT
    push        ebx
    push        -1
    push        OFFSET TxtBoxMethodLabel
    push        hdc
    call        DrawText

    lea         eax, ps
    push        eax
    push        hwnd
    call        EndPaint
    jmp         WndProcDefRet


MsgEqWM_COMMAND:
; Check if ButtonPerform was clicked
    cmp         wParam, BN_CLICKED
    jne         WndProcDefRet

    mov         eax, lParam
    cmp         eax, ButtonPerform
    jne         WndProcDefRet               ; only command supposed to be handled in this app is a ButtonPerform click event


; Get the data required to perform the HTTP request
    push        128
    lea         eax, TxtBoxURLInput
    push        eax
    push        TxtBoxURL
    call        GetWindowText
    mov         TxtBoxURLInput + 127, 0      ; null terminates the string on the last index in case the control's text was too long to include the null terminator

    push        32
    lea         eax, TxtBoxMethodInput
    push        eax
    push        TxtBoxMethod
    call        GetWindowText
    mov         TxtBoxMethodInput + 31, 0

    push        4096
    lea         eax, TxtFieldRequestInput
    push        TxtFieldRequest
    call        GetWindowText
    mov         TxtFieldRequestInput + 4095, 0


; Perform the HTTP request
    call        PerformRequest
    push        0
    push        0
    push        eax
    push        0
    call        MessageBox



    jmp WndProcDefRet

MsgEqWM_DESTROY:
    push        0
    call        PostQuitMessage
    jmp         WndProcDefRet



WndProcDefRet:
    xor         eax, eax
    ret

WndProc endp

END MainEntry

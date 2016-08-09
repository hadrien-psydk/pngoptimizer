; This file is a trick to redirect EncodePointer and DecodePointer functions
; that are available only since WinXP SP2. This way we stay compatible with
; older Windows version. This trick does not apply to x64 builds.

.model flat

.data
__imp__EncodePointer@4 dd dummy
__imp__DecodePointer@4 dd dummy
EXTERNDEF __imp__EncodePointer@4 : DWORD
EXTERNDEF __imp__DecodePointer@4 : DWORD

.code
dummy proc
mov eax, [esp+4]
ret 4
dummy endp

end

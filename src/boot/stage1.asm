ORG 0x7c00
[BITS 16]

jmp short start
nop

; FAT16 Header
OEMIdentifier           db 'SLOBOS  '   ;string must be 8 bits
BytesPerSector          dw 0x200
SectorsPerCluster       db 0x80
ReservedSectors         dw 200
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFat           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773594

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'SLOBOS BOO'
SystemIDString          db 'FAT16   '   ;string must be 8 bits


start:

; RESET THE DISK
; DL Register contains current drive as passed to us by bios
; Won't bother checking status here waste of time its emulated
mov ah, 0x00
int 0x13

; Segment where stage2 will load
mov ax, 0x7e0
mov es, ax

; LOAD stage2 INTO MEMORY
stage2_load:
    mov ah, 0x02
    mov al, 3
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov bx, 0x00
    int 0x13
    jc .problem

    mov ax, 0x7e0
    mov ds, ax

    ; jump to the stage2 bootloader
    jmp 0x7e0:0x0000
.problem:
    mov si, problem_loading_stage2
    call print
    jmp $

print:
    push ax
    mov ah, 0eh
.nextc:
    lodsb
    cmp al, 0
    je .pdone
    int 0x10
    jmp .nextc
.pdone:
    pop ax
    ret


problem_loading_stage2: db 'issue loading the stage2 bootloader', 0

TIMES 510-($-$$) db 0
dw 0xAA55
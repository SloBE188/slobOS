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
    cli                 ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti                 ; Enable interrupts

    ; Load the second sector (Stage2 bootloader) into memory at 0x7E00
    mov ah, 0x02        ; BIOS read sector function
    mov al, 1           ; Number of sectors to read
    mov ch, 0           ; Cylinder number (low byte)
    mov cl, 2           ; Sector number (starts from 1)
    mov dh, 0           ; Head number
    mov dl, [DriveNumber]
    mov bx, 0x7E00      ; Destination memory address
    int 0x13            ; Call BIOS

    ;jc error            ; Jump to error if carry flag is set
    jmp 0x0000:0x7E00   ; Jump to Stage2 bootloader

error:
    mov si, error_message
    call print

    jmp $               ; Infinite loop

print:
    mov bx, 0
.loop:
    lodsb
    cmp al, 0
    je .done
    call print_char
    jmp .loop
.done:
    ret

print_char:
    mov ah, 0eh
    int 0x10
    ret

error_message: db 'Failed to load Sector', 0

times 510- ($ - $$) db 0
dw 0xAA55       ; Boot signature
ORG 0x7c00
[BITS 16]


CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


;Das Label "_start" braucht es wegen dem BIOS Parameter Block
_start:
    jmp short start
    nop

times 33 db 0
start:
    jmp 0:step2



step2:
    ;Hier werden nun die Segment register manuell gesetzt, dass sie nicht random vom BIOS gesetzt werden. 
    ;Das SS muss anders gesetzt werden als die restlichen.
    cli ;Clear Interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ;Enable Interrupts


.load_protected:
    cli 
    lgdt[gdt_descriptor]    ;loads GDT
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32     ;In diese Adresse wird nun der Kernel geladen.
    jmp $

;GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

;offset 0x8
gdt_code:       ;CS
    dw 0xffff   ;Segment limit first 0-15 bits
    dw 0        ;Base first 0-15 bits
    db 0        ;Base 16-23 bits
    db 0x9a     ;Access byte
    db 11001111b;High 4 bit flags and the low 4 bit flags
    db 0        ;Base 24-31 bits

;offset 0x10
gdt_data:       ;DS,SS,ES,FS,GS
    dw 0xffff   ;Segment limit first 0-15 bits
    dw 0        ;Base first 0-15 bits
    db 0        ;Base 16-23 bits
    db 0x92     ;Access byte
    db 11001111b;High 4 bit flags and the low 4 bit flags
    db 0        ;Base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1
    dd gdt_start

[BITS 32]
load32:
    mov eax, 1          ;Der Sektor im Speicher, wovon wir den kernel laden möchten (0 ist der Bootsektor)
    mov ecx, 100        ;Totale Nummer von Sektoren, welche wir laden wollen.
    mov edi, 0x0100000  ;Adresse, wohin wir die Sektoren laden wollen.
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax        ;Backup the LBA
    ;Send the hightes 8 bits of the lba to the hard disk controller (on the mainboard)
    shr eax, 24
    or eax, 0xE0        ;Select the master drive
    mov dx, 0x1F6
    out dx, al
    ;Finished sending the hightest 8 bits of the lba
 
    ;Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ;Finisehd sending the total sectors to read


    ;Send more bits of the lba
    mov eax, ebx        ;Restore the backup lba
    mov dx, 0x1F3
    out dx, al
    ;Finished sending more bits of the lba


    ;Send more bits of the lba
    mov dx, 0x1F4
    mov eax, ebx        ;Restore the backup lba
    shr eax, 8
    out dx, al
    ;Finished sending more bits of the lba

    ;Send upper 16 bits of the lba
    mov dx, 0x1F5
    mov eax, ebx        ;Restore the backup lba
    shr eax, 16
    out dx, al
    ;Finished sending upper 16 bits of the lba

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ;Read all sectors into memory
.next_sector:
    push ecx

    ;Checking if we need to read
.try_again:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

;Need to read 256 words at a time
    mov ecx, 256    ;256 words = 512 Bytes (1 sector)
    mov dx, 0x1F0
    rep insw        ;Diese insw wird nun 256x ausgeführt (wegen ecx=256).
    pop ecx
    loop .next_sector   ;Es geht zum nächsten sector und dekrementiert immer um 1, bis ich von sector 100 bis auf 0 bin (alles sectors gelesen)

    ;End of reading sectors into memory
    ret



times 510- ($ - $$) db 0
dw 0xAA55       ;boot signature

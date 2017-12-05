	.arch msp430g2553
	.p2align 1,0
	.text
	
	.data
num:	.word 0 		; state

	.text
buzz:	.word case1     ; case 1
        .word case2     ; case 2
        .word case3     ; case 3
        .word case4     ; case 4



	.global stateAs
stateAs:

case1:
	call #p2sw_read
	cmp #1, r12
	jnz case2
	mov #1, r12
	jmp end
case2:
	call #p2sw_read
	cmp #2, r12
	jnz case3
	mov #2, r12
	jmp end
case3:
	call #p2sw_read
	cmp #3, r12
	jnz case4
	mov #3, r12
	jmp end
case4:
    call #p2sw_read
	cmp #4, r12
	jnz case3
	mov #4, r12
	jmp end
end:
	pop r0			; return  

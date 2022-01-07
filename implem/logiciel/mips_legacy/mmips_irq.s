# code pour gérer les interruptions avec une ISR en C
# l'ISR est une fonction obligatoirement déclarée par:  
# void isr()
# cette fonction est appelée par le vecteur d'interruption
# l'appel est précédé d'une sauvegarde des registres dans la pile
# l'ISR est suivie d'une restauration des registres et de la pile

.set noat                  
.set noreorder
.section __start, "x" # point de départ standard
#		la $gp, _gp				
		la $sp, .irq_vec-4 #initialise la pile
		j main
        
.text
irq_start:        #place les registres en pile
        addiu  $sp,  $sp, -124 #accroit la pile
 sw $1, 0($sp)
sw $2, 4($sp)
sw $3, 8($sp)
sw $4, 12($sp)
sw $5, 16($sp)
sw $6, 20($sp)
sw $7, 24($sp)
sw $8, 28($sp)
sw $9, 32($sp)
sw $10, 36($sp)
sw $11, 40($sp)
sw $12, 44($sp)
sw $13, 48($sp)
sw $14, 52($sp)
sw $15, 56($sp)
sw $16, 60($sp)
sw $17, 64($sp)
sw $18, 68($sp)
sw $19, 72($sp)
sw $20, 76($sp)
sw $21, 80($sp)
sw $22, 84($sp)
sw $23, 88($sp)
sw $24, 92($sp)
sw $25, 96($sp)
sw $26, 100($sp)
sw $27, 104($sp)
sw $28, 108($sp)
sw $29, 112($sp)
sw $30, 116($sp)
sw $31, 120($sp)       
        jal isr
lw $1, 0($sp)
lw $2, 4($sp)
lw $3, 8($sp)
lw $4, 12($sp)
lw $5, 16($sp)
lw $6, 20($sp)
lw $7, 24($sp)
lw $8, 28($sp)
lw $9, 32($sp)
lw $10, 36($sp)
lw $11, 40($sp)
lw $12, 44($sp)
lw $13, 48($sp)
lw $14, 52($sp)
lw $15, 56($sp)
lw $16, 60($sp)
lw $17, 64($sp)
lw $18, 68($sp)
lw $19, 72($sp)
lw $20, 76($sp)
lw $21, 80($sp)
lw $22, 84($sp)
lw $23, 88($sp)
lw $24, 92($sp)
lw $25, 96($sp)
lw $26, 100($sp)
lw $27, 104($sp)
lw $28, 108($sp) 
lw $29, 112($sp) # inutile mais on peut rechargerla pile 
lw $30, 116($sp)
lw $31, 120($sp)
        addiu  $sp,  $sp, 124 # remet la pile comme on l'a trouvé
        eret

        
.section .irq_vec, "x"
		j irq_start


	
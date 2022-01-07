                
.set noreorder
.section __start, "x" # point de départ standard
		la $gp, _gp	# démarre la pile en haut de la mémoire
		la $sp, _gp
		j main
        

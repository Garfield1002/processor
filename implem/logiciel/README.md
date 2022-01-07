Liste des répertoires:
---------------------

* *apps* -> Contient les applications qui vont pouvoir tourner sur FPGA et/ou sur QEMU
* *kernel* -> Contient le code de boot et libfmet (une librairie C lègère)
* *mips_legacy* -> Code divers hérité du MIPS (repertoire destiné à disparaître)

Libfemto:
---------

Remarques en vrac sur le *printf*:

Les formats supportés sont : 

* ***%d***  affiche un int
* ***%ld*** affiche un long
* ***%x***  affiche un int en hexa
* ***%lx*** affiche un long en hexa
* ***%s***  affiche une string (c.a.d un tableau de charactère terminé par '\0')
* ***%c***  affiche un char 

Les formats ***%u*** et ***%d*** *ne sont pas supportés* à l'heure actuelle.



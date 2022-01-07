Répertoire qui contient le démarrage du TP sur la réalisation du processeur riscv.

## Récupérer le projet

Faire un clone pour récupérer le projet.

`git clone git@gitlab-student.centralesupelec.fr:comparch/processeur.git`

## Dépendances

- _Vivado_ -> La version minimale _19.1 WebPAck Edition_ est requise pour faire fonctionner le contrôleur HDMI ([Téléchargement de Vivado](https://www.xilinx.com/support/download.html))
- _Toolchain gcc-riscv_ -> Elle peut être compilée et installée à l'aide du [dépôt Outils](https://gricad-gitlab.univ-grenoble-alpes.fr/riscv-ens/outils))

## Environnement

Pour pouvoir utiliser les Makefiles du projet afin de simuler/synthetiser les modèles matériels VHDL et de compiler les programes de test et applications, il faut faire connaître à son environnement de travail les chemins vers les outils utilisés :

- Ajout du chemin vers la chaîne de compilation dans le PATH:

`export PATH=${PATH}:/opt/riscv-cep-tools/bin`

Remplacer _/opt/riscv-cep-tools_ par le chemin où votre chaîne
de compilation RISC-V est installée sur votre machine

- Ajout des chemins vers les outils Vivado:

`source /opt/Xilinx/Vivado/2019/settings64.sh`

Remplacer _/opt/_ par le chemin où Vivado est installé sur votre machine

## Makefiles

### Simuler

Test de l'instruction lui :

`cd implem && make simulation PROG=lui `

### Synthétiser

Test des leds sur carte :

`cd implem && make fpga PROG=test_led_x31`

Space Invader sur carte :

`cd implem && make fpga PROG=invader LIB=libfemto`

### Autres

`cd implem && make help`

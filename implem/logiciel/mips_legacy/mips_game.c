#include "sprite.h"
typedef unsigned int uint32;
typedef struct Objet {
  uint32 vivant;
  uint32 periode;
  uint32 echeance;
  int x,y;
  int dx, dy;
  uint32 motif[8];
  int ax,ay;
} Objet;
int tab[32];
#define NOBJETS 9
Objet objet[NOBJETS]={
  {1,3,1, 3,1, 0,0, sprite_four,0,0},
  {0,1,1, 3,2, 0,0, {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},0,0},
  {1,4,1, 10,1, -1,0, sprite_two,0,0},
  {1,4,1, 26,1, -1,0, sprite_three,0,0},
#if NOBJETS==4
};
#else
{1,4,1, 10,3, -1,0, sprite_one,0,0},
{1,4,1, 14,3, -1,0, sprite_five,0,0},
{1,4,1, 18,3, -1,0, sprite_six,0,0},
{1,4,1, 22,3, -1,0, sprite_three,0,0},
{1,4,1, 26,3, -1,0, sprite_one,0,0},
    };
#endif
void affiche_sprite(Objet * objet);
extern void affiche_motif_ligne(uint32 m,int x,int y);
extern void affiche_motif(uint32 motif[8], int x, int y);
volatile uint32 *img  = (volatile uint32 *)0x80000;
void affiche_pixel(int pix, int x, int y);

void main(void)
{
  int i=0;
  int decl=0;

  while (1){
    for (i=0;i<20;i++){
      affiche_pixel(0x0FFF,i+50,i);
    }
    for (i=0;i<9;i++){
      affiche_motif_ligne(0x55,i,i);
    }
        for (i=0;i<9;i++){
      affiche_sprite(&objet[i]);
    }/*
    for (i=0;i<9;i++){
      affiche_motif(objet[i].motif,100,i<<3);
      }*/
    
  }	
}

void affiche_pixel(int pix, int x, int y)
{
  img[y * 320 + x]=pix;
}

void affiche_motif_ligne(uint32 m,int x,int y){
  int i;
  for (i=0;i<8;i++){
    //int pix=lit_pixel(x+i,y);
    int color = ((m & 1) == 1)?0xFFFF:0;
    m=m>>1;
    //    pix = pix ^ color;
    affiche_pixel(color,x+i,y);
  }
}

void affiche_motif(uint32 motif[8], int x, int y)
{
  int i; 
  for(i=0;i<8;i++)
    {
      affiche_motif_ligne(motif[i],x,y+i);
    }
}
void affiche_sprite(Objet *objet){
  int dx,dy;
  if (objet->ax>-1 && objet->ay>-1) {
    for (dx=0;dx<8;dx++)
      for (dy=0;dy<8;dy++)
	affiche_pixel(0x00FF,((objet->ax)<<3)+dx,((objet->ay)<<3)+dy);
  }
  (objet->ax)=(objet->x);(objet->ay)=(objet->y);
  affiche_motif(objet->motif,(objet->x)<<3,(objet->y)<<3);
}

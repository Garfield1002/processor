volatile int compteur;

void main(void)
{
  compteur=0;
  register int d asm ("$28");
  d=0x0100;
  while (1) {
    while (compteur<32) {
      compteur=(compteur + 1) % 16;
      d=d|compteur;
    }
    d=d+0x0100;
  }
}

void isr()
{
  compteur=33;
}


/*
 * main.c
 */

void kprint(char * str);
void clearscreen(void);

volatile unsigned short * const vid = (void *)0xB8000;
volatile unsigned short * const vidmax = (void *)0xB87D0;

void kmain(void)
{
    clearscreen();
    kprint("AndromedOS loaded successfully!");

}

void kprint(char *str)
{
    int pos=0;
    while (str[pos] && pos<(vidmax-vid))
    {
        vid[pos] = 0x0F00 | str[pos];
        pos++;
    }
}

void clearscreen(void)
{
    volatile unsigned short *c = vid;
    while (c < vidmax)
    {
        *c = 0x0;
        c++;
    }
}

/*
 * kernel_main.c
 */

void kprint(char * str);

void kmain(void)
{
    int j;
    char *vidptr = (char *)0xB8000;
    while (j<80*25*2) {
        vidptr[j] = ' ';
        j++;
    }
    kprint("AndromedOS loaded successfully!");
}

void kprint(char *str)
{
    char *vid = (char *)0xB8000;
    while (*str && vid<(char *)0xB8FA0)
    {
        if (*str == '\n')
        {
            int line = ((int)vid - 0xB8000)/80;
            vid = (char *)( line*80+0xB80A0 );
        } else {
            *vid = *str;
            vid+=2;
        }
        str++;
    }
    return;
}

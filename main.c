#include <stdio.h>
#include "volutar/pt3player.h"
#include "pt3.h"

static unsigned char buf[7623];

int main()
{
    PT3 pt3[3];

    FILE* f = fopen("music.pt3", "rb");
    fread(buf, 1, 7623, f);
    fclose(f);

    PT3_init(&pt3[0], buf);
    func_setup_music((uint8_t*)buf, 7623, 0);

    for (;;) {
        func_play_tick(0);
        uint8_t r[16];
        func_getregs(r, 0);

        PT3_play(&pt3[0]);

        fprintf(stdout,
            "\nVOL %02X%02X %02X%02X %02X%02X %02X %02X %02X %02X %02X %02X%02X %02X\n",
            r[1], r[0], r[3], r[2], r[5], r[4], r[6], r[7], r[8],
            r[9], r[10], r[12], r[11], r[13]);

        fprintf(stdout,
            "BUL %04X %04X %04X %02X %02X %02X %02X %02X\n",// %04X %02X %04X\n",
            pt3[0].AY.tonA, pt3[0].AY.tonB, pt3[0].AY.tonC, pt3[0].AY.noise, pt3[0].AY.mixer,
            pt3[0].AY.amplA, pt3[0].AY.amplB, pt3[0].AY.amplC//, AY.env//, AY.envTp, AY.envBase
            );
    }
}

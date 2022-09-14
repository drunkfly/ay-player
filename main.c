#include <stdio.h>
#include "volutar/pt3player.h"
#include "pt3.h"

static unsigned char buf[7623];

extern AYRegs AY;

void PT3_init(const unsigned char* data);

int main()
{
    FILE* f = fopen("music.pt3", "rb");
    fread(buf, 1, 7623, f);
    fclose(f);

    PT3_init(buf);
    func_setup_music((uint8_t*)buf, 7623, 0);

    for (;;) {
        func_play_tick(0);
        uint8_t r[16];
        func_getregs(r, 0);

        PT3_play();

        fprintf(stdout,
            "\nVOL %02X%02X %02X%02X %02X%02X %02X %02X %02X %02X %02X %02X%02X %02X\n",
            r[1], r[0], r[3], r[2], r[5], r[4], r[6], r[7], r[8],
            r[9], r[10], r[12], r[11], r[13]);

        fprintf(stdout,
            "BUL %04X %04X %04X %02X %02X %02X %02X %02X\n",// %04X %02X %04X\n",
            AY.tonA, AY.tonB, AY.tonC, AY.noise, AY.mixer,
            AY.amplA, AY.amplB, AY.amplC//, AY.env//, AY.envTp, AY.envBase
            );
    }
}

#include <stdio.h>
#include "volutar/pt3player.h"
#include "pt3.h"

static unsigned char buf[7623];

extern AYRegs AY;

void PT3_init(const unsigned char* data);
void WriteAY();

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

        if (r[0] != (AY.tonA & 0xFF) ||
            r[1] != (AY.tonA >> 8) ||
            r[2] != (AY.tonB & 0xFF) ||
            r[3] != (AY.tonB >> 8) ||
            r[4] != (AY.tonC & 0xFF) ||
            r[5] != (AY.tonC >> 8) ||
            r[6] != AY.noise ||
            r[7] != AY.mixer ||
            r[8] != AY.amplA ||
            r[9] != AY.amplB ||
            r[10] != AY.amplC ||
            r[11] != (AY.env & 0xFF) ||
            r[12] != (AY.env >> 8) ||
            r[13] != AY.envTp) {

            fprintf(stdout,
                "\nVOL %02X%02X %02X%02X %02X%02X %02X %02X %02X %02X %02X %02X%02X %02X\n",
                r[1], r[0], r[3], r[2], r[5], r[4], r[6], r[7], r[8],
                r[9], r[10], r[12], r[11], r[13]);

            WriteAY();
        }
    }
}

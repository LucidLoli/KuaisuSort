// C++ Source
// Random Number Generator

#include <bits/stdc++.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [Count] [OutputFile]\nNothing to do.\n", argv[0]);
        return 0;
    }
    std::mt19937 mt;
    auto file = fopen(argv[2], "w");
    uint32_t len = atoi(argv[1]);
    mt.seed(uint32_t(time(0)) ^ len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % 500 == 0) {
            if (i) fprintf(file, "\n%-9x", i / 500);
            else fprintf(file, "%-9x", i / 500);
        }
        else fprintf(file, "%-9x", (uint32_t) (mt() & 0x7fffffff));
    }
    fprintf(file, "\n");
    fclose(file);
    return 0;
}

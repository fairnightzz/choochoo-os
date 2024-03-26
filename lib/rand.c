#include "rand.h"
#include "kern/timer.h"

uint32_t rand_num;
uint32_t seed;
void rand_init(void)
{
    seed = timer_get();
    rand_num = seed;
}

uint32_t rand_int(void)
{
    // https://en.wikipedia.org/wiki/Linear_congruential_generator
    rand_num = (1103515245 * rand_num + 12345) % 2147483648;
    return rand_num;
}

uint32_t get_rand_seed(void)
{
    return seed;
}
#include "k2-perf.h"
#include "stdlib.h"

#define TEST_ITERATIONS 20

static int receive_tid;
static int sender_tid;

void send_perf_test(uint32_t msglen)
{
    // char *send_msg = alloc_variable_size(sizeof(char) * msglen);
    // char *reply_buf = alloc_variable_size(sizeof(char) * msglen);
    char send_msg[300];
    char reply_buf[300];

    PerfTimingState timer = new_perf_timing_state();
    int64_t last_iteration;
    for (uint32_t i = 0; i < TEST_ITERATIONS; i++)
    {
        set_start(&timer, SSR_TIME);
        Send(receive_tid, send_msg, msglen, reply_buf, msglen);
        set_end(&timer, SSR_TIME);
        last_iteration = get_perf_time(&timer, SSR_TIME);
    }
    // free_variable_size(reply_buf);
    // free_variable_size(send_msg);
    PRINT("Last Iteration: %d, Average %d, Max %d", last_iteration, get_avg_perf_time(&timer, SSR_TIME), get_max_perf_time(&timer, SSR_TIME));
    Exit();
}

void receive_perf_test(uint32_t msglen)
{
    int from_tid;
    // char *receive_buf = alloc_variable_size(sizeof(char) * msglen);
    // char *reply_msg = alloc_variable_size(sizeof(char) * msglen);
    char receive_buf[300];
    char reply_msg[300];

    for (uint32_t i = 0; i < TEST_ITERATIONS; i++)
    {
        Receive((int *)&from_tid, receive_buf, msglen);
        Reply(sender_tid, reply_msg, msglen);
    }
    // free_variable_size(receive_buf);
    // free_variable_size(reply_msg);
    Exit();
}

void send4() { send_perf_test(4); }
void send64() { send_perf_test(64); }
void send256() { send_perf_test(256); }

void receive4() { receive_perf_test(4); }
void receive64() { receive_perf_test(64); }
void receive256() { receive_perf_test(256); }

void k2_performance_measuring()
{
    // PRINT("Send-first, 4 bytes");
    // sender_tid = Create(11, &send4);
    // receive_tid = Create(12, &receive4);

    // PRINT("Send-first, 64 bytes");
    // sender_tid = Create(13, &send64);
    // receive_tid = Create(14, &receive64);

    // PRINT("Send-first, 256 bytes");
    // sender_tid = Create(15, &send256);
    // receive_tid = Create(16, &receive256);

    // PRINT("Receive-first, 4 bytes");
    // sender_tid = Create(18, &send4);
    // receive_tid = Create(17, &receive4);

    // PRINT("Receive-first, 64 bytes");
    // sender_tid = Create(20, &send64);
    // receive_tid = Create(19, &receive64);

    // PRINT("Receive-first, 256 bytes");
    // sender_tid = Create(22, &send256);
    // receive_tid = Create(21, &receive256);
}

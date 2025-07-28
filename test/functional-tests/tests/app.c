#include <stdio.h>
#include <stdlib.h>
#include "../../../include/telemetry_busmessage_sender.h"

void main(int argc, char *argv[])
{
    int i=0, n;
    n= (argc < 2)?100:atoi(argv[1]);
    // Initialize Telemetry2.0
    t2_init("event_spammer");

    while(i<=n)
    {
	t2_event_d("T2_INFO_Test", i);
        i++;
    }
    t2_uninit();
    printf("Sent %d t2_event_d events.\n", n);
}



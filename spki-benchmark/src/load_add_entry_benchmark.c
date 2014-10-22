#define _GNU_SOURCE

#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include "util/getusage.c"
#include <signal.h>
#include <sys/time.h>
#include "spki_test_data.h"

unsigned int get_timediff(const struct timeval start,
        const struct timeval end) {
    return (((end.tv_sec * 1000000) + end.tv_usec) -
            ((start.tv_sec * 1000000) + start.tv_usec));
}

void sig_handler(){
    fprintf(stderr, "FLUSHING\n");
    fflush(NULL);
}


int main(int argc, char* argv[])
{
    if(argc != 3){
        printf("Usage:\n");
        printf("%s <num_of_records_to_create> <num_of_passes>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    unsigned int num_of_records_to_create = atoi(argv[1]);
    unsigned int passes = atoi(argv[2]);

    const pid_t pid = getpid();
    struct pstat start_cpu;
    struct pstat end_cpu;
    long unsigned int ucpu_usage;
    long unsigned int scpu_usage;

    double average_usecs = 0;
    for(unsigned int i = 0; i < passes; i++){
        struct spki_table spkit;
        spki_test_data* test_data;
        struct spki_record* records;
        spki_table_init(&spkit, NULL);

        printf("Generate records...\n");
        test_data = spki_test_data_new();
        test_data = spki_test_data_add_records(test_data, num_of_records_to_create);
        records = spki_test_data_get_records(test_data);

        printf("Start measurement... Pass %u\n", i);

        if(get_usage(pid, &start_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            spki_test_data_free(test_data);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }
        struct timeval stime;
        gettimeofday(&stime, NULL);

        //Add records
        for(unsigned int i = 0; i < spki_test_data_size(test_data); i++){
            assert(spki_table_add_entry(&spkit, &records[i]) == SPKI_SUCCESS);
        }

        struct timeval etime;
        gettimeofday(&etime, NULL);
        if(get_usage(pid, &end_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            spki_test_data_free(test_data);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }

        calc_cpu_usage(&end_cpu, &start_cpu, &ucpu_usage, &scpu_usage);
        printf("cpu ticks: %lu\n", ucpu_usage + scpu_usage);
        printf("usecs: %u\n\n", get_timediff(stime, etime));
        average_usecs += get_timediff(stime, etime);
        spki_table_free(&spkit);
        spki_test_data_free(test_data);
    }

    printf("Average %.6f s\n", average_usecs/(double)passes/(1000.0*1000.0));
    return 0;
}

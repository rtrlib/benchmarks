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
    struct pstat start;
    struct pstat end;

    unsigned long long int average_rss = 0;
    spki_test_data* test_data;
    for(unsigned int i = 0; i < passes; i++){
        struct spki_table spkit;
        spki_table_init(&spkit, NULL);

        struct spki_record* records;
        test_data = spki_test_data_new();
        test_data = spki_test_data_add_records(test_data, num_of_records_to_create);
        records = spki_test_data_get_records(test_data);

        printf("Start measurement... Pass %u\n", i);
        if(get_usage(pid, &start) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            spki_test_data_free(test_data);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }

        for(unsigned int i = 0; i < num_of_records_to_create; i++){
            if(spki_table_add_entry(&spkit, &records[i]) == SPKI_ERROR){
                printf("Error while adding record %u\n", i);
                spki_test_data_free(test_data);
                spki_table_free(&spkit);
                exit(EXIT_FAILURE);
            }
        }

        if(get_usage(pid, &end) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            spki_test_data_free(test_data);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }
        printf("RSS %lu byte\n", end.rss-start.rss);
        average_rss += (end.rss-start.rss);

        spki_table_free(&spkit);
    }

    printf("\n\nAverage RSS\n%llu Byte\n%llu MiB\n", average_rss/passes, average_rss/(1024*1024)/passes);
    spki_test_data_free(test_data);
    return 0;
}

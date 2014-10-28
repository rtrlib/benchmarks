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
        const struct timeval end)
{
    return (((end.tv_sec * 1000000) + end.tv_usec) -
            ((start.tv_sec * 1000000) + start.tv_usec));
}

void sig_handler()
{
    fprintf(stderr, "FLUSHING\n");
    fflush(NULL);

}

int main(int argc, char* argv[])
{
    if(argc != 4){
        printf("Usage:\n");
        printf("%s <num_of_records_to_create> <num_of_passes> <duplicated_asn_chance>\n", argv[0]);
        printf("duplicated_asn_chance - Chance in percent that an ASN is more then one time in the list.\n");
        exit(EXIT_SUCCESS);
    }

    unsigned int num_of_records_to_create = atoi(argv[1]);
    unsigned int passes = atoi(argv[2]);
    double chance_duplicated_asn = atoi(argv[3]);

    const pid_t pid = getpid();
    struct pstat start_cpu;
    struct pstat end_cpu;
    long unsigned int ucpu_usage;
    long unsigned int scpu_usage;

    unsigned int average_usecs = 0;
    struct spki_table spkit;
    spki_test_data* test_data;

    struct spki_record* records;
    test_data = spki_test_data_new();
    test_data = spki_test_data_add_records(test_data, num_of_records_to_create -
                                           (num_of_records_to_create * (chance_duplicated_asn / 100)));
    test_data = spki_test_data_duplicate_random_records(test_data,
                                                        (num_of_records_to_create * (chance_duplicated_asn / 100)));
    printf("Number of duplicates: %u\n", (int)(num_of_records_to_create *
                                          (chance_duplicated_asn / 100)));

    test_data = spki_test_data_shuffle(test_data);
    records = spki_test_data_get_records(test_data);

    for(unsigned int i = 0; i < passes; i++){
        spki_table_init(&spkit, NULL);

        for(unsigned int i = 0; i < spki_test_data_size(test_data); i++){
            assert(spki_table_add_entry(&spkit, &records[i]) == SPKI_SUCCESS);
        }

        printf("Start measurement... Pass %u\n", i);

        if(get_usage(pid, &start_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            spki_test_data_free(test_data);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }
        struct timeval stime;
        gettimeofday(&stime, NULL);

        //Get all records
        struct spki_record* result;
        unsigned int result_size;
        for(unsigned int j = 0; j < num_of_records_to_create; j++){
            if(spki_table_get_all(&spkit, records[j].asn, records[j].ski, &result, &result_size) == SPKI_ERROR){
                printf("spki_table_get_all() error !\n");
                spki_test_data_free(test_data);
                exit(EXIT_FAILURE);
            }
            assert(result_size == 1);
            free(result);
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
    }

    spki_test_data_free(test_data);
    printf("Average %.6f seconds\n", average_usecs/(double)passes/(1000.0*1000.0));
    printf("Average %.6f microseconds\n", average_usecs/(double)passes);
    return 0;
}

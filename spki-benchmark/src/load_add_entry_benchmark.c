#define _GNU_SOURCE

#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include "util/getusage.c"
#include <signal.h>
#include <sys/time.h>


static struct spki_record* create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket) {
    struct spki_record *record = malloc(sizeof(struct spki_record));
    record->asn = ASN;
    uint32_t i;

    for(i = 0; i < sizeof(record->ski); i++){
        record->ski[i] = i + ski_offset;
    }

    for(i = 0; i < sizeof(record->spki); i++){
        record->spki[i] = i + spki_offset;
    }
    record->socket = socket;
    return record;
}

unsigned int get_timediff(const struct timeval start,
        const struct timeval end) {
    return (((end.tv_sec * 1000000) + end.tv_usec) -
            ((start.tv_sec * 1000000) + start.tv_usec));
}

void sig_handler(){
    fprintf(stderr, "FLUSHING\n");
    fflush(NULL);

}

void generate_spki_records(struct spki_record **records, unsigned int num_of_records){
    *records = malloc(num_of_records * sizeof(**records));
    if(*records == NULL){
        printf("malloc error\n");
        exit(EXIT_FAILURE);
    }

    printf("\nGenerating %i spki_records...\n", num_of_records);

    srand (time(NULL));

    //Create i SPKI records with different ASN but sometimes same SKI/SPKI
    for(unsigned int i = 0; i < num_of_records; i++){
        int ski = rand() % (num_of_records/2);
        int spki = rand() % (num_of_records/2);
        struct rtr_socket* socket = (struct rtr_socket*) 0x13;
        struct spki_record* record = create_record(i,ski, spki, socket);

        memcpy(&(*records)[i], record, sizeof(struct spki_record));
        free(record);
    }
    printf("Done");
    printf("\n\n");
}

void fill_router_key_table(struct spki_table* spki_table, struct spki_record **records, unsigned int num_of_records){
    for(unsigned int i = 0; i < num_of_records; i++){
        if(spki_table_add_entry(spki_table, &(*records)[i]) == SPKI_ERROR){
            free(*records);
            printf("Faild to add record %u", i);
            exit(EXIT_FAILURE);
        }
    }
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

    struct spki_record* records;
    generate_spki_records(&records, num_of_records_to_create);


    const pid_t pid = getpid();
    struct pstat start_cpu;
    struct pstat end_cpu;
    long unsigned int ucpu_usage;
    long unsigned int scpu_usage;

    printf("Adding records to spki_table...\n");

    double average_usecs = 0;
    for(unsigned int i = 0; i < passes; i++){
        struct spki_table spkit;
        spki_table_init(&spkit, NULL);

        printf("Start measurement... Pass %u\n", i);

        if(get_usage(pid, &start_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            free(records);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }
        struct timeval stime;
        gettimeofday(&stime, NULL);

        fill_router_key_table(&spkit, &records, num_of_records_to_create);

        struct timeval etime;
        gettimeofday(&etime, NULL);
        if(get_usage(pid, &end_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            free(records);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }

        calc_cpu_usage(&end_cpu, &start_cpu, &ucpu_usage, &scpu_usage);
        printf("cpu ticks: %lu\n", ucpu_usage + scpu_usage);
        printf("usecs: %u\n\n", get_timediff(stime, etime));
        average_usecs += get_timediff(stime, etime);
        spki_table_free(&spkit);
    }

    printf("Average %.6f s\n", average_usecs/(double)passes/(1000.0*1000.0));


    free(records);

    return 0;
}

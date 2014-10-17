#define _GNU_SOURCE

#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include "util/getusage.c"
#include <signal.h>
#include <sys/time.h>

struct spki_table spkit;

static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket) {
    struct spki_record *record = malloc(sizeof(struct spki_record));
    memset(record, 0, sizeof(*record));

    record->asn = ASN;
    uint32_t i;

    for(i = 0; i < sizeof(record->ski)/sizeof(u_int32_t); i++){
        ((u_int32_t*)record->ski)[i] = i + ski_offset;
    }

    for(i = 0; i < sizeof(record->spki)/sizeof(u_int32_t); i++){
        ((u_int32_t*)record->spki)[i] = i + spki_offset;
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

void switch_records(struct spki_record* r1, struct spki_record* r2){
    struct spki_record tmp_record;
    memcpy(&tmp_record, r2, sizeof(tmp_record));
    memcpy(r2, r1, sizeof(tmp_record));
    memcpy(r1, r2, sizeof(tmp_record));
}

void generate_records(unsigned int number_of_records, unsigned int duplicates_quota_in_percent,
                      struct spki_record** records){
    *records = malloc(number_of_records * sizeof(**records));
    srand(time(NULL));
    struct spki_record* record;
    struct rtr_socket* socket = (struct rtr_socket*) 0x13;

    unsigned int number_of_duplicats = (number_of_records * duplicates_quota_in_percent)/100;

    //Generate (number_of_records-number_of_duplicats) records.
    for(unsigned int i = 0; i < (number_of_records-number_of_duplicats); i++){
        record = create_record(i, i, i, socket);

        assert(spki_table_add_entry(&spkit, record) == SPKI_SUCCESS);
        memcpy(&(*records)[i], record, sizeof(*record));
        free(record);
    }

    //Generated the duplicated records
    for(unsigned int i = (number_of_records - number_of_duplicats); i < number_of_records; i++){
        unsigned int record_index = rand()%(number_of_records-number_of_duplicats+1);
        printf("Duplicate of %u created\n", record_index);
        fflush(NULL);
        record = create_record((*records)[record_index].asn, i, i, socket);

        assert(spki_table_add_entry(&spkit, record) == SPKI_SUCCESS);
        memcpy(&(*records)[i], record, sizeof(*record));
        free(record);
    }

    //Shuffle the records
    for(unsigned int i = 0; i < number_of_records; i++){
        unsigned int rand1 = rand()%number_of_records;
        unsigned int rand2 = rand()%number_of_records;
        switch_records(&(*records)[rand1], &(*records)[rand2]);
    }
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
    for(unsigned int i = 0; i < passes; i++){
        spki_table_init(&spkit, NULL);

        printf("Adding records to spki_table...\n");
        struct spki_record* records;
        generate_records(num_of_records_to_create, chance_duplicated_asn, &records);

        printf("Start measurement... Pass %u\n", i);

        if(get_usage(pid, &start_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            free(records);
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
                free(records);
                exit(EXIT_FAILURE);
            }
            assert(result_size == 1);
            free(result);
        }

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
        free(records);
    }
    printf("Average %.6f s\n", average_usecs/(double)passes/(1000.0*1000.0));
    return 0;
}

#define _GNU_SOURCE

#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include "getusage.c"
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
    (*records) = malloc((num_of_records) * sizeof(struct spki_record));
    if((*records) == NULL){
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
    struct pstat start;
    struct pstat end;
    printf("Adding records to spki_table...\n");


    unsigned int average_rss = 0;
    for(unsigned int i = 0; i < passes; i++){
        struct spki_table spkit;
        spki_table_init(&spkit, NULL);

        printf("Pass %u\n", i);

        if(get_usage(pid, &start) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            free(records);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }


        for(unsigned int i = 0; i < num_of_records_to_create; i++){
            if(spki_table_add_entry(&spkit, &records[i]) == SPKI_ERROR){
                printf("Error while adding record %u\n", i);
                free(records);
                spki_table_free(&spkit);
                exit(EXIT_FAILURE);
            }
        }

        if(get_usage(pid, &end) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            free(records);
            spki_table_free(&spkit);
            exit(EXIT_FAILURE);
        }
        printf("RSS %u byte\n", end.rss-start.rss);
        printf("RSS %u MB\n\n", (end.rss-start.rss)/1024/1024);
        average_rss += (end.rss-start.rss);

        spki_table_free(&spkit);
    }

    printf("Average RSS\n%u Byte\n%u MB\n", average_rss, average_rss/(1024*1024)/passes);

    free(records);



    return 0;
}

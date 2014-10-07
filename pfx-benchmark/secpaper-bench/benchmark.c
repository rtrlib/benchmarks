#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "getusage.c"
#include <signal.h>
#include <sys/time.h>

unsigned int get_timediff(const struct timeval start,
        const struct timeval end);

void sig_handler(){
    fprintf(stderr, "FLUSHING\n");
    fflush(NULL);

}

inline unsigned int get_timediff(const struct timeval start,
        const struct timeval end) {
    return (((end.tv_sec * 1000000) + end.tv_usec) -
            ((start.tv_sec * 1000000) + start.tv_usec));
}

struct val_pref{
    ip_addr prefix;
    uint32_t asn;
    uint8_t mask_len;
};

void read_prefixes(const char* filename, struct val_pref** recs, unsigned int*
        recs_len){
    unsigned int records_added = 0;
    char buf[100];

    printf("\nLoading prefixes from %s into memory...\n", filename);
    FILE* f = fopen(filename, "r");
    if(f == NULL){
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    int fscanf_rtval = 1;
    do{
        if(*recs_len < records_added +1){
            void* tmp = realloc(*recs, (*recs_len +1) * sizeof(struct val_pref));
            if(tmp == NULL){
                perror("realloc error");
                exit(EXIT_FAILURE);
                free(recs);
            }
            *recs = tmp;
        }

        printf("\rLoading prefix: %6u", records_added);
        fflush(stdout);

        fscanf_rtval = fscanf(f, "%s %hhu %u", buf,
                &((*recs)[records_added].mask_len),
                &((*recs)[records_added].asn));
        if(ip_str_to_addr(buf, &((*recs)[records_added].prefix)) == -1){
            fprintf(stderr, "rtr_str_to_ipaddr error: %s",buf);
            exit(EXIT_FAILURE);
        }
        if(fscanf_rtval != EOF){
            records_added++;
            (*recs_len)++;
        }
        else{
            void* tmp = realloc(*recs, (*recs_len) * sizeof(struct val_pref));
            if(tmp == NULL){
                perror("realloc error");
                exit(EXIT_FAILURE);
                free(recs);
            }
            *recs = tmp;
        }


    } while(fscanf_rtval != EOF);
    printf("\rLoading prefix: %6u", records_added);

    fclose(f);
    printf("\n");

}

void fill_pfx_table(pfx_table* pfxt, const char* filename){
    unsigned int records_added = 0;
    pfx_record rec;
    char buf[100];

    printf("Loading ROAs from %s into pfx_table...\n", filename);
    FILE* f = fopen(filename, "r");
    if(f == NULL){
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    while(fscanf(f, "%s %hhu %hhu %u", buf, &rec.min_len, &rec.max_len, 
                &rec.asn) != EOF){

        printf("\rAdding record: %6u", records_added +1);
        fflush(stdout);

        if(ip_str_to_addr(buf, &rec.prefix) == -1){
            fprintf(stderr, "rtr_str_to_ipaddr error");
            exit(EXIT_FAILURE);
        }
        if(pfx_table_add(pfxt, &rec) == PFX_ERROR){
            fprintf(stderr, "\nError adding the following record: "
                    "%s %u %u %u", buf, rec.min_len, rec.max_len, rec.asn);
        }

        //validate added prefix to be sure that the pfx_table works correct
        pfxv_state val_state;
        if(pfx_table_validate(pfxt, rec.asn, &rec.prefix, rec.min_len, &val_state)
                != PFX_SUCCESS){
            fprintf(stderr, "\npfx_table_validate unsucessfull");

            if(val_state != BGP_PFXV_STATE_VALID){
                printf("\nError added pfx_record (%s %u %u %u)"
                        "could'nt be validated \n",
                        buf, rec.min_len, rec.max_len, rec.asn);
            }
            exit(EXIT_FAILURE);
        }

        records_added++;
    }
    printf("\rAdding record: %6u", records_added);

    fclose(f);
    printf("\n");
}


int main(int argc, char* argv[])
{
    unsigned int passes;
    char roa_file[200];
    char val_file[200];
    char logfile_prefix[200];
    if(argc != 5){
        printf("Usage:\n");
        printf("%s <passes> <roa-file> <validation-prefix-file>"
                " <logfile-prefix>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    passes = atoi(argv[1]);
    strncpy(roa_file, argv[2], sizeof(roa_file));
    strncpy(val_file, argv[3], sizeof(roa_file));
    strncpy(logfile_prefix, argv[4], sizeof(logfile_prefix));

    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);
    fill_pfx_table(&pfxt, roa_file);

    unsigned int prefix_len = 0;
    struct val_pref* prefixes = NULL;
    read_prefixes(val_file, &prefixes, &prefix_len);

    printf("\n");

    char logfile_s[250];
    snprintf(logfile_s, sizeof(logfile_s), "%s", logfile_prefix);
    FILE* f = fopen(logfile_s, "a");
    if(f == NULL) {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "#utime_ticks stime_ticks usecs valid_prefixes"
            " invalid_prefixes not_found_prefixes\n");
    for(unsigned int i = 0; i < passes; i++) {
        pid_t pid = getpid();

        printf("Validating Prefixes, pass: %u/%u, Logfile : %s\n", i ,
                passes, logfile_s);

        struct pstat start_cpu;
        if(get_usage(pid, &start_cpu) == -1){
            fprintf(stderr, "GET USAGE ERROR\n");
            exit(EXIT_FAILURE);
        }

        struct timeval stime;
        gettimeofday(&stime, NULL);

        unsigned int valid = 0;
        unsigned int invalid = 0;
        unsigned int not_found = 0;
        for(unsigned int j = 0; j < prefix_len ; j++){
            pfxv_state val_state;
            if (pfx_table_validate(&pfxt, prefixes[j].asn,
                        &(prefixes[j].prefix), prefixes[j].mask_len,
                        &val_state) == PFX_ERROR){
                exit(EXIT_FAILURE);
            }
            switch(val_state){
                case BGP_PFXV_STATE_VALID:
                    valid++; break;
                case BGP_PFXV_STATE_INVALID:
                    invalid++; break;
                case BGP_PFXV_STATE_NOT_FOUND:
                    not_found++; break;
            }

        }
        struct pstat end_cpu;
        if(get_usage(pid, &end_cpu) == -1) {
            fprintf(stderr, "GET USAGE ERROR\n");
        }
        struct timeval etime;
        gettimeofday(&etime, NULL);

        unsigned int usecs = get_timediff(stime, etime);

        long unsigned int ucpu = 0;
        long unsigned int scpu = 0;
        calc_cpu_usage(&end_cpu, &start_cpu, &ucpu, &scpu);
        fprintf(f, "%lu %lu %u %u %u %u\n", ucpu, scpu, usecs, valid, invalid,
                not_found);
    }

    //tidy up
    fclose(f);
    pfx_table_free(&pfxt);
    free(prefixes);
    struct pstat sum_end_cpu;
    if(get_usage(getpid(), &sum_end_cpu) == -1){
        fprintf(stderr, "GET USAGE ERROR\n");    
        exit(EXIT_FAILURE);
    }
    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "getusage.c"
#include <signal.h>
#include <stdio_ext.h>
#include <stdbool.h>

FILE* pref_f; 
FILE* bench_f; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned long int valid_state = 0;
unsigned long int invalid_state = 0;
unsigned long int not_found_state = 0;

void sig_handler(){
    fprintf(stderr, "FLUSHING\n");
    fflush(NULL);

}

void read_announcement(unsigned int* asn, char* prefix, unsigned int* prefix_len, char* as_path){
    bool retry=true;
    while (retry) {
        if(fscanf(stdin,"%u %s %u %s\n", asn, prefix, prefix_len, as_path) == EOF){
            if (ferror(stdin) != 0) {
                perror("FSCANF ERROR\n");
                exit(EXIT_FAILURE);
            }
            else{
                clearerr(stdin);
                printf("fscanf returned EOF, but ferror not set\n");
            }
        }
        else{
            retry=false;
            printf("Retrying to read from stdin\n");
        }
    }
}
void* log_thr(){
    pid_t pid = getpid();
    printf("PID: %u\n", pid);
    printf("PAGESIZE: %u\n", getpagesize());

    struct pstat last_cpu;
    struct pstat cur_cpu;
    double ucpu;
    double scpu;
    time_t curtime;

    if(get_usage(pid, &cur_cpu) == -1){
        fprintf(stderr, "GET USAGE ERROR\n");    
        exit(EXIT_FAILURE);
    }

    fprintf(bench_f, "%s", "#clock date VALID_STATES INVALID_STATES NOTFOUND_STATES CPU_USAGE VSIZE RSS\n");
    while(true){
        last_cpu = cur_cpu;
        get_usage(pid, &cur_cpu);
        calc_cpu_usage(&cur_cpu, &last_cpu, &ucpu, &scpu);
        curtime = time(NULL);
        pthread_mutex_lock(&mutex);
        char tmp[100];
        time_t t = time(NULL);
        struct tm* tm = localtime(&t);
        strftime(tmp, sizeof(tmp), "%R:%S %F", tm);
        fprintf(bench_f,"%s %lu %lu %lu %f %lu %lu\n",tmp, valid_state, invalid_state, not_found_state, ucpu+scpu, cur_cpu.vsize, cur_cpu.rss);
        fflush(NULL);
        valid_state = 0;
        invalid_state = 0;
        not_found_state = 0;
        pthread_mutex_unlock(&mutex);
        /*
        if((curtime - starttime) >= 86400){
            exit(EXIT_SUCCESS);
        }
        */
        sleep(60);
    }
}

void fill_pfx_table(pfx_table* pfxt){
    FILE* df = fopen("pfx_records.txt", "r");
    if(df == NULL){
        fprintf(stderr, "cant open pfx_records.txt");
        exit(EXIT_FAILURE);
    }
    char ip[256];
    unsigned int pref_len;
    unsigned int asn;
    unsigned int count = 0;
    pfx_record rec;
    while (fscanf(df, "%s %u %u", ip, &pref_len, &asn) != EOF){
        rec.max_len = pref_len;
        rec.min_len = pref_len;
        rec.asn = asn;
        rec.socket_id = 990;
        printf("%u: IP: %s/%u %u\n", count, ip, pref_len, asn);
        ip_version ver;
        if (strchr("ip", ':') == NULL)
            ver = IPV6;
        else
            ver = IPV4;
        if(ip_str_to_addr(ip, &(rec.prefix)) == -1){
            fprintf(stderr, "ip_str_to_addr_error\n");
            exit(EXIT_FAILURE);
        }
        if(pfx_table_add(pfxt, &rec) == PFX_ERROR){
            fprintf(stderr, "pfx_table_add error\n");
            exit(EXIT_FAILURE);
        }
        char tmp[100];
        ip_addr_to_str(&(rec.prefix), tmp, sizeof(tmp));
        //printf("IP: %s/%u-%u %u\n", t, rec.min_len, rec.max_len,rec.asn);

        pfxv_state state;
        pfx_table_validate(pfxt, rec.asn, &(rec.prefix), rec.min_len, &state);
       if(state != BGP_PFXV_STATE_VALID){
           printf("Error added pfx_record couldnt be validated as valid\n");
           exit(EXIT_FAILURE);
       }
        count++;
    }
    printf("added %u records to the pfx_table\n", count);
    fclose(df);
}

int main()
{
    signal(SIGINT, &sig_handler);

    tr_tcp_config tcp_config = {
        "rpki.realmv6.org",          //IP
        "42420"                      //Port
    };
    tr_socket tr_tcp;
    tr_tcp_init(&tcp_config, &tr_tcp);
    rtr_socket rtr_tcp;
    rtr_tcp.tr_socket = &tr_tcp;

    tr_tcp_config tcp1_config = {
        "rpki.realmv6.org",          //IP
        "8282"                      //Port
    };
    tr_socket tr_tcp1;
    tr_tcp_init(&tcp1_config, &tr_tcp1);
    rtr_socket rtr_tcp1;
    rtr_tcp1.tr_socket = &tr_tcp1;

    rtr_mgr_group groups[2];
    groups[0].sockets_len = 1;
    groups[0].sockets = malloc(1 * sizeof(rtr_socket*));
    groups[0].sockets[0] = &rtr_tcp;
    groups[0].preference = 1;
    groups[1].sockets_len = 1;
    groups[1].sockets = malloc(1 * sizeof(rtr_socket*));
    groups[1].sockets[0] = &rtr_tcp1;
    groups[1].preference = 2;

    rtr_mgr_config conf;
    conf.groups = groups;
    conf.len = 2;

    rtr_mgr_init(&conf, 240, 520, NULL);
    rtr_mgr_start(&conf);

    //fill_pfx_table(groups[1].sockets[0]->pfx_table);
    while(!rtr_mgr_conf_in_sync(&conf)){
        sleep(1);
    }


    unsigned int asn;
    char prefix[256] = "";
    unsigned int prefix_len;
    ip_addr ip_addr;
    pfxv_state state;
    char tmp[100];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    strftime(tmp, sizeof(tmp), "log/prefixes-%F", tm);
    strcat(tmp, ".log");
    int day = tm->tm_mday;
    pref_f = fopen(tmp, "w");
    if(pref_f == NULL){
        printf("can't open %s\n", tmp);
        exit(EXIT_FAILURE);
    }
    fprintf(pref_f, "#CLOCK DATE;ANNOUNCED_PREFIX PREF_LEN ASN;AS_PATH_ELEM{,AS_PATH_ELEM};ROA_ASN ROA_PREFIX/ROA_MIN_LEN-ROA_MAX_LEN {,ROA_ASN ROA_PREFIX/ROA_MIN_LEN-ROA_MAX_LEN };VAL_STATE");

    strftime(tmp, sizeof(tmp), "log/benchmark-%F", tm);
    strcat(tmp, ".log");
    bench_f = fopen(tmp, "w");
    if(bench_f == NULL){
        printf("can't open %s\n", tmp);
        exit(EXIT_FAILURE);
    }

    //purge stdin buffer
    __fpurge(stdin);

    pthread_t thrd_id;
    pthread_create(&thrd_id, NULL, &log_thr, NULL);


    printf("Benchmark started\n");
    char as_path[1024];

    pfx_record* reason = NULL;
    unsigned int reason_len = 0;

    while(true){
        read_announcement(&asn, prefix, &prefix_len, as_path);
        strftime(tmp, sizeof(tmp), "%R:%S %F", tm);
        if(ip_str_to_addr(prefix, &ip_addr) == -1){ fprintf(stderr, "ERROR STR TO IPADDR\n");
            exit(EXIT_FAILURE);
        }
        if(pfx_table_validate_r(conf.groups[0].sockets[0]->pfx_table, &reason, &reason_len, asn, &ip_addr, prefix_len, &state) == -1){
            fprintf(stderr, "VALIDATE ERROR\n");
            exit(EXIT_FAILURE);
        }
        if(fprintf(pref_f, "%s;%s %u %u;%s;", tmp, prefix, prefix_len, asn, as_path) < 0)
            perror("printf error");

        if(reason != NULL && reason_len > 0){
            for(unsigned int i = 0; i < reason_len; i++){
                ip_addr_to_str(&(reason[i].prefix), tmp, sizeof(tmp));
                fprintf(pref_f, "%u %s/%u-%u", reason[i].asn, tmp, reason[i].min_len, reason[i].max_len);
                if((i+1) < reason_len)
                    fprintf(pref_f, ",");

            }
            fprintf(pref_f, ";");
        }
        if(state == BGP_PFXV_STATE_VALID){
            pthread_mutex_lock(&mutex);
            valid_state++;
            pthread_mutex_unlock(&mutex);
            if (fprintf(pref_f, "VALID\n") < 0 )
                perror("printf error");
        }
        else if (state == BGP_PFXV_STATE_INVALID){
            pthread_mutex_lock(&mutex);
            invalid_state++;
            pthread_mutex_unlock(&mutex);
            if (fprintf(pref_f, "INVALID\n") < 0)
                perror("printf error");
        }
        else if (state == BGP_PFXV_STATE_NOT_FOUND){
            pthread_mutex_lock(&mutex);
            not_found_state++;
            pthread_mutex_unlock(&mutex);
            if (fprintf(pref_f, "NOT_FOUND\n") < 0)
                perror("printf error");
        }

        //tagwechsel
        //
        t = time(NULL);
        if(t == ((time_t) -1)){
            perror("time returned error value");
            exit(EXIT_FAILURE);
        }
        struct tm* tm = localtime(&t);
        tm = localtime(&t);
        if(tm == NULL){
            perror("localtime error");
            exit(EXIT_FAILURE);
        }
        if(tm->tm_mday != day){
            day = tm->tm_mday;
            fclose(bench_f);
            fclose(pref_f);
            strftime(tmp, sizeof(tmp), "log/prefixes-%F", tm);
            strcat(tmp, ".log");
            pref_f = fopen(tmp, "w");
            if(pref_f == NULL){
                printf("can't open prefixes.log\n");
                exit(EXIT_FAILURE);
            }

            pthread_mutex_lock(&mutex);
            strftime(tmp, sizeof(tmp), "log/benchmark-%F", tm);
            strcat(tmp, ".log");
            bench_f = fopen(tmp, "w");
            pthread_mutex_unlock(&mutex);
        }

    }
    fclose(bench_f);
    fclose(pref_f);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "/home/fho/git/rtrlib/rtrlib/pfx/lpfst/lpfst-pfx.c"


int main()
{
    tr_tcp_config tcp_config = {
        "141.22.26.252",          //IP
        "8282"                      //Port
    };
    tr_socket tr_tcp;
    tr_tcp_init(&tcp_config, &tr_tcp);
    rtr_socket rtr_tcp;
    rtr_tcp.tr_socket = &tr_tcp;

    tr_tcp_config tcp1_config = {
        "141.22.27.161",          //IP
        "42420"                      //Port
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

    while(!rtr_mgr_conf_in_sync(&conf)){
        sleep(1);
    }
    printf("RTR-MGR rdy\n");
    FILE* f = fopen("prefixes.log", "r");
    //FILE* fn = fopen("prefixes-new.log", "w");
    if(f ==NULL)
        exit(EXIT_FAILURE);
    char ip[256];
    unsigned int asn;
    unsigned int pref_len;
    char old_state[50];
    struct pfx_table* pfxt = groups[0].sockets[0]->struct pfx_table;

    printf("#PREFIX PREFIX_LEN ASN;ROA_PREFIX ROA_MIN_LEN ROA_MAX_LEN ROA_ASN{,ROA_PREFIX ROA_MIN_LEN ROA_MAX_LEN ROA_ASN}\n");

    while (fscanf(f, "%s\t%u\t%u\t%s", ip, &pref_len, &asn, old_state) != EOF){
        pfxv_state result = BGP_PFXV_STATE_NOT_FOUND;
        ip_addr prefix;
        if(ip_str_to_addr(ip, &prefix) == -1)
            exit(EXIT_FAILURE);

        pthread_rwlock_rdlock(&(pfxt->lock));
        lpfst_node* root = pfxt->ipv4;
        lpfst_node* inv_node[50];
        bzero(inv_node, sizeof(inv_node));
        int inv_ind = 0;

        if(root == NULL){
            pthread_rwlock_unlock(&pfxt->lock);
            result = BGP_PFXV_STATE_NOT_FOUND;
        }
        else{
            unsigned int lvl = 0;
            lpfst_node* node = lpfst_lookup(root, &prefix, pref_len, &lvl);
            if(node == NULL){
                pthread_rwlock_unlock(&pfxt->lock);
                result = BGP_PFXV_STATE_NOT_FOUND;
            }
            else if(node != NULL){
                result = BGP_PFXV_STATE_VALID;
                inv_node[inv_ind] =  node;
                inv_ind++;
                while(!struct pfx_table_elem_matches(node->data, asn, pref_len)){
                    if(ip_addr_is_zero(ip_addr_get_bits(&prefix, lvl, 1)))
                        node = node->lchild;
                    else
                        node = node->rchild;
                    lvl++;
                    node = lpfst_lookup(node, &prefix, pref_len, &lvl);
                    if(node == NULL){
                        pthread_rwlock_unlock(&pfxt->lock);
                        result = BGP_PFXV_STATE_INVALID;
                        break;
                    }
                    inv_node[inv_ind] =  node;
                    inv_ind++;
                }
                pthread_rwlock_unlock(&pfxt->lock);
            }
        }
        if(strncmp("INVALID", old_state, sizeof(old_state)) == 0){
        //printf("%s\n", old_state);
            //if(result == BGP_PFXV_STATE_NOT_FOUND)
                //printf("new state: NOT FOUND");
        //if(result == BGP_PFXV_STATE_VALID)
            //printf("VALID\n");
        if(result ==  BGP_PFXV_STATE_INVALID){
            printf("%s %u %u;", ip, pref_len, asn);
                for(int i =0;i< inv_ind+1;i++){
                    lpfst_node* node = inv_node[i];
                    if(node != NULL){
                        if(i > 0)
                            printf(",");
                        char tmp[256];
                        ip_addr_to_str(&(node->prefix), tmp, sizeof(tmp));
                        node_data* data = node->data;
                        for(unsigned int i = 0; i < data->len; i++){
                            if(i > 0)
                                printf(",");
                            printf("%s %u %u %u", tmp, node->len, data->ary[i].max_len, data->ary[i].asn);
                        }
                    }
                }
                printf("\n");
            }
        }
    }
    printf("DONE\n");
}

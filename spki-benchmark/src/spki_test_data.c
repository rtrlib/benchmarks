/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * written by Nils Bars, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

/*
* Util to generate test data to benchmark the spki_table
*/


#include "rtrlib/rtrlib.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include <string.h>
#include <stdio.h>
#include "spki_test_data.h"

static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket)
{
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

spki_test_data* spki_test_data_new(){
    spki_test_data* data = malloc(sizeof(spki_test_data));
    data->size = 0;
    data->records = NULL;
    return data;
}

void spki_test_data_free(spki_test_data* data)
{
    free(data->records);
    free(data);
}

struct spki_record* spki_test_data_get_records(spki_test_data* data)
{
    return data->records;
}

unsigned int spki_test_data_size(spki_test_data* data)
{
    return data->size;
}

spki_test_data* spki_test_data_add_records(spki_test_data* data, unsigned int number_of_records)
{
    assert(data != NULL);

    struct spki_record* tmp = data->records;
    data->records = realloc(data->records, (data->size + number_of_records) * sizeof(*data->records));
    if(data == NULL){
        printf("realloc error\n");
        free(tmp);
        return;
    }

    for(unsigned int i = data->size; i < data->size + number_of_records; i++){
        struct spki_record* record = create_record(i,i,i,NULL);
        memcpy(&data->records[i], record, sizeof(*record));
        free(record);
    }
    data->size = data->size + number_of_records;
    return data;
}

spki_test_data* spki_test_data_add_duplicated_records(spki_test_data* data, unsigned int number_of_duplicates)
{
    struct spki_record* tmp = data->records;
    data->records = realloc(data->records, (data->size + number_of_duplicates) * sizeof(*data->records));
    if(data == NULL){
        printf("realloc error\n");
        free(tmp);
        return;
    }

    for(unsigned int i = data->size; i < data->size + number_of_duplicates; i++){
        struct spki_record* record = create_record(data->size,i,i,NULL);
        memcpy(&data->records[i], record, sizeof(*record));
        free(record);
    }
    data->size = data->size + number_of_duplicates;
    return data;
}

spki_test_data* spki_test_data_duplicate_random_records(spki_test_data* data,
                                                  unsigned int number_of_duplicates)
{
    srand (time(NULL));
    //If there are no records to duplicate, fall back to
    //spki_test_data_add_duplicated_records()
    if(data->size <= 0){
        return spki_test_data_add_duplicated_records(data, number_of_duplicates);
    }

    struct spki_record* tmp = data->records;
    data->records = realloc(data->records, (data->size + number_of_duplicates) * sizeof(*data->records));
    if(data == NULL){
        printf("realloc error\n");
        free(tmp);
        return NULL;
    }

    for(unsigned int i = data->size; i < data->size + number_of_duplicates; i++){
        struct spki_record* record = create_record(rand()%data->size,i,i,NULL);
        memcpy(&data->records[i], record, sizeof(*record));
        free(record);
    }
    data->size = data->size + number_of_duplicates;
    return data;
}


void spki_test_data_shuffle(spki_test_data* data);


void spki_test_data_concatenate(spki_test_data* src, spki_test_data* dest);

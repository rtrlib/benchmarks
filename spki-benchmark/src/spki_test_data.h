#include "rtrlib/rtrlib.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"

/*
* Util to measure the cpu time and memory consumption.
*/

typedef struct spki_test_data{
    struct spki_record* records;
    unsigned int size;
} spki_test_data;

/**
 * @brief Creates a new spki_test_data object.
 * @return
 */
spki_test_data* spki_test_data_new();

/**
 * @brief Release the spki_test_data object.
 * @return
 */
void spki_test_data_free(spki_test_data* data);


struct spki_record* spki_test_data_get_records(spki_test_data* data);

unsigned int spki_test_data_size(spki_test_data* data);

/**
 * @brief Fills the test_data object with number_of_records records.
 * @param data
 * @param number_of_records
 */
spki_test_data* spki_test_data_add_records(spki_test_data* data, unsigned int number_of_records);

/**
 * @brief Choose number_of_duplicates times a record from the spki_test_data
 * and creates a duplicate.
 * spki_test_data must contain more then 0 records.
 * @param data
 * @param number_of_duplicates
 */
spki_test_data* spki_test_data_duplicate_random_records(spki_test_data* data,unsigned int number_of_duplicates);

/**
 * @brief Add number_of_duplicats records with the same ASN.
 * @param data
 * @param number_of_duplicats
 * @return
 */
spki_test_data* spki_test_data_add_duplicated_records(spki_test_data* data, unsigned int number_of_duplicates);

/**
 * @brief Shuffle the records in the given spki_test_data object.
 * @param data
 */
spki_test_data* spki_test_data_shuffle(spki_test_data* data);

/**
 * @brief Append records from src to dest.
 * @param src
 * @param dest
 */
spki_test_data* spki_test_data_concatenate(spki_test_data* src, spki_test_data* dest);

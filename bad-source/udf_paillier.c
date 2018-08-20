#include "udf_paillier.h"

#include <stdio.h>
/*************************************************************************
** init function
** Arguments:
** my_bool maybe_null 1 if function can return NULL
** Default value is 1 if any of the arguments
** is declared maybe_null
**
** initid Points to a structure that the init function should fill.
** This argument is given to all other functions.
**
** unsigned int decimals Number of decimals.
** Default value is max decimals in any of the
** arguments.
**
** unsigned int max_length Length of string result.
** The default value for integer functions is 21
** The default value for real functions is 13+
** default number of decimals.
** The default value for string functions is
** the longest string argument.
**
** char *ptr; A pointer that the function can use.
**
**
** args Points to a structure which contains:
** unsigned int arg_count Number of arguments
**
** enum Item_result *arg_type Types for each argument.
** Types are STRING_RESULT, REAL_RESULT
** and INT_RESULT.
**
** char **args Pointer to constant arguments.
** Contains 0 for not constant argument.
**
** unsigned long *lengths; max string length for each argument
** char *maybe_null Information of which arguments
** may be NULL
**
**
** message Error message that should be passed to the user on fail.
** The message buffer is MYSQL_ERRMSG_SIZE big, but one should
** try to keep the error message less than 80 bytes long!
**
** This function should return 1 if something goes wrong. In this case
** message should contain something usefull!
**************************************************************************/

my_bool udf_paillier_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
    struct encrypted_data *data = NULL;

    /* Arguments counts */
    if(args->arg_count!=1)
    {
        strcpy(message, "Wrong number of argument, PAILLIER function requires one argument from balance row data");
        return 1;
    }

    fprintf(stderr,"Error at _init");

    /* Check input argument data-type*/
    if(args->arg_type[0]!= INT_RESULT)
    {
        strcpy(message, "Wrong Argument type, PAILLIER function requires an Integer type argument");
        return 1;
    }

    /* Initialize initid value */
    initid->maybe_null = 0; /* Yes, results can be NULL */
    initid->decimals = 0;   /* 4 decimals in results after decryption */
    initid->max_length = 20; /* 6 digits + . + 10 decimals */

    fprintf(stderr,"Error at _init\n");

    /* Check memory allocation */

    if ((data = (struct encrypted_data *) malloc(sizeof(struct encrypted_data))) != NULL) {

        /* Initialize encryption_data */
        data->pubKey = paillier_init_public_key();
        data->privKey = paillier_init_private_key();
        data->hr = hcs_init_random();

        initid->ptr = (char *) data; // TODO : Suspected to be the trouble maker!!!!

        return 0;

    } else {
        strmov(message, "Couldn't allocate memory.");
        return 1;
    }
}

/****************************************************************************
** Deinit function. This should free all resources allocated by
** this function.
**
** Arguments:
** initid Return value from xxxx_init
****************************************************************************/
void udf_paillier_deinit(UDF_INIT* initid)
{
    struct encrypted_data *data = (struct encrypted_data*)initid->ptr; //TODO: suspected to be trouble maker!!!!

    mpz_clear(data->e_totalSum);

    paillier_free_public_key(data->pubKey);
    paillier_free_private_key(data->privKey);
    hcs_free_random(data->hr);

    free(initid->ptr);
}

/****************************************************************************
** Paillier Aggregate Function.
**
** There are 3 extra functions for an aggregate function, udf_paillier_reset,
** udf_paillier_clear, and udf_paillier_add
****************************************************************************/

/****************************************************************************
** udf_paillier_reset
**
** Arguments:
** initid Structure filled by udf_paillier_init
**
** args The same structure as to xxx_init. This structure
** contains values for all parameters.
** Note that the functions MUST check and convert all
** to the type it wants! Null values are represented by
** a NULL pointer
**
** is_null If the result is null, one should store 1 here.
**
** message Error message that should be passed to the user on fail.
** The message buffer is MYSQL_ERRMSG_SIZE big, but one should
** try to keep the error message less than 80 bytes long!.
**
** reset gets done every time we begin processing the records for a new
** group. It calls udf_paillier_clear and udf_paillier_add
****************************************************************************/

/* This is needed to get things to work in MySQL 4.1.1 and above */

void udf_paillier_reset(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message)
{
    udf_paillier_clear(initid, is_null, message);
    udf_paillier_add(initid, args, is_null, message);
}


/****************************************************************************
** udf_paillier_clear
**
** Arguments:
** initid Structure filled by udf_paillier_init
**
** args The same structure as to xxx_init. This structure
** contains values for all parameters.
** Note that the functions MUST check and convert all
** to the type it wants! Null values are represented by
** a NULL pointer
**
** is_null If the result is null, one should store 1 here.
**
** message Error message that should be passed to the user on fail.
** The message buffer is MYSQL_ERRMSG_SIZE big, but one should
** try to keep the error message less than 80 bytes long!.
**
** udf_paillier_clear resets the processing variables to their initial values for a
** new group
****************************************************************************/

/* This is needed to get things to work in MySQL 4.1.1 and above */

void udf_paillier_clear(UDF_INIT* initid, char* is_null __attribute__((unused)),
                        char* message __attribute__((unused)))
{

}


/****************************************************************************
** paillier_add
**
** Arguments:
** initid Structure filled by xxx_init
**
** args The same structure as to xxx_init. This structure
** contains values for all parameters.
** Note that the functions MUST check and convert all
** to the type it wants! Null values are represented by
** a NULL pointer
**
** is_null If the result is null, one should store 1 here.
**
** message Error message that should be passed to the user on fail.
** The message buffer is MYSQL_ERRMSG_SIZE big, but one should
** try to keep the error message less than 80 bytes long!.
**
** paillier_add is the main processing workhorse of a anggregate UDF. It processes
** each new record in the group.
****************************************************************************/

void udf_paillier_add(UDF_INIT* initid, UDF_ARGS* args,
                      char* is_null __attribute__((unused)),
                      char* error __attribute__((unused)))
{
    if(args->args[0])
    {
        struct encrypted_data *data = (struct encrypted_data *)initid->ptr;
        long long int balance = *((long long int *) args->args[0]);

        /* Homomorphic summation operation */

        mpz_t e_balance;

        mpz_init(e_balance); //TODO: have e_balance initialized properly?
        mpz_set_si(e_balance, balance);  // Casts Int type to mpz type

        fprintf(stderr, "error at _add\n");

        paillier_encrypt(data->pubKey, data->hr, e_balance, e_balance); // Encrypt balance in each row
        paillier_ee_add(data->pubKey, data->e_totalSum, data->e_totalSum, e_balance); // Summation operation

        fprintf(stderr, "error at _ee_add\n");
        mpz_clear(e_balance);   // clean 'e_balance' variable

    }
}


/***************************************************************************
** UDF unsigned long int function.
**
** Arguments:
** initid Structure filled by xxx_init
**
** args The same structure as to xxx_init. This structure
** contains values for all parameters.
** Note that the functions MUST check and convert all
** to the type it wants! Null values are represented by
** a NULL pointer
**
** is_null If the result is null, one should store 1 here.
**
** error If something goes fatally wrong one should store 1 here.
**
** This function should return a double. It returns a value at the end of
** each group.
***************************************************************************/

long long int udf_paillier(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
    struct encrypted_data *data = (struct encrypted_data *)initid->ptr;

    if(data != NULL)
    {
        mpz_t g_result; // initialize 'mpz' type to store result of aggregate sum from decryption
        mpz_init(g_result);

        paillier_decrypt(data->privKey, g_result, data->e_totalSum); //decrypt the result and store to g_result
        long long int si_result = mpz_get_si(g_result); // store decryption result back to double type

        fprintf (stderr, "Error after getting si_result\n");
        mpz_clear(g_result); // free variable memory

        *is_null = 0;
        return si_result; // return the result to the user.
    }

    else    // means table set is empty in the very beginning.
    {
        fprintf(stderr,"error data is null\n");
        *is_null = 1;
        return 0;
    }

}
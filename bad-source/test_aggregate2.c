#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else

#include <my_global.h>
#include <my_sys.h>

#if defined(MYSQL_SERVER)
#include <m_string.h>		/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>

#define strmov(a, b) stpcpy(a,b)
#endif
#endif

#include <mysql.h>
#include <ctype.h>

#ifdef _WIN32
/* inet_aton needs winsock library */
#pragma comment(lib, "ws2_32")
#endif

#ifdef HAVE_DLOPEN

#if !defined(HAVE_GETHOSTBYADDR_R) || !defined(HAVE_SOLARIS_STYLE_GETHOST)
static pthread_mutex_t LOCK_hostname;
#endif

#include <gmp.h>    // gmp is included implicitly
#include <libhcs.h> // master header includes everything


/*************************************************************************
** Example of init function
** Arguments:
** initid	Points to a structure that the init function should fill.
**		This argument is given to all other functions.
**	my_bool maybe_null	1 if function can return NULL
**				Default value is 1 if any of the arguments
**				is declared maybe_null.
**	unsigned int decimals	Number of decimals.
**				Default value is max decimals in any of the
**				arguments.
**	unsigned int max_length  Length of string result.
**				The default value for integer functions is 21
**				The default value for real functions is 13+
**				default number of decimals.
**				The default value for string functions is
**				the longest string argument.
**	char *ptr;		A pointer that the function can use.
**
** args		Points to a structure which contains:
**	unsigned int arg_count		Number of arguments
**	enum Item_result *arg_type	Types for each argument.
**					Types are STRING_RESULT, REAL_RESULT
**					and INT_RESULT.
**	char **args			Pointer to constant arguments.
**					Contains 0 for not constant argument.
**	unsigned long *lengths;		max string length for each argument
**	char *maybe_null		Information of which arguments
**					may be NULL
**
** message	Error message that should be passed to the user on fail.
**		The message buffer is MYSQL_ERRMSG_SIZE big, but one should
**		try to keep the error message less than 80 bytes long!
**
** This function should return 1 if something goes wrong. In this case
** message should contain something usefull!
**************************************************************************/

my_bool paillier_init(UDF_INIT *initid, UDF_ARGS *args, char *message);

void paillier_deinit(UDF_INIT *initid);

void paillier_reset(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

void paillier_clear(UDF_INIT *initid, char *is_null, char *error);

void paillier_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

double paillier(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

my_bool is_const_init(UDF_INIT *initid, UDF_ARGS *args, char *message);


/*
** Syntax for the new aggregate commands are:
** create aggregate function <function_name> returns {string|real|integer}
**		  soname <name_of_shared_library>
**
** Syntax for avgcost: avgcost( t.quantity, t.price )
**	with t.quantity=integer, t.price=double
** (this example is provided by Andreas F. Bobak <bobak@relog.ch>)
*/


struct paillier_data {
    ulonglong count;

    pcs_public_key *pubKey;
    pcs_private_key *privKey;
    hcs_random *hr;

    mpz_t e_totalSum;
};

/*
** Average Cost Aggregate Function.
*/
my_bool
paillier_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    struct paillier_data *data;

    if (args->arg_count != 2) {
        strcpy(
                message,
                "wrong number of arguments: AVGCOST() requires two arguments"
        );
        return 1;
    }

    if ((args->arg_type[0] != INT_RESULT) || (args->arg_type[1] != REAL_RESULT)) {
        strcpy(
                message,
                "wrong argument type: AVGCOST() requires an INT and a REAL"
        );
        return 1;
    }

    /*
    **	force arguments to double.
    */
    /*args->arg_type[0]	= REAL_RESULT;
      args->arg_type[1]	= REAL_RESULT;*/

    initid->maybe_null = 0;        /* The result may be null */
    initid->decimals = 4;        /* We want 4 decimals in the result */
    initid->max_length = 20;        /* 6 digits + . + 10 decimals */

    if (!(data = (struct paillier_data *) malloc(sizeof(struct paillier_data)))) {
        strmov(message, "Couldn't allocate memory");
        return 1;
    }

    // initialize data structures
    data->pubKey = pcs_init_public_key();
    data->privKey = pcs_init_private_key();
    data->hr = hcs_init_random();

    // Generate a key pair with modulus of size 2048 bits
    pcs_generate_key_pair(data->pubKey, data->privKey, data->hr, 2048);

    initid->ptr = (char *) data;

    return 0;
}

void paillier_deinit(UDF_INIT *initid) {
    free(initid->ptr);
}


/* This is only for MySQL 4.0 compability */
void paillier_reset(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *message) {
    paillier_clear(initid, is_null, message);
    paillier_add(initid, args, is_null, message);
}

/* This is needed to get things to work in MySQL 4.1.1 and above */

void paillier_clear(UDF_INIT *initid, char *is_null __attribute__((unused)),
                    char *error __attribute__((unused))) {
    struct paillier_data *data = (struct paillier_data *) initid->ptr;

    mpz_clear(data->e_totalSum);
    pcs_free_public_key(data->pubKey);
    pcs_free_private_key(data->privKey);
    hcs_free_random(data->hr);

    data->count = 0;
}


void paillier_add(UDF_INIT *initid, UDF_ARGS *args,
                  char *is_null __attribute__((unused)),
                  char *error __attribute__((unused))) {
    if (args->args[0] && args->args[1]) {
        struct paillier_data *data = (struct paillier_data *) initid->ptr;
        double balance = *((double *) args->args[1]);

        // Encryption part
        mpz_t encrypted;
        mpz_init(encrypted);
        mpz_set_d(encrypted, balance);
        pcs_encrypt(data->pubKey, data->hr, encrypted, encrypted);
        pcs_ee_add(data->pubKey, data->e_totalSum, data->e_totalSum, encrypted);

        // clean encrypted, not used anymore
        mpz_clear(encrypted);

        // increment count
        data->count++;
    }
}


double paillier(UDF_INIT *initid, UDF_ARGS *args __attribute__((unused)),
                char *is_null, char *error __attribute__((unused))) {
    struct paillier_data *data = (struct paillier_data *) initid->ptr;
    if (!data->count) {
        *is_null = 1;
        return 0.0;
    }

    *is_null = 0;

    // return result of operation here :
    mpz_t result;
    mpz_init(result);

    pcs_decrypt(data->privKey, result, data->e_totalSum);
    double result_d = mpz_get_d(result);

    mpz_clear(result);

    return result_d;
}

#endif /* HAVE_DLOPEN */
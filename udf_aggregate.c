#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>

#include <gmp.h>
#define DeclareAndInit(n) mpz_t n; mpz_init(n)

#include<libcry.h>

struct CryptoBlock
{
    paillier_public_key *pub_key;
    paillier_private_key *priv_key;
    hcs_random *hcs_rand;
    mpz_t sum;
};

signed long PaillierAggregate(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
my_bool PaillierAggregate_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void PaillierAggregate_deinit(UDF_INIT *initid);
void PaillierAggregate_reset(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error );
void PaillierAggregate_clear(UDF_INIT *initid, char *is_null, char *error);
void PaillierAggregate_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


signed long PaillierAggregate(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    signed long c ;
    struct CryptoBlock* crypto = (struct CryptoBlock*) initid->ptr;
    paillier_decrypt(crypto->priv_key, crypto->sum, crypto->sum);
    c = mpz_get_si(crypto->sum);
    return c;

}

my_bool PaillierAggregate_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    struct CryptoBlock* crypto;
    initid->maybe_null = 0;

    if (args->arg_count != 1 ||
            args->arg_type[0] != INT_RESULT)
    {
        strcpy(message, "PaillierAggregate requires an argument of type [INT]");
        return 1;
    }

    if (!(crypto = (struct CryptoBlock*)malloc(sizeof(struct CryptoBlock))))
    {
        strcpy(message, "PaillierAggregate couldn't allocate memory");
        return 1;
    }

    crypto->pub_key = paillier_init_public_key();
    crypto->priv_key = paillier_init_private_key();
    crypto->hcs_rand = hcs_init_random();

    paillier_generate_key_pair(crypto->pub_key, crypto->priv_key,
                          crypto->hcs_rand, 2048);

    mpz_init(crypto->sum);
    mpz_set_si(crypto->sum, 0);
    paillier_encrypt(crypto->pub_key, crypto->hcs_rand, crypto->sum, crypto->sum);

    initid->ptr = (char*) crypto;

    return 0;

}

void PaillierAggregate_reset(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
    PaillierAggregate_clear(initid, is_null, error);
    PaillierAggregate_add(initid, args, is_null, error);

}

void PaillierAggregate_clear(UDF_INIT *initid, char *is_null, char *error)
{
    struct CryptoBlock* crypto = (struct CryptoBlock*) initid->ptr;

    mpz_set_si(crypto->sum, 0);
    paillier_encrypt(crypto->pub_key, crypto->hcs_rand, crypto-> sum, crypto->sum);

}

void PaillierAggregate_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
    if(args->args[0])
    {
        struct CryptoBlock* crypto = (struct CryptoBlock*) initid->ptr;
        long long int X_int = *((long long int*) args->args[0]);

        DeclareAndInit(X);
        mpz_set_si(X, X_int);

        paillier_encrypt(crypto->pub_key, crypto->hcs_rand, X, X);
        paillier_ee_add(crypto->pub_key, crypto->sum, crypto->sum, X);

        mpz_clear(X);
    }

}

void PaillierAggregate_deinit(UDF_INIT *initid)
{
    if(initid->ptr != NULL)
    {
        struct CryptoBlock* crypto = (struct CryptoBlock*) initid->ptr;

        paillier_free_public_key(crypto->pub_key);
        paillier_free_private_key(crypto->priv_key);
        hcs_free_random(crypto->hcs_rand);
        mpz_clear(crypto->sum);
    }

}

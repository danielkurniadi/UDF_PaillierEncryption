#include <mysql.h>
#include <string.h>
#include <stdio.h>

#include <gmp.h>
#define ORDER   -1
#define DeclareAndInit(n) mpz_t n; mpz_init(n)

#include<libcry.h>

char* PaillierAddition(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
my_bool PaillierAddition_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void PaillierAddition_deinit(UDF_INIT *initid);


char* PaillierAddition(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    
    long long int A_int = *((long long int*) args->args[0]);
    long long int B_int = *((long long int*) args->args[1]);
    
    DeclareAndInit(A); // mpz_t declaration
    DeclareAndInit(B);
    DeclareAndInit(C);
    
    paillier_public_key *pubKey = paillier_init_public_key();     // init public key
    paillier_private_key *privKey = paillier_init_private_key();  // init private key
    hcs_random *hcsRand = hcs_init_random();            // generate random big int
    
    paillier_generate_key_pair(pubKey, privKey, hcsRand, 2048);  // generate encryption keypair
    
    mpz_set_si(A, A_int);                   // assign cast integer to big integer
    mpz_set_si(B,B_int);
    
    paillier_encrypt(pubKey, hcsRand, A, A);     // encrypt number
    paillier_encrypt(pubKey, hcsRand, B, B);
    
    paillier_ee_add(pubKey, C, A, B);            // C = E(AxB) ~ a+b
    
    result = mpz_get_str(NULL, 10, C);      // result stored as string
    
    mpz_clear(A);                    // clear dynamic memory
    mpz_clear(B);
    mpz_clear(C);
    
    paillier_free_public_key(pubKey);    // clear dynamic memory
    paillier_free_private_key(privKey);
    hcs_free_random(hcsRand);
    
    return result;
}

my_bool PaillierAddition_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    initid->maybe_null = 0;
    if (args->arg_count != 2 ||
        args->arg_type[0] != INT_RESULT ||
        args->arg_type[1] != INT_RESULT)
    {
        strcpy(message, "PaillierAddition requires 2 arguments of type [INT]");
        return 1;
    }
    return 0;
}

void PaillierAddition_deinit(UDF_INIT *initid)
{
    
}

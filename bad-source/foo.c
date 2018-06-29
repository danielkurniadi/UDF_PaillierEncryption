#include <stdio.h>
#include <stdlib.h>

/* Require homomorphic cryptosystem and gmp library*/
#include <libhcs.h>
#include <gmp.h>

#ifdef HAVE_DLOPEN

#if !defined(HAVE_GETHOSTBYADDR_R) || !defined(HAVE_SOLARIS_STYLE_GETHOST)
static pthread_mutex_t LOCK_hostname;
#endif

#endif /* HAVE_DLOPEN */

/**
 ** MARK    :
 **    - Declaration of keys and global variables
 **
 */
struct CryptoKey {
    pcs_public_key *pubKey;
    pcs_private_key *privKey;
    hcs_random *hcsRandom;
    mpz_t e_balanceSum;
};


long long int arr[] = {1000203197, 210298377, 18826363, 191928364, -59062243, 111726666226, 701999277, 81027772, 199277269 ,-1233221210, 11, 12, 13, 14, 15, 16 ,17 ,18, 19};

/**
 ** MARK    :
 *      - Declaration of User-Defined Functions (UDF)
 **
 */
void udf_init(struct CryptoKey** cryptoKey);

void udf_add(int i, struct CryptoKey** cryptoKey);

void udf_clear(struct CryptoKey** cryptoKey);

// void udf_reset();

void udf_deinit(struct CryptoKey** cryptoKey);


/**
 ** MARK   :
 **    - Implementation of User-Defined Functions (UDF)
 **
 */
void udf_init(struct CryptoKey** cryptoKey){
    printf("UDF_INIT\n");

    struct CryptoKey *CSKeys;

// Step 2 : Check memory allocation for crypto key structure
    if (!(CSKeys = (struct CryptoKey*)malloc(sizeof(struct CryptoKey))))
    {
        printf("Unable to allocate memory\n");
        return;
    }else{ppp
        CSKeys->pubKey = pcs_init_public_key();
        CSKeys->privKey = pcs_init_private_key();
        CSKeys->hcsRandom = hcs_init_random();
        mpz_init(CSKeys->e_balanceSum);

        pcs_generate_key_pair(CSKeys->pubKey, CSKeys->privKey, CSKeys->hcsRandom, 2048);

// Step 3 : Initialize variable Sum that records the E(Balance_Sum) with zero balance
        mpz_set_si(CSKeys->e_balanceSum, 0);
        pcs_encrypt(CSKeys->pubKey, CSKeys->hcsRandom, CSKeys->e_balanceSum, CSKeys->e_balanceSum);
        // gmp_printf("UDF_INIT Init e_balance to zero: %Zd\n", CSKeys->e_balanceSum);
// Step 4 : Send the reference to the main _function
        *cryptoKey = CSKeys;
    }
}

void udf_add(int i, struct CryptoKey** cryptoKey){

    long long int balance = arr[i];

// Setup a 2048-bit for each balance
    mpz_t e_Balance;
    mpz_init(e_Balance);
    mpz_set_si(e_Balance, balance);

// Encrypt each balance
    pcs_encrypt((*cryptoKey)->pubKey, (*cryptoKey)->hcsRandom, e_Balance, e_Balance);

// Operation of addition
    pcs_ee_add((*cryptoKey)->pubKey, (*cryptoKey)->e_balanceSum, (*cryptoKey)->e_balanceSum, e_Balance);

// clear up free unused memory
    mpz_clear(e_Balance);
}

void udf_clear(struct CryptoKey** cryptoKey){
// Clear e_balance to a zero balance
    mpz_set_si((*cryptoKey)->e_balanceSum, 0);
    pcs_encrypt((*cryptoKey)->pubKey, (*cryptoKey)->hcsRandom, (*cryptoKey)->e_balanceSum,(*cryptoKey)->e_balanceSum);
    printf("UDF_CLEAR\n");
}

void udf_deinit(struct CryptoKey** cryptoKey){
    mpz_clear((*cryptoKey)->e_balanceSum);
    pcs_free_public_key((*cryptoKey)->pubKey);
    pcs_free_private_key((*cryptoKey)->privKey);
    hcs_free_random((*cryptoKey)->hcsRandom);

    free((struct CryptoKey*)(*cryptoKey));

    printf("UDF_DEINIT\n");
}

/**
 *
 * @return
 */

int main(void){
// Step 1 : Initialize crypto-system Key
    struct CryptoKey* cryptoKey = NULL;
    long long int a;
    udf_init(&cryptoKey); // UDF_INIT

// Step 2 : Starts operation
    printf("UDF_ADD Iteration\n");
    for(int i = 0; i<20; i++) {
        udf_add(i, &cryptoKey); // UDF_ADD
    }

// Step 3 : Display operation results
    char *res = mpz_get_str(NULL, 10, cryptoKey->e_balanceSum);
    printf("res: %s\n",res);

    pcs_decrypt(cryptoKey->privKey, cryptoKey->e_balanceSum, cryptoKey->e_balanceSum);
    gmp_printf("Final Sum = %Zd\n", cryptoKey->e_balanceSum);

    for(int i=0; i<20; i++){
        a += arr[i];
    }
    printf("check : %lld\n",a);

// Step 4 : Cleanup all Data and Key
    udf_clear(&cryptoKey); // UDF_CLEAR
    // gmp_printf("Cleared : %Zd\n",cryptoKey->e_balanceSum);

    a = 0;

// Step 2 : Starts operation
    printf("2nd UDF_ADD Iteration\n");
    for(int i = 0; i<20; i++) {
        udf_add(i, &cryptoKey); // UDF_ADD
    }
// Step 3 : Display operation results
    res = mpz_get_str(NULL, 10, cryptoKey->e_balanceSum);
    printf("res: %s\n",res);

    pcs_decrypt(cryptoKey->privKey, cryptoKey->e_balanceSum, cryptoKey->e_balanceSum);
    gmp_printf("Final Sum = %Zd\n", cryptoKey->e_balanceSum);

    for(int i=0; i<20; i++){
        a += arr[i];
    }
    printf("check : %lld\n",a);

    udf_deinit(&cryptoKey); // UDF_DEINIT


    return 0;
}


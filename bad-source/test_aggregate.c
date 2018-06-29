#include <stdio.h>

#include <gmp.h>
#include <libhcs.h>

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #ifdef __WIN__
        typedef unsigned __int64 ulonglong; /* Microsofts 64 bit types */
        typedef __int64 longlong;
    #else
        typedef unsigned long long ulonglong;
        typedef long long longlong;
    #endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>   /* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#define strmov(a,b) stpcpy(a,b)
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
#endif

my_bool PaillierEncrypt_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void PaillierEncrypt_deinit(UDF_INIT *initid);
char* PaillierEncrypt(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);


my_bool PaillierEncrypt_init(UDF_INIT *initid, UDF_ARGS *args, char *message){

    if(args->arg_count!=1 || args->arg_type[0] !=INT_RESULT){
        strcpy(message,"Wrong arguments to PaillierEncrypt;  Use the source");
        return 1;
    }


    return 0;

}

void PaillierEncrypt_deinit(UDF_INIT *initid){

}

char *PaillierEncrypt(UDF_INIT *initid __attribute__((unused)), UDF_ARGS *args, char *result, unsigned long *length,
                      char *is_null, char *error __attribute__((unused))) {

    longlong A = (longlong)args->args[0];
    pcs_public_key *pubKey = pcs_init_public_key();
    pcs_private_key *privKey = pcs_init_private_key();
    hcs_random *hr = hcs_init_random();

    pcs_generate_key_pair(pubKey, privKey, hr, 2048);

    fprintf(stderr,"Reached init key line\n");

    mpz_t A_encrypted;
    mpz_init(A_encrypted);
    mpz_set_si(A_encrypted, A);
    
    fprintf(stderr,"rm Reached init mpz_t\n");

    pcs_encrypt(pubKey, hr, A_encrypted, A_encrypted);

    fprintf(stderr,"Reached  line\n");

    int mydim = (mpz_sizeinbase(A_encrypted, 2) +7)/ 8;
    result =  ( char*) malloc(sizeof(char) * mydim);
    length = (size_t*) malloc(sizeof(size_t));
    mpz_export((void *)result, length, 1, sizeof( char), 0, 0, A_encrypted);

    fprintf(stderr,"res: %s\n",result);

    mpz_clear(A_encrypted);
    pcs_free_public_key(pubKey);
    pcs_free_private_key(privKey);
    hcs_free_random(hr);
    fprintf(stderr,"Clearing\n");
    return result;

}

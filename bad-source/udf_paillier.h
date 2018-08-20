#ifndef UDF_PAILLIER_LIBRARY_H
#define UDF_PAILLIER_LIBRARY_H

/******************************************************************************
** udf_paillier.c
**
** This MySQL user-defined function (UDF) sorts through an aggregate dataset of balance sheet,
** then returns the paillier decryption of the sum of balance through operation :
**  E(balance) * E(balance) = balance + balance
**
**
** Author:        Daniel Kurniadi
** Creation Date: 21/05/2018
** Modifications: None
******************************************************************************/

/******************************************************************************
** A dynamically loadable file should be compiled shared.
** (something like: gcc -shared -o my_func.so -I /usr/includes/mysql/ my_func.c).
** You can easily get all switches right by doing:
** cd sql ; make udf_example.o
** Take the compile line that make writes, remove the '-c' near the end of
** the line and add -shared -o udf_example.so to the end of the compile line.
** The resulting library (udf_example.so) should be copied to some dir
** searched by ld. (/usr/lib/mysql/plugin/ ?)
**
** CREATE FUNCTION UDF_PAILLIER RETURNS STRING SONAME "udf_paillier.so";
**
** After this the functions will work exactly like native MySQL functions.
** Functions should be created only once.
**
** The functions can be deleted by:
**
** DROP FUNCTION UDF_PAILLIER;
**
** The CREATE FUNCTION and DROP FUNCTION update the func@mysql table. All
** Active function will be reloaded on every restart of server
** (if --skip-grant-tables is not given)
**
** If you ge problems with undefined symbols when loading the shared
** library, you should verify that mysqld is compiled with the -rdynamic
** option.
**
** If you can't get AGGREGATES to work, check that you have the column
** 'type' in the mysql.func table. If not, run 'mysql_upgrade'.
**
**************************************************************************/

/*************************************************************************
** Syntax for the new aggregate commands are:
** CREATE AGGREGATE FUNCTION <function_name> RETURNS {STRING|REAL|INTEGER}
** SONAME <name_of_shared_library>
**
** Syntax for paillier: UDF_PAILLIER( t.balance )
** with t.balance is in REAL / DOUBLE
** Cheers!
****************************************************************************/

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions*/
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

#include <stdbool.h>

/** Include GNU multi-precession library
 ** Include Homomorphic encryption library from github:
 **   [https://github.com/tiehuis/libhcs]
 **/
#include <gmp.h>
#include <libhcs.h>


/** Function Declaration */
my_bool udf_paillier_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void udf_paillier_deinit(UDF_INIT *initid);
void udf_paillier_clear(UDF_INIT* initid, char *is_null, char *error);
void udf_paillier_reset(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
void udf_paillier_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
long long udf_paillier(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );


/** Data Structure Declaration */
struct encrypted_data{

    paillier_public_key *pubKey;
    paillier_private_key *privKey;
    hcs_random *hr;

    mpz_t e_totalSum;
};

#endif //ifndef UDF_PAILLIER_LIBRARY.H






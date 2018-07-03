# Paillier-Encryption UDF for mysql

This code is a Paillier Encryption implementation on UDF(s) (mysql user-defined functions).

Mainly, there are two source codes : the **aggregate UDF** (udf_aggregate.c) and the **non-aggregate UDF** (udf_paillier.c). 
These two source codes are independent for each other, both are mean for different UDF. 

You can ignore the bad-source folder. It only contains experimental source codes that I made previously. Also, we need not to use the CMakefile.txt and cmake-build-debug. I will clean up my repo soon.

## Requirement
* GCC compiler
* CMake 

## Dependencies
Before running this UDF source code, we need to settle some dependencies. There are mainly three dependencies : **mysql-client**, **GMP Library** and **HCS Library**.
The following is the step of how to install them in UNIX-based environment.

### Mysql Client API
MySQL Server and Connector/C installation packages(libmysqlclient-dev) both provide the files needed to build and run MySQL C API client programs.
The package contains mysql.h file that defines the client API to MySQL and also the ABI of the
dynamically linked libmysqlclient.

Simply use apt-get :

    $ apt-get install libmysqlclient-dev 
which will automatically pull the latest libmysqlclient18-dev. 

Or if you use yum :

    yum install mysql-devel -y

All the connector files from the libmysqlclient-dev Ubuntu package usually located at ```/usr/include/mysql/mysql.h```
(please check your on your computer). This is not a standard search path for compilers, however ```/usr/include is``` 

You'd typically use the mysql.h header in your code like this:
```#include <mysql/mysql.h>```

### GMP library
First, there is a dependency on the [GMP library (link here)](https://gmplib.org/). This also
means that some familiarity with this library is required. A useful manual can
be found on the website linked above.

To obtain the needed requirement on Ubuntu 15.10, one may run the following
command:
    sudo apt-get install libgmp-dev cmake
Please refer to the GMP documentation website for further information regarding how to install GMP library.  

### HCS Library 
Secondly, this implementation also uses libhcs , a C library implementing a number of partially homormophic encryption
schemes. 
The source code for **HCS Library (libhcs)** by [tiehuis](https://github.com/tiehuis) can be found [here](https://tiehuis.github.io/libhcs).

Assuming all dependencies are on your system, the following will install the HCS library on a typical UNIX-based system.

    git clone https://github.com/Tiehuis/libhcs.git
    cmake .
    make
    sudo make install   # Will install to /usr/local by default

To uninstall all installed HCS library's files, one can run the following command:

    sudo xargs rm < install_manifest.txt

## Compiling UDF Source Code
Once the dependencies has been installed, go to the directory folder where this project is located and run the following 
command to compile the source code into a shared library. 

    gcc -shared\    
    -o lib_udf.so udf_aggregate.c \
    -I /usr/local/include \ 
    -L/usr/local/lib -lhcs -lgmp 

The second line in above command will generate a shared library called *lib_udf.so* from the source code *udf_aggregate.c*
You can also compile the shared library from the second source code : *udf_paillier.c*
The third line specify the location of header files of GMP and HCS library, and mysql header file.  
The fourth line specify the location of shared objects of our GMP and HCS library. 

Next step, the shared library will be copied and installed under mysql plugin directory
to be executed inside your database as a UDF.
    
Refer to documentation at official mysql site: [UDF installing](https://dev.mysql.com/doc/refman/8.0/en/udf-compiling.html) for further explaination how to install shared library into use-ready UDF inside mysql database.

    

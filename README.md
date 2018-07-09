# Paillier-Encryption UDF for mysql

This code is a Paillier Encryption implementation on UDF(s) (mysql user-defined functions).

Mainly, there are two source codes : the **aggregate UDF** (udf_aggregate.c) and the **non-aggregate UDF** (udf_paillier.c). 
These two source codes are independent for each other, both are mean for different UDF. 

You can ignore the bad-source folder. It only contains experimental source codes that I made previously. Also, we need not to use the CMakefile.txt and cmake-build-debug. I will clean up my repo soon.

## Usage: How to Run Paillier Encryption UDF in Mysql 
The purpose of this section is just to illustrate calling and passsing argument(s) to UDF. This section assumes that you have setup everything properly until the UDF is ready for use inside mysql. Refer to section 2: 'Setup and Install Everything' for setups.

### Paillier Aggregate (Additive) Encryption
This UDF (User-Defined Function) is named ```PaillierAggregate(arg)```
Suppose we have a table with columns of integers (here we call it 'Target Column'), the UDF Aggregate function can be called to encrypt (integer)value from each rows, sum them up using Paillier additive operation, then returns the Blob value of the sum.  

    'Mytable' Table :
    +---------------+--------------+
    | Column_name_1 | Target_Column|
    +---------------+--------------+
    |    .......    |  10          | 
    +---------------+--------------+
    |    .......    |  791         | 
    +---------------+--------------+
    |    .......    |  1001        | 
    +---------------+--------------+
    |    .......    |  826         | 
    +---------------+--------------+
    |    .......    |  ....        | 
    +---------------+--------------+
    
    mysql> SELECT PaillierAggregate(Target_Column) FROM Mytable;
    
    result: 240734644527746656466488704360217622599391344833506933214.........
    
The UDF calls in ```mysql>``` command passes the targeted column and returns the blob values of Paillier aggregate operation as explained just below.

### Paillier Non-Aggregate (Additive) Encryption
This UDF is named ```PaillierAddition(arg1, arg2)```. The calling for the non-aggregate udf is much simpler. It takes **two** INT (integer) value as argument. The the function will encrypt both integer, sum them (the integers encrypted values) together and returns the blob values of the Paillier Additive operation. Just type this command:

    mysql> SELECT PaillierAddition(arg1, arg2);
The UDF calls in ```mysql>``` command passes the two arguments and returns the blob values of Paillier aggregate operation as explained just below.

## Tool Requirement
* Mysql Server
* Mysql Client
* GCC compiler
* CMake compiler

## 2. Setup and Install everything
This section assumes user to use Ubuntu 14.04 with starting with no tool requirement. The purpose of this section is to guide whoever reading this to eventually setup everything properly before compiling and running the source code.
If you starting from scratch, I suggest to install everything as follows:

### 2.1 Build-Essential
Run this command on your terminal to install ```build-essential package```. This package includes GCC Compiler, Make and CMake tool we will use later.

    apt-get install build-essential

### 2.2 Mysql Server
Run the command below for installing mysql if you don't have one. Skip this installation if you already have a server.
    
    apt-get update
    apt-get install mysql-server
    service mysql status
    service mysql start

### 2.3 Mysql Client
Mysql Server doesn't come with mysql Client. Check if you have mysql Client installed on your computer. Below is the command  for installing mysql Client.
    
    apt-get install libmysqlclient-dev
    find / -name *mysql.h* 

All the connector files from the libmysqlclient-dev Ubuntu package is usually located at ```/usr/include/mysql/mysql.h```. This is not a standard search path for compilers, however ```/usr/include```.
Hence, You'd typically wanna add/use the ```mysql.h header``` in the downloaded source code like this:
```#include <mysql/mysql.h>```

### 2.4 Dependencies Library 
Before running this UDF source code, we need to settle some dependencies. There are mainly three dependencies : **mysql-client**, **GMP Library** and **HCS Library**.
The following is the step of how to install them in UNIX-based environment.

#### 2.4.1 GMP library
First, there is a dependency on the [GMP library (link here)](https://gmplib.org/). This also
means that some familiarity with this library is required. A useful manual can
be found on the website linked above.

To obtain the needed requirement on Ubuntu, one may run the following
command:
    
    sudo apt-get install libgmp-dev cmake
Please refer to the GMP documentation website for further information regarding how to install GMP library.  

#### 2.4.2 HCS Library 
Secondly, this implementation also uses libhcs , a C library implementing a number of partially homormophic encryption
schemes. 
The source code for **HCS Library (libhcs)** by [tiehuis](https://github.com/tiehuis) can be found [here](https://tiehuis.github.io/libhcs).

Assuming all dependencies are on your system, the following will install the HCS library on a typical UNIX-based system.

    git clone https://github.com/Tiehuis/libhcs.git
    cmake .
    make
    sudo make install   # Will install to /usr/local by default

## 2.5 Compiling UDF Source Code
Once the dependencies has been installed, go to the directory folder where this project is located and run the following 
command to compile the source code into a shared library. 

    gcc - fPIC -shared -o lib_udf.so udf_aggregate.c\
         -I /usr/local/include -L /usr/local/lib -lhcs -lgmp 

Then make sure the dependencies library is linked to the path you have set :
    
    export LD_LIBRARY_PATH=/path/to/dependencies/library
If you use OSX platform, please change ```LD_LIBRARY_PATH``` to ```DYLD_LIBRARY_PATH``` (for Mac user).

## 2.6 Install UDF under Mysql Plugin

### 2.6.1 Copy/Move Shared Library Under plugin_dir
Now  we already have the shared library ``lib_udf.so`` that we just generate from compiling. Copy/move and install the shared library under mysql-plugin directory. Your mysql-plugin directory can be found by typing (inside mysql server command):
    
    mysql> SHOW VARIABLES LIKE 'plugin_dir'
and the result will be something like this: 

    +---------------+------------------------------+
    | Variable_name | Value                        |
    +---------------+------------------------------+
    | plugin_dir    | /usr/local/mysql/lib/plugin  | 
    +---------------+------------------------------+

### 2.7 Create UDF function
For UDF created from udf_aggregate.c (source code), follow section 2.7.1 . For UDF created from udf_paillier.c (source code) follow section 2.7.2 instead. 

#### 2.7.1 Creating Aggregate UDF
Inside mysql server, type this command to create a new function based on our UDF:
    
    mysql> CREATE AGGREGATE FUNCTION PaillierAggregate
            RETURNS STRING SONAME 'lib_udf.so'
The name ```PaillierAggregate``` must exactly the same as to match the function name inside the [UDF's C source code](https://github.com/iqDF/UDF_PaillierEncryption/udf_aggregate.c).

#### 2.7.2 Creating Non-Aggregate UDF
Inside mysql server, type this command to create a new function based on our UDF:
    
    mysql> CREATE AGGREGATE FUNCTION PaillierAddition
            RETURNS STRING SONAME 'lib_udf.so'
The name ```PaillierAddition``` must exactly the same as to match the function name inside the [UDF's C source code](https://github.com/iqDF/UDF_PaillierEncryption/udf_paillier.c).


Refer to documentation at official mysql site: [UDF installing](https://dev.mysql.com/doc/refman/8.0/en/udf-compiling.html) for further explaination how to install shared library into use-ready UDF inside mysql database.

    

# Paillier-Encryption UDF for mysql

This code is a Paillier Encryption implementation on UDF(s) (mysql user-defined functions).

There are two source codes : the **aggregate UDF** (udf_aggregate.c) and the **non-aggregate UDF** (udf_paillier.c). 
These two source codes are independent for each other, both are mean for different UDF.

## Requirement
* GCC compiler
* CMake 

## Dependencies
Before running this UDF source code, we need to settle some dependencies. There are two dependencies : **GMP Library** and **HCS Library**.
The following is the step of how to install them in UNIX-based environment.

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
    


    

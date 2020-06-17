# The Oxidian Programming-Language

## Contents
- [Installation](#Installation)
    - [On Windows](#On-Windows)
    - [On Linux](#On-Linux)
- [Configuration](#Configuration)
- [Prerequisites](#Prerequisites)
    - [On Windows](#If-you-are-on-Windows)
    - [On Linux](#If-you-are-on-Linux)
- [The Compiler](#The-Compiler)
- [How to Program](#How-to-program-in-Oxidian)
    - [Program Sections](#Program-Sections)
        - [The Preprocessor Section](#The-Preprocessor-Section)
        - [The Data Section](#The-Data-Section)
        - [The Code Section](#The-Code-Section)
- [Cross Platform Limitations](#Cross-Platform-Limitations)


## Installation

Since the Oxidian rewrite supports both, Linux and[*](#Cross-Platform-Limitations) Windows I will describe  
how to install and configure Oxidian on both Operating Systems.  

### On Windows

1. Install the [Prerequisites](#Prerequisites)
2. Clone the repository using `git clone https://github.com/fietensen/Oxidian` or download and extract the zip
3. [Configure](#Configuration) Oxidian however you wish
4. Open CMD and use `cd` to navigate to the Oxidian folder
5. Type `build` and hit enter, if you did everything correctly there should be no errors or warnings
6. Add the created `bin` path into your PATH environment variable 
7. Create an environmen variable called `OXY_LIBS` and insert the path to `bin\OxyLibs`

### On Linux

1. Install the [Prerequisites](#Prerequisites)
2. Clone the repository using `git clone https://github.com/fietensen/Oxidian` or download and extract the zip
3. [Configure](#Configuration) Oxidian however you wish
4. Open a Terminal and use `cd` to navigate to the Oxidian folder
5. Type `sudo make` and hit enter, if you did everything correctly there should be no errors or warnings
6. That's it! All the environment steps Windows users have to do are done automatically.

## Configuration

Oxidian provides you with several configurations, all for enabling/disabling certain parts  
the configuration is located under `include/Oxidian/config.h`.  
When you open the file you can see several options, the option name to the left and the value  
on the right, the 0 meaning disabled and 1 enabled.  
If let's say you want to disable the IO library you should go to the line where it sais  
`IOLIB` and set the value to 0.  
`N_SOCKS` specifies the maximum number of sockets that can be active at the same time.  
4096 should be far enough for the beginning.

A special case is the OPENSSL option. You can only enable it under Linux (see [here](#Cross-Platform-Limitations) for more info on that)  
If you are on linux you have to install the `lib32-openssl` package and edit the `Makefile` after the line which sais  
`remove comments to add additional functionality`. (remove the `#` before `CFLAGS+=-lssl`)

## Prerequisites

In order to install Oxidian you will have to install several prerequisites.

### If you are on Windows

- [git](https://gitforwindows.org/) (If you want to download the repository from your command line)
- [Python 3](https://www.python.org/downloads/release/python-382/) (For using olc.py, the compiler)
- [MinGW](https://osdn.net/projects/mingw/releases/) (For compiling the project)

**NOTE:** make sure to add each of them to your system's [PATH environment variable](https://helpdeskgeek.com/windows-10/add-windows-path-environment-variable/) so you can  
access them easily from your command line 

additionally you may want to get a bit into using the [windows command line (cmd)](https://www.bleepingcomputer.com/tutorials/windows-command-prompt-introduction/) so you will get  
a little insight in what you're actually doing.

### If you are on Linux

- [git](https://git-scm.com/download/linux) (If you want to download the repository from your command line)
- [Python 3](https://realpython.com/installing-python/) (For using olc.py, the compiler)
- build-essential (Includes gcc for compiling the project)
- lib32-openssl (Optional if you want SSL socket support)

**NOTE:** because you're a linux user I'd expect you to know how  
to use the terminal, if you don't, please read [this article](https://www.digitalocean.com/community/tutorials/an-introduction-to-the-linux-terminal) in order to get a little  
insight in what you're actually doing.

## The Compiler

Oxidian ships with it's own compiler, `olc.py`.  
It does only have really few command line arguments, 3 to be precise.

The first argument has to be the filename to the Oxidian source file which  
should have an `.oxd` extension.

The second argument is `-v` or `--verbose`. Like the name implies, it makes  
`olc.py` output debug information that can get you a neat little overview    
on what the compiler did.

The third and last argument is `-o=` or `--output=`. Normally the compiled  
file will be saved under `out.oxy`, but when using this argument you can  
specify a custom output file name.

Example usage:
`python olc.py main.oxd -o=MyProgram.oxy -v`  
This would compile the file `main.oxd`, show you debug information  
and save the resulting file in `MyProgram.oxy`

## How to program in Oxidian

### Program Sections

Every Oxidian program effectively consists of 3 sections.  
The Preprocessor-/ Data-/ and Code Section.

Each section is denoted by it's name followed by a curly opening  
bracket **ON THE SAME LINE**, the section content in the following  
lines and a closing bracket which is placed **ON A SINGLE** line.

Example:
```
SECTION_NAME {
    Content
    Content
    ...
}
```

In Oxidian you can place comments. These can be used to document your  
code in case you forget what certain parts do. **Every line starting** with  
a `;` is seen as a comment and will not be handled by the compiler.

#### The Preprocessor Section

The Preprocessor Section serves it's purpurse for defining macros,  
including libraries and setting Options for the Program.

First of all, what are macros? Macros are little "words" a value  
can be assigned to. When this word is placed somewhere in the code,  
the compiler automatically replaces the word with the value. This can  
come in handy if you have to write a long line over and over again.  
Instead you can just define a macro that holds this long line and  
insert it in your code and your code will become readable again.

Next off are the libraries. Libraries are little extensions for Oxidian.  
They can define certain macros or variables that can be used in your program.  
When you install Oxidian, it comes with 5 libraries. Each library is placed  
in the OxyLibs directory.

You can use the `including` keyword to include files. The filename surrounded by  
<>, makes the compiler include the file from the OxyLibs path.  
When surrounded by "", makes the compiler include the file from a relative path.

Additionally you can write macro/variable names seperated by commas in block brackets  
to make the compiler only include certain macros/variables from a module.

Examples:  
`including[PRINT] <stdio.json>` would make the compiler include only the macro PRINT from stdio.json  
`including <stdmem.json>` would make the compiler include every variable/macro from stdmem.json  
`including "my_lib.json"` would make the compiler include every variable/macro from a file named  
`my_lib.json` laying in the same directory as the file that includes it.

##### stdio.json

| Definition             |        Type          | Explanation                                                                                                                         |
|:----------------------:|:--------------------:|:-----------------------------------------------------------------------------------------------------------------------------------:|
| PRINT                  |        macro         | outputs string in argument 0 to console                                                                                             |
| READ                   |        macro         | reads n bytes from user input and stores it on the stack, n being the value of argument 0. Puts bytes read into argument 0          |
| PRINTSTACK             |        macro         | outputs n bytes from the stack to the console                                                                                       |
| FOPEN                  |        macro         | opens file with filename in argument 0 in mode specified in argument 1, moving a handle to the file into argument 2                 |
| FREAD                  |        macro         | reads n bytes from file handle in argument 0 to stack at address in argument 2, n being argument 1. Puts bytes read into argument 1 |
| FWRITE                 |        macro         | writes argument 1 bytes from file handle in argument 0 from stack at address in argument 2. Puts bytes written into argument 1      |
| FTELL                  |        macro         | puts the file position from file handle in argument 0 into argument 1                                                               |
| FSEEK                  |        macro         | sets the position from file handle in argument 0 to offset in argument 1 beginning from value in argument 2                         |
| FCLOSE                 |        macro         | closes the file from file handle in argument 0                                                                                      |
| SYSTEM                 |        macro         | executes bash/batch command in argument 0                                                                                           |
| O_READ                 |       variable       | file opening mode for reading                                                                                                       |
| O_WRITE                |       variable       | file opening mode for writing                                                                                                       |
| O_APP                  |       variable       | file opening mode for appending (writing)                                                                                           |
| O_CREAT                |       variable       | file opening mode for creating a file (must be paired with another opening mode like read)                                          |


##### stdmem.json

| Definition             |        Type          | Explanation                                                                                                                         |
|:----------------------:|:--------------------:|:-----------------------------------------------------------------------------------------------------------------------------------:|
| SETSTACK               |        macro         | writes n bytes from each of the k arguments starting from argument 3 to stack at address in argument 1, n=argument 0; k=argument 2  |
| CPYSTACK               |        macro         | copies n bytes from stack at address in argument 1 to stack at address in argument 2, n=argument 0                                  |
| SWPSTACK               |        macro         | swaps n bytes from stack at address in argument 1 with those on stack at address 2, n=argument 0                                    |
| REVERSESTACK           |        macro         | reverses n bytes from stack at address in argument 0, n=argument 1                                                                  |


##### stdnet.json

| Definition             |     Type             | Explanation                                                                                                                         |
|:----------------------:|:--------------------:|:-----------------------------------------------------------------------------------------------------------------------------------:|
| SOCKET                 |        macro         | creates socket with domain=argument0, type=argument1, protocol=argument2 and writes handle to argument 3                            |
| SCONNECT               |        macro         | connects socket handle in argument 0 domain=argument1 to address at argument 2 to port at argument 3                                |
| SSEND                  |        macro         | sends argument2 bytes from argument 1 via socket handle in argument 0, flags=argument3                                              |
| SCLOSE                 |        macro         | closes socket handle in argument 0                                                                                                  |
| SRECV                  |        macro         | receives argument1 bytes from socket handle in argument 0 and writes them to stack address argument2, puts bytes received to arg3   |
| GETHOSTBYNAME          |        macro         | finds IPv4 address of domain in argument0 and puts it into argument 1                                                               |
| SETSOCKOPT             |        macro         | sets sockopt for socket handle in argument0, level=argument1, option=argument2, option_value=argument3, option_length=argument4     |
| SLISTEN                |        macro         | sets backlog for socket handle in argument 0 to argument 1                                                                          |
| SBIND                  |        macro         | binds address in argument 2 and port in argument 3 with domain in argument 1 to socket handle in argument 0                         |
| SACCEPT                |        macro         | accept a connection to socket handle in argument 0 and store client handle in argument 1                                            |
| GETSOCKADDR            |        macro         | gets address of socket handle in argument 0 and writes it to argument 1                                                             |
| GETSOCKPORT            |        macro         | gets port of socket handle in argument 1 and writes it to argument 1                                                                |
| SETSOCKSSL             |        macro         | enables SSL for socket handle in arg0, doesn't work for sockets operating as server sockets. (Only for Linux with OpenSSL compiled) |
| SOL_SOCKET             |      variable        | socket level                                                                                                                        |
| SOCK_DGRAM             |      variable        | socket type                                                                                                                         |
| SOCK_STREAM            |      variable        | socket type                                                                                                                         |
| SOCK_SEQPACKET         |      variable        | socket type                                                                                                                         |
| SO_DEBUG               |      variable        | option for setsockopt                                                                                                               |
| SO_ACCEPTCONN          |      variable        | option for setsockopt                                                                                                               |
| SO_BROADCAST           |      variable        | option for setsockopt                                                                                                               |
| SO_REUSEADDR           |      variable        | option for setsockopt                                                                                                               |
| SO_KEEPALIVE           |      variable        | option for setsockopt                                                                                                               |
| SO_LINGER              |      variable        | option for setsockopt                                                                                                               |
| SO_OOBINLINE           |      variable        | option for setsockopt                                                                                                               |
| SO_RCVTIMEO            |      variable        | option for setsockopt                                                                                                               |
| SO_SNDTIMEO            |      variable        | option for setsockopt                                                                                                               |
| SO_SNDBUF              |      variable        | option for setsockopt                                                                                                               |
| SO_RCVBUF              |      variable        | option for setsockopt                                                                                                               |
| SO_ERROR               |      variable        | option for setsockopt                                                                                                               |
| SO_TYPE                |      variable        | option for setsockopt                                                                                                               |
| AF_UNIX                |      variable        | socket domain                                                                                                                       |
| AF_INET                |      variable        | socket domain                                                                                                                       |


##### stderr.json

| Definition             |    Type              | Explanation                                                                    |
|:----------------------:|:--------------------:|:------------------------------------------------------------------------------:|
| ERR_UNKNOWN            |        macro         | raised when an unknown error occoured                                          |
| ERR_NOINST             |        macro         | raised when an invalid instruction was found                                   |
| ERR_NOLIB              |        macro         | raised when an function from a unknown library was called                      |
| ERR_NOCALL             |        macro         | raised when an unknown function from a library was called                      |
| ERR_STACKSPACE         |        macro         | raised when there was not enough stack space to perform operation              |
| ERR_INVALIDFMODE       |        macro         | raised when an invalid mode for opening a file was used                        |
| ERR_NOFILE             |        macro         | raised when program tried to open non-existing file                            |
| ERR_FOPENERR           |        macro         | raised when opening file failed                                                |
| ERR_SEEK               |        macro         | raised when seeking to file position failed                                    |
| ERR_FCLOSE             |        macro         | raised when closing a file failed                                              |
| ERR_NOSOCK             |        macro         | raised when no socket could be created because limit of N_SOCKS was reached    |
| ERR_SOCKET             |        macro         | raised when socket could not be created                                        |
| ERR_INVALIDSOCK        |        macro         | raised when socket handle for function is invalid                              |
| ERR_CONNECT            |        macro         | raised when connecting to server failed                                        |
| ERR_SOCKSEND           |        macro         | raised when sending data failed                                                |
| ERR_SOCKCLOSE          |        macro         | raised when closing socket failed                                              |
| ERR_RECV               |        macro         | raised when receiving data failed                                              |
| ERR_GETHOSTBYNAME      |        macro         | raised when getting IPv4 from domain name failed                               |
| ERR_SETSOCKOPT         |        macro         | raised when setting socket option failed                                       |
| ERR_LISTEN             |        macro         | raised when call to listen failed                                              |
| ERR_BIND               |        macro         | raised when binding socket failed                                              |
| ERR_SOCKACCEPT         |        macro         | raised when accepting connection failed                                        |
| ERR_SUCCESS            |        macro         | raised when no error occoured.                                                 |


##### stdvars.json

| Definition             |    Type              | Explanation                                                                    |
|:----------------------:|:--------------------:|:------------------------------------------------------------------------------:|
| NULL                   |      variable        | defines the value 0                                                            |


Last but not least are the Program settings, they change certain aspects of the program execution:

| Option                | Arguments               | Explanation                                                                          |
|:---------------------:|:-----------------------:|:------------------------------------------------------------------------------------:|
| .DEBUG                | -                       | Makes Oxidian output debug infos during execution                                    |
| .STACKSIZE            | size                    | Sets the stacksize to size, default is UINT_MAX. Should be set to 0 when not needed. |
| .NULLSTACK            | -                       | Sets the whole bytes to zeros, may take a while when stack is big.                   |


Example preprocessor section:

```
PREPROCESSOR {
    .STACKSIZE 0
    including[READ,PRINTSTACK] <stdio.json>
    including <stdvars.json>
    test = CALL SHORT(0)
}
```

this will make Oxidian not initialize a stack and include the macros READ  
and PRINTSTACK from stdio.json. Aswell as every variable/macro from stdvars.json.  
It also creates a macro with the name `test`. This will replace every occurency of `test`  
with `CALL SHORT(0)`.


#### The Data Section

The second section is the data section. It exists for the purpurse of defining variables  
which can be used in [the code section](#The-Code-Section). In Oxidian there is no such thing as "data types"  
for Oxidian, there's just data of one or the other length. Because of this you can use  
every function with every variable, wohooo! But, it also gives you more oppertunities to  
fxck up.

So in order for you to be able to mix certain value types like hexadecimal, strings and numbers,  
you have to tell the compiler where one data type ends and the other starts.  
This is done trough functions and prefixes. If no function or prefix is given,  
olc.py assumes the value to be hexadecimal.
To declare text, you have to prepend the prefix `ASCII.`.
You could declare a variable in the datasection like this:  
```
my_variable = ASCII.Hello + 20 + ASCII.World! + 0a
```
the plus sign seperates the types from each other. Since spaces are discarded,  
we have to use 20 which is the hexadecimal equivalent to a space. You can find  
a list of hexadecimal-codes [here.](https://www.asciitable.com/)

Then there are following functions:

| Name                 | Value(s)                       | Explanation                                               |
|:--------------------:|:------------------------------:|:---------------------------------------------------------:|
| POINTER              | number from 0 to 2^32 or 2^64  | defines either a 32 or 64bit value based on your Computer |
| LONG                 | number from 0 to 2^64          | defines a value from 0 to 2^64                            |
| INT                  | number from 0 to 2^32          | defines a value from 0 to 2^32                            |
| SHORT                | number from 0 to 2^16          | defines a value from 0 to 2^16                            |
| BYTE                 | number from 0 to 2^8           | defines a value from 0 to 2^8                             |
| TIMES                | number, value                  | defines `value`, `number` times                           |

There's one last function that I haven't put into this table because it is different from the others.  
The function is called `RESV` and takes a number from 0 to 2^32.  
`RESV` can be used when a variable is supposed to be used for storing text that is not pre-defined.  
The `RESV` function is special, because you cannot combine it with other functions, with text or hexadecimal  
values.  
The reason for this is, that the variable's value will not be stored in the file. Space for it is first  
allocated during the program execution. This can come in handy when you want a buffer for something like  
networking and don't want your oxy file to be huge.
Instead the space will be allocated at runtime.

Here is an example for a data section:  
```
DATA {
    string1 = ASCII.Hello + 20 + ASCII.there! + 0a
    string2 = RESV(1024)
    a_pointer = POINTER(1)
    a_number = INT(1337)
    SPAM = TIMES(20, ASCII.A)
}
```

#### The Code Section

The probably most important section is the code section. Here you can store instructions. 
Oxidian will execute the instructions from top to bottom.  
Following instructions are supported:

| Name           | Argument 1                   | Argument 2      | Description                                                           |
|:--------------:|:----------------------------:|:---------------:|:---------------------------------------------------------------------:|
| LOADLIB        | library                      | -               | loads a library, currently supported are IO; MEMORY; NET              |
| SETARG         | byte(number)                 | variable        | assigns a variable to one of the 256 arguments                        |
| CALL           | short(number)                | -               | calls a function of the loaded library                                |
| GOTO           | label or INT(N)              | -               | jumps to the label or the Nth instruction starting from 0             |
| ADD            | variable                     | INT(N)          | adds N to variable                                                    |
| iADD           | variable                     | variable2       | adds variable2 to variable                                            |
| SUB            | variable                     | INT(N)          | subtracts N from the variable                                         |
| iSUB           | variable                     | variable2       | subtracts variable2 from variable                                     |
| CMP            | variable                     | INT(N)          | compares variable and N and saves the result                          |
| iCMP           | variable                     | variable        | compares variable and variable2 and saves the result                  |
| CH             | -                            | -               | only executes next instruction if compare result is higher            |
| CNH            | -                            | -               | only executes next instruction if compare result is not higher        |
| CE             | -                            | -               | only executes next instruction if compare result is equal             |
| CNE            | -                            | -               | only executes next instruction if compare result is not equal         |
| CL             | -                            | -               | only executes next instruction if compare result is lower             |
| CNL            | -                            | -               | only executes next instruction if compare result is not lower         |
| INC            | variable                     | -               | increments variable                                                   |
| DEC            | variable                     | -               | decrements variable                                                   |
| PSH            | SHORT(N)                     | -               | pushes the value N onto the stack                                     |
| iPSH           | variable                     | -               | pushes the variable onto the stack                                    |
| POP            | variable                     | INT(N)          | pops N bytes from the stack into variable                             |
| iPOP           | variable                     | variable2       | pops variable2 bytes from the stack into variable                     |
| SSPP           | INT(N)                       | -               | sets the position of the stack pointer to N                           |
| iSSPP          | variable                     | -               | sets the position of the stack pointer to variable                    |
| GSPP           | variable                     | -               | moves the current stack pointer position into variable                |
| iGETL          | variable                     | variable2       | moves length of variable2 into variable                               |
| AND            | variable                     | INT(N)          | performs logical AND on variable and N and stores in variable         |
| iAND           | variable                     | variable2       | performs logical AND on variable and variable2 and stores in variable |
| OR             | variable                     | INT(N)          | performs logical OR on variable and N and stores in variable          |
| iOR            | variable                     | variable2       | performs logical OR on variable and variable2 and stores in variable  |
| XOR            | variable                     | INT(N)          | performs logical XOR on variable and N and stores in variable         |
| iXOR           | variable                     | variable2       | performs logical XOR on variable and variable2 and stores in variable |
| iREF           | variable                     | variable2       | stores position of variable2 in variable                              |
| INVOKE         | label or INT(N)              | -               | jumps to the label or Nth instruction and stores current position     |
| RETURN         | -                            | -               | returns to the stored position                                        |
| CHKERR         | INT(N)                       | -               | only executes next instruction if the error N occoured                |
| EOF            | -                            | -               | exits the program                                                     |

most should make sense to you now, except labels ofcourse, because we haven't discussed them yet.  
Labels are little markers noting positions in the program, you can use instructions like GOTO or  
INVOKE in order to jump to them rather than always counting which Nth instruction you want to jump  
to.
```
an_example_label:
    GOTO an_example_label
```
should the program end up executing this instruction, it will hang in a loop until you brutally kill it.  
as an example I will show you an hello world code without using any librarys:

```
PREPROCESSOR {
    .STACKSIZE 0
}
DATA {
    string = ASCII.Hello + 20 + ASCII.World! + 0a
    num    = INT(10)
}
CODE {
    LOADLIB IO
    SETARG BYTE(0) string
_loop:
    CMP num INT(0)
    CE
    EOF
    CALL SHORT(0)
    DEC num
    GOTO _loop
}
```

This will pring "Hello World!" 10 times.



This document is still under construction and more info will be added soon!

## Cross Platform Limitations

I did originally not intend to make Oxidian cross-platform but since it's not that hard of an task to  
add several preprocessor checks, I decided to make Oxidian compatible with Windows aswell.  
However, this has some limitations. At the time of writing, the only way to get the function  
`SETSOCKSSL` working is with OpenSSL which I haven't been able to implement under Windows yet.  
However, this is pretty far up on my priority list and thus will be implemented in near future.  

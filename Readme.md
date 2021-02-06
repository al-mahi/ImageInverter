##### Bitmap Photo Negative Creator

The program `imginv` is a command line tool that takes a bitmap photo as input and creates 
a photo negative of that bitmap image. This program was developed in and tested in ubuntu 
linux. It can be built on Windows or Mac OS as well but I have not tested it in those machine.
The program also tries to detect some common malformed bitmap and ignores processing them.

###### Build
It is CMake project. To make create directory to store the build files.

`ImageInverter$ mkdir build`

`ImageInverter$cd build`

`build$ cmake ..`

`build$ make`

`./imginv input.bmp`

###### Usage
`$./imginv [OPTIONS] FILE` For example `$./imginv input.bmp` will create a photo negative 
of the bitmap image of `input.bmp`. 

If no output file name is specified the output will be
saved in a file named same name of input file but appended by `_output.bmp` by default. To
specify output file name one can run the program with option `-output outputFileName` or
 `-o outputFileName` for short option. `$./imginv -o output.bmp input.bmp` 
Run the program with `-v` or `--verbose` option to get more details about information while
the program is running.
 
To see all the option and get help run the program with `--help` or `-h` option.

###### Limitations of this Program
* This program only works with 24-bit uncompressed bitmap images
* This program does not work with bitmaps which has ill formatted headers. 
See format (https://en.wikipedia.org/wiki/BMP_file_format)
* Optional headers and padding are ignored for parsing and processing


 
###### Sample Input Output
Here is a photo of Bangladeshi Cricket Player Mashrafi Bin Mortuza [2]
![Tiger Image Input](images/bcb.bmp)
Negative of the image. 
![Tiger Image Input](images/bcb.bmp_negative.bmp)


###### References

* [1] https://en.wikipedia.org/wiki/BMP_file_format
* [2] https://en.wikipedia.org/wiki/Mashrafe_Mortaza

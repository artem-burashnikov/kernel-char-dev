# rngdrv

[![GPLv3][license-shield]][license-url]

## Overview

Linux Kernel module which provides a character device that generates random bytes.

## Getting Started

### Prerequisites

- Make
- Tested on Ubuntu 22.04 Kernel 6.5.0-28-generic

### Building

Open the terminal and follow these steps:

1. Clone the repository:

    ```sh
    git clone git@github.com:artem-burashnikov/rngdrv.git
    ```

2. Navigate to the project root:

    ```sh
    cd rngdrv/
    ```

3. Build object files using Make:

    ```sh
    make
    ```

4. Insert the module into the kernel:

    The module takes 3 parameters:

    1. **crs_ord=\<num\>** &mdash; where num a positive number.

    2. **crs_coeffs=\<num1,num2,...\>** &mdash; where **\<num1,num2,...\>**. are positive numbers of length **crs_ord**.

    3. **crs_vals=\<num1,num2,...\>** &mdash; same as **crs_coeffs**.

    For example:

    ```sh
    sudo insmod rngdrv.ko crs_order=3 crs_coeffs=2,4,8 crs_vals=15,3,9
    ```

5. Output the stream of random bytes:

    ```bash
    xxd /dev/rngdrv
    ```

6. To unload the module and delete the device you can use:

    ```bash
    make unload
    ```

7. To clean up generated object and system files you can use:

    ```bash
    make clean
    ```

## Licenses

The project is licensed under [GPLv3][license-url].

<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[license-shield]: https://img.shields.io/github/license/artem-burashnikov/rngdrv.svg?style=for-the-badge&color=blue
[license-url]: LICENSE

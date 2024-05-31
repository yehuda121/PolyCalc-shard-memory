# PolyCalc-shard-memory
PolyCalc is a multi-threaded polynomial calculator program written in C. It allows users to perform arithmetic operations on two polynomials. The program utilizes shared memory and pthreads

PolyCalc is a C program that provides functionalities for performing arithmetic operations on polynomials using shared memory and pthreads. Users can input polynomial operations through the terminal, and the program concurrently processes these operations, displaying the results.

Usage:
1. Compile the program:
    $ gcc -o polycalc polycalc.c -pthread
2. Run the program:
    $ ./polycalc

3. Enter polynomial operations:
    - To add two polynomials, enter: (degree1:coeff1,coeff2,...)ADD(degree2:coeff1,coeff2,...)
    - To subtract two polynomials, enter: (degree1:coeff1,coeff2,...)SUB(degree2:coeff1,coeff2,...)
    - To multiply two polynomials, enter: (degree1:coeff1,coeff2,...)MUL(degree2:coeff1,coeff2,...)
    - Enter "END" to exit the program.

Example:
    (2:1,2,3)ADD(1:4,5)    // Adds two polynomials: 1x^2 + 2x + 3 and 4x + 5

4. View the results:
    - The program will display the result of each polynomial operation.

Running Multiple Instances:
- You can run multiple instances of the program simultaneously in different terminal windows or tabs.
- Each instance will read from and write to the same shared memory segment, enabling communication between them.
- Input polynomial operations in one terminal window, and see the results in that window and other instances running in different terminals.

Features:
- Concurrent processing: Utilizes pthreads to enable concurrent input and computation of polynomial operations.
- Shared memory: Uses shared memory for inter-thread communication, enhancing performance.
- User-friendly interface: Provides a simple and intuitive interface for entering polynomial operations.
- Error handling: Includes error handling to ensure robustness and stability.

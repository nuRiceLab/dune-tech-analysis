# Running Programs

## Running Python

### Option 1: Notebook
1. Create a Interactive Python Notebook file by clicking the Python icon in the first row of the launcher.
---
2. Type your program.
---
3. Save you program.
---
4. To run your program, locate and click the fast-forward button at the top of the page. (It should like two triangles pointing to the right.)

### Option 2: Python File
1. Create a Python File either by clicking on the Python icon in the third row of the launcher or creating a new text file then renaming it to end with .py instead of .txt
---
2. Type your program
---
3. Save your program.
---
4. In the terminal, type the command in the following format to run your program:
   ```bash
   python [py_file]

   For example, for a python file name solution1.py, you would type python solution1.py
   


## Running a C++ File
1. Make sure that the file ends with .cpp
---
2. In the terminal, type in the following format to compile the program:
   ```bash 
   g++ -o [executable_name] [file_name.cpp] 

   An example for the C++ file, solution1.cpp, would be this:
   ```bash
   g++ -o solution1 solution1.cpp
   Errors may appear that prevent the completion of the compilation of the executable. Address the errors first before atempting the compilation again.
--- 
3. To run the executable, in your terminal type in this format:
    ```bash
   ./[executable] [arguements]
   
   For a program that does not accept arguments, an example would be solution1:
    ```bash
    ./solution1
    
    An example would be for solution 2:
    ```bash
    ./solution2 0 1 2 2 1 0



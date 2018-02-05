# sudoku
reads and interactively solves sudoku puzzles for fun and learning (with opencv + Qt)

The solver works like a human, eliminating options and drawing logical conclusions.
The board is interactive - can use to try and solve puzzle yourself. Hitting the solve button will find solution + show errors and show the solution logic to some degree.
See the /data folder for inputs. It reads the .sudoku files (see examples for format: text, 1 row title, 9x9 grid where 0's are blank, can put multiple puzzles in one file)
Also does (not quite yet) OCR given an image of a sudoku puzzle, identifies the board and reads in the numbers.


I'm using: <br>
Qt 5.7.1 <br>
Qt Creator 4.2.1 <br>
mingw 5.3 <br>
OpenCV 3.1.0 <br>


To get this running on a new machine:
  Install the stuff above
	
  Clone opencv3 and make with CMake. use mingw for compiler (comes with Qt Creator)
	
  Change code/sudoku.pro:
    LIBS += <location of .dll.a files for opencv>
    INCLUDEPATH += <location of opencv's build/include directory> (include should contain opencv2, and opencv2 contains the .hpp's)
														 
  by default should save compile output to sudoku/compile/<release|debug>. I think? unless that's in the .user file
  if stuff isn't loading right, make sure /data files are there. I have hard-coded "../data/<puzzle files>" as the path to load. So like         running the executable from /compile/release, you'd need to copy /data into /compile
    

# sudoku
reads and solves sudoku puzzles for fun and learning

the solver works like a human, eliminating options and drawing logical conclusions.
the board is interactive - can use to try and solve puzzle yourself. Hitting the solve button will find solution + show errors and show the solution logic to some degree.
see the /data folder for inputs. reads the .sudoku files in the example format (text, 1 row title 9x9 grid where 0's are blank, can put multiple puzzles in one file)
also does (not quite yet) OCR given an image of a sudoku puzzle, identifies the board and reads in the numbers.

to compile code: see code/sudoku.pro
would need to change LIBS and INCLUDEPATH to location of opencv/include and /libs on local machine

I'm using:
Qt 5.7.1
Qt Creator 4.2.1
mingw 5.3
OpenCV 3.1.0 - this definitely isn't compatible with opencv2.X due to the ml stuff
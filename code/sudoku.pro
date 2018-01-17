QT       += widgets

QMAKE_CXXFLAGS +=   -Wno-reorder

TARGET = sudoku

HEADERS +=  SudokuBoard.h \
            SudokuWindow.h \
            DrawBorderDelegate.h \
            SudokuTable.h \
            WidgetLegendLines.h \
            ColorMaps.h \
            DigitRecognizer.h

SOURCES +=  main.cpp \
            SudokuBoard.cpp \
            SudokuWindow.cpp \
            DrawBorderDelegate.cpp \
            SudokuTable.cpp \
            WidgetLegendLines.cpp \
            ColorMaps.cpp \
            DigitRecognizer.cpp

INCLUDEPATH += "C:/Users/m/Box Sync/Sickle Cell Work - Shared/Red Blood Cell Tracking/Code/Qt/common/build/include"

LIBS += -L"C:/Users/m/Box Sync/Sickle Cell Work - Shared/Red Blood Cell Tracking/Code/Qt/common/build/x86/mingw/lib" \
        -lopencv_highgui310 \
        -lopencv_core310 \
        -lopencv_imgcodecs310 \
        -lopencv_imgproc310 \
        -lopencv_calib3d310 \
        -lopencv_ml310

#include "MainWindowSudoku.h"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindowSudoku sWindow;
    sWindow.show();
    return app.exec();
}

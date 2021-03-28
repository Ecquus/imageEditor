#include <mainwindow.h>
#include <QApplication>
#include <cstring>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    bool debug = argc > 1 && strcmp(argv[1], "-d") == 0;
    
    MainWindow w(debug);
    w.show();

    return a.exec();
}

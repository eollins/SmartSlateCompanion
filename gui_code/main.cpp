#include "notetaker.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NoteTaker w;
    w.show();
    return a.exec();
}

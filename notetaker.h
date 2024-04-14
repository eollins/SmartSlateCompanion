#ifndef NOTETAKER_H
#define NOTETAKER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class NoteTaker;
}
QT_END_NAMESPACE

class NoteTaker : public QMainWindow
{
    Q_OBJECT

public:
    NoteTaker(QWidget *parent = nullptr);
    ~NoteTaker();

private:
    Ui::NoteTaker *ui;
};
#endif // NOTETAKER_H

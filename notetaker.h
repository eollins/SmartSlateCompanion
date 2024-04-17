#ifndef NOTETAKER_H
#define NOTETAKER_H

#include <QMainWindow>
#include <QTcpSocket>

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

    void attemptConnection();
    void onConnected();
    void onDisconnected();
    void processSlateData();
    void onTimeout();
    void setEnabled(bool state);

private slots:
    void on_push_button_clicked();

private:
    Ui::NoteTaker *ui;
    QTimer *reconnectTimer;
    QTimer *heartbeatTimer;
    QTcpSocket *socket;
    int interval = 5000;
};
#endif // NOTETAKER_H

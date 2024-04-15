#include "notetaker.h"
#include "./ui_notetaker.h"
#include <QtConcurrent>
#include <chrono>

void establishConnection(QTcpSocket* socket);
int unixTime();
int lastTimestamp = 0;

NoteTaker::NoteTaker(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::NoteTaker)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &NoteTaker::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NoteTaker::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NoteTaker::processSlateData);

    // Reconnection and monitoring timer
    reconnectTimer = new QTimer(this);
    connect(reconnectTimer, &QTimer::timeout, this, &NoteTaker::attemptConnection);
    reconnectTimer->start(5000); // check every 5 seconds

    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &NoteTaker::onTimeout);
}

void NoteTaker::attemptConnection() {
    if (socket->state() != QTcpSocket::ConnectedState) {
        qDebug() << "Attempting to connect...";
        socket->connectToHost("192.168.4.1", 5001);
    }
}

void NoteTaker::onConnected() {
    qDebug() << "Established.";
    ui->label_2->setText("Slate Connected");

    QString msg = "Hello";
    socket->write(msg.toUtf8());
    socket->flush();

    heartbeatTimer->start(1000); // e.g., send heartbeat every 10 seconds
}

void NoteTaker::onDisconnected() {
    qDebug() << "Disconnected.";
    ui->label_2->setText("Slate Lost");
    socket->close();
    reconnectTimer->start(5000); // Re-attempt connection every 5 seconds
}

void NoteTaker::processSlateData() {
    qDebug() << "Received response";
    lastTimestamp = unixTime();
}

void NoteTaker::onTimeout() {
    if (socket->state() == QTcpSocket::ConnectedState) {
        QString msg = "Heartbeat";
        socket->write(msg.toUtf8());
        socket->flush();

        qDebug() << "Sent heartbeat";
    } else {
        qDebug() << "Could not send heartbeat due to disconnected state.";
    }
}

int unixTime() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    return seconds;
}

NoteTaker::~NoteTaker()
{
    delete ui;
    delete socket;
}

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
    reconnectTimer->start(1000); // check every 5 seconds

    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &NoteTaker::onTimeout);
}

void NoteTaker::attemptConnection() {
    if (socket->state() != QTcpSocket::ConnectedState) {
        qDebug() << "Attempting to connect...";
        socket->connectToHost("192.168.4.1", 2024);
    }
}

void NoteTaker::onConnected() {
    qDebug() << "Established.";
    ui->slate_connection_state->setText("Slate Connected");
    setEnabled(true);

    QString msg = "Hello";
    socket->write(msg.toUtf8());
    socket->flush();

    heartbeatTimer->start(2500); // e.g., send heartbeat every 10 seconds
}

void NoteTaker::onDisconnected() {
    qDebug() << "Disconnected.";
    ui->slate_connection_state->setText("Slate Lost");
    setEnabled(false);
    socket->close();
    reconnectTimer->start(4000); // Re-attempt connection every 5 seconds
}

void NoteTaker::processSlateData() {
    qDebug() << "Received response";
    lastTimestamp = unixTime();
}

void NoteTaker::onTimeout() {
    if (socket->state() == QTcpSocket::ConnectedState) {
        if (unixTime() - lastTimestamp > 10) {
            socket->close();
            qDebug() << "Timeout";
            return;
        }

        QString msg = "Heartbeat";
        socket->write(msg.toUtf8());
        socket->flush();

        qDebug() << "Sent heartbeat " << std::to_string(unixTime() - lastTimestamp);
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

void NoteTaker::setEnabled(bool state) {
    ui->roll_input->setEnabled(state);
    ui->scene_input->setEnabled(state);
    ui->take_input->setEnabled(state);
    ui->notes_input->setEnabled(state);
    ui->push_button->setEnabled(state);
    ui->audio_input->setEnabled(state);
    ui->device_input->setEnabled(state);
    ui->confirm_button->setEnabled(state);
}

NoteTaker::~NoteTaker()
{
    socket->close();

    delete ui;
    delete socket;
}

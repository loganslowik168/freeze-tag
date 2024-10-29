#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

//Other Header Files
#include <QDialog>
#include <QLabel>

//Network Header Files
#include <QUdpSocket>
#include <QtNetwork>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkProxy>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QKeyEvent>
#include <QByteArray>
#include <QStringList>

//Debug header files
#include <QtDebug>

namespace Ui{
class dialogudpmulticast;
}

class dialogUdpMulticast : public QDialog
{
    Q_OBJECT

public:
    Ui::dialogudpmulticast *ui;
    QUdpSocket *udpSocket;              // UDP communication socket
    QHostAddress groupAddress;          // Record the Multicast Address
    QString getLocalIP();               // Obtain the local ip address of object
    QString localIP;                    // Local ip variable
    QString hostIP;                     // Local ip variable
    QStringList playerIPAddressList;    // List of IP Addresses stored in QStrings
    QString enteredName;                // Enter name for user

public:
    explicit dialogUdpMulticast(QWidget *parent = nullptr);
    QStringList playerNetworkUpdatePositionMessage;
    ~dialogUdpMulticast();

public slots:
    void settupNetwork();
    void readDatagramsNetworking();
    void sendChatMsg();
    void sendNetworkMsg(QString message);
    void startMulticast();
    void stopMulticast();
    void hostGame();
    void joinGame();
    void startGame();
};

#endif // NETWORKCONFIG_H

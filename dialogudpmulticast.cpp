#include "networkconfig.h"
#include "ui_dialogudpmulticast.h"
#include <player.h>
#include "dialog.h"
bool net_init=false;

// All qDebug() statements should start with "NETWORKING:  "


dialogUdpMulticast::dialogUdpMulticast(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialogudpmulticast)
{
    ui->setupUi(this);
    this->show(); // No longer a second screen

}

dialogUdpMulticast::~dialogUdpMulticast() // Destructor needs to delete the socket and the ui interface
{
    delete udpSocket;
    delete ui;
}

void dialogUdpMulticast::settupNetwork() // Gets network ready to be set up
{
    localIP = getLocalIP(); //Stores local IP address
    QString port = "60853"; //35806 //NEED TO MAKE GLOBAL //Sugggested can be changed
    ui->connectedTextLabel->setText("Not Connected"); //Connection status set to no

    udpSocket = new QUdpSocket(this); //New socket that will work with

    //Need to play around with this command to see if it will work differently because right now it is just looping back
    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption,1);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(readDatagramsNetworking())); // Signal Slot that connects the recieve message signal to the read funtion
    connect(ui->connectMulticastButton, &QPushButton::clicked, this, &dialogUdpMulticast::startMulticast);
    connect(ui->disconnectMulticastButton, &QPushButton::clicked, this, &dialogUdpMulticast::stopMulticast);
    connect(ui->sendPregameChatButton, &QPushButton::clicked, this, &dialogUdpMulticast::sendChatMsg);
    connect(ui->hostRadioButton_2, &QRadioButton::clicked, this, &dialogUdpMulticast::hostGame);
    connect(ui->joinRadioButton_2, &QRadioButton::clicked, this, &dialogUdpMulticast::joinGame);
    connect(ui->confirmButton, &QPushButton::clicked, this, &dialogUdpMulticast::startGame);
    connect(ui->confirmButton, &QPushButton::clicked, this, &dialogUdpMulticast::close);


    ui->lineEdit_IP->setText(localIP);
    ui->lineEdit_bind->setText(port);
    ui->confirmButton->setEnabled(false);
    ui->hostOnlySettingsGroupBox->hide();

    //This is to store the lists of ip addresses
    playerIPAddressList.append(localIP);
}

QString dialogUdpMulticast::getLocalIP() // Returns the local IP address of machine
{
    QString hostName = QHostInfo::localHostName();
    QHostInfo hostInfo = QHostInfo::fromName(hostName); //Sets local ip address from machine as from address
    QString localIP = "";
    QList<QHostAddress> addList = hostInfo.addresses();

    if (!addList.isEmpty()) //Looks through all the ip addresses until finds the one that is an IPv4 address then saves it
    {
         for (int i=0; i < addList.count(); i++)
        {
            QHostAddress aHost = addList.at(i);
            if(QAbstractSocket::IPv4Protocol == aHost.protocol()) // Must be IPv4 Protocol ip address for this to work the way it is implemented
            {
                qDebug() << "NETWORKING:  LocalIP is set to QHostAddress: " << aHost.toString();
                localIP = aHost.toString();
                break;
            }
        }
    }
    return localIP;
}

void dialogUdpMulticast::startMulticast() //Start multicast group
{
    QString IP = ui->lineEdit_IP->text();
    groupAddress = QHostAddress(IP);//multicast group address
    hostIP = groupAddress.toString();
    quint16 groupPort = ui->lineEdit_bind->text().toUShort();
    if(udpSocket->bind(QHostAddress::AnyIPv4, groupPort, QUdpSocket::ShareAddress)) // If connection success...
    {
        udpSocket->joinMulticastGroup(groupAddress); // join the group

        // This is giving feedback through the debug statements
        qDebug() << "NETWORKING:  **Joined the Multicast Successfully!";
        qDebug() << "NETWORKING:  **Multicast Address IP:" << IP;
        qDebug() << "NETWORKING:  **Bind Port:" << QString::number(groupPort);

        //This is giving feedback to the player through the pregame chat
        ui->pregameChatTextEdit->append("**Joined the Multicast Successfully!");
        //ui->networkLogTextEdit->append("Multicast Address IP:"+IP);
        //ui->networkLogTextEdit->append("Bind Port:" +QString::number(groupPort));

        //ui->confirmButton->setEnabled(false); default state of the button is off until valid connection is made

        ui->connectedTextLabel->setText("Connected! :)"); //Connection status set to yes
        ui->lineEdit_IP->setEnabled(false);  //Don't allow editing after connected
        ui->lineEdit_bind->setEnabled(false);//Don't allow editing after connected

        //sendNetworkMsg("Player 1 joined the game!");
        if (groupAddress.toString() != localIP)
        {
            playerIPAddressList.append(groupAddress.toString());
        }
        sendNetworkMsg("joining"); // Send joining message
        qDebug() << "Sent joining mesage";

        for (int size = 4, i = 0; i < size; i++) //Iterates through the list of ip addresses for the game
        {
            playerNetworkUpdatePositionMessage.append("a");
        }
        for (int size = 4, i = 0; i < size; i++) //Iterates through the list of ip addresses for the game
        {
            qDebug() << "PlayerNetworkUpdatePosition at " << i << " = " << playerNetworkUpdatePositionMessage.at(i);
        }
    }
    else // ELSE connection is not success...
    {
        qDebug() << "NETWORKING:  **Failed to Bind Port\n";
        ui->connectedTextLabel->setText("Not Connected"); //Connection status set to No
        ui->pregameChatTextEdit->setText("Could not connect...");
        //ui->networkLogTextEdit->setText("**Failed to Bind Port\n");
    }
}

void dialogUdpMulticast::stopMulticast() // End the multicast connection
{
    sendNetworkMsg("leaving");
    udpSocket->leaveMulticastGroup(groupAddress);
    udpSocket->abort();                     //Unbind the multicast connection
    playerIPAddressList.clear();
    ui->lineEdit_IP->setEnabled(true);
    ui->lineEdit_bind->setEnabled(true);
    ui->connectedTextLabel->setText("Not Connected"); //Connection status set to no
    qDebug() << "NETWORKING:  **Exited multicast and unbound port!";// for debug statements
    //ui->networkLogTextEdit->append("**Exited multicast and unbound port!");// added to Network chat for user convience
    ui->pregameChatTextEdit->append("Disconnected...");// added to PreGame chat for user convience
}

void dialogUdpMulticast::sendChatMsg() // Send message
{
    quint16 groupPort = ui->lineEdit_bind->text().toUShort();
    QString msg = ui->pregameChatLineEdit->text(); //Send a message
    QByteArray Datagram = msg.toUtf8();

    if (msg == "") // Stops the sending of an Empty message
    {
        return;
    }
    for (int size = playerIPAddressList.size(), i = 0; i < size; i++) //Iterates through the list of ip addresses for the game
    {
        udpSocket->writeDatagram(Datagram,QHostAddress(playerIPAddressList.at(i)),groupPort); // Sends a message to each individual ip Address
    }
    ui->pregameChatTextEdit->append("[Sent message:]"+msg);
    ui->pregameChatLineEdit->clear();
    ui->pregameChatLineEdit->setFocus();
}

void dialogUdpMulticast::sendNetworkMsg(QString message)
{
    quint16 groupPort = ui->lineEdit_bind->text().toUShort();
    if (message == "") // Stops the sending of an Empty message
    {
        return;
    }
    else if (message.front() != "\"")
    {
        message.insert(0, "\"");
    }
    QByteArray Datagram = message.toUtf8();
    for (int size = playerIPAddressList.size(), i = 0; i < size; i++) //Iterates through the list of ip addresses for the game
    {
        udpSocket->writeDatagram(Datagram,QHostAddress(playerIPAddressList.at(i)),groupPort); // Sends a message to each individual ip Address
    }
    qDebug() << "NETWORKING:  [Log:]" << message;
}

void dialogUdpMulticast::readDatagramsNetworking() // Read datagram
{//Read received datagram
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        qint64 test = udpSocket->pendingDatagramSize();
        QString int64toqstinrg = QString::number(test);
        int stringtoint = int64toqstinrg.toInt();
        datagram.resize(stringtoint);

        QHostAddress peerAddr; // receiving address
        quint16 peerPort; // reveiving port
        udpSocket->readDatagram(datagram.data(),datagram.size(),&peerAddr,&peerPort);
        QString str = datagram.data();
        QString peer = "[From "+peerAddr.toString()+":"+QString::number(peerPort)+"]";
        //qDebug() << peer << str; //Showing the datagram was sent
        if (peerAddr != QHostAddress(localIP))// Stops feedback
        {
            if (str.front() == "\"")// Distinguish between networking chat box. Checks for the " to distinguish network commands
            {               // ^ \" is the way to represent " in a string. It doesn't represent the backslash itself at all
                //qDebug() << "before first remove" << str;
                str.remove(0,1); // removes the first character from the string AKA the "
                //qDebug() << "After first remove" << str;
                if (str.front() == "@")// Player Movement
                {
                    //qDebug() << "before Second remove" << str;
                    str.remove(0,1); // Remove the @
                    //qDebug() << "After Second remove" << str;
                    if (str.front() == "0")
                    {
                        playerNetworkUpdatePositionMessage[0] = str;
                        qDebug() << "Update 0 with " << str;
                    }
                    else if (str.front() == "1")
                    {
                        playerNetworkUpdatePositionMessage[1] = str;
                        qDebug() << "Update 1 with " << str;
                    }
                    else if (str.front() == "2")
                    {
                        playerNetworkUpdatePositionMessage[2] = str;
                        qDebug() << "Update 2 with " << str;
                    }
                    else if (str.front() == "3")
                    {
                        playerNetworkUpdatePositionMessage[3] = str;
                        qDebug() << "Update 3 with " << str;
                    }
                    qDebug() << "NETWORKING:  Movement: From " << peerAddr.toString() << " Updated to " << str;
                }
                else if (str.front() == "~" && peerAddr.toString() == hostIP/*And host sends this message*/)// Erase clients playerIPAddressList and removes ~
                {
                    QStringList emptyList;
                    playerIPAddressList = emptyList; // Resets the IP Address List to an empty list
                    qDebug() << "NETWORKING:  Erased IP Address List";
                }
                else if (str.front() == "^" && peerAddr.toString() == hostIP)// Add to NEW playerIPAddressList and removes ^
                {
                    str.remove(0,1);
                    playerIPAddressList.append(str); // appends the ip address
                    qDebug() << "NETWORKING:  Adding IP Address " << str << " to List";
                }
                else if (str.front() == "%")// Game Timer and removes %
                {
                    str.remove(0,1);
                    ui->secondsGameLineEdit->setText(str);
                    qDebug() << "NETWORKING:  GameTime is " << str << " seconds";
                }
                else if (str == "start")// Start message is recieved
                {
                    //Start the game
                    qDebug() << "Game started!";
                    ui->confirmButton->clicked(true);
                }
                else if (str == "joining") // Join message is sent
                {
                    playerIPAddressList.append(peerAddr.toString());
                    qDebug() << peerAddr.toString() << " added to list.";
                    if (localIP == hostIP)
                    {
                        sendNetworkMsg("~"); // Clear all client ip address lists
                        for (int size = playerIPAddressList.size(), i = 0; i < size; i++) //Iterates through the list of ip addresses for the game
                        {
                            sendNetworkMsg("^" + playerIPAddressList.at(i)); // Send add to playerIPAddressList message
                        }
                    }
                }
                else if (str == "hostreset") // host reset message is sent
                {
                    stopMulticast();
                    startMulticast();
                }
                else if (str == "leaving")// Leave message is sent
                {
                    if (playerIPAddressList.contains(peerAddr.toString())) // checks to see if the sender's ip address in is the list
                    {
                        // Basically removes the ipAddress that matches the one at the location of the sender's ip address
                        playerIPAddressList.removeAt(playerIPAddressList.indexOf(peerAddr.toString()));
                        qDebug() << "Player " << peerAddr.toString() << " removed successfully";
                    }
                    else
                    {
                        qDebug() << "ERROR!!: Player " << peerAddr.toString() << " NOT removed!"; // Player not removed but tried to be removed
                    }
                }
                else
                {
                    //qDebug() << "NETWORKING:  [Recieved:] From:" << peerAddr.toString() << " with Message: " << str;
                }
            }
            else // Default Case: Append message to Pregame Chat
            {
                ui->pregameChatTextEdit->append(peer+str); //adds message to pregame chat
                //qDebug() << "NETWORKING:  [Recieved:] From:" << peerAddr.toString() << " with Message: " << str;
            }
        }
    }
}

void dialogUdpMulticast::hostGame()
{
    ui->lineEdit_IP->setEnabled(false);
    ui->lineEdit_bind->setEnabled(false);
    ui->joinRadioButton_2->setEnabled(false);
    ui->hostRadioButton_2->setEnabled(true);
    ui->confirmButton->setEnabled(true);
    ui->hostOnlySettingsGroupBox->show();
    startMulticast(); //Connecting the host to the game
}

void dialogUdpMulticast::joinGame()
{
    ui->label->setText("Joining Game");
    ui->lineEdit_IP->setEnabled(true);
    ui->lineEdit_bind->setEnabled(true);
    ui->joinRadioButton_2->setEnabled(true);
    ui->hostRadioButton_2->setEnabled(false);
    ui->confirmButton->setEnabled(false);
}

void dialogUdpMulticast::startGame()
{
    enteredName=ui->usernameEdit->text();
    if (localIP==hostIP)
    {
        if (net_init==true)
            return;
        //playerIPAddressList.sort();
        sendNetworkMsg("~"); // Clear all client ip address lists
        for (int size = playerIPAddressList.size(), i = 0; i < size; i++) //Iterates through the list of ip addresses for the game
        {
            sendNetworkMsg("^" + playerIPAddressList.at(i)); // Send add to playerIPAddressList message
        }
        qDebug() << "NETWORKING:  Final IP List Sent";
        sendNetworkMsg("%"+ui->secondsGameLineEdit->text());
        sendNetworkMsg("start"); // Send start message
        net_init=true;
    }
}

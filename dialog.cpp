#include <QRandomGenerator>
#include <QRandomGenerator64>
#include "dialog.h"
#include "ui_dialog.h"
#include "ui_dialogudpmulticast.h"
#include "networkconfig.h"
#include "player.h"
#include "tagger.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(udpmulticast.ui->confirmButton, &QRadioButton::clicked, this, &Dialog::dialogSettup);

    // Allocate and configure scene
    scene = new QGraphicsScene(-SCENE_WIDTH/2, -SCENE_HEIGHT/2,
                               SCENE_WIDTH, SCENE_HEIGHT);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setBackgroundBrush(Qt::white);
    // Make scene object the data source for the view object
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    BACKGROUND = QImage(":/sprite_folders/grass2.png");
    scene->setBackgroundBrush(QBrush(BACKGROUND));

    udpmulticast.settupNetwork();
    myIP = udpmulticast.getLocalIP();
    LastPlayerTagged=0;
    // CREATE PLAYER OBJECTS
    db=QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/full/path/to/sql/database");

    //SQLite database is a local file so no host name, port number, username, password, etc.

    if(!db.open())
    {
        qDebug()<<"SQL: Error Opening Database: "<<db.lastError();
        //exit(1);
    }

    QSqlQuery qprep;
    //qprep.exec(("DROP TABLE stuff"));

    if (!qprep.exec("CREATE TABLE IF NOT EXISTS stuff(player TEXT, score INT)"))
    {
        qDebug()<<"Error Creating Table: "<<db.lastError();
        return;
    }


}

void Dialog::dialogSettup()
{
    //random number seeded with time
    QRandomGenerator generator =  QRandomGenerator(QDateTime::currentMSecsSinceEpoch() - QDateTime(QDate::currentDate()).toMSecsSinceEpoch());
    int playerColors[4];
    for (int i=0;i<4;i++)
        playerColors[i]=-1;//initialize array to hold all -1s
    for (int i=0; i<4;i++)
    {
        int tempColor;
        do
        {
            tempColor = generator.bounded(0,8);
        }
        while (std::find(std::begin(playerColors),std::end(playerColors),tempColor)!=std::end(playerColors));
        playerColors[i]=tempColor;
    }
    spawnTagger(0, 200);
    spawnPlayer(-750, -350, 1, static_cast<charColor>(playerColors[1]));
    spawnPlayer(0, -350, 2, static_cast<charColor>(playerColors[2]));
    spawnPlayer(750, -350, 3, static_cast<charColor>(playerColors[3]));

    myNum = defineMyPlayer();//returns your player number (0=tagger)

    //initialize alive players array
    for (int i = 0; i<udpmulticast.playerIPAddressList.count();i++)
    {
        AlivePlayers[i]=i;
    }

    //collision timer
    QTimer* collisionTimer = new QTimer;
    collisionTimer->setInterval(25);
    connect(collisionTimer, &QTimer::timeout,this, &Dialog::CollisionDetection);
    collisionTimer->start();
    // Configure timer object to drive animatio
    QTimer* timer = new QTimer;
    timer->setInterval(24);
    connect(timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
    timer->start();

    connect(gameOverTimer, &QTimer::timeout, this, &Dialog::endGameFunction); // Calls the game over function
    if ((udpmulticast.ui->secondsGameLineEdit->text().toInt() > 0))
    {
        seconds = std::min(udpmulticast.ui->secondsGameLineEdit->text().toInt(),600);

    }
    else
    {
        seconds = 180; //Default of 3 minutes
    }
    gameOverTimer->start(seconds * 1000);

    clockupdate->setInterval(1000);
    ui->minuteTimerDisplay->display(QString::number(seconds / 60)); // Number of minutes per the clock
    ui->secondTimerDisplay->display(QString::number(seconds % 60)); // Number of seconds per the clock
    connect (clockupdate, &QTimer::timeout, this, &Dialog::updateMultipleTimerDisplays);
    clockupdate->start();

    sendMovementDataTimer->setInterval(50);
    connect (sendMovementDataTimer,&QTimer::timeout,this,&Dialog::sendMoveData);
    sendMovementDataTimer->start();
}

void Dialog::updateMultipleTimerDisplays()
{
    gamePlayingTime++;

    static int count = ui->secondTimerDisplay->intValue();
    count--;
    if ((ui->minuteTimerDisplay->intValue() == 0 && ui->secondTimerDisplay->intValue() == 0) || ui->minuteTimerDisplay->intValue() < 0)
    {
        return;
    }
    if (count % 60 == 0 || count == -1)
   {
        ui->minuteTimerDisplay->display(QString::number(ui->minuteTimerDisplay->intValue()-1));
        ui->secondTimerDisplay->display(QString::number(59));
   }
   else
   {
       ui->secondTimerDisplay->display(QString::number(ui->secondTimerDisplay->intValue()-1));
   }

    if (player1->IsFrozen())
    {
        player1->addToSecondsFrozen();
    }
    if (player2->IsFrozen())
    {
        player2->addToSecondsFrozen();
    }
    if (player3->IsFrozen())
    {
        player3->addToSecondsFrozen();
    }
    if (player1->IsFrozen() && player2->IsFrozen() && player3->IsFrozen()) // If players 1, 2, & 3 are frozen
    {
        endGameFunction(); // Ends the game
    }

    switch (myNum){
        case 0:
            myScore = player1->getSecondsFrozen() + player2->getSecondsFrozen() + player3->getSecondsFrozen();;
            break;
    case 1:
            if (player1->freezeCount >= 2)
            {
                player1->score = 0;
                break;
            }
            myScore = gamePlayingTime - player1->getSecondsFrozen();
            break;
    case 2:
            if (player2->freezeCount >= 2)
            {
                player2->score = 0;
                break;
            }
            myScore = gamePlayingTime - player3->getSecondsFrozen();
            break;
    case 3:
            if (player3->freezeCount >= 2)
            {
                player3->score = 0;
                break;
            }
            myScore = gamePlayingTime - player3->getSecondsFrozen();
            break;
    }
    ui->scoreMainUiLabel->setText(QString::number(myScore));
}

void Dialog::showTitleScreen()
{
    QDialog * dialogudpmulticast = new QDialog; //can add parameters (0,0) to "new QDialog" if necessary later
    Ui_dialogudpmulticast dialogudpmulticastUi;
    dialogudpmulticastUi.setupUi(dialogudpmulticast);
}
void Dialog::keyPressEvent(QKeyEvent *e)
{
    if (e && (myNum==1) && !player1->IsFrozen())
    {
        switch (e->key())
        {
            case Qt::Key_A:
            case Qt::Key_J:
            case Qt::Key_Left:
                                    player1->goLeft();
                                    break;
            case Qt::Key_D:
            case Qt::Key_L:
            case Qt::Key_Right:     player1->goRight();
                                    break;
            case Qt::Key_W:
            case Qt::Key_I:
            case Qt::Key_Up:        player1->goUp();
                                    break;
            case Qt::Key_S:
            case Qt::Key_K:
            case Qt::Key_Down:      player1->goDown();
                                    break;
            case Qt::Key_Q:
            case Qt::Key_U:
                                    player1->goUL();
                                    break;
            case Qt::Key_E:
            case Qt::Key_O:
                                    player1->goUR();
                                    break;
            case Qt::Key_Z:
            case Qt::Key_M:
                                    player1->goDL();
                                    break;
            case Qt::Key_C:
            case Qt::Key_Period:
                                    player1->goDR();
                                    break;
            case Qt::Key_Space:
                                    break;
            default:
                                    break;
        }
    }
    else if (e && (myNum==2) && !player2->IsFrozen())
    {
        switch (e->key())
        {
            case Qt::Key_A:
            case Qt::Key_J:
            case Qt::Key_Left:
                                    player2->goLeft();
                                    break;
            case Qt::Key_D:
            case Qt::Key_L:
            case Qt::Key_Right:     player2->goRight();
                                    break;
            case Qt::Key_W:
            case Qt::Key_I:
            case Qt::Key_Up:        player2->goUp();
                                    break;
            case Qt::Key_S:
            case Qt::Key_K:
            case Qt::Key_Down:      player2->goDown();
                                    break;
            case Qt::Key_Q:
            case Qt::Key_U:
                                    player2->goUL();
                                    break;
            case Qt::Key_E:
            case Qt::Key_O:
                                    player2->goUR();
                                    break;
            case Qt::Key_Z:
            case Qt::Key_M:
                                    player2->goDL();
                                    break;
            case Qt::Key_C:
            case Qt::Key_Period:
                                    player2->goDR();
                                    break;
            case Qt::Key_Space:
                                    break;
            default:
                                    break;
        }
    }
    else if (e && (myNum==3) && !player3->IsFrozen())
    {
        switch (e->key())
        {
            case Qt::Key_A:
            case Qt::Key_J:
            case Qt::Key_Left:
                                    player3->goLeft();
                                    break;
            case Qt::Key_D:
            case Qt::Key_L:
            case Qt::Key_Right:     player3->goRight();
                                    break;
            case Qt::Key_W:
            case Qt::Key_I:
            case Qt::Key_Up:        player3->goUp();
                                    break;
            case Qt::Key_S:
            case Qt::Key_K:
            case Qt::Key_Down:      player3->goDown();
                                    break;
            case Qt::Key_Q:
            case Qt::Key_U:
                                    player3->goUL();
                                    break;
            case Qt::Key_E:
            case Qt::Key_O:
                                    player3->goUR();
                                    break;
            case Qt::Key_Z:
            case Qt::Key_M:
                                    player3->goDL();
                                    break;
            case Qt::Key_C:
            case Qt::Key_Period:
                                    player3->goDR();
                                    break;
            case Qt::Key_Space:
                                    break;
            default:
                                    break;
        }
    }
    else if (e && (myNum==0))
    {
        switch (e->key())
        {
            case Qt::Key_A:
            case Qt::Key_J:
            case Qt::Key_Left:
                                    playerT->goLeft();
                                    break;
            case Qt::Key_D:
            case Qt::Key_L:
            case Qt::Key_Right:
                                    playerT->goRight();
                                    break;
            case Qt::Key_W:
            case Qt::Key_I:
            case Qt::Key_Up:
                                    playerT->goUp();
                                    break;
            case Qt::Key_S:
            case Qt::Key_K:
            case Qt::Key_Down:
                                    playerT->goDown();
                                    break;
            case Qt::Key_Q:
            case Qt::Key_U:
                                    playerT->goUL();
                                    break;
            case Qt::Key_E:
            case Qt::Key_O:
                                    playerT->goUR();
                                    break;
            case Qt::Key_Z:
            case Qt::Key_M:
                                    playerT->goDL();
                                    break;
            case Qt::Key_C:
            case Qt::Key_Period:
                                    playerT->goDR();
                                    break;
            case Qt::Key_Space:
                                    break;
            default:
                                    break;
        }
    }
    QDialog::keyPressEvent(e);
}
void Dialog::keyReleaseEvent(QKeyEvent *event)
{
    if (myNum==0)
    {
        if((event->key() == Qt::Key_A) || (event->key() == Qt::Key_Period)|| (event->key() == Qt::Key_O) || (event->key() == Qt::Key_J) || (event->key() == Qt::Key_Left) || (event->key() == Qt::Key_E) || (event->key() == Qt::Key_C))
        {
            playerT->setStand("left");
        }
        else if((event->key() == Qt::Key_W) || (event->key() == Qt::Key_M) || (event->key() == Qt::Key_U) ||(event->key() == Qt::Key_I) || (event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Q) || (event->key() == Qt::Key_Z))
        {
            playerT->setStand("up");
        }
        else if((event->key() == Qt::Key_D) || (event->key() == Qt::Key_L) || (event->key() == Qt::Key_Right))
        {
            playerT->setStand("right");
        }
        else if((event->key() == Qt::Key_S) || (event->key() == Qt::Key_K) || (event->key() == Qt::Key_Down))
        {
            playerT->setStand("down");
        }
    }
    else if (myNum==1)
    {
        if((event->key() == Qt::Key_A) || (event->key() == Qt::Key_Period)|| (event->key() == Qt::Key_O) || (event->key() == Qt::Key_J) || (event->key() == Qt::Key_Left) || (event->key() == Qt::Key_E) || (event->key() == Qt::Key_C))
        {
            player1->setStand("left");
        }
        else if((event->key() == Qt::Key_W) || (event->key() == Qt::Key_M) || (event->key() == Qt::Key_U) || (event->key() == Qt::Key_I) || (event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Q) || (event->key() == Qt::Key_Z))
        {
            player1->setStand("up");
        }
        else if((event->key() == Qt::Key_D) || (event->key() == Qt::Key_L) || (event->key() == Qt::Key_Right))
        {
            player1->setStand("right");
        }
        else if((event->key() == Qt::Key_S) || (event->key() == Qt::Key_K) || (event->key() == Qt::Key_Down))
        {
            player1->setStand("down");
        }
    }
    else if (myNum==2)
    {
        if((event->key() == Qt::Key_A) || (event->key() == Qt::Key_Period)|| (event->key() == Qt::Key_O) || (event->key() == Qt::Key_J) || (event->key() == Qt::Key_Left) || (event->key() == Qt::Key_E) || (event->key() == Qt::Key_C))
        {
            player2->setStand("left");
        }
        else if((event->key() == Qt::Key_W) || (event->key() == Qt::Key_M) || (event->key() == Qt::Key_U) || (event->key() == Qt::Key_I) || (event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Q) || (event->key() == Qt::Key_Z))
        {
            player2->setStand("up");
        }
        else if((event->key() == Qt::Key_D) || (event->key() == Qt::Key_L) || (event->key() == Qt::Key_Right))
        {
            player2->setStand("right");
        }
        else if((event->key() == Qt::Key_S) || (event->key() == Qt::Key_K) || (event->key() == Qt::Key_Down))
        {
            player2->setStand("down");
        }
    }
    else if (myNum==3)
    {
        if((event->key() == Qt::Key_A) || (event->key() == Qt::Key_Period)|| (event->key() == Qt::Key_O) || (event->key() == Qt::Key_J) || (event->key() == Qt::Key_Left) || (event->key() == Qt::Key_E) || (event->key() == Qt::Key_C))
        {
            player3->setStand("left");
        }
        else if((event->key() == Qt::Key_W) || (event->key() == Qt::Key_M) || (event->key() == Qt::Key_U) || (event->key() == Qt::Key_I) || (event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Q) || (event->key() == Qt::Key_Z))
        {
            player3->setStand("up");
        }
        else if((event->key() == Qt::Key_D) || (event->key() == Qt::Key_L) || (event->key() == Qt::Key_Right))
        {
            player3->setStand("right");
        }
        else if((event->key() == Qt::Key_S) || (event->key() == Qt::Key_K) || (event->key() == Qt::Key_Down))
        {
            player3->setStand("down");
        }
    }
    QDialog::keyReleaseEvent(event);
}

void Dialog::spawnPlayer(int x, int y, int playernumber, charColor color)
{
    // Spawn custom Player object in center of scene
    switch(playernumber)
    {
        case 1:
            player1 = new Player;
            player1->setNumber(playernumber);
            player1->setPos(x,y);
            player1->setPlayerColor(color);
            scene->addItem(player1);
            break;
        case 2:
            player2 = new Player;
            player2->setNumber(playernumber);
            player2->setPos(x,y);
            player2->setPlayerColor(color);
            scene->addItem(player2);
            break;
        case 3:
            player3 = new Player;
            player3->setNumber(playernumber);
            player3->setPos(x,y);
            player3->setPlayerColor(color);
            scene->addItem(player3);
            break;
        default:
            qDebug() << "FATAL ERROR: CREATING PLAYER";
            break;//not required but good practice
    }
}
void Dialog::spawnTagger(int x, int y)
{
    // Spawn custom Player object in center of scene
    playerT = new Tagger;
    playerT->setPos(x,y);
    scene->addItem(playerT);
}

int Dialog::defineMyPlayer()
{
    qDebug() << ">>PLAYER DEFINED";
    return udpmulticast.playerIPAddressList.indexOf(myIP);
}

void Dialog::scoreUpdate()
{
    // Increment score and update display object contents
    myScore++;
    display->setPlainText(QString::number(myScore));
}

Dialog::~Dialog()
{
    delete ui;
}
//reimplementation of some networking functions in dialog.cpp
QString Dialog::NET_getLocalIP() // Returns the local IP address of machine
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
void Dialog::printPlayerList()
{
    for (int i=0;i<locallyAccuratePlayerIPs.size();i++)
    {
        qDebug() << "Dialog Network: " << locallyAccuratePlayerIPs[i] << " is player " << i << endl;
    }
}

void Dialog::getMoveData()
{
    QString s;
    QString temp;
    int index;
    double tempX;
    double tempY;

    if (myNum != 0 && udpmulticast.playerNetworkUpdatePositionMessage[0] != QString("a"))
    {
        //Tagger
        s = udpmulticast.playerNetworkUpdatePositionMessage.at(0);//"1,50,50,6";
        s.remove(0,s.indexOf(',')+1);
        temp = s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempX=temp.toDouble();
        s.remove(0,index+1);
        temp=s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempY=temp.toDouble();
        s.remove(0,(index+1));
        temp = s;
        playerT->setPos(tempX, tempY);
        playerT->setFrame(temp.toInt());
        qDebug() << "PLAYER T Movement COMPLETE with frame" << temp.toInt();
        s = udpmulticast.playerNetworkUpdatePositionMessage.at(1);//"1,50,50,6";
        s.remove(0,s.indexOf(',')+1);
        temp = s;
    }
    if (myNum != 1 && udpmulticast.playerNetworkUpdatePositionMessage[1] != QString("a"))
    {
        //Player 1
        s = udpmulticast.playerNetworkUpdatePositionMessage.at(1);//"1,50,50,6";
        s.remove(0,s.indexOf(',')+1);
        temp = s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempX=temp.toDouble();
        s.remove(0,index+1);
        temp=s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempY=temp.toDouble();
        s.remove(0,(index+1));
        temp = s;
        player1->setPos(tempX, tempY);
        player1->setFrame(temp.toInt());
        qDebug() << "PLAYER 1 Movement COMPLETE";
    }
    if (myNum != 2 && udpmulticast.playerNetworkUpdatePositionMessage[2] != QString("a"))
    {
        //Player 2
        s = udpmulticast.playerNetworkUpdatePositionMessage.at(2);//"1,50,50,6";
        s.remove(0,s.indexOf(',')+1);
        temp = s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempX=temp.toDouble();
        s.remove(0,index+1);
        temp=s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempY=temp.toDouble();
        s.remove(0,(index+1));
        temp = s;
        player2->setPos(tempX, tempY);
        player2->setFrame(temp.toInt());
        qDebug() << "PLAYER 2 Movement COMPLETE";
    }
    if (myNum != 3 && udpmulticast.playerNetworkUpdatePositionMessage[3] != QString("a"))
    {
        //Player 3
        s = udpmulticast.playerNetworkUpdatePositionMessage.at(3);//"1,50,50,6";
        s.remove(0,s.indexOf(',')+1);
        temp = s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempX=temp.toDouble();
        s.remove(0,index+1);
        temp=s;
        index = s.indexOf(',');
        temp.remove(index, temp.size()-index);
        tempY=temp.toDouble();
        s.remove(0,(index+1));
        temp = s;
        player3->setPos(tempX, tempY);
        player3->setFrame(temp.toInt());
        qDebug() << "PLAYER 3 Movement COMPLETE";
    }
}

void Dialog::sendMoveData()
{
    qDebug() << myNum << " is sending Move data";
    switch(myNum)
    {
        case 1: udpmulticast.sendNetworkMsg("@"+QString::number(myNum)+","+player1->getX()+","+player1->getY()+","+QString::number(player1->getFrame()));
                break;
        case 2: udpmulticast.sendNetworkMsg("@"+QString::number(myNum)+","+player2->getX()+","+player2->getY()+","+QString::number(player2->getFrame()));
                break;
        case 3: udpmulticast.sendNetworkMsg("@"+QString::number(myNum)+","+player3->getX()+","+player3->getY()+","+QString::number(player3->getFrame()));
                break;
    case 0: udpmulticast.sendNetworkMsg("@"+QString::number(myNum)+","+playerT->getX()+","+playerT->getY()+","+QString::number(playerT->getFrame()));
                break;
    }
    if (player1->freezeCount>=2)
    {
        scene->removeItem(player1);
    }
    if (player2->freezeCount>=2)
    {
        scene->removeItem(player2);
    }
    if (player3->freezeCount>=2)
    {
        scene->removeItem(player3);
    }
    //ALLOW ONLY TAGGING ONCE
    QList<QGraphicsItem*> list = playerT->collidingItems();
    getMoveData();
}

void Dialog::readMsgNetwork()
{
    udpmulticast.readDatagramsNetworking();
}

void Dialog::CollisionDetection()
{
    QList<QGraphicsItem*> tagList = playerT->collidingItems();
    foreach (QGraphicsItem * i,tagList)
    {
        Player * p_player = dynamic_cast<Player*>(i);
        if (p_player&&(LastPlayerTagged!=p_player->playerNumber)&&!(p_player->IsFrozen()))
        {
            LastPlayerTagged=p_player->playerNumber;
            bool shouldElim=p_player->Freeze();
            qDebug() << p_player->playerNumber << " has a count of " << p_player->freezeCount;
            if (shouldElim)
            {
                scene->removeItem(p_player);
                AlivePlayers[p_player->playerNumber]=-1;
            }
        }
    }
    //player collision detection
    QList<QGraphicsItem*> p1List = player1->collidingItems();
    foreach (QGraphicsItem * i,p1List)
    {
        Player * p_player = dynamic_cast<Player*>(i);
        if (p_player&&(player1->IsFrozen()))
        {
            player1->Unfreeze();
        }
    }
    QList<QGraphicsItem*> p2List = player2->collidingItems();
    foreach (QGraphicsItem * i,p2List)
    {
        Player * p_player = dynamic_cast<Player*>(i);
        if (p_player&&(player2->IsFrozen()))
        {
            player2->Unfreeze();
        }
    }
    QList<QGraphicsItem*> p3List = player3->collidingItems();
    foreach (QGraphicsItem * i,p3List)
    {
        Player * p_player = dynamic_cast<Player*>(i);
        if (p_player&&(player3->IsFrozen()))
        {
            player3->Unfreeze();
        }
    }
}

void Dialog::endGameFunction()
{
    //Timers
    gameOverTimer->stop();
    clockupdate->stop();
    sendMovementDataTimer->stop();

    // SCORE STUFF
    qDebug() << "Scores: ";

    //Would like to change Score to player variable
    playerT->score = player1->getSecondsFrozen() + player2->getSecondsFrozen() + player3->getSecondsFrozen();
    qDebug() << "Tagger: " << playerT->score;
    if (player1->freezeCount >= 2)
    {
        player1->score = 0;
    }
    else
    {
        player1->score = gamePlayingTime - player1->getSecondsFrozen();
    }
    qDebug() << "Player 1: " << player1->score;
    if (player2->freezeCount >= 2)
    {
        player2->score = 0;
    }
    else
    {
        player2->score = gamePlayingTime - player2->getSecondsFrozen();
    }
    qDebug() << "Player 2: " << player2->score;
    if (player3->freezeCount >= 2)
    {
        player3->score = 0;
    }
    else
    {
        player3->score = gamePlayingTime - player3->getSecondsFrozen();
    }
    qDebug() << "Player 3: " << player3->score; // 173 was the score
    switch (myNum){
        case 0:
                myScore = playerT->score;
                break;
        case 1:
                myScore = player1->score;
                break;
        case 2:
                myScore = player2->score;
                break;
        case 3:
                myScore = player3->score;
                break;
        default:
                break;
    }

    ui->minuteTimerDisplay->hide();
    ui->secondTimerDisplay->hide();
    ui->graphicsView->hide();
    QDialog* d = new QDialog();
    d->setWindowTitle("GAME OVER");
    QLabel* label2 = new QLabel("Your Score: ");
    QLabel* label3 = new QLabel(QString::number(myScore));
    QLabel* spaceLabel2 = new QLabel(" ");
    QLabel* label5 = new QLabel("High Scores:");
    QTextBrowser* textBrowser = new QTextBrowser;

    //Inserts the hider2's name and score into the SQL database
    QString hiderName="Player"+QString::number(myNum);
    QString hunterName="Hunter";
    QSqlQuery qprep;
    if (udpmulticast.enteredName.length() > 0)
    {
       if (myNum!=0)
       {
           hiderName=udpmulticast.enteredName;
           qDebug() << "NAME CHANGED TO " << udpmulticast.enteredName;
       }
       if (myNum==0)
       {
           hunterName=udpmulticast.enteredName;
           qDebug() << "NAME CHANGED TO " << udpmulticast.enteredName;
       }
    }
    if (myNum==0)
    {
        //Hunter SQL Insertion
        qprep.prepare("INSERT INTO stuff (player, score) "
                      "VALUES (:username, :score)");
        qprep.bindValue(":username", hunterName);
        qprep.bindValue(":score", playerT->score);
        if (qprep.exec())
        {
            // Insertion was successful
        }
        else
        {
            // Handle the error
            qDebug() << "Error inserting  Tagger Data" << db.lastError();
        }
    }
    if (myNum==1)
    {
        //Player 1 SQL Insertion
        qprep.prepare("INSERT INTO stuff (player, score) "
                      "VALUES (:username, :score)");
        qprep.bindValue(":username", hiderName);
        qprep.bindValue(":score", player1->score);
        if (qprep.exec())
        {
            // Insertion was successful
        }
        else
        {
            // Handle the error
            qDebug() << "Error inserting  Player 1 Data" << db.lastError();
        }
    }
    if (myNum==2)
    {
        //Player 2 SQL Insertion
        qprep.prepare("INSERT INTO stuff (player, score) "
                      "VALUES (:username, :score)");
        qprep.bindValue(":username", hiderName);
        qprep.bindValue(":score", player2->score);
        if (qprep.exec())
        {
            //Insertion was successful
        }
        else
        {
            // Handle the error
            qDebug() << "Error inserting Player 2 Data" << db.lastError();
        }
    }
    if (myNum==3)
    {
        //Player 3 SQL Insertion
        qprep.prepare("INSERT INTO stuff (player, score) "
                      "VALUES (:username, :score)");
        qprep.bindValue(":username", hiderName);
        qprep.bindValue(":score", player3->score);
        if (qprep.exec()) {
            // Insertion was successful
        }
        else
        {
            // Handle the error
            qDebug() << "Error inserting Player 3 Data" << db.lastError();
        }
    }

    QThread::sleep(1);
    textBrowser->append("***** HIGHSCORE *****"); //appends header above scores
    QSqlQuery selectQuery("SELECT player, score FROM stuff ORDER BY score DESC"); //sorts the SQL table by the highest to lowest score
    int rank=1; // shows what place the player received
    while (selectQuery.next()) //cycles throught the database
    {
        QString s ="# "+ QString::number(rank)+ " "+ selectQuery.value(0).toString()+ " " + selectQuery.value(1).toString();
        textBrowser->append(s);
        rank++; //increases rank
    }
    textBrowser->append("********************\n"); //append at end of SQL data

    QPushButton* restartButton = new QPushButton("Close!");
    QVBoxLayout* layout = new QVBoxLayout(d);

    layout->addWidget(label2);
    layout->addWidget(label3);
    layout->addWidget(spaceLabel2);
    layout->addWidget(label5);
    layout->addWidget(textBrowser);
    layout->addWidget(restartButton);

    connect(restartButton, SIGNAL(clicked()), this, SLOT(close())); //closes game
    connect(restartButton, SIGNAL(clicked()), d, SLOT(close())); //closes popup window

    //Update Score
    ui->scoreMainUiLabel->setText(QString::number(myScore));
    d->exec(); //Modal
}

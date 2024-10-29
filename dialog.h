#ifndef DIALOG_H
#define DIALOG_H

#include <QObject>
#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QTime>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QTextBrowser>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QtGlobal>
#include <QtDebug>
#include "player.h"
#include "tagger.h"
#include "defs.h"
#include "networkconfig.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    void getMoveData();
    explicit Dialog(QWidget *parent = nullptr);
    int AlivePlayers[4];
    ~Dialog();

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    QTimer* gameOverTimer = new QTimer;             // Game Over Timer (Decides when the game is over)
    QTimer* clockupdate = new QTimer;
    QTimer* sendMovementDataTimer = new QTimer;

private:
    Ui::Dialog *ui;
    QGraphicsScene* scene = nullptr;
    QSqlDatabase db;
    QSqlQueryModel qm;
    Player* player1 = nullptr;
    Player* player2 = nullptr;
    Player* player3 = nullptr;
    Tagger* playerT= nullptr;
    dialogUdpMulticast udpmulticast;
    QStringList locallyAccuratePlayerIPs;
    QImage BACKGROUND;
    QGraphicsTextItem *header,*display;             // Score header and display object ptrs
    int myScore = 0;                                // Current score
    QString myIP;                                   //holds the current IP address
    int myNum;
    int LastPlayerTagged;
    int seconds;                                    // Game time in seconds
    int gamePlayingTime = 0;                        // Time the game is actually playing

    void drawPerimeterLines();
    void drawScoreDisplay();
    void spawnPlayer(int x, int y, int playernumber, charColor color);
    void spawnTagger(int x, int y);
    void spawnPucks();
    void showTitleScreen();
    int defineMyPlayer();
    void printPlayerList();
    void sendMoveData();
    void CollisionDetection();
    void readMsgNetwork();

    QTimer* getMovementDataTimer;
    QString NET_getLocalIP();

private slots:
    void dialogSettup();
    void updateMultipleTimerDisplays();
    void scoreUpdate();                             // Executes in response to player-puck collision
    void endGameFunction();
};

#endif // DIALOG_H

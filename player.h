#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QPainter>
#include <QtDebug>
#include "defs.h"
#include "tagger.h"

class Player : public QGraphicsItem
{
public:
    Player(QGraphicsItem *parent = nullptr);
         int freezeCount;
    ~Player() override;

    int score = 0;                              // Current Player score
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void advance(int phase) override;

    // Provide externally accessible steering interface
    void setStand(QString dir);
    void setUSP();
    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();
    void goUR();
    void goDR();
    void goUL();
    void goDL();
    void setNumber(int playernumber);
    int getNumber();
    QString getX();
    QString getY();
    void setFrame(int frame);
    int getFrame();
    bool Freeze();
    void Unfreeze();
    bool IsFrozen();
    int getSecondsFrozen();
    void addToSecondsFrozen();
    bool ShouldDie;
    int playerNumber = 0;                       //Sets player number to null value
    void setPlayerColor(charColor color);       //functions used for initialization of the player:

private:
     charColor color;
     bool frozen;
     int secondsFrozen = 0;
     QPixmap images[NUM_IMAGES];    // Animation sprites
     int index;                     // Index within an image subset
     int offset;                    // Offset within images for direction of travel
     qreal x = 0.0;                 // Current player position (x,y)
     qreal y = 0.0;
     qreal player_dx = 4.5;         // Fixed increment of travel (player_dx, player_dy);
     qreal player_dy = 4.5;
     qreal player_diag = 3;         // pythagorean theorem used to determine the dx and dy values necessary for diagonal movement
     qreal dx = 0.0;                // Desired direction of travel
     qreal dy = 0.0;
};

#endif // PLAYER_H

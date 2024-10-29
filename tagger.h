#ifndef TAGGER_H
#define TAGGER_H
#include <QGraphicsItem>
#include <QPixmap>
#include <QPainter>
#include <QtDebug>
#include <QTimer>
#include <QTime>
#include "defs.h"

class Tagger : public QGraphicsItem
{
//    Player player;
public:
    Tagger(QGraphicsItem *parent = nullptr);
    bool canTag;
    ~Tagger() override;

    int score = 0;                          // Current Tagger score
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
    QString getX();
    QString getY();
    void setFrame(int frame);
    int getFrame();
    QTimer* tagTimer;
    void CannotTag();
    void CanTag();

private:
     QPixmap images[NUM_IMAGES];        // Animation sprites
     int index;                         // Index within an image subset
     int offset;                        // Offset within images for direction of travel
     qreal x = 0.0;                     // Current player position (x,y)
     qreal y = 0.0;
     qreal player_dx = 5;               // Fixed increment of travel (player_dx, player_dy);
     qreal player_dy = 5;
     qreal player_diag = 3.16227766017; // pythagorean theorem used to determine the dx and dy values necessary for diagonal movement
     qreal dx = 0.0;                    // Desired direction of travel
     qreal dy = 0.0;
};

#endif // TAGGER_H

#include "tagger.h"
bool t_USP = false; //t_USP (Update Stand Position)
bool t_animate = false;
bool t_init = false;
Tagger::Tagger(QGraphicsItem *parent)  //: QGraphicsItem(parent)
{
    for(int k = 0; k < NUM_IMAGES; k++)
    {
        QString s = ":/sprite_folders/taggerSprites/taggerFrame" + QString::number(k+1) + QString(".png");
        //QString s = ":/sprite_folders/CYAN_sprites/Cyan" + QString::number(k+1) + QString(".png");
        images[k] = QPixmap(s);
        images[k] = images[k].scaled(TAGGER_WIDTH, TAGGER_HEIGHT);
    }

    // Start at first image of walking towards viewer pixmaps
    index = 0;
    offset = 0;
    tagTimer = new QTimer;
    tagTimer->setInterval(1000);
    //QObject::connect(tagTimer, &QTimer::timeout, this, &Tagger::CanTag);
    canTag=true;
}

Tagger::~Tagger()
{

}

void Tagger::setUSP()
{
    t_USP=true;
}

void Tagger::setStand(QString dir)
{
    stop();
    if (dir=="left")
        offset=18;
    else if (dir=="right")
        offset=27;
    else if (dir=="down")
        offset=0;
    else if (dir=="up")
        offset=9;
    else {
        qDebug() << "BAD INPUT IN setStand()";
    }
}

void Tagger::goLeft()
{
    dx=-player_dx;
    dy=0.0;
    offset=18;
    t_animate = true;
}

void Tagger::goRight()
{
    dx=player_dx;
    dy=0.0;
    offset=27;
    t_animate = true;
}

void Tagger::goUp()
{
    dy=-player_dy;
    dx=0.0;
    offset=9;
    t_animate = true;
}

void Tagger::goDown()
{
    dy=player_dy;
    dx=0.0;
    offset=0;
    t_animate = true;
}

void Tagger::stop()
{
    dx=0.0;
    dy=0.0;
    t_animate = false;
}

void Tagger::goUR()
{
    dx=player_diag;
    dy=-player_diag;
    offset=27;
    t_animate = true;
}

void Tagger::goUL()
{
    dx=-player_diag;
    dy=-player_diag;
    offset=18;
    t_animate = true;
}

void Tagger::goDL()
{
    dx=-player_diag;
    dy=player_diag;
    offset=18;
    t_animate = true;
}

void Tagger::goDR()
{
    dx=player_diag;
    dy=player_diag;
    offset=27;
    t_animate = true;
}

QRectF Tagger::boundingRect() const
{
    return QRectF(-TAGGER_WIDTH/2, -TAGGER_HEIGHT/2, TAGGER_WIDTH, TAGGER_HEIGHT);
}

QPainterPath Tagger::shape() const   //THIS IS FOR COLLISION DETECTION ONLY
{
    QPainterPath path;
    path.addRect(-TAGGER_WIDTH/8,-TAGGER_HEIGHT/8,TAGGER_WIDTH/4,TAGGER_HEIGHT/4); //moves hitbox inwards to cover the head instead of whole body
    return path;
}

void Tagger::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (t_init==false)//draw initial standing sprite
    {
        painter->drawPixmap(-TAGGER_WIDTH/2, -TAGGER_HEIGHT/2, images[0]);
        t_init = true;
    }
    if (t_animate==true)
    {
        index=(index+1)%NUM_FRAMES;
        if (index==0)//removes standing frame from animation
            index++;
        painter->drawPixmap(-TAGGER_WIDTH/2, -TAGGER_HEIGHT/2, images[offset+index]); //update player animation
    }
    if (t_USP==true)
    {
        painter->drawPixmap(-TAGGER_WIDTH/2, -TAGGER_HEIGHT/2, images[offset]); //update player animation
        //qDebug() << "updating to stand position";
        t_USP=false;
    }

    painter->drawPixmap(-TAGGER_WIDTH/2, -TAGGER_HEIGHT/2, images[offset+index]); //update player animation
    painter->setPen(QPen(Qt::transparent));
    painter->drawRect(QRect(-PLAYER_WIDTH/5,-PLAYER_HEIGHT/2.3,PLAYER_WIDTH/2.3,PLAYER_HEIGHT/1));
}

void Tagger::advance(int phase)
{
    if (phase == 0)
    {
        return;
    }
    // Compute new coordinates
    qreal x = this->pos().rx();
    qreal y = this->pos().ry();
    x = x + dx;
    y = y + dy;
    // Force new coordinates to remain within scene bounds
    if (x < -SCENE_WIDTH/2 + TAGGER_WIDTH/4)
    {
        x = -SCENE_WIDTH/2 + TAGGER_WIDTH/4;
    }
    else if (x > SCENE_WIDTH/2 - TAGGER_WIDTH/4)
    {
        x = SCENE_WIDTH/2 - TAGGER_WIDTH/4;
    }

    if (y < -SCENE_HEIGHT/2 + TAGGER_HEIGHT/2)
    {
        y = -SCENE_HEIGHT/2 + TAGGER_HEIGHT/2;
    }
    else if (y > SCENE_HEIGHT/2 - TAGGER_HEIGHT/2)
    {
        y = SCENE_HEIGHT/2 - TAGGER_HEIGHT/2;
    }
    // Move player to new coordinates
    this->setPos(x, y);
    // Detect any collisions
    QList<QGraphicsItem*> list = collidingItems();
    if (!list.isEmpty()){}
}

QString Tagger::getX()
{
    return QString::number(QGraphicsItem::x());
}

QString Tagger::getY()
{
    return QString::number(QGraphicsItem::y());
}

int Tagger::getFrame()
{
    //qDebug() << "Current frame is " << index+offset;
    return (index+offset);
}

void Tagger::setFrame(int frame) // 16
{
    index=frame%9;
    offset=(frame/9) * 9;//rounds down due to nature of int // 1
    qDebug() << "Tagger offset " << offset;
    qDebug() << "Tagger frame" << frame;
}

void Tagger::CannotTag()
{
    tagTimer->start();
    canTag=false;
}

void Tagger::CanTag()
{
    tagTimer->stop();
    canTag=true;
}

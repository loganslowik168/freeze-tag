#include "player.h"
bool USP = false; //USP (Update Stand Position)
bool animate = false;
bool init = false;
Player::Player(QGraphicsItem *parent)  //: QGraphicsItem(parent)
{
    for(int k = 0; k < NUM_IMAGES; k++)
    {
        QString s = ":/images/tile0" + QString::number(k) + QString(".png");
        //QString s = ":/sprite_folders/CYAN_sprites/Cyan" + QString::number(k+1) + QString(".png");
        images[k] = QPixmap(s);
        images[k] = images[k].scaled(PLAYER_WIDTH, PLAYER_HEIGHT);
    }
    // Start at first image of walking towards viewer pixmaps
    index = 0;
    offset = 0;
    freezeCount=0;
    frozen=false;
    ShouldDie=false;
}

Player::~Player()
{

}

void Player::setPlayerColor(charColor color)
{
    QString fn; //filename
    switch(color)
    {
        case BLUE:
                    fn="Blue";
                    break;
        case CYAN:
                    fn="Cyan";
                    break;
        case GREEN:
                    fn="Green";
                    break;
        case LBLUE:
                    fn="LBlue";
                    break;
        case LGREEN:
                    fn="LGreen";
                    break;
        case LPINK:
                    fn="LPink";
                    break;
        case MOSS:
                    fn="Moss";
                    break;
        case PINK:
                    fn="Pink";
                    break;
        case PURPLE:
                    fn="Purple";
                    break;
        default:
                    qDebug() << "ERROR IN setPlayerColor() function";
                    break;
    }
    QString pfn = fn.toUpper();//parent folder name
    if (fn=="Pink")
        fn="LPink";
    for(int k = 0; k < NUM_IMAGES; k++)
    {
        QString s = ":/sprite_folders/" + pfn + "_sprites/" + fn + QString::number(k+1) + QString(".png");
        images[k] = QPixmap(s);
        images[k] = images[k].scaled(PLAYER_WIDTH, PLAYER_HEIGHT);
    }
    //qDebug() << color;
}

void Player::setUSP()
{
    USP=true;
}

void Player::setStand(QString dir)
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

void Player::goLeft()
{
    dx=-player_dx;
    dy=0.0;
    offset=18;
    animate = true;
}

void Player::goRight()
{
    dx=player_dx;
    dy=0.0;
    offset=27;
    animate = true;
}

void Player::goUp()
{
    dy=-player_dy;
    dx=0.0;
    offset=9;
    animate = true;
}

void Player::goDown()
{
    dy=player_dy;
    dx=0.0;
    offset=0;
    animate = true;
}

void Player::stop()
{
    dx=0.0;
    dy=0.0;
    animate = false;
}

void Player::goUR()
{
    dx=player_diag;
    dy=-player_diag;
    offset=27;
    animate = true;
}

void Player::goUL()
{
    dx=-player_diag;
    dy=-player_diag;
    offset=18;
    animate = true;
}

void Player::goDL()
{
    dx=-player_diag;
    dy=player_diag;
    offset=18;
    animate = true;
}

void Player::goDR()
{
    dx=player_diag;
    dy=player_diag;
    offset=27;
    animate = true;
}

void Player::setNumber(int playernum)
{
    playerNumber = playernum;
}

int Player::getNumber()
{
    return playerNumber;
}

QRectF Player::boundingRect() const
{
    return QRectF(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT);
}

QPainterPath Player::shape() const   //THIS IS FOR COLLISION DETECTION ONLY
{
    QPainterPath path;
    path.addRect(-PLAYER_WIDTH/6,-PLAYER_HEIGHT/2.75,PLAYER_WIDTH/2.8,PLAYER_HEIGHT/1.3); //moves hitbox to cover entire body for tag and collisoins
    return path;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (init==false)//draw initial standing sprite
    {
        painter->drawPixmap(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, images[0]);
        init = true;
    }
    if (animate==true)
    {
        index=(index+1)%NUM_FRAMES;
        if (index==0)//removes standing frame from animation
            index++;
        painter->drawPixmap(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, images[offset+index]); //update player animation
    }
    if (USP==true)
    {
        painter->drawPixmap(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, images[offset]); //update player animation
        //qDebug() << "updating to stand position";
        USP=false;
    }
    painter->drawPixmap(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, images[offset+index]); //update player animation
    painter->setPen(QPen(Qt::transparent));
    painter->drawRect(QRect(-PLAYER_WIDTH/6,-PLAYER_HEIGHT/2.75,PLAYER_WIDTH/2.8,PLAYER_HEIGHT/1.3));
}

void Player::advance(int phase)
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
    if (x < -SCENE_WIDTH/2 + PLAYER_WIDTH/4)
    {
        x = -SCENE_WIDTH/2 + PLAYER_WIDTH/4;
    }
    else if (x > SCENE_WIDTH/2 - PLAYER_WIDTH/4)
    {
        x = SCENE_WIDTH/2 - PLAYER_WIDTH/4;
    }
    if (y < -SCENE_HEIGHT/2 + PLAYER_HEIGHT/2)
    {
        y = -SCENE_HEIGHT/2 + PLAYER_HEIGHT/2;
    }
    else if (y > SCENE_HEIGHT/2 - PLAYER_HEIGHT/2)
    {
        y = SCENE_HEIGHT/2 - PLAYER_HEIGHT/2;
    }
    // Move player to new coordinates
    this->setPos(x, y);
    // Detect any collision of player
    QList<QGraphicsItem*> list = collidingItems();
    if (!list.isEmpty()){}
}


QString Player::getX()
{
    return QString::number(QGraphicsItem::x());
}

QString Player::getY()
{
    return QString::number(QGraphicsItem::y());
}

int Player::getFrame()
{
    //qDebug() << index << "with offset " << offset;
    return (index+offset);
}

void Player::setFrame(int frame)
{
    index=frame%9;
    offset=(frame/9) * 9;//rounds down due to nature of int
    //qDebug() << index << "with offset " << offset;
}

bool Player::Freeze() //returns rather the player should be eliminated
{
    freezeCount++;
    frozen = true;
    if (freezeCount>=2)
        return true;
    else
        return false;
}

void Player::Unfreeze()
{
    frozen=false;
}

bool Player::IsFrozen()
{
    return frozen;
}

int Player::getSecondsFrozen()
{
    return secondsFrozen;
}

void Player::addToSecondsFrozen()
{
    secondsFrozen++;
}

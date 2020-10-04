#include "switch.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>

/******************* class Switch *******************/
Switch::Switch(QWidget *parent) : QWidget(parent){
    anim = new QPropertyAnimation(this, "position");
    anim->setDuration(150);
}

QPointF Switch::position(){
    return posit;
}

void Switch::setPosition(QPointF posi){
    posit = posi;
    update();
}

Switch::State Switch::state(){
    return stat;
}

void Switch::click(){
    posit = QPoint(rect().width() - distBetweenMargins, height() / 2);
    stat = Clicked;
    update();
}

void Switch::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(stat == NotClicked){
        painter.setPen(QPen(Qt::black, 2));
        painter.setBrush(QColor(255, 255, 255));
    }
    else{
        painter.setPen(QPen(QColor("#009fc7"), 2));
        painter.setBrush(QColor("#009fc7"));
    }

    painter.drawRoundedRect(QRect(1, 1, rect().width() - 2, rect().height() - 2), borderRadius, borderRadius);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QColor());
    painter.drawEllipse(posit, elHeight, elHeight);

    painter.setPen(QColor("#009fc7"));
    if(stat == NotClicked)
        painter.drawText(QPointF(posit.x() - distBetweenMargins / 2, posit.y() + borderRadius / 2), "✕");
    else if(posit == QPointF(rect().width() - distBetweenMargins, height() / 2))
        painter.drawText(QPointF(posit.x() - borderRadius / 2, posit.y() + borderRadius / 2), "✔");
}

void Switch::mousePressEvent(QMouseEvent *event){
   if(event->buttons().testFlag(Qt::LeftButton)){
        if(stat == NotClicked){
            anim->setStartValue(posit);
            anim->setEndValue(QPoint(rect().width() - distBetweenMargins, height() / 2));
            stat = Clicked;
        }
        else{
            anim->setStartValue(posit);
            anim->setEndValue(QPointF(distBetweenMargins, height() / 2));
            stat = NotClicked;
        }

        emit clicked();
        anim->start();
    }

    QWidget::mousePressEvent(event);
}

void Switch::enterEvent(QEvent *event){
    setCursor(QCursor(Qt::PointingHandCursor));
    QWidget::enterEvent(event);
}

void Switch::resizeEvent(QResizeEvent * event){
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    elHeight = scaleFactor * 6;
    distBetweenMargins = scaleFactor * 12;
    borderRadius = scaleFactor * 10;

    QFont fnt = font();
    fnt.setPointSizeF(scaleFactor + 7.8);
    setFont(fnt);

    if(stat == NotClicked)
        posit = QPointF(distBetweenMargins, event->size().height() / 2);
    else
        posit = QPointF(event->size().width() - distBetweenMargins, event->size().height() / 2);

    update();
}

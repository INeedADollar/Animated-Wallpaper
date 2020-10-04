#include "slider.h"

#include <cmath>
#include <QTime>
#include <QPainter>
#include <QResizeEvent>
#include <QApplication>
#include <QScreen>

/******************* class Slider *******************/
Slider::Slider(QWidget *parent) : QWidget(parent)
{
    createUI();
    setType(DurationBar);
}

void Slider::createUI(){
    currentValueLabel = new QLabel(this);
    currentValueLabel->setStyleSheet("QLabel {color: white;}");
    currentValueLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    totalValueLabel = new QLabel(this);
    totalValueLabel->setStyleSheet("QLabel {color: white;}");
    totalValueLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
}

QString Slider::getTimeString(QTime time){
    int seconds = time.second();
    int minutes = time.minute();
    int hours = time.hour();
    QString durText;

    if(hours < 10)
        durText += "0" + QString::number(hours) + ":";
    else
        durText += QString::number(hours) + ":";

    if(minutes < 10)
        durText += "0" + QString::number(minutes) + ":";
    else
        durText += QString::number(minutes) + ":";

    if(seconds < 10)
        durText += "0" + QString::number(seconds);
    else
        durText += QString::number(seconds);

    return durText;
}

void Slider::setDuration(qint64 durat){
    duration = durat;

    QTime time(0, 0, 0, 0);
    time = time.addMSecs(durat);

    totalValueLabel->setText(getTimeString(time));
}

int Slider::sliderPos(){
    return pos / 10;
}

void Slider::setPosition(qint64 position){
    if(position != 0){
        if(duration == 0){
            if(isVisible())
                hide();
        }
        else{
            if(type == DurationBar){
                QTime time(0, 0, 0, 0);
                time = time.addMSecs(position);
                currentValueLabel->setText(getTimeString(time));
            }
            else
                totalValueLabel->setText(QString::number(position));

            int newPos = (1000 * position) / duration;
            if(pos < newPos){
                pos = newPos;
                update();
            }
        }
    }
    else{
        if(duration == 0){
            if(isVisible())
                hide();
        }
        else{
            if(type == DurationBar)
                currentValueLabel->setText("00:00:00");
            else
                totalValueLabel->setText("0");

            pos = 0;
            update();
        }
    }
}

void Slider::setType(Slider::Type tp){
    type = tp;

    if(tp == VolumeBar){
        currentValueLabel->setText("0");
        totalValueLabel->setText(QString::number(duration));
    }
    else{
        currentValueLabel->setText("00:00:00");
        totalValueLabel->setText(getTimeString(QTime(0, 0, 0, 0).addMSecs(duration)));
    }
}

void Slider::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);

    if((pos != 0 && type == DurationBar) || type == VolumeBar){
        QPainter painter(this);
        painter.setBrush(QColor("#009fc7"));
        painter.setRenderHint(QPainter::Antialiasing);

        if(type == VolumeBar){
            if(pos > 900){
                if(totalValueLabel->pos() == totalValueLabelPos)
                    totalValueLabel->move(totalValueLabelPos + QPoint(10, 0));
            }
            else if(totalValueLabel->pos() != totalValueLabelPos)
                totalValueLabel->move(totalValueLabelPos);

            painter.drawRect(QRectF(currentValueLabel->width() - currentValueLabel->width() / 1.5, barY, pos * frameWidth, barHeight));
            painter.drawEllipse(QPointF((pos * frameWidth) + currentValueLabel->width() - currentValueLabel->width() / 1.5, barY + barHeight / 2), barHeight, barHeight);
        }
        else
            painter.drawRect(QRectF(currentValueLabel->width() - 8, barY, pos * frameWidth, barHeight));
    }
}

qreal Slider::calculateWidthOfOneFrame(int barWidth){
    qreal width = 0.001 * barWidth;
    return width;
}

void Slider::resizeEvent(QResizeEvent *){
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    currentValueLabel->resize(QSize(scaleFactor * 60, scaleFactor * 20));
    QFont fnt = currentValueLabel->font();
    fnt.setPointSizeF(scaleFactor + 7.5);
    currentValueLabel->setFont(fnt);

    totalValueLabel->resize(QSize(scaleFactor * 60, scaleFactor * 20));
    totalValueLabel->move(width() - totalValueLabel->width() - 10, 0);
    totalValueLabelPos = QPoint(width() - totalValueLabel->width() - 10, 0);
    fnt = totalValueLabel->font();
    fnt.setPointSizeF(scaleFactor + 7.5);
    totalValueLabel->setFont(fnt);

    barHeight = scaleFactor * 5;
    barY = scaleFactor * 8;
    int barWidth;
    if(type == VolumeBar)
        barWidth = width() - currentValueLabel->width() - totalValueLabel->width() + scaleFactor * 25;
    else
        barWidth = width() - currentValueLabel->width() - totalValueLabel->width() - 10;

    frameWidth = calculateWidthOfOneFrame(barWidth);
    update();
}

bool pointInsideRect(QPointF point, QRectF rect){
     return point.x() >= rect.x() &&
             point.x() <= rect.x() + rect.width() &&
             point.y() >= rect.y() &&
             point.y() <= rect.y() + rect.height();
}

void Slider::mousePressEvent(QMouseEvent *event){
    int xDragWidget = (pos * frameWidth) + currentValueLabel->width() - currentValueLabel->width() / 1.5;
    if(pointInsideRect(event->pos(), QRectF(xDragWidget - barHeight, barY + barHeight / 2 - barHeight, 2 * barHeight, 2 * barHeight)) && type == VolumeBar)
        isValid = true;

    QWidget::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event){
    if(isValid && event->localPos().x() - (currentValueLabel->width() - currentValueLabel->width() / 1.5) >= 0 && event->localPos().x() - (currentValueLabel->width() - currentValueLabel->width() / 1.5) <= 1000 * frameWidth){
        if(event->localPos().x() - (currentValueLabel->width() - currentValueLabel->width() / 1.5) > 990 * frameWidth){
            emit positionChanged(100);
            totalValueLabel->setText("100");
            pos = 1000;
        }
        else{
            qreal res = fmod((event->localPos().x() - (currentValueLabel->width() - currentValueLabel->width() / 1.5)), frameWidth);
            if(QString::number(res).startsWith("0.0") || res == 0){
                pos = (event->pos().x() - (currentValueLabel->width() - currentValueLabel->width() / 1.5)) / frameWidth;
                totalValueLabel->setText(QString::number(pos / 10));
                emit positionChanged(pos / 10);
            }
        }

        update();
    }

    QWidget::mouseMoveEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent *event){
    if(isValid)
        isValid = false;

    QWidget::mouseReleaseEvent(event);
}

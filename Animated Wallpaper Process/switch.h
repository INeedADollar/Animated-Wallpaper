#ifndef SWITCH_H
#define SWITCH_H

#include <QWidget>
#include <QPropertyAnimation>

/******************* class Switch *******************/
class Switch : public QWidget{
    Q_OBJECT
    Q_PROPERTY(QPointF position READ position WRITE setPosition)

public:
    Switch(QWidget *parent = Q_NULLPTR);
    QPointF position();
    void setPosition(QPointF);
    void click();

    enum State{
        Clicked,
        NotClicked
    };
    State state();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEvent *event) override;

private:
    QPointF posit = QPointF(12, 10);
    QPropertyAnimation *anim;
    State stat = NotClicked;
    qreal elHeight = 6;
    qreal distBetweenMargins = 12;
    qreal borderRadius = 10;

signals:
    void clicked();
};

#endif // SWITCH_H

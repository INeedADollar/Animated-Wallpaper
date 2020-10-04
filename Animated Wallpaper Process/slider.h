#ifndef SLIDER_H
#define SLIDER_H

#include <QLabel>

/******************* class Slider *******************/
class Slider : public QWidget
{
    Q_OBJECT
public:
    enum Type{
        DurationBar,
        VolumeBar
    };

    explicit Slider(QWidget *parent = Q_NULLPTR);
    void setPosition(qint64 pos);
    void setDuration(qint64 duration);
    void setType(Slider::Type);
    int sliderPos();
    static QString getTimeString(QTime);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    qreal calculateWidthOfOneFrame(int barWidth);
    void createUI();

    QLabel *currentValueLabel;
    QLabel *totalValueLabel;
    QPoint totalValueLabelPos;

    qint64 duration = 0;
    int pos = 0;
    qreal frameWidth = 0;
    Type type;
    qreal barHeight = 5;
    qreal barY = 8;

    bool isValid = false;

signals:
    void positionChanged(int pos);

};

#endif // SLIDER_H

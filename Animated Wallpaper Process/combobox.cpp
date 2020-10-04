#include "combobox.h"
#include "utility.h"

#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>

/******************* class Window *******************/
Window::Window(QWidget *parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::Dialog){
    setAttribute(Qt::WA_TranslucentBackground);
}

void Window::setAnimationDirection(AnimationDirection dir){
    animDir = dir;
}

QSize Window::sizeHint() const{
    return childrenRect().size() + QSize(5, 5);
}

void Window::showEvent(QShowEvent *){
    if(animDir == Down){
        QPropertyAnimation *animat = new QPropertyAnimation(this, "size");
        animat->setStartValue(QSize(sizeHint().width(), 0));
        animat->setEndValue(sizeHint());
        animat->setDuration(150);
        animat->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else{
        QPropertyAnimation *animat = new QPropertyAnimation(this, "geometry");
        animat->setStartValue(QRect(x(), y() + sizeHint().height(), sizeHint().width(), 0));
        animat->setEndValue(QRect(pos(), sizeHint()));
        animat->setDuration(150);
        animat->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void Window::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    painter.setBrush(QColor(0, 0, 0, 10));
    painter.setPen(QColor(0, 0, 0, 10));
    if(animDir == Down)
        painter.drawRoundedRect(QRect(QPoint(), size()), scaleFactor * 10, scaleFactor * 10);
    else
        painter.drawRect(QRect(QPoint(0, scaleFactor * 10), size()));

    painter.setBrush(QColor(0, 0, 0, 70));
    painter.setPen(QColor(0, 0, 0, 70));
    if(animDir == Down)
        painter.drawRoundedRect(QRectF(QPoint(), (QSizeF)size() - QSizeF(1.66, 1.66)), scaleFactor * 10, scaleFactor * 10);
    else
        painter.drawRect(QRectF(QPoint(0, scaleFactor * 10), (QSizeF)size() - QSizeF(1.66, 1.66)));

    painter.setBrush(QColor(0, 0, 0, 130));
    painter.setPen(QColor(0, 0, 0, 130));
    if(animDir == Down)
        painter.drawRoundedRect(QRectF(QPoint(), (QSizeF)size() - QSizeF(3.32, 3.32)), scaleFactor * 10, scaleFactor * 10);
    else
        painter.drawRect(QRectF(QPoint(0, scaleFactor * 10), (QSizeF)size() - QSizeF(3.32, 3.32)));
}

/******************* class ListView *******************/
ListView::ListView(QWidget *parent) : QListWidget(parent){

}

void ListView::setAnimationDirection(AnimationDirection dir){
    animDir = dir;
}

void ListView::focusOutEvent(QFocusEvent *event){
    if(animDir == Down){
        QPropertyAnimation *animat = new QPropertyAnimation(parentWidget(), "size");
        animat->setStartValue(parentWidget()->sizeHint());
        animat->setEndValue(QSize(parentWidget()->sizeHint().width(), 0));
        animat->setDuration(150);
        animat->start(QAbstractAnimation::DeleteWhenStopped);
        QObject::connect(animat, &QPropertyAnimation::finished, [=]{
            emit focusLost();
        });
    }
    else{
        QPropertyAnimation *animat = new QPropertyAnimation(parentWidget(), "geometry");
        animat->setStartValue(QRect(parentWidget()->pos(), parentWidget()->sizeHint()));
        animat->setEndValue(QRect(parentWidget()->x(), parentWidget()->y() + parentWidget()->height(), parentWidget()->sizeHint().width(), 0));
        animat->setDuration(150);
        animat->start(QAbstractAnimation::DeleteWhenStopped);
        QObject::connect(animat, &QPropertyAnimation::finished, [=]{
            emit focusLost();
        });
    }

    QListWidget::focusOutEvent(event);
}

/******************* class ComboBox *******************/
ComboBox::ComboBox(QWidget *parent) : QLabel(parent)
{
    setStyleSheet("QLabel { border: 1px solid #009fc7; background-color: #009fc7;}");
    createMenu();
}

void ComboBox::addItem(const QString item){
    m_view->addItem(item);

    QRect visItemRect = m_view->visualItemRect(m_view->item(m_view->model()->rowCount() - 1));
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;
    m_view->resize(visItemRect.width() + scaleFactor * 5, m_view->height() + visItemRect.height());

    menuWind->resize(menuWind->sizeHint());
    selectItem(m_view->model()->rowCount() - 1);
}

bool ComboBox::isPopupShown(){
    return menuWind->isVisible();
}

void ComboBox::selectItem(int row){
    if(row > m_view->model()->rowCount() - 1)
        return;

    m_view->setCurrentRow(row);
    setText(QFontMetrics(font()).elidedText(m_view->item(row)->data(Qt::DisplayRole).toString(), Qt::ElideRight, width() - button->width()));
}

void ComboBox::createMenu(){
    menuWind = new Window(this);

    m_view = new ListView(menuWind);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QObject::connect(m_view, &ListView::itemClicked, this, &ComboBox::handleItemClick);
    QObject::connect(m_view, &ListView::focusLost, this, &ComboBox::hidePopupWindow);

    button = new QPushButton(this);
    button->setStyleSheet("QPushButton {background: transparent; border: none}");
    button->setIcon(QIcon(":/photos/arrow-down.png"));
    button->setAttribute(Qt::WA_TransparentForMouseEvents);
}

QPoint ComboBox::getRightPopupPos(){
    QPoint parPos = mapToGlobal(rect().bottomLeft());
    QSize sSize = QApplication::primaryScreen()->size();
    RECT taskbarRect = GetTaskbarPos();
    TaskBarPos taskBPos;

    if(!IsRectEmpty(&taskbarRect)){
        if(taskbarRect.left == 0 && taskbarRect.top == 0){
            if(taskbarRect.right > taskbarRect.bottom)
                taskBPos = Top;
            else
                taskBPos = Left;
        }
        else if(taskbarRect.left == 0)
            taskBPos = Bottom;

        else
            taskBPos = Right;
    }
    else
        taskBPos = None;

    if(parPos.x() < taskbarRect.right && taskBPos == Left){
        if(parPos.y() + m_view->height() > sSize.height())
            return QPoint(taskbarRect.right, mapToGlobal(rect().topLeft()).y() - m_view->height());
        else
            return QPoint(taskbarRect.right, parPos.y());
    }
    else if(parPos.x() + m_view->width() > taskbarRect.left && taskBPos == Right){
        if(mapToGlobal(rect().bottomRight()).y()  + m_view->height() > sSize.height())
            return QPoint(taskbarRect.left - m_view->width(), mapToGlobal(rect().topLeft()).y() - m_view->height());
        else
            return QPoint(taskbarRect.left - m_view->width(), parPos.y());
    }
    else if(parPos.x() < 0){
        if((parPos.y() + m_view->height() > taskbarRect.top && taskBPos == Bottom) || parPos.y() + m_view->height() > sSize.height())
            return QPoint(0, mapToGlobal(rect().topLeft()).y() - m_view->height());
        else
            return QPoint(0, parPos.y());
    }
    else if(parPos.x() + m_view->width() > sSize.width()){
        if((parPos.y() + m_view->height() > taskbarRect.top && taskBPos == Bottom) || parPos.y() + m_view->height() > sSize.height())
            return QPoint(sSize.width() - m_view->width(), mapToGlobal(rect().topLeft()).y() - m_view->height());
        else
            return QPoint(sSize.width() - m_view->width(), parPos.y());
    }
    else if((parPos.y() + m_view->height() > taskbarRect.top && taskBPos == Bottom) || parPos.y() + m_view->height() > sSize.height())
        return QPoint(parPos.x(), mapToGlobal(rect().topLeft()).y() - m_view->height());
    else
        return parPos;
}

void ComboBox::showPopupWindow(){
    QPoint menuPos = getRightPopupPos();
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;
    if(menuPos.y() == mapToGlobal(rect().bottomLeft()).y()){
        animDir = Down;
        m_view->setStyleSheet(QString("QListWidget {border: 1px solid black; border-bottom-left-radius: %1px; border-bottom-right-radius: %2px; background-color: #009fc7;} QListView::item:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e8b37d, stop: 1 #db8c3b);} QListView::item:hover {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FAFBFE, stop: 1 #DCDEF1); color: black;}").arg(scaleFactor * 10).arg(scaleFactor * 10));
    }
    else{
        animDir = Up;
        m_view->setStyleSheet(QString("QListWidget {border: 1px solid black; border-top-left-radius: %1px; border-top-right-radius: %2px; background-color: #009fc7;} QListView::item:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e8b37d, stop: 1 #db8c3b);} QListView::item:hover {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FAFBFE, stop: 1 #DCDEF1); color: black;}").arg(scaleFactor * 10).arg(scaleFactor * 10));
    }

    m_view->setAnimationDirection(animDir);
    menuWind->setAnimationDirection(animDir);

    menuWind->move(menuPos);
    button->setIcon(QIcon(":/photos/arrow-up.png"));
    menuWind->show();
}

void ComboBox::hidePopupWindow(){
    setStyleSheet("QLabel { border: 1px solid #009fc7; background-color: #009fc7;}");
    button->setIcon(QIcon(":/photos/arrow-down.png"));
    menuWind->hide();
}

void ComboBox::valueReady(QVariant value){
    if(value.toRect().height() == 0)
        hidePopupWindow();
}

void ComboBox::handleItemClick(QListWidgetItem *item){
    QString itemText = item->data(Qt::DisplayRole).toString();
    setText(fontMetrics().elidedText(itemText, Qt::ElideRight, width() - button->width()));

    if(animDir == Down){
        QPropertyAnimation *animat = new QPropertyAnimation(menuWind, "size");
        animat->setStartValue(menuWind->sizeHint());
        animat->setEndValue(QSize(menuWind->sizeHint().width(), 0));
        animat->setDuration(150);
        animat->start(QAbstractAnimation::DeleteWhenStopped);
        QObject::connect(animat, &QPropertyAnimation::finished, [=]{
            hidePopupWindow();
        });
    }
    else{
        QPropertyAnimation *animat = new QPropertyAnimation(menuWind, "geometry");
        animat->setStartValue(QRect(menuWind->pos(), menuWind->sizeHint()));
        animat->setEndValue(QRect(mapToGlobal(rect().topLeft()), QSize(menuWind->sizeHint().width(), 0)));
        animat->setDuration(150);
        animat->start(QAbstractAnimation::DeleteWhenStopped);
        QObject::connect(animat, &QPropertyAnimation::finished, [=]{
            hidePopupWindow();
        });
    }

    emit itemSelected(m_view->row(item));
}

void ComboBox::mousePressEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        if(isPopupShown()){
            QPropertyAnimation *animat = new QPropertyAnimation(menuWind, "size");
            animat->setStartValue(menuWind->sizeHint());
            animat->setEndValue(QSize(menuWind->sizeHint().width(), 0));
            animat->setDuration(150);
            animat->start(QAbstractAnimation::DeleteWhenStopped);
            QObject::connect(animat, &QPropertyAnimation::finished, [=]{
                hidePopupWindow();
            });
        }
        else
            showPopupWindow();
    }

    QLabel::mousePressEvent(e);
}

void ComboBox::enterEvent(QEvent *event){
    setCursor(QCursor(Qt::PointingHandCursor));
    QWidget::enterEvent(event);
}

void ComboBox::resizeEvent(QResizeEvent *event){
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    m_view->resize(0, 5);
    m_view->setFont(font());
    button->setGeometry(event->size().width() - scaleFactor * 30, 0, scaleFactor * 30, event->size().height());
    button->setIconSize(button->size());

    int items = m_view->model()->rowCount();
    for(int i = 0; i < items; i++){
        QListWidgetItem *item = m_view->item(i);
        item->setFont(font());
        QRect visItemRect = m_view->visualItemRect(item);
        m_view->resize(visItemRect.width() + scaleFactor * 5, m_view->height() + visItemRect.height());
    }

    setText(QFontMetrics(font()).elidedText(m_view->currentItem()->data(Qt::DisplayRole).toString(), Qt::ElideRight, event->size().width() - button->width()));
    m_view->setMinimumWidth(event->size().width());
    menuWind->resize(menuWind->sizeHint()); 
}

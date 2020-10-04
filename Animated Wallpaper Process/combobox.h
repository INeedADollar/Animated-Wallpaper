#ifndef COMBOBOX_H
#define COMBOBOX_H

#include "utility.h"

#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>

/******************* class Window *******************/
class Window : public QWidget{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);
    QSize sizeHint() const override;
    void setAnimationDirection(AnimationDirection);

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    AnimationDirection animDir;
};

/******************* class ListView *******************/
class ListView : public QListWidget{
    Q_OBJECT

public:
    explicit ListView(QWidget *parent = nullptr);
    void setAnimationDirection(AnimationDirection);

protected:
    void focusOutEvent(QFocusEvent *event) override;

private:
    AnimationDirection animDir;

signals:
    void focusLost();
};

/******************* class ComboBox *******************/
class ComboBox : public QLabel{
    Q_OBJECT

public:
    explicit ComboBox(QWidget *parent = nullptr);
    void addItem(const QString);
    bool isPopupShown();
    void selectItem(int);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void createMenu();
    void menuRequested(QPoint);
    void showPopupWindow();
    void hidePopupWindow();
    QPoint getRightPopupPos();

    Window *menuWind;
    ListView *m_view;
    QPushButton *button;
    AnimationDirection animDir;

private slots:
    void handleItemClick(QListWidgetItem *);
    void valueReady(QVariant);

signals:
    void itemSelected(int);
};

#endif // COMBOBOX_H

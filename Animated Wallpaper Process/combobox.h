/*******************************************************************************
MIT License
Copyright (c) 2020 INeedADollar
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 
*******************************************************************************/

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

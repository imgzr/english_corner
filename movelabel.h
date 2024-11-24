#ifndef MOVELABEL_H
#define MOVELABEL_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QString>
#include <widget.h>
#include <windows.h>

class MoveLabel : public QLabel
{
    Q_OBJECT
public:
    MoveLabel(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
private:
    bool m_isDragging = false;
    QPoint m_dragStartPosition;
    HWND hWnd;
};

#endif // MOVELABEL_H

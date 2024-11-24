#include "movelabel.h"
#include <widget.h>


MoveLabel::MoveLabel(QWidget *parent)
    : QLabel(parent) {
    //setWindowFlags(Qt::FramelessWindowHint);
    //setAttribute(Qt::WA_TranslucentBackground);
    hWnd = (HWND)parent -> winId();
    qDebug() << "[setWindowBaseConfig]窗口句柄(hWnd)的值为: " << hWnd;
}

void MoveLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = true;
        m_dragStartPosition = event->pos();
    }
}

void MoveLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
    }
}

void MoveLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging)
    {
        QPoint globalPos = this->mapToGlobal(event->pos());
        QPoint diff = globalPos - m_dragStartPosition;
        SetWindowPos(this->hWnd, NULL, diff.x(), diff.y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}



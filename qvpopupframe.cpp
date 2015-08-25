/* QVisu (c) 2015 Maximilian Gauger mgauger@kalassi.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QFrame>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include "qvpopupframe.h"

QVPopupFrame::QVPopupFrame(QWidget *parent) :
    QFrame(parent)
{
    inner_frame = new QFrame(this);
    move(0,0);
    setFixedSize(QApplication::desktop()->screenGeometry().width(),QApplication::desktop()->screenGeometry().height());
    hide();
}

void QVPopupFrame::place(int x, int y, int w, int h) {
    inner_frame->move(x,y);
    inner_frame->setFixedSize(w,h);
}

void QVPopupFrame::show() {
    raise();
    QWidget::show();
    inner_frame->show();
}

QFrame* QVPopupFrame::content() {
    return inner_frame;
}

void QVPopupFrame::mousePressEvent(QMouseEvent *e) {
    if ((e->x() > inner_frame->x()) && (e->y() > inner_frame->y()) && (e->x() < inner_frame->x()+inner_frame->width()) && (e->y() < inner_frame->y()+inner_frame->height())) {
        /* Inside inner frame, reject */
        emit clicked(e->x() - inner_frame->x(),e->y() - inner_frame->y());
        return;
    }
    /* Outside click, hide popup */
    hide();
}

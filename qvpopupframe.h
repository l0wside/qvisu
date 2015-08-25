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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QVPOPUPFRAME_H
#define QVPOPUPFRAME_H

#include <QLabel>
#include <QFrame>
#include <QGridLayout>

class QVPopupFrame : public QFrame
{
    Q_OBJECT
public:
    explicit QVPopupFrame(QWidget *parent = 0);
    QFrame *content();

protected:
    void mousePressEvent(QMouseEvent *);

signals:
    void clicked(int,int);

public slots:
    void show();
    void place(int x, int y, int w, int h);

private:
    QFrame *inner_frame;
};

#endif // QVPOPUPFRAME_H

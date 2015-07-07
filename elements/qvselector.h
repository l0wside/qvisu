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

#ifndef QVSELECTOR_H
#define QVSELECTOR_H

#include <QLabel>
#include "../qvelement.h"
#include "../qviconwidget.h"
#include "../qvsvgwidget.h"

class display_value_t {
public:
    QString item;
    QString label;
    QString unit;
    int precision;
    double value;
    QWidget *widget;
};

class QVSelector : public QVElement
{
    Q_OBJECT
public:
    explicit QVSelector(QDomElement xml_desc,QString container,QWidget *parent = 0);

public slots:
    void onContainerChanged(QString);
    void onValueChanged(QString,QString);

protected:
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);

signals:
    void containerChanged(QString);

protected slots:
    void onSvgPressed();

private:
    QByteArray svg_icon;
    QVIconWidget *w_icon;
    QLabel *w_text;
    QList<display_value_t*> disp_contents;

    QString target;

};

#endif // QVSELECTOR_H

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

#ifndef QVHEATING_H
#define QVHEATING_H

#include <QLabel>
#include "../qvsvgwidget.h"
#include "../qviconwidget.h"
#include "../qvelement.h"

class heating_value_t {
public:
    QString item;
    QString label;
    QString unit;
    int precision;
    double value;
    QWidget *widget;
};

class QVHeating : public QVElement
{
    Q_OBJECT
public:
    explicit QVHeating(QDomElement xml_desc, QString container, QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent * event);

signals:

public slots:
    void onValueChanged(QString,QString);

private slots:
    void svgPressed();

private:
    QByteArray svg_icon;
    QByteArray svg_sw_icon;
    QVIconWidget *w_icon;
    QVSvgWidget *w_sw_icon;
    QLabel *w_title;
    QLabel *w_temp_value;
    QLabel *w_temp_setpoint;
    QVSvgWidget *w_temp_setpoint_minus, *w_temp_setpoint_plus;

    double temp_value, temp_setpoint, temp_setpoint_step;
    QList<heating_value_t*> sw_contents;
};

#endif // QVHEATING_H

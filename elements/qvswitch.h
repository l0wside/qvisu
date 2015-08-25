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

#ifndef QVSWITCH_H
#define QVSWITCH_H

#include <QByteArray>
#include <QLabel>
#include "../qvsvgwidget.h"
#include "../qviconwidget.h"
#include "../qvswitchicon.h"
#include <QGridLayout>
#include "../qvelement.h"

class QVSwitch : public QVElement
{
    Q_OBJECT
public:
    explicit QVSwitch(QDomElement xml_desc, QString container, QString type="switch", QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *);

signals:

public slots:
    void onValueChanged(QString,QString);

private slots:
    void svgPressed();

private:
    QByteArray svg_icon;
    QByteArray svg_sw_icon, svg_sw_icon_on;
    QVIconWidget *w_icon;
    QVSwitchIcon *w_sw_icon;
    QLabel *w_text;

    bool value;
    bool trigger_value;
    enum {
        type_switch,
        type_status,
        type_trigger,
        type_confirm
    } type;
};

#endif // QVSWITCH_H

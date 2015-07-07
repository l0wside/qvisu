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

#ifndef QVSHUTTER_H
#define QVSHUTTER_H

#include <QLabel>
#include <QHBoxLayout>
#include "../qvelement.h"
#include "../qvsvgwidget.h"
#include "../qvpopupframe.h"
#include "../qviconwidget.h"

class QVShutter : public QVElement
{
    Q_OBJECT
public:
    explicit QVShutter(QDomElement xml_desc, QString container, QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent * event);
    void mousePressEvent(QMouseEvent *);

signals:

public slots:
    void onValueChanged(QString,QString);

private slots:
    void onBtnLeft(double,double);
    void onBtnRight(double,double);
    void onShutterClicked();
    void onPopupShutterClicked(double,double);
    void onBladeClicked(double xrel, double yrel);
    void onBladeDragged(double xrel, double yrel);
    void onBladeReleased(double xrel, double yrel);


private:
    QByteArray svg_icon;
    QVIconWidget *w_icon;
    QLabel *w_text;
    QByteArray shutter_icon;
    QVSvgWidget *w_shutter_icon;
    QByteArray blade_icon;
    QVSvgWidget *w_blade_icon;
    QVSvgWidget *w_buttons_l, *w_buttons_r;
    double position, position2;
    double max_position1, max_position2;
    double max_blade_position;

    bool has_blades;
    bool two_halves;

    QVPopupFrame *popup_frame;
    QLabel *popup_title;
    QVSvgWidget *popup_shutter_icon;
    QVSvgWidget *popup_blade_icon;

    double max_arrow_width() { return 0.3; }
    double max_shutter_width() { return 0.5; }

    double drag_start;
    double blade_value;
    bool was_dragged;

    static const int padding = 5;
};

#endif // QVSHUTTER_H

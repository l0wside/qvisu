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
 *
 * Alle Icons sind unter einer Creative Commons Lizenz vom Typ Namensnennung -
 * Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugänglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-sa/3.0/de/ oder wenden Sie
 * sich brieflich an
 * Creative Commons, Postfach 1866, Mountain View, California, 94042, USA.
 */

#ifndef QVDIMMER_H
#define QVDIMMER_H

#include <QByteArray>
#include <QLabel>
#include <QGridLayout>
#include "../qvsvgwidget.h"
#include "../qvswitchicon.h"
#include "../qvelement.h"

/** Implements the dimmer element, providing a slider and a switch */
class QVDimmer : public QVElement {
    Q_OBJECT
public:
    explicit QVDimmer(QDomElement xml_desc, QString container, QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent * event);
    /** Clicks outside any widget are interpreted as on/off for the switch item */
    void mousePressEvent(QMouseEvent *);

signals:

public slots:
    void onValueChanged(QString,QString);

private slots:
    /** Used to handle taps/clicks on the switch icon */
    void svgPressed();
    /** Handle a simple click/tap on the dimmer widget. xrel and yrel are the relative position on the SVG widget */
    void dimmerClicked(double xrel, double yrel);
    /** Handle dragging of the dimmer widget. xrel and yrel are the relative position on the SVG widget */
    void dimmerDragged(double xrel, double yrel);

private:
    void updateDimmerWidget(double value);

    QByteArray svg_icon;
    QByteArray svg_dimmer;
    QVIconWidget *w_icon;
    QVSwitchIcon *w_sw_icon;
    QVSvgWidget *w_dimmer;
    QLabel *w_text;

    bool switch_value;
    double dim_value;

    double display_ratio;
    enum {
        type_int,
        type_float
    } data_type;
    double max_value;

    double max_dimmer_height() { return 0.3; }
};

#endif // QVDIMMER_H

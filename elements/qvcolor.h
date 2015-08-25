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

#ifndef QVCOLOR_H
#define QVCOLOR_H

#include <QLabel>
#include <QHBoxLayout>
#include <QSettings>
#include <QElapsedTimer>
#include "../qvelement.h"
#include "../qvsvgwidget.h"
#include "../qvpopupframe.h"
#include "../qviconwidget.h"
#include "../qvswitchicon.h"

typedef struct {
    int r, g, b, w;
} rgbw_t;

typedef struct {
    double h,s,i;
} hsi_t;


class QVColor : public QVElement
{
    Q_OBJECT
public:
    explicit QVColor(QDomElement xml_desc, QString container, QWidget *parent = 0);

    QString hsiToString(hsi_t);
    bool hsiFromString(QString color, hsi_t&);
    void rgbToHsi(rgbw_t, hsi_t&);
    void rgbwToHsi(rgbw_t, hsi_t&);
    void hsiToRgbw(hsi_t hsi, rgbw_t &rgbw);
    void hsiToRgb_int(hsi_t, rgbw_t &rgbw);
    void hsiToRgb(hsi_t, rgbw_t &rgbw);

protected:
    void resizeEvent(QResizeEvent * event);

signals:

public slots:
    void onValueChanged(QString,QString);

private slots:
    void onSwitchClicked();
    void onRGBClicked();
    void onHistoryClicked();
    void onHistoryReleased(double,double);
    void onHistoryTimer();
    /** Handle a simple click/tap on the dimmer widget. xrel and yrel are the relative position on the SVG widget */
    void dimmerClicked(double xrel, double yrel);
    /** Handle dragging of the dimmer widget. xrel and yrel are the relative position on the SVG widget */
    void dimmerDragged(double xrel, double yrel);
    void onPopupClicked(int x, int y);
    void onCircleClicked(double x, double y);
    void sendModified();

private:
    QVIconWidget *w_icon, *w_rgb_icon;
    QVSwitchIcon *w_switch_icon;
    QList<QVIconWidget*> history_icons;
    QStringList history_colors;
    QLabel *w_text;

    bool has_white;
    int n_history;
    QTimer history_timer;

    QVPopupFrame *popup_frame;
    QLabel *popup_title;
    QLabel *popup_colorlabel;
    QImage *popup_image;
    QVSvgWidget *popup_circle;

    QSettings *registry_settings;
    QString registry_key;

    bool switch_status;
    double drag_start;
    bool was_dragged;

    static const int padding = 5;

    void updatePopup();

    QByteArray svg_dimmer;
    QVSvgWidget *w_dimmer;

    enum {
        type_int,
        type_float
    } data_type;

    rgbw_t rgbw;
    hsi_t hsi;
    int current_history_index;

    double max_dimmer_height() { return 0.3; }

};

#endif // QVCOLOR_H

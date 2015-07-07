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

#ifndef QVPLOT_H
#define QVPLOT_H

#include <QLabel>
#include "../qvelement.h"
#include "../qvsvgwidget.h"
#include "../qviconwidget.h"

class series_t {
public:
    QString name, type;
    int axis;
    QString color;
    QMap<double,double> data;
};

class QVPlot : public QVElement
{
    Q_OBJECT
public:
    explicit QVPlot(QDomElement xml_desc, QString container, QWidget *parent = 0);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);

public slots:
    void onInitCompleted();
    void onSeriesReceived(QString,QMap<double,double>);

private:
    void redrawSeries();
    void addPlot(QDomElement xml_elem, int axis);

    QMap<QString,series_t*> series;
    QVSvgWidget *plot_widget;
    QLabel *w_title;
    QVIconWidget *w_icon;

    QString series_start;
    double axis1_min, axis1_max, axis2_min, axis2_max;
    bool has_axis1, has_axis2;
    bool grid1, grid2, xgrid;
    bool common_zero;

    static const int label_size = 12;
    static const int padding = 6;
    QString background_color;
};

#endif // QVPLOT_H

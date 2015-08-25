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

#ifndef QVICONWIDGET_H
#define QVICONWIDGET_H

#include <QSvgWidget>
#include <QLabel>
#include <QByteArray>
#include <QtXml>
#include "qvsvgwidget.h"

class QVIconWidget : public QFrame
{
    Q_OBJECT
public:
    explicit QVIconWidget(QWidget *parent = 0);
    explicit QVIconWidget(QString file, QWidget *parent = 0);

    QSize defaultSize() const;
    QSize sizeHint() const;
    float aspectRatio() const;

    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);

signals:
    void clicked(double xrel, double yrel);
    void dragged(double xrel, double yrel);
    void released(double xrel, double yrel);

public slots:
    void load(const QByteArray&);
    void setPathStyle(QString property, QString value, QString id = "", bool force_insert = false);
    void preserveAspectRatio(bool);

private slots:
    void onSvgClicked(double,double);
    void onSvgDragged(double,double);
    void onSvgReleased(double,double);

private:
    QVSvgWidget *svg_widget;
    QLabel *pix_widget;
    QPixmap *pixmap;

    bool preserve_aspect_ratio;
};

#endif // QVICONWIDGET_H

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

#ifndef QVSVGWIDGET_H
#define QVSVGWIDGET_H

#include <QSvgWidget>
#include <QByteArray>
#include <QtXml>

class QVSvgWidget : public QSvgWidget
{
    Q_OBJECT
public:
    explicit QVSvgWidget(QWidget *parent = 0);
    explicit QVSvgWidget(QString file, QWidget *parent = 0);

    QSize defaultSize() const;
    float aspectRatio() const;

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void resizeEvent(QResizeEvent*);
	void mouseReleaseEvent(QMouseEvent*);

signals:
    void clicked(double xrel, double yrel);
    void dragged(double xrel, double yrel);
    void released(double xrel, double yrel);

public slots:
    void load(const QByteArray&);
    void setPathStyle(QString property, QString value, QString id = "", bool force_insert = false);
    void preserveAspectRatio(bool);

private:
    QByteArray svg_data;
    void updateSubPath(QDomElement &elem, QString property, QString value, QString id, bool force_insert);
    QTime drag_timer;

    bool preserve_aspect_ratio;
};

#endif // QVSVGWIDGET_H

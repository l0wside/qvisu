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

#include <QString>
#include <QtXml>
#include <QSvgRenderer>
#include <QMouseEvent>
#include "qvsvgwidget.h"

QVSvgWidget::QVSvgWidget(QWidget *parent) :
    QSvgWidget(parent)
{
    drag_timer.start();
    preserve_aspect_ratio = false;
}

QVSvgWidget::QVSvgWidget(QString file, QWidget *parent) :
    QSvgWidget(parent)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "file not found";
        return;
    }
    load(f.readAll());
    f.close();
}

void QVSvgWidget::preserveAspectRatio(bool preserve) {
    preserve_aspect_ratio = preserve;
}

QSize QVSvgWidget::defaultSize() const {
    if (svg_data.isEmpty()) {
        return QSize(0,0);
    }
    return renderer()->defaultSize();
}

float QVSvgWidget::aspectRatio() const {
    QSize size = defaultSize();
    if ((size.width() == 0) || (size.height() == 0)) {
        return 1.0;
    }
    return (float)(size.height())/(float)(size.width());
}

void QVSvgWidget::mousePressEvent(QMouseEvent *e) {
    double xrel = (double)(e->x())/width();
    double yrel = (double)(e->y())/height();
    emit clicked(xrel,yrel);
}

void QVSvgWidget::mouseMoveEvent(QMouseEvent *e) {
    if (drag_timer.elapsed() < 100) {
        return;
    }
    drag_timer.start();
    double xrel = (double)(e->x())/width();
    double yrel = (double)(e->y())/height();
    emit dragged(xrel,yrel);
}

void QVSvgWidget::mouseReleaseEvent(QMouseEvent *e) {
    double xrel = (double)(e->x())/width();
    double yrel = (double)(e->y())/height();
    emit released(xrel,yrel);
}

void QVSvgWidget::resizeEvent(QResizeEvent *e) {
    if (!preserve_aspect_ratio) {
        e->ignore();
        return;
    }
    e->accept();
    double resize_aspect_ratio = ((float)height())/((float)width());
    if (resize_aspect_ratio > aspectRatio()) {
        setFixedHeight((int)(width()*aspectRatio()));
    } else {
        setFixedWidth((int)(height()/aspectRatio()));
    }
}

void QVSvgWidget::load(const QByteArray& data) {
    svg_data = data;

    QSvgWidget::load(data);
}

void QVSvgWidget::updateSubPath(QDomElement &elem, QString property, QString value, QString id, bool force_insert) {
    for (QDomElement sub_elem = elem.firstChildElement(); !sub_elem.isNull(); sub_elem = sub_elem.nextSiblingElement()) {
        updateSubPath(sub_elem,property,value,id, force_insert);
    }

    for (QDomElement path_elem = elem.firstChildElement(); !path_elem.isNull(); path_elem = path_elem.nextSiblingElement()) {
        if (!id.isEmpty() && (!path_elem.hasAttribute("id") || (path_elem.attribute("id") != id))) {
            continue;
        }
        if (!path_elem.hasAttribute("style")) {
            continue;
        }
        QString style = path_elem.attribute("style");
        QStringList style_parts = style.split(";");
        QMap<QString,QString> style_map;
        for (QStringList::iterator iter = style_parts.begin(); iter != style_parts.end(); iter++) {
            QStringList splitter = iter->split(":");
            if (splitter.count() < 2) {
                continue;
            }
            style_map.insert(splitter.at(0),splitter.at(1));
        }

        if (style_map.contains(property)) {
            style_map[property] = value;
        } else if (force_insert) {
            style_map.insert(property,value);
        }

        style.clear();
        for (QMap<QString,QString>::iterator iter = style_map.begin(); iter != style_map.end(); iter++) {
            style.append(iter.key());
            style.append(":");
            style.append(iter.value());
            if (iter != style_map.end()) {
                style.append(";");
            }
        }
        path_elem.setAttribute("style",style);
    }


}

void QVSvgWidget::setPathStyle(QString property, QString value, QString id, bool force_insert) {
    if (svg_data.isEmpty()) {
        return;
    }

    QString str_data(svg_data);
    QDomDocument xml_doc;
    xml_doc.setContent(str_data);

    QDomElement xml_svg = xml_doc.documentElement();
    if (xml_svg.isNull()) {
        return;
    }

    updateSubPath(xml_svg,property,value,id,force_insert);

    svg_data = xml_doc.toByteArray();
    QSvgWidget::load(svg_data);

}

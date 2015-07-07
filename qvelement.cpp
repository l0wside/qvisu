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
#include <QColor>
#include "qvelement.h"
#include "elements/qvselector.h"
#include "elements/qvswitch.h"
#include "elements/qvdimmer.h"
#include "elements/qvshutter.h"
#include "elements/qvplot.h"
#include "elements/qvheating.h"
#include "elements/qvfritz.h"
#include "elements/qvweather.h"
#include "elements/qvgooglecalendar.h"
#include "elements/qvvideo.h"

/** Get position of element and container. All other info is extracted in the derived classes */

QVElement::QVElement(QDomElement xml_desc,QString container,QWidget *parent) : QFrame(parent) {
    bool ok;

    /* Determine position and size.
       If an error occurs, container is set to empty. This leads to the element
       being created, but never shown.
    */
    this->container.clear();
    x = y = w = h = -1;
    if (!xml_desc.hasAttribute("position")) {
        return;
    }
    QString pos_string = xml_desc.attribute("position");
    QStringList pos_string_elems = pos_string.split(",");
    if (pos_string_elems.count() != 2) {
        return;
    }
    x = pos_string_elems.first().toInt(&ok);
    if (!ok) {
        x = -1;
        return;
    }
    pos_string_elems.pop_front();
    y = pos_string_elems.first().toInt(&ok);
    if (!ok) {
        x = y = -1;
        return;
    }

    if (xml_desc.hasAttribute("width")) {
        w = xml_desc.attribute("width").toInt(&ok);
        if (!ok) {
            w = -1;
        }
    }

    if (xml_desc.hasAttribute("height")) {
        h = xml_desc.attribute("height").toInt(&ok);
        if (!ok) {
            h = -1;
        }
    }

    /* Find color and active color, if defined */
    QDomElement c_elem = xml_desc.firstChildElement("color");
    if (!c_elem.isNull()) {
        QString c_font;
        QColor c = QColor(colorString(c_elem.text()));
        if (c.isValid()) {
            if (c.blueF() + c.greenF() + c.redF() < 1.5) {
                c_font = QStringLiteral("\nQLabel { color:#ffffff }");
            } else {
                c_font = QStringLiteral("\nQLabel { color:#000000 }");
            }
        }

        color = QStringLiteral("QFrame { background-color:") + colorString(c_elem.text()) + QStringLiteral(" }");
        color += c_font;
        setStyleSheet(color);
    }
    c_elem = xml_desc.firstChildElement("active-color");
    if (!c_elem.isNull()) {
        QString c_font;
        QColor c = QColor(colorString(c_elem.text()));
        if (c.isValid()) {
            if (c.blueF() + c.greenF() + c.redF() < 1.5) {
                c_font = QStringLiteral("\nQLabel { color:#ffffff }");
            } else {
                c_font = QStringLiteral("\nQLabel { color:#000000 }");
            }
        }
        active_color = QStringLiteral("QFrame { background-color:") + colorString(c_elem.text()) + QStringLiteral(" }");
        active_color += c_font;
    } else {
        active_color = color;
    }
    QColor bg_color = this->palette().color(QWidget::backgroundRole());
    if (bg_color.redF() + bg_color.greenF() + bg_color.blueF() < 1.5) {

    }

    /* Iterate over items and add to item list*/
    QDomNodeList xml_items = xml_desc.elementsByTagName("item");
    for (int n=0; n < xml_items.count(); n++) {
        if (xml_items.at(n).isElement()) {
            items.insert(xml_items.at(n).toElement().text(),QVItem(xml_items.at(n).toElement(),n+1));
        }
    }

    this->container = container;
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    setObjectName("element");
}

QList<QString> QVElement::getItemList() {
    return items.keys();
}

QString QVElement::getItemByAction(QString action, int index) {
    for (QMap<QString,QVItem>::iterator iter = items.begin(); iter != items.end(); iter++) {
        if ((iter->getAction() == action) && ((index == 0) || (iter->getIndex() == index))) {
            return iter.key();
        }
    }
    return QString();
}

QList<int> QVElement::getGeometry() {
    QList<int> ret;

    ret.append(x);
    ret.append(y);
    ret.append(w);
    ret.append(h);
    return ret;
}

QString QVElement::findFilePath(QString name) {
    QFile f(name);
    if (f.exists()) {
        return name;
    }

    QStringList paths = qApp->property("paths").toStringList();
    if (paths.count() == 0) {
        return QString();
    }
    for (QStringList::iterator iter = paths.begin(); iter != paths.end(); iter++) {
        QString fname = iter->append(QDir::separator()).append(name);
        QFile f(fname);
        if (f.exists()) {
            return fname;
        }
    }
    return QString();
}

void QVElement::resizeEvent(QResizeEvent*) { }
void QVElement::onValueChanged(QString,QString) { }
void QVElement::onSeriesReceived(QString,QMap<double,double>) { }
void QVElement::onInitCompleted() { }

void QVElement::onContainerChanged(QString container) {
    if ((container == this->container) || (this->container == "permanent")) {
        show();
    } else {
        hide();
    }
}

int QVElement::width() {
    return contentsRect().width();
}

int QVElement::height() {
    return contentsRect().height();
}

int QVElement::ofs_x() {
    return contentsRect().x();
}

int QVElement::ofs_y() {
    return contentsRect().y();
}

/** Returns the color code for a Windows color name.
 * If the name is not found, it will return the input string (as lowercase). */
QString QVElement::colorString(QString color) {
    color = color.toLower();
    if (color == "amber") {
        return QStringLiteral("#F0A30A");
    }
    if (color == "brown") {
        return QStringLiteral("#825A2C");
    }
    if (color == "cobalt") {
        return QStringLiteral("#0050EF");
    }
    if (color == "crimson") {
        return QStringLiteral("#D80073");
    }
    if (color == "cyan") {
        return QStringLiteral("#1BA1E2");
    }
    if (color == "magenta") {
        return QStringLiteral("#D80073");
    }
    if (color == "lime") {
        return QStringLiteral("#A4C400");
    }
    if (color == "indigo") {
        return QStringLiteral("#6A00FF");
    }
    if (color == "green") {
        return QStringLiteral("#60A916");
    }
    if (color == "emerald") {
        return QStringLiteral("#008A00");
    }
    if (color == "mauve") {
        return QStringLiteral("#76608A");
    }
    if (color == "olive") {
        return QStringLiteral("#6D8764");
    }
    if (color == "orange") {
        return QStringLiteral("#FA8600");
    }
    if (color == "pink") {
        return QStringLiteral("#F472D0");
    }
    if (color == "red") {
        return QStringLiteral("#E51400");
    }
    if (color == "sienna") {
        return QStringLiteral("#7A3B3F");
    }
    if (color == "steel") {
        return QStringLiteral("#647687");
    }
    if (color == "teal") {
        return QStringLiteral("#00ABA9");
    }
    if (color == "violet") {
        return QStringLiteral("#AA00FF");
    }
    if (color == "yellow") {
        return QStringLiteral("#D8C100");
    }
    return color;
}


/** Generate element depending on XML configuration.
 * When implementing new element types, add them here after creating the class.
 */
QVElement *createQVElement(QDomElement xml_desc,QString container,QWidget *parent) {
    QString type = xml_desc.attribute("type");
    if (type.isNull() || (type.length() == 0)) {
        return NULL;
    }
    QVElement *element = NULL;

    /* List of element types */
    if (type == "selector") {
        element = new QVSelector(xml_desc,container,parent);
    } else if (type == "switch") {
        element = new QVSwitch(xml_desc,container,false,parent);
    } else if (type == "status") {
        element = new QVSwitch(xml_desc,container,true,parent);
    } else if (type == "dimmer") {
        element = new QVDimmer(xml_desc,container,parent);
    } else if (type == "shutter") {
        element = new QVShutter(xml_desc,container,parent);
    } else if (type == "plot") {
        element = new QVPlot(xml_desc,container,parent);
    } else if (type == "heating") {
        element = new QVHeating(xml_desc,container,parent);
    } else if (type == "fritzbox") {
        element = new QVFritz(xml_desc,container,parent);
    } else if (type == "weather") {
        element = new QVWeather(xml_desc,container,parent);
    } else if (type == "calendar") {
        element = new QVGoogleCalendar(xml_desc,container,parent);
#ifdef VIDEO
    } else if (type == "video") {
        element = new QVVideo(xml_desc,container,parent);
#endif
    }
    if (element == NULL) {
        return NULL;
    }
    if ((element->getGeometry().at(2) == 0) || (element->getGeometry().at(3) == 0)) {
        delete element;
        return NULL;
    }

    return element;
}


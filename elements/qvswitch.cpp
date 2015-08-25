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

#include "qvswitch.h"
#include <QLabel>

QVSwitch::QVSwitch(QDomElement xml_desc, QString container, QString type, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    QDomElement xml_elem;

    w = h = 2;
    if (xml_desc.hasAttribute("mini") && ((xml_desc.attribute("mini") == "1") || (xml_desc.attribute("mini").toLower() == "true"))) {
        w = h = 1;
    }

    w_icon = 0;
    xml_elem = xml_desc.firstChildElement("icon");
    if (!xml_elem.isNull()) {
        QFile file_icon(findFilePath(xml_elem.text()));
        if (file_icon.open(QIODevice::ReadOnly)) {
            svg_icon = file_icon.readAll();
            file_icon.close();
        }
        if (!svg_icon.isEmpty()) {
            w_icon = new QVIconWidget (this);
            w_icon->load(svg_icon);
            QObject::connect(w_icon,SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));
        }
    }

    w_sw_icon = new QVSwitchIcon(xml_desc.firstChildElement("sw-icon"),this);
    QObject::connect(w_sw_icon,SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));

    xml_elem = xml_desc.firstChildElement("text");
    w_text = new QLabel(xml_elem.text(),this);
    w_text->setAlignment(Qt::AlignCenter);
    if ((w == 1) && (h == 1)) {
        w_text->setObjectName("mini");
    }

    trigger_value = false;
    if (xml_desc.hasAttribute("value")) {
        QString s_value = xml_desc.attribute("value").toLower();
        if ((s_value == "true") || (s_value == "1")) {
            trigger_value = true;
        }
    }

    value = false;
    if (type.toLower() == "status") {
        this->type = type_status;
    } else if (type.toLower() == "trigger") {
        this->type = type_trigger;
    } else if (type.toLower() == "confirm") {
        this->type = type_confirm;
    } else {
        this->type = type_switch;
    }

    qDebug() << "QVSwitch color" << color;
    setStyleSheet(color);
}


void QVSwitch::resizeEvent(QResizeEvent * event) {
    if (event->oldSize().height() == height()) {
        return;
    }

    int icon_height = 0, icon_width = 0, icon_sw_height = 0, icon_sw_maxheight = height(), icon_sw_width = 0;

    if (w_icon != 0) {
        icon_width = (int)(width() * max_icon_width());
        icon_height = (int)(icon_width / w_icon->aspectRatio());
        if (icon_height > height() * max_icon_width()) {
            icon_height = height() * max_icon_width();
            icon_width = (int)(icon_height * w_icon->aspectRatio());
        }
    }

    if (icon_height > w_text->sizeHint().height()) {
        icon_sw_maxheight = height() - icon_height;
    } else {
        icon_sw_maxheight = height() - w_text->sizeHint().height();
    }

    if (w_sw_icon != 0) {
        icon_sw_height = icon_sw_maxheight;
        icon_sw_width = (int)(icon_sw_height/w_sw_icon->aspectRatio());
        if (icon_sw_width > width()) {
            icon_sw_width = width();
            icon_sw_height = (int)(icon_sw_width*w_sw_icon->aspectRatio());
        }
        w_sw_icon->setFixedSize(icon_sw_width,icon_sw_height);
        w_sw_icon->move(ofs_x()+(int)((width()-icon_sw_width)/2),ofs_y()+(int)((icon_sw_maxheight-icon_sw_height)/2));
        if (type == type_trigger) {
            w_sw_icon->setStatus(false);
        }
    }

    int ofs_icon = ofs_y() + icon_sw_maxheight;
    int ofs_text = ofs_y() + icon_sw_maxheight;
    if (icon_height > w_text->sizeHint().height()) {
        ofs_text += (int)((icon_height-w_text->sizeHint().height())/2);
    } else {
        ofs_icon += (int)((w_text->sizeHint().height()-icon_height)/2);
    }
    if (w_icon != 0) {
        w_icon->setFixedSize(icon_width,icon_height);
        w_icon->move(ofs_x()+(int)((width()-w_text->sizeHint().width()-icon_width)/2),ofs_icon);
    }
    w_text->move(ofs_x()+(int)((width()-w_text->sizeHint().width()+icon_width)/2),ofs_text);
}

void QVSwitch::mousePressEvent(QMouseEvent *event) {
    if (event) {
        event->accept();
    }

    switch(type) {
    case type_confirm:
        if (!value) {
            return;
        }
        value = false;
        break;
    case type_status:
        return;
    case type_trigger:
        value = trigger_value;
        break;
    default:
        value = !value;
        break;
    }

    QString item = getItemByAction("switch");
    if (item.isNull()) {
        return;
    }

    if (type != type_trigger) {
        if (w_sw_icon != 0) {
            w_sw_icon->setStatus(this->value);
        }
        if (this->value) {
            setStyleSheet(active_color);
        } else {
            setStyleSheet(color);
        }
    }

    qDebug() << "Emitting " << item << " to " << QString::number(value);
    emit valueModified(item,QString::number(value));
}

void QVSwitch::svgPressed() {
    mousePressEvent(0);
}


void QVSwitch::onValueChanged(QString item, QString value) {
    if (!items.contains(item) || (type == type_trigger)) {
        return;
    }
//    qDebug() << "modified: " << item << " to " << value;
    QMap<QString,QVItem>::iterator iter = items.find(item);
    if (iter->getAction() != "switch") {
        return;
    }

    bool ok;
    int value_int = value.toInt(&ok);
    if (!ok) {
        return;
    }
    if (value_int == 0) {
        this->value = false;
    } else {
        this->value = true;
    }

    if (w_sw_icon != 0) {
        w_sw_icon->setStatus(this->value);
    }
    if (this->value) {
        setStyleSheet(active_color);
    } else {
        setStyleSheet(color);
    }
}

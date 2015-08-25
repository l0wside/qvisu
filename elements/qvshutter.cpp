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
 *
 * Alle Icons sind unter einer Creative Commons Lizenz vom Typ Namensnennung -
 * Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugaenglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-sa/3.0/de/ oder wenden Sie
 * sich brieflich an
 * Creative Commons, Postfach 1866, Mountain View, California, 94042, USA. */

#include "qvshutter.h"
#include <cmath>
#include <QHBoxLayout>
#include <QColor>
#include <QPalette>

static const QString svg_shutter_elem = QStringLiteral("<path style=\"stroke:#FFFFFF;stroke-width:10;stroke-linecap:round;\" d=\"M75,##POS## l210,0\" />\n");
static const QString svg_shutter_elem_l = QStringLiteral("<path style=\"stroke:#FFFFFF;stroke-width:10;stroke-linecap:round;\" d=\"M75,##POS## l100,0\" />\n");
static const QString svg_shutter_elem_r = QStringLiteral("<path style=\"stroke:#FFFFFF;stroke-width:10;stroke-linecap:round;\" d=\"M185,##POS## l100,0\" />\n");

QVShutter::QVShutter(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w <= 0) {
        w = 2;
    }
    h = 2;
    QString filename_icon;
    QDomElement xml_elem;

    popup_frame = new QVPopupFrame(parent);

    xml_elem = xml_desc.firstChildElement("icon");
    if (!xml_elem.isNull()) {
        filename_icon = xml_elem.text();
    }
    QFile file_icon(findFilePath(filename_icon));
    if (file_icon.open(QIODevice::ReadOnly)) {
        svg_icon = file_icon.readAll();
        file_icon.close();
        w_icon = new QVIconWidget (this);
        if (!svg_icon.isEmpty()) {
            w_icon->load(svg_icon);
        }
        w_icon->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
        QObject::connect(w_icon,SIGNAL(clicked(double,double)),popup_frame,SLOT(show()));
    } else {
        w_icon = NULL;
    }

    QFile file_shutter_icon(":/icons/fts_shutter_bg.svg");
    if (file_shutter_icon.open(QIODevice::ReadOnly)) {
        shutter_icon = file_shutter_icon.readAll();
    }
    file_shutter_icon.close();

    w_shutter_icon = new QVSvgWidget (this);
    if (!shutter_icon.isEmpty()) {
        QString svg_string(shutter_icon);
        svg_string.replace("##PLACEHOLDER##","");
        w_shutter_icon->load(svg_string.toUtf8());
   }
    QObject::connect(w_shutter_icon,SIGNAL(clicked(double,double)),popup_frame,SLOT(show()));

    xml_elem = xml_desc.firstChildElement("text");
    w_text = new QLabel(xml_elem.text(),this);
    w_text->setAlignment(Qt::AlignHCenter);

    two_halves = false;
    has_blades = false;
    if (!getItemByAction("up-down2").isEmpty() || !getItemByAction("position2").isEmpty()) {
        two_halves = true;
    } else if (!getItemByAction("blade-position").isEmpty()) {
        has_blades = true;
    }

    max_position1 = max_position2 = max_blade_position = 255.0;
    QDomNodeList e_items = xml_desc.elementsByTagName("item");
    for (int n=0; n < e_items.count(); n++) {
        xml_elem = e_items.at(n).toElement();
        bool ok;
        double val;
        if ((xml_elem.attribute("action") == "position") && xml_elem.hasAttribute("max-value")) {
            val = xml_elem.attribute("max-value").toDouble(&ok);
            if (ok) {
                max_position1 = val;
            }
        }
        if ((xml_elem.attribute("action") == "position2") && xml_elem.hasAttribute("max-value")) {
            val = xml_elem.attribute("max-value").toDouble(&ok);
            if (ok) {
                max_position2 = val;
            }
        }
        if ((xml_elem.attribute("action") == "blade-position") && xml_elem.hasAttribute("max-value")) {
            val = xml_elem.attribute("max-value").toDouble(&ok);
            if (ok) {
                max_blade_position = val;
            }
        }
    }

    popup_frame->content()->setStyleSheet(color);
    QObject::connect(w_shutter_icon,SIGNAL(clicked(double,double)),popup_frame,SLOT(show()));

    QFrame *popup = popup_frame->content();

    popup_shutter_icon = new QVSvgWidget (popup);
    if (!shutter_icon.isEmpty()) {
        QString svg_string(shutter_icon);
        svg_string.replace("##PLACEHOLDER##","");
        popup_shutter_icon->load(svg_string.toUtf8());
        QObject::connect(popup_shutter_icon,SIGNAL(clicked(double,double)),this,SLOT(onPopupShutterClicked(double,double)));
    }

    w_buttons_l = new QVSvgWidget(popup);
    if (two_halves) {
        w_buttons_r = new QVSvgWidget(popup);
    }
    QFile file_icon_buttons(":/icons/shutter_arrows.svg");
    if (file_icon_buttons.open(QIODevice::ReadOnly)) {
        QByteArray svg = file_icon_buttons.readAll();
        w_buttons_l->load(svg);
        QObject::connect(w_buttons_l,SIGNAL(clicked(double,double)),this,SLOT(onBtnLeft(double,double)));
        if (two_halves) {
            w_buttons_r->load(svg);
            QObject::connect(w_buttons_r,SIGNAL(clicked(double,double)),this,SLOT(onBtnRight(double,double)));
        }
    }
    file_icon_buttons.close();

    if (has_blades) {
        QFile f(":/icons/blade.svg");
        if (f.open(QIODevice::ReadOnly)) {
            blade_icon = f.readAll();
        }
        QByteArray s = blade_icon;
        s.replace("##ROT1","0");
        s.replace("##ROT2","0");
        w_blade_icon = new QVSvgWidget(this);
        w_blade_icon->load(s);
        QObject::connect(w_blade_icon,SIGNAL(clicked(double,double)),popup_frame,SLOT(show()));
        popup_blade_icon = new QVSvgWidget(popup);
        popup_blade_icon->load(s);

        QObject::connect(popup_blade_icon,SIGNAL(clicked(double,double)),this,SLOT(onBladeClicked(double,double)));
        QObject::connect(popup_blade_icon,SIGNAL(dragged(double,double)),this,SLOT(onBladeDragged(double,double)));
        QObject::connect(popup_blade_icon,SIGNAL(released(double,double)),this,SLOT(onBladeReleased(double,double)));

        blade_value = 0.5;
    }

    QObject::connect(popup_frame,SIGNAL(clicked()),popup_frame,SLOT(hide()));
}


void QVShutter::resizeEvent(QResizeEvent * event) {
    if (event->oldSize().height() == QWidget::height()) {
        return;
    }

    /* Height for icon and label */
    int label_height = 0, icon_height = 0, icon_width = 0;
    if (w_icon != 0) {
        label_height = height() * max_label_height();
        icon_height = label_height;
        icon_width = icon_height / w_icon->aspectRatio();
        if (icon_width > width() * max_icon_width()) {
            icon_width = (int)(width() * max_icon_width());
        }
    }
    if (label_height < w_text->sizeHint().height()) {
        label_height = w_text->sizeHint().height();
    }

    int content_height = height() - label_height;

    int icon_shutter_height = 0, icon_shutter_width = 0;

    if (!has_blades) {
        /* Size for shutter icon */
        icon_shutter_height = content_height;
        icon_shutter_width = (int)(icon_shutter_height/w_shutter_icon->aspectRatio());
        if (icon_shutter_width > width() * max_shutter_width()) {
            icon_shutter_width = (int)(width() * max_shutter_width());
            icon_shutter_height = (int)(icon_shutter_width*w_shutter_icon->aspectRatio());
        }
        int shutter_icon_ofs = (int)((content_height-icon_shutter_height)/2);

        w_shutter_icon->setFixedSize(icon_shutter_width,icon_shutter_height);
        w_shutter_icon->move(ofs_x()+(int)((width()-icon_shutter_width)/2),ofs_y()+shutter_icon_ofs);
    } else {
        int icon_width = (int)floor((width()-padding)/2);
        int icon_shutter_height = (int)floor(icon_width*w_shutter_icon->aspectRatio());
        int icon_blade_height = (int)floor(icon_width*w_blade_icon->aspectRatio());

        w_shutter_icon->setFixedSize(icon_width,icon_shutter_height);
        w_shutter_icon->move(ofs_x(),ofs_y()+(int)((height()-icon_shutter_height)/2));
        w_blade_icon->setFixedSize(icon_width,icon_blade_height);
        w_blade_icon->move(ofs_x()+icon_width+padding,ofs_y()+(int)((height()-icon_blade_height)/2));
    }

    /* Icon and label */
    if ((icon_width == 0) || (w_icon == 0)) {
        if (w_icon) {
            w_icon->hide();
        }
        w_text->setFixedSize(w_text->sizeHint());
        w_text->move(ofs_x()+(int)((width()-(w_text->width()))/2),height()-w_text->height()+5);
    } else {
        w_icon->setFixedSize(icon_width,icon_height);
        w_icon->move(ofs_x()+(int)((width()-icon_width-w_text->width())/2),height()-icon_height);
        w_text->setFixedSize(w_text->sizeHint());
        w_text->move(ofs_x()+(int)((width()+icon_width-w_text->width())/2),height()-(int)((icon_height+w_text->height())/2)+5);
    }
    qDebug() << "shutter label size" << w_text->size() << w_text->pos() << size();


    /* Place elements in popup */
    int popup_width;
    if (has_blades) {
        popup_width = QWidget::width()*3;
    } else {
        popup_width = QWidget::width()*2;
    }
    int popup_height = QWidget::height();
    int popup_x = pos().x();
    int popup_y = pos().y();
    if (popup_x + popup_width > ((QWidget*)parent())->width()) {
        popup_x = ((QWidget*)parent())->width() - popup_width;
    }
    if (popup_x + popup_width > ((QWidget*)parent())->height()) {
        popup_x = ((QWidget*)parent())->height() - popup_height;
    }
    popup_frame->place(popup_x,popup_y,popup_width,popup_height);

    int remaining_width = popup_width;
    /* Buttons */
    int button_height = popup_height-2*padding;
    int button_width = (int)(button_height / w_buttons_l->aspectRatio());

    w_buttons_l->move(3*padding,padding);
    w_buttons_l->setFixedSize(button_width,button_height);
    remaining_width -= button_width + 3*padding;

    if (two_halves) {
        w_buttons_r->move(popup_width-button_width-3*padding,padding);
        w_buttons_r->setFixedSize(button_width,button_height);
        remaining_width -= (button_width + 9*padding);
    }
    //qDebug() << w_text->text() << remaining_width;

    int popup_shutter_width, popup_shutter_height, popup_shutter_x, popup_shutter_y;
    int popup_blade_width, popup_blade_height, popup_blade_x, popup_blade_y;
    if (has_blades) {
        int icon_space = (int)(remaining_width/2-7.5*padding);

        popup_shutter_width = icon_space;
        popup_shutter_height = (int)(popup_shutter_width * popup_shutter_icon->aspectRatio());
        if (popup_shutter_height > popup_height-2*padding) {
            popup_shutter_height = popup_height-2*padding;
            popup_shutter_width = (int)(popup_shutter_height / popup_shutter_icon->aspectRatio());
        }
        popup_shutter_x = 9*padding + (int)((icon_space-popup_shutter_width)/2) + button_width;
        popup_shutter_y = (int)((popup_height-popup_shutter_height)/2);

        popup_blade_width = popup_blade_height = icon_space;
        if (popup_blade_height > popup_height-2*padding) {
            popup_blade_width = popup_blade_height = popup_height-2*padding;
        }
        popup_blade_x = 12*padding + icon_space + (int)((icon_space-popup_shutter_width)/2) + button_width;
        popup_blade_y = (int)((popup_height-popup_blade_height)/2);
    } else {
        int icon_space = remaining_width;

        popup_shutter_width = icon_space;
        popup_shutter_height = (int)(popup_shutter_width * popup_shutter_icon->aspectRatio());
        if (popup_shutter_height > popup_height-2*padding) {
            popup_shutter_height = popup_height-2*padding;
            popup_shutter_width = (int)(popup_shutter_height/popup_shutter_icon->aspectRatio());
        }
        popup_shutter_x = 6*padding + (int)((icon_space-popup_shutter_width)/2) + button_width;
        popup_shutter_y = (int)((popup_height-popup_shutter_height)/2);
    }

    popup_shutter_icon->setFixedSize(popup_shutter_width,popup_shutter_height);
    popup_shutter_icon->move(popup_shutter_x,popup_shutter_y);
    if (has_blades) {
        popup_blade_icon->setFixedSize(popup_blade_width,popup_blade_height);
        popup_blade_icon->move(popup_blade_x,popup_blade_y);
    }

    if (color.isEmpty()) {
        popup_frame->content()->setStyleSheet("QFrame { background-color:" + palette().color(QWidget::backgroundRole()).name(QColor::HexRgb) + "}");
        popup_frame->content()->setAutoFillBackground( true );
    } else {
        popup_frame->content()->setStyleSheet(color);
    }
    popup_shutter_icon->setStyleSheet("QSvgWidget { background-color:#cccc00 }");
    if (has_blades) {
        popup_blade_icon->setStyleSheet("QSvgWidget { background-color:#00cccc }");
    }
 }


void QVShutter::mousePressEvent(QMouseEvent *event) {
    event->accept();
    popup_frame->show();
}

void QVShutter::onShutterClicked() {
    //qDebug() << "Raising";
    popup_frame->show();
}

void QVShutter::onValueChanged(QString item, QString value) {
    if (!items.contains(item)) {
        return;
    }

    QMap<QString,QVItem>::iterator iter = items.find(item);
    QString action = iter->getAction();
    if ((action == "position") || (action == "position2")) {
        bool ok;
        double position = value.toDouble(&ok);
        if (!ok) {
            return;
        }

        if (action == "position") {
            if (position > max_position1) {
                position = max_position1;
            }
            this->position = position;
        } else {
            if (position > max_position2) {
                position = max_position2;
            }
            this->position2 = position;
        }

        QString bars;
        double rel_pos = this->position / max_position1 * 10;

        while (rel_pos > 0) {
             QString bar;
            if (two_halves) {
                bar = svg_shutter_elem_l;
            } else {
                bar = svg_shutter_elem;
            }
            double ypos = rel_pos * 12.7 + 122;
            bar.replace("##POS##",QString::number(ypos,'f',3));
            bars.append(bar);
            rel_pos--;
        }
        if (two_halves) {
            rel_pos = this->position2 / this->max_position2 * 10;
            while (rel_pos > 0) {
                QString bar;
                bar = svg_shutter_elem_r;
                double ypos = rel_pos * 12.7 + 122;
                bar.replace("##POS##",QString::number(ypos,'f',3));
                bars.append(bar);
                rel_pos--;
            }
        }
        QString svg_string = shutter_icon;
        svg_string.replace("##PLACEHOLDER##",bars);
        w_shutter_icon->load(svg_string.toUtf8());
        popup_shutter_icon->load(svg_string.toUtf8());
        return;
    }
    if (action == "blade-position") {
        bool ok;
        double val = value.toDouble(&ok);
        if (!ok) {
            return;
        }
        if (val > max_blade_position) {
            val = max_blade_position;
        }
        if (val < 0.0) {
            val = 0.0;
        }

        blade_value = val / max_blade_position;

        double arc1 = 60-(blade_value * 90.0);
        double arc2 = 60-(blade_value * 90.0);

        QByteArray s = blade_icon;
        s.replace("##ROT1##",QString::number(arc1,'f',3).toUtf8());
        s.replace("##ROT2##",QString::number(arc2,'f',3).toUtf8());
        w_blade_icon->load(s);
        popup_blade_icon->load(s);
    }
}

void QVShutter::onBtnLeft(double,double y) {
    popup_frame->hide();

    if (y < 1.0/3.0) {
        QString item = getItemByAction("up-down");
        if (item.isNull()) {
            return;
        }
        emit valueModified(item,QStringLiteral("0"));
        return;
    }
    if (y < 2.0/3.0) {
        QString item = getItemByAction("stop");
        if (item.isNull()) {
            return;
        }
        emit valueModified(item,QStringLiteral("1"));
        return;
    }
    QString item = getItemByAction("up-down");
    if (item.isNull()) {
        return;
    }
    emit valueModified(item,QStringLiteral("1"));
}

void QVShutter::onBtnRight(double,double y) {
    popup_frame->hide();

    if (y < 1.0/3.0) {
        QString item = getItemByAction("up-down2");
        if (item.isNull()) {
            return;
        }
        emit valueModified(item,QStringLiteral("0"));
        return;
    }
    if (y < 2.0/3.0) {
        QString item = getItemByAction("stop2");
        if (item.isNull()) {
            return;
        }
        emit valueModified(item,QStringLiteral("1"));
        return;
    }
    QString item = getItemByAction("up-down2");
    if (item.isNull()) {
        return;
    }
    emit valueModified(item,QStringLiteral("1"));
}

void QVShutter::onPopupShutterClicked(double x, double y) {
    popup_frame->hide();

    QString item;
    double max_position;
    if (!two_halves || (x <= 0.5)) {
        item = getItemByAction("position");
        max_position = max_position1;
    } else {
        item = getItemByAction("position2");
        max_position = max_position2;
    }
    if (item.isEmpty()) {
        return;
    }

    const double upper_end = 0.164, lower_end = 0.9;
    double target_value;

    if (y < upper_end) {
        target_value = 0.0;
    } else if (y > lower_end) {
        target_value = 1.0;
    } else {
        target_value = (y-upper_end)/(lower_end-upper_end);
    }

    emit valueModified(item,QString::number(target_value*max_position,'f',3));
}

void QVShutter::onBladeClicked(double xrel, double yrel) {
    if (abs(xrel-0.5) < 0.15) {
        xrel = (xrel < 0.5)?0.35:0.65;
    }

    was_dragged = false;

    drag_start = atan((0.5-yrel)/(xrel-0.5));
    qDebug() << "drag start" << drag_start/M_PI*180;
}

void QVShutter::onBladeDragged(double xrel, double yrel) {
    if (abs(xrel-0.5) < 0.15) {
        xrel = (xrel < 0.5)?0.35:0.65;
    }

    was_dragged = true;
    double drag_pos = atan((0.5-yrel)/(xrel-0.5));

    double drag_path = drag_pos - drag_start;
    double temp_blade_value = blade_value + drag_path/M_PI_2;

    if (temp_blade_value < 0.0) {
        temp_blade_value = 0.0;
    }
    if (temp_blade_value > 1.0) {
        temp_blade_value = 1.0;
    }

    qDebug() << drag_start << drag_pos << round(drag_path/M_PI*180) << blade_value << temp_blade_value;

    double arc1 = 60-(temp_blade_value * 90.0);
    double arc2 = 60-(temp_blade_value * 90.0);

    QByteArray s = blade_icon;
    s.replace("##ROT1##",QString::number(arc1,'f',3).toUtf8());
    s.replace("##ROT2##",QString::number(arc2,'f',3).toUtf8());
    w_blade_icon->load(s);
    popup_blade_icon->load(s);


}

void QVShutter::onBladeReleased(double xrel, double yrel) {
    if (abs(xrel-0.5) < 0.15) {
        xrel = (xrel < 0.5)?0.35:0.65;
    }
    double release_pos = atan((0.5-yrel)/(xrel-0.5));

    qDebug() << "released" << xrel-0.5 << 0.5-yrel << (0.5-yrel)/(xrel-0.5) << release_pos/M_PI*180;

    QString item = getItemByAction("blade-position");
    if (item.isEmpty()) {
        return;
    }

    if (!was_dragged) {
        if (release_pos > M_PI_4) {
            release_pos = M_PI_4;
        }
        if (release_pos < -M_PI_4) {
            release_pos = -M_PI_4;
        }
        blade_value = (release_pos + M_PI_4)/M_PI_2;
        qDebug() << "blade now" << blade_value << "pos" << release_pos << round(release_pos/M_PI*180);

        emit valueModified(item,QString::number(blade_value*max_blade_position,'f',3));
        return;
    } else {
        double drag_pos = atan((0.5-yrel)/(xrel-0.5));
        blade_value += (drag_pos-drag_start)/M_PI_2;
        if (blade_value > 1.0) {
            blade_value = 1.0;
        }
        if (blade_value < 0.0) {
            blade_value = 0.0;
        }
        qDebug() << "dragged: " << (drag_pos-drag_start)/M_PI*180;

        emit valueModified(item,QString::number(blade_value*max_blade_position,'f',3));
    }


}

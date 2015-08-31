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

#include "qvcolor.h"
#include <cmath>
#include <QHBoxLayout>
#include <QColor>
#include <QPalette>
#include <QCryptographicHash>

QVColor::QVColor(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    w = 2;
    h = 2;
    QDomElement xml_elem;

    popup_frame = new QVPopupFrame(parent);

    w_icon = 0;
    xml_elem = xml_desc.firstChildElement("icon");
    if (!xml_elem.isNull()) {
        QFile f(findFilePath(xml_elem.text()));
        if (f.open(QIODevice::ReadOnly)) {
            w_icon = new QVIconWidget(this);
            w_icon->load(f.readAll());
            f.close();
        }
    }

    w_rgb_icon = 0;
    xml_elem = xml_desc.firstChildElement("rgb-icon");
    if (!xml_elem.isNull()) {
        w_rgb_icon = new QVIconWidget(findFilePath(xml_elem.text()),this);
    } else {
        w_rgb_icon = new QVIconWidget(":/icons/light_rgb.svg",this);
    }

    w_switch_icon = 0;
    if (!getItemByAction("switch").isNull()) {
        xml_elem = xml_desc.firstChildElement("sw-icon");
        if (!xml_elem.isNull()) {
            w_switch_icon = new QVSwitchIcon(xml_elem,this);
        } else {
            w_switch_icon = new QVSwitchIcon(":/icons/onoff.svg","#ffffff","orange","replace",this);
        }
        switch_status = false;
        QObject::connect(w_switch_icon,SIGNAL(clicked(double,double)),this,SLOT(onSwitchClicked()));
    }


    xml_elem = xml_desc.firstChildElement("text");
    if (!xml_elem.isNull()) {
        w_text = new QLabel(xml_elem.text(),this);
    } else {
        w_text = new QLabel(this);
    }

    if (getItemByAction("white").isEmpty()) {
        has_white = false;
    } else {
        has_white = true;
    }

    xml_elem = xml_desc.firstChildElement("history");
    if (!xml_elem.isNull()) {
        bool ok;
        int n_history = xml_elem.text().toInt(&ok);
        QString hash_string = getItemByAction("red") + getItemByAction("green") + getItemByAction("blue") + getItemByAction("white");
        if (!hash_string.isEmpty() && ok && (n_history > 0)) {
            registry_key = "rgb/" + QString(QCryptographicHash::hash(hash_string.toUtf8(),QCryptographicHash::Sha256).toBase64()).replace("/","_");
            for (int n=0; n < n_history; n++) {
                history_icons.append(new QVIconWidget(":/icons/colorpick.svg",this));
                QObject::connect(history_icons.last(),SIGNAL(clicked(double,double)),this,SLOT(onHistoryClicked()));
                QObject::connect(history_icons.last(),SIGNAL(released(double,double)),this,SLOT(onHistoryReleased(double,double)));
                history_colors.append("");
            }
            registry_settings = new QSettings("kalassi","QVisu");
            QString stored = registry_settings->value(registry_key).toString();
            if (!stored.isEmpty()) {
                QStringList stored_list = stored.split(",");
                for (int n=0; (n < stored_list.count()) && (n < history_colors.count()); n++) {
                    history_colors[n] = stored_list[n];
                    if (history_colors[n].length() > 0) {
                        hsi_t hsi;
                        if (hsiFromString(history_colors[n],hsi)) {
                            rgbw_t rgb;
                            hsiToRgb_int(hsi,rgb);
                            history_icons[n]->setPathStyle("fill",QColor(rgb.r,rgb.g,rgb.b).name());
                        }
                    }
                }
            }
        }
        QObject::connect(&history_timer,SIGNAL(timeout()),this,SLOT(onHistoryTimer()));
    }


    popup_frame->content()->setStyleSheet(color);
    QObject::connect(w_rgb_icon,SIGNAL(clicked(double,double)),popup_frame,SLOT(show()));

    QFrame *popup = popup_frame->content();
    popup_colorlabel = new QLabel(popup);
    popup_image = 0;
    popup_circle = new QVSvgWidget(":/icons/colorcircle.svg",popup);
    QObject::connect(popup_circle,SIGNAL(clicked(double,double)),this,SLOT(onCircleClicked(double,double)));
    QObject::connect(popup_circle,SIGNAL(dragged(double,double)),this,SLOT(onCircleClicked(double,double)));

    w_dimmer = new QVSvgWidget(popup);
    QFile file_dimmer(":/icons/dimmer.svg");
    if (file_dimmer.open(QIODevice::ReadOnly)) {
        svg_dimmer = file_dimmer.readAll();
        file_dimmer.close();
    } else {
        qDebug() << ":/icons/dimmer.svg not found";
    }

    hsi.h = hsi.s = 0; hsi.i = 0.5;
    hsiToRgbw(hsi,rgbw);

    updatePopup();

    QObject::connect(w_dimmer,SIGNAL(dragged(double,double)),this,SLOT(dimmerDragged(double,double)));
    QObject::connect(w_dimmer,SIGNAL(clicked(double,double)),this,SLOT(dimmerClicked(double,double)));

    QObject::connect(popup_frame,SIGNAL(clicked(int,int)),this,SLOT(onPopupClicked(int,int)));
}


void QVColor::resizeEvent(QResizeEvent * event) {
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

    /* Icon and label */
    if ((icon_width == 0) || (w_icon == 0)) {
        if (w_icon) {
            w_icon->hide();
        }
        w_text->move(ofs_x()+(int)((width()-(w_text->sizeHint().width()))/2),height()-w_text->sizeHint().height());
    } else {
        w_icon->setFixedSize(icon_width,icon_height);
        w_icon->move(ofs_x()+(int)((width()-icon_width-w_text->sizeHint().width())/2),height()-icon_height);
        w_text->move(ofs_x()+(int)((width()+icon_width-w_text->sizeHint().width())/2),height()-(int)((icon_height+w_text->sizeHint().height())/2));
    }

    /* History icons */
    if (history_icons.count() > 0) {
        int history_height = max_label_height()*height();
        int history_step = (int)(((double)width())/((double)history_icons.count()));
        content_height -= history_height;
        for (int n=0; n < history_icons.count(); n++) {
            fitInBox(history_icons[n],n*history_step+ofs_x(),content_height+ofs_y(),history_step,history_height);
        }
    }

    /* Switch and RGB icons */
    if (w_switch_icon == 0) {
        w_rgb_icon->setFixedSize(content_height,content_height);
        w_rgb_icon->move((int)((width()-content_height)/2)+ofs_x(),ofs_y());
        qDebug() << "rgb" << w_rgb_icon->x() << w_rgb_icon->y() << w_rgb_icon->width() << w_rgb_icon->height();
    } else {
        fitInBox(w_switch_icon,ofs_x(),ofs_y(),(int)(width()/2),content_height);
        fitInBox(w_rgb_icon,ofs_x()+(int)(width()/2),ofs_y(),(int)(width()/2),content_height);
    }

    /* Popup */
    int popup_width = QWidget::width()*2;
    int popup_height = (int)(QWidget::height()*2.5);
    int popup_x = pos().x();
    int popup_y = pos().y();
    if (popup_x + popup_width > ((QWidget*)parent())->width()) {
        popup_x = ((QWidget*)parent())->width() - popup_width;
    }
    if (popup_y + popup_height > ((QWidget*)parent())->height()) {
        popup_y = ((QWidget*)parent())->height() - popup_height;
    }
    popup_frame->place(popup_x,popup_y,popup_width,popup_height);

    popup_colorlabel->setFixedSize(popup_width,popup_width);
    popup_colorlabel->move(0,0);

    /* Color Circle */
    if (popup_image) {
        delete popup_image;
    }
    popup_image = new QImage(popup_width,popup_width,QImage::Format_ARGB32);
    popup_image->fill(qRgba(0,0,0,0));

    for (int x=0; x < popup_width; x++) {
        double xnorm = (double)(2*x-popup_width)/((double)(2*popup_width));
        for (int y=0; y < popup_width; y++) {
            double ynorm = (double)(popup_width-2*y)/((double)(2*popup_width));
            double r = 2*sqrt(xnorm*xnorm+ynorm*ynorm);
            double phi;
            if ((xnorm >= 0) && (ynorm > 0)) {
                phi = atan(xnorm/ynorm);
            } else if ((xnorm > 0) && (ynorm <= 0)) {
                phi = atan(-ynorm/xnorm) + 3.14157/2;
            } else if ((xnorm <= 0) && (ynorm < 0)) {
                phi = atan(xnorm/ynorm) + 3.14157;
            } else if ((xnorm < 0) && (ynorm >= 0)) {
                phi = atan(-ynorm/xnorm) + 3.14157*3/2;
            } else  {
                phi = 0;
            }
            if (r < 1) {
                QColor c;
                c.setHsvF(phi/(2*3.14157),r,0.3+(r*0.7));
                popup_image->setPixel(x,y,c.rgb());
            }
        }
    }
    popup_colorlabel->setPixmap(QPixmap::fromImage(*popup_image));

    popup_circle->setFixedSize((int)(popup_frame->content()->width()/20),(int)(popup_frame->content()->width()/20));

    w_dimmer->setFixedSize(popup_width,popup_height-popup_width);
    w_dimmer->move(0,popup_width);

    updatePopup();
    popup_frame->content()->raise();
    popup_circle->raise();
}

void QVColor::onSwitchClicked() {
    switch_status = !switch_status;
    emit valueModified(QVElement::getItemByAction("switch"),switch_status?QStringLiteral("1"):QStringLiteral("0"));
}

void QVColor::onRGBClicked() {
    popup_frame->show();
    popup_frame->content()->raise();
}

void QVColor::onHistoryClicked() {
    int index = -1;
    for (int n=0; n < history_icons.count(); n++) {
        if ((void*)(history_icons[n]) == (void*)(QObject::sender())) {
            index = n;
            break;
        }
    }
    if (index < 0) {
        return;
    }
    current_history_index = index;

    history_timer.setInterval(1000);
    history_timer.setSingleShot(true);
    history_timer.start();
}

void QVColor::onHistoryTimer() {
    if (current_history_index < 0) {
        return;
    }

    /* Store current color */
    history_colors[current_history_index] = hsiToString(hsi);
    registry_settings->setValue(registry_key,history_colors.join(","));

    rgbw_t rgb;
    hsiToRgb_int(hsi,rgb);
    QString path_color = QColor(rgb.r,rgb.g,rgb.b,1.0).name();
    history_icons[current_history_index]->setPathStyle("fill",path_color);
    current_history_index = -1;
}

void QVColor::onHistoryReleased(double x,double y) {
    if ((x < 0.0) || (x > 1.0) || (y < 0.0) || (y > 1.0)) {
        return;
    }
    if (!history_timer.isActive() || (current_history_index < 0)) {
        return;
    }
    history_timer.stop();

    /* Recall stored color */
    hsi_t hsi;
    if (hsiFromString(history_colors[current_history_index],hsi)) {
        qDebug() << "hsiFromString" << history_colors[current_history_index] << hsi.i;
        this->hsi = hsi;
        sendModified();

        /* Update color circle */
        updatePopup();
    }
    current_history_index = -1;

}


void QVColor::dimmerClicked(double xrel, double) {
    //    qDebug() << "clicked" << xrel;
    dimmerDragged(xrel,0);
}

void QVColor::dimmerDragged(double xrel, double) {
    qDebug() << "dragged" << xrel;
    if (xrel < 0.04) {
        xrel = 0.04;
    }
    if (xrel > 0.96) {
        xrel = 0.96;
    }

    double value = (xrel-0.04)/0.92;
    if (fabs(hsi.i - value) < 1e-4) {
        return;
    }
    hsi.i = value;
    updatePopup();
    sendModified();
}



void QVColor::onValueChanged(QString item, QString value) {
    if (!items.contains(item)) {
        return;
    }
    qDebug() << item << "now" << value;

    QMap<QString,QVItem>::iterator iter = items.find(item);
    QString action = iter->getAction();
    if (action == "switch") {
        bool ok;
        int status = value.toInt(&ok);
        if (!ok) {
            return;
        }
        if (w_switch_icon == 0) {
            return;
        }
        switch_status = (bool)status;
        w_switch_icon->setStatus(switch_status);
        return;
    }
    if ((action == "white") && has_white) {
        bool ok;
        int w = value.toInt(&ok);
        if ((w < 0) || (w > 255)) {
            qDebug() << "QVColor: Invalid w value" << w;
            return;
        }
        rgbw.w = w;
    } else if (action == "red") {
        bool ok;
        int r = value.toInt(&ok);
        if ((r < 0) || (r > 255)) {
            qDebug() << "QVColor: Invalid r value" << r;
            return;
        }
        rgbw.r = r;
    } else if (action == "green") {
        bool ok;
        int g = value.toInt(&ok);
        if ((g < 0) || (g > 255)) {
            qDebug() << "QVColor: Invalid g value" << g;
            return;
        }
        rgbw.g = g;
    } else if (action == "blue") {
        bool ok;
        int b = value.toInt(&ok);
        if ((b < 0) || (b > 255)) {
            qDebug() << "QVColor: Invalid b value" << b;
            return;
        }
        rgbw.b = b;
    } else {
        return;
    }
    if (has_white) {
        rgbwToHsi(rgbw,hsi);
    } else {
        rgbToHsi(rgbw,hsi);
    }
    qDebug() << "OVC, i now" << hsi.i;
    updatePopup();
}


void QVColor::updatePopup() {
    QString svg_string = svg_dimmer;

    svg_string.replace("#POS#",QString::number(hsi.i*90,'f',3));
    double slpos = hsi.i*90;
    svg_string.replace("#SLPOS#",QString::number(slpos,'f',3));
    svg_string.replace("#SLW#",QString::number(10,'f',3));
    w_dimmer->load(svg_string.toUtf8());

    rgbw_t rgb;
    hsiToRgb_int(hsi,rgb);
    popup_frame->content()->setStyleSheet("QFrame { background:" + QColor(rgb.r,rgb.g,rgb.b,1.0).name() + " }");

    int circle_x = (int)(sin(hsi.h)*hsi.s*popup_frame->content()->width()/2 + popup_frame->content()->width()/2-popup_circle->width()/2);
    int circle_y = (int)(popup_frame->content()->width()/2 - cos(hsi.h)*hsi.s*popup_frame->content()->width()/2-popup_circle->height()/2);
    popup_circle->move(circle_x,circle_y);

}

void QVColor::onPopupClicked(int x, int y) {
    double xnorm = (double)(2*x-popup_frame->content()->width())/((double)(2*popup_frame->content()->width()));
    double ynorm = (double)(popup_frame->content()->width()-2*y)/((double)(2*popup_frame->content()->width()));
    double r = 2*sqrt(xnorm*xnorm+ynorm*ynorm);

    if (r > 1) {
        return;
    }

    double phi;
    if ((xnorm >= 0) && (ynorm > 0)) {
        phi = atan(xnorm/ynorm);
    } else if ((xnorm > 0) && (ynorm <= 0)) {
        phi = atan(-ynorm/xnorm) + 3.14157/2;
    } else if ((xnorm <= 0) && (ynorm < 0)) {
        phi = atan(xnorm/ynorm) + 3.14157;
    } else if ((xnorm < 0) && (ynorm >= 0)) {
        phi = atan(-ynorm/xnorm) + 3.14157*3/2;
    } else  {
        phi = 0;
    }

    popup_circle->move((int)(x-(popup_circle->width()/2)),(int)(y-(popup_circle->height()/2)));
    popup_circle->raise();

    hsi.h = phi;
    hsi.s = r;

    rgbw_t rgb;
    hsiToRgb_int(hsi,rgb);
    popup_frame->content()->setStyleSheet("QFrame { background:" + QColor(rgb.r,rgb.g,rgb.b,1.0).name() + " }");

    qDebug() << "*****************************************************************************";
    qDebug() << "after click, HSI now" << hsi.h << hsi.s << hsi.i;
    sendModified();
}

void QVColor::onCircleClicked(double x, double y) {
    onPopupClicked((int)(popup_circle->x()+popup_circle->width()*x),(int)(popup_circle->y()+popup_circle->height()*y));
}

/*void QVColor::onCircleDragged(double x, double y) {
    onCircleClicked(x,y);
} */

void QVColor::sendModified() {
    rgbw_t rgbw;

    if (has_white) {
        hsiToRgbw(hsi,rgbw);
        if (rgbw.w != this->rgbw.w) {
            this->rgbw.w = rgbw.w;
            emit valueModified(getItemByAction("white"),QString::number(this->rgbw.w));
        }
    } else {
        hsiToRgb(hsi,rgbw);
    }

    if (rgbw.r != this->rgbw.r) {
        this->rgbw.r = rgbw.r;
        emit valueModified(getItemByAction("red"),QString::number(this->rgbw.r));
    }
    if (rgbw.g != this->rgbw.g) {
        this->rgbw.g = rgbw.g;
        emit valueModified(getItemByAction("green"),QString::number(this->rgbw.g));
    }
    if (rgbw.b != this->rgbw.b) {
        this->rgbw.b = rgbw.b;
        emit valueModified(getItemByAction("blue"),QString::number(this->rgbw.b));
    }
}

QString QVColor::hsiToString(hsi_t hsi) {
    if (hsi.h < 0) {
        hsi.h = 0;
    }
    if (hsi.h > 2*3.14157) {
        hsi.h = 0;
    }
    if (hsi.s <= 1e-6) {
        hsi.s = 0;
        hsi.h = 0;
    }
    if (hsi.s > 1) {
        hsi.s = 1;
    }
    if (hsi.i <= 1e-6) {
        hsi.i = 0;
        hsi.s = 0;
        hsi.h = 0;
    }
    if (hsi.i > 1) {
        hsi.i = 1;
    }

    return QString::number(hsi.h,'f',4) + ":" + QString::number(hsi.s,'f',4) + ":" + QString::number(hsi.i,'f',4);
}

bool QVColor::hsiFromString(QString color, hsi_t &hsi) {
    QStringList l = color.split(":");
    if (l.count() != 3) {
        return false;
    }

    bool ok[3];
    hsi.h = l[0].toDouble(ok);
    hsi.s = l[1].toDouble(ok+1);
    hsi.i = l[2].toDouble(ok+2);
    if (ok[0] && ok[1] && ok[2]) {
        return true;
    }
    return false;
}



/** RGB values for monitor display */
void QVColor::hsiToRgb_int(hsi_t hsi, rgbw_t &rgbw) {
    double h = hsi.h;
    double s = hsi.s;
    double i = hsi.i;

    s = s>0?(s<1?s:1):0; // clamp S and I to interval [0,1]
    i = i>0?(i<1?i:1):0;

    double d_r, d_g, d_b;

    if (h < 1.04719) { /* rot bis gelb */
        d_r = 1.0;
        d_g = s*h/1.04719 + (1-s);
        d_b = 1-s;
    } else if (h < 2.09439) { /* gelb bis grün */
        h -= 1.04719;
        d_g = 1.0;
        d_r = s*(1-h/1.04719) + (1-s);
        d_b = 1-s;
    } else if (h < 3.14157) { /* grün bis cyan */
        h -= 2.09439;
        d_r = 1-s;
        d_g = 1.0;
        d_b = s*h/1.04719 + (1-s);
    } else if (h < 4.18876) { /* cyan bis blau */
        h -= 3.14157;
        d_r = 1-s;
        d_b = 1.0;
        d_g = s*(1-h/1.04719) + (1-s);
    } else if (h < 5.23595) { /* blau bis magenta */
        h -= 4.18876;
        d_g = 1-s;
        d_b = 1.0;
        d_r = s*(h/1.04719) + (1-s);
    } else { /* magenta bis rot */
        h -= 5.23595;
        d_g = 1-s;
        d_r = 1.0;
        d_b = s*(1-h/1.04719) + (1-s);
    }

    rgbw.r = (int)255*d_r*i;
    rgbw.g = (int)255*d_g*i;
    rgbw.b = (int)255*d_b*i;
    rgbw.w = 0;
}

/* RGB values for illuminant */
void QVColor::hsiToRgb(hsi_t hsi, rgbw_t &rgbw) {
    double h = hsi.h;
    double s = hsi.s;
    double i = hsi.i;

    s = s>0?(s<1?s:1):0; // clamp S and I to interval [0,1]
    i = i>0?(i<1?i:1):0;

    double d_r, d_g, d_b;

    if (h < 2.09439) { /* rot bis grün */
        d_r = s*(1-h/2.09439);
        d_g = s*h/2.09439;
        d_b = 0;
    } else if (h < 4.18876) { /* grün bis blau */
        h -= 2.09439;
        d_r = 0;
        d_g = s*(1-h/2.09439);
        d_b = s*h/2.09439;
    } else { /* blau bis rot */
        h -= 4.18876;
        d_b = s*(1-h/2.09439);
        d_g = 0;
        d_r = s*h/2.09439;
    }
    d_r += (1-s);
    d_g += (1-s);
    d_b += (1-s);

    rgbw.r = (int)round(255*d_r*i);
    rgbw.g = (int)round(255*d_g*i);
    rgbw.b = (int)round(255*d_b*i);
    rgbw.w = 0;
}

/* FIXED */
void QVColor::hsiToRgbw(hsi_t hsi, rgbw_t &rgbw) {
    double h = hsi.h;
    double s = hsi.s;
    double i = hsi.i;

    s = s>0?(s<1?s:1):0; // clamp S and I to interval [0,1]
    i = i>0?(i<1?i:1):0;

    double d_r, d_g, d_b;

    if (h < 2.09439) { /* rot bis grün */
        d_r = s*(1-h/2.09439);
        d_g = s*h/2.09439;
        d_b = 0;
    } else if (h < 4.18876) { /* grün bis blau */
        h -= 2.09439;
        d_r = 0;
        d_g = s*(1-h/2.09439);
        d_b = s*h/2.09439;
    } else { /* blau bis rot */
        h -= 4.18876;
        d_b = s*(1-h/2.09439);
        d_g = 0;
        d_r = s*h/2.09439;
    }

    rgbw.r = (int)round(255*d_r*i);
    rgbw.g = (int)round(255*d_g*i);
    rgbw.b = (int)round(255*d_b*i);
    rgbw.w = (int)round(255*(1-s)*i);
}

/* FIXED */
void QVColor::rgbToHsi(rgbw_t rgb, hsi_t &hsi) {
#if 0 /* old, only for internal (monitor) values */
    /* Intensity */
    double i = (rgb.r>rgb.b)?((rgb.r>rgb.g)?rgb.r:rgb.g):((rgb.b>rgb.g)?rgb.b:rgb.g); /* max (r,g,b) */

    double d_i = ((double)i)/255.0;

    if (d_i < 1e-4) {
        /* Black */
        hsi.i = hsi.h = hsi.s = 0;
        return;
    }

    double d_r = ((double)rgb.r)/(255.0*d_i);
    double d_g = ((double)rgb.g)/(255.0*d_i);
    double d_b = ((double)rgb.b)/(255.0*d_i);

//    qDebug() << "++i" << d_i << "r" << d_r << "g" << d_g << "b" <<b;



    /* Saturation */
    double d_s = 1-((d_r < d_b)?((d_r < d_g)?d_r:d_g):((d_b < d_g)?d_b:d_g)); /* 1 - min (r,g,b) */
    if (d_s == 0) {
        /* White (or grey) */
        hsi.h = 0;
        hsi.s = 0;
        hsi.i = i;
        return;
    }

    qDebug() << "sat" << d_s;

    /* Hue */
    d_r -= (1-d_s);
    d_g -= (1-d_s);
    d_b -= (1-d_s);
    if (d_r < 1e-5) { d_r = 0.0; }
    if (d_g < 1e-5) { d_g = 0.0; }
    if (d_b < 1e-5) { d_b = 0.0; }

    double d_h;

    if ((d_r > 0.0) && (d_g > 0.0)) {
        d_h = d_g/(d_r+d_g) * 3.14157*2/3; /* 0..2/3*PI, red to green */
    } else if ((d_g > 0.0) && (d_b > 0.0)) {
        d_h = (d_b/(d_g+d_b) + 1) * 3.14157*2/3; /* 2/3*PI..4/3*PI, green to blue */
    } else if ((d_b > 0) && (d_r > 0)) {
        d_h = (d_r/(d_b+d_r) + 2) * 3.14157*2/3; /* 4/2*PI..2*PI, blue to red */
    } else if (d_r > 0) {
        d_h = 0; /* Pure red */
    } else if (d_g > 0) {
        d_h = 3.14157*2/3; /* Pure green */
    } else if (d_b > 0) {
        d_h = 3.14157*4/3; /* Pure blue */
    } else { /* Default, should never happen */
        qDebug() << "Invalid color";
        d_h = 0;
        d_s = 0;
        d_i = 0;
    }
    hsi.h = d_h;
    hsi.s = d_s;
    hsi.i = d_i;
#endif
    qDebug() << "conv rgb->hsi, RGB" << rgb.r << rgb.g << rgb.b << " hsi:" << hsi.h << hsi.s << hsi.i;
    double d_r = ((double)rgb.r)/255.0;
    double d_g = ((double)rgb.g)/255.0;
    double d_b = ((double)rgb.b)/255.0;
    d_r = ((d_r > 1.0)?1.0:((d_r < 0.0)?0.0:d_r));
    d_g = ((d_g > 1.0)?1.0:((d_g < 0.0)?0.0:d_g));
    d_b = ((d_b > 1.0)?1.0:((d_b < 0.0)?0.0:d_b));
    qDebug() << "rgb norm" << d_r << d_g << d_b;

    if (d_r + d_g + d_b < 1e-5) {
        /* Black */
        hsi.h = hsi.s = hsi.i = 0;
        return;
    }

    double d_min = ((d_r < d_b)?((d_r < d_g)?d_r:d_g):((d_b < d_g)?d_b:d_g));
    double d_max = ((d_r > d_b)?((d_r > d_g)?d_r:d_g):((d_b > d_g)?d_b:d_g));
    qDebug() << "min/max" << d_min << d_max;
    if (d_max - d_min < 1e-5) {
        /* White/grey */
        hsi.h = hsi.s = 0;
        hsi.i = d_max;
        return;
    }

    double d_h, d_s, d_i;
    if ((d_r >= d_b) && (d_g >= d_b)) { /* red to green */
        qDebug() << "r->g";
        d_h = (d_g-d_b)/(d_r+d_g-2*d_b)*2.09439;
        if (fabs(d_r-d_b*d_h/2.09439) >= 1e-5) {
            d_s = (d_r-d_b)/(d_r-d_b*d_h/2.09439);
            d_i = d_b/(1-d_s);
        } else {
            d_s = 1-(d_r/d_g);
            d_i = d_g;
        }
    } else if ((d_g >= d_r) && (d_b >= d_r)) { /* green to blue */
        qDebug() << "g->b";
        d_h = ((d_b-d_r)/(d_g+d_b-2*d_r))*2.09439;
        if (fabs(d_r-d_g*d_h/2.09439) >= 1e-5) {
            d_s = (d_g-d_r)/(d_g-d_r*d_h/2.09439);
            d_i = d_r/(1-d_s);
        } else {
            d_s = 1-(d_g/d_b);
            d_i = d_b;
        }
        d_h += 2.09439;
    } else { /* blue to red */
        d_h = ((d_r-d_g)/(d_b+d_r-2*d_g))*2.09439;
        if (fabs(d_r-d_g*d_h/2.09439) >= 1e-5) {
            d_s = (d_b-d_g)/(d_b-d_g*d_h/2.09439);
            d_i = d_g/(1-d_s);
        } else {
            d_s = 1-(d_b/d_r);
            d_i = d_r;
        }
        d_h += 2*2.09439;
    }
    qDebug() << "HSI old" << hsi.h << hsi.s << hsi.i;
    qDebug() << "HSI new" << d_h << d_s << d_i;
    hsi.h = d_h;
    hsi.s = d_s;
    hsi.i = d_i;
}

void QVColor::rgbwToHsi(rgbw_t rgbw,hsi_t &hsi) {
    qDebug() << "conv rgbw->hsi, RGBW" << rgbw.r << rgbw.g << rgbw.b << rgbw.w << " hsi:" << hsi.h << hsi.s << hsi.i;
    double d_r = ((double)rgbw.r)/255.0;
    double d_g = ((double)rgbw.g)/255.0;
    double d_b = ((double)rgbw.b)/255.0;
    double d_w = ((double)rgbw.w)/255.0;
    d_r = ((d_r > 1.0)?1.0:((d_r < 0.0)?0.0:d_r));
    d_g = ((d_g > 1.0)?1.0:((d_g < 0.0)?0.0:d_g));
    d_b = ((d_b > 1.0)?1.0:((d_b < 0.0)?0.0:d_b));
    d_w = ((d_w > 1.0)?1.0:((d_w < 0.0)?0.0:d_w));
    qDebug() << "rgbw norm" << d_r << d_g << d_b << d_w;

    if (d_r + d_g + d_b + d_w < 1e-5) {
        /* Black */
        hsi.h = hsi.s = hsi.i = 0;
        return;
    }

    double d_min = ((d_r < d_b)?((d_r < d_g)?d_r:d_g):((d_b < d_g)?d_b:d_g));
    d_w += 3*d_min;
    d_r -= d_min;
    d_g -= d_min;
    d_b -= d_min;

    if (d_r + d_g + d_b < 1e-5) {
        /* White/grey */
        hsi.h = hsi.s = 0;
        hsi.i = (d_w <= 1.0)?d_w:1.0;
        return;
    }

    double d_h, d_i;
    double d_s = (d_r + d_g + d_b)/(d_r + d_g + d_b + d_w);
    if (d_b < 1e-5) { /* red to green */
        d_h = (d_g-d_b)/(d_r+d_g-2*d_b)*2.09439;
    } else if (d_r < 1e-5) { /* green to blue */
        d_h = ((d_b-d_r)/(d_g+d_b-2*d_r) + 1)*2.09439;
    } else {
        d_h = ((d_r-d_g)/(d_b+d_r-2*d_g) + 2)*2.09439;
    }
    d_i = d_r+d_g+d_b+d_w;

    qDebug() << "RGBW->HSI old" << hsi.h << hsi.s << hsi.i;
    qDebug() << "RGBW->HSI new" << d_h << d_s << d_i;
    hsi.h = d_h;
    hsi.s = d_s;
    hsi.i = (d_i < 1.0)?d_i:1.0;
}

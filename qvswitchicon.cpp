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
 * Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugänglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-sa/3.0/de/ oder wenden Sie
 * sich brieflich an
 * Creative Commons, Postfach 1866, Mountain View, California, 94042, USA.
 */

#include <QFile>
#include <QStringList>
#include <QRegularExpression>
#include "qvswitchicon.h"
#include "qvelement.h"

QVSwitchIcon::QVSwitchIcon(QDomElement e_item, QWidget *parent = 0) :
    QVIconWidget(parent)
{
    QString icon_file;
    QString color, active_color;
    QString color_mode;

    icon_file = e_item.text();
    color = e_item.attribute("color");
    active_color = e_item.attribute("active-color");
    color_mode = e_item.attribute("color-mode");

    init(icon_file,color,active_color,color_mode);
}

QVSwitchIcon::QVSwitchIcon(QString icon_file, QString color, QString active_color, QString s_color_mode, QWidget *parent = 0) :
  QVIconWidget(parent)
{
    init(icon_file,color,active_color,s_color_mode);
}

void QVSwitchIcon::init(QString icon_file, QString color, QString active_color, QString s_color_mode) {

    QStringList filenames;
    if (icon_file.isEmpty()) {
        filenames.append(":/icons/onoff.svg");
    } else {
        filenames = icon_file.split(",");
    }

    QFile f;
    f.setFileName(QVElement::findFilePath(filenames.at(0)));
    if (f.open(QIODevice::ReadOnly)) {
        icon_default = f.readAll();
        f.close();

        /* Try guessing the original color (for replacement purposes) */
        QRegularExpression color_match("stroke:\\s?(#[0-9a-fA-F]+)");
        QRegularExpressionMatch match = color_match.match(icon_default);
        if (!match.hasMatch()) {
            color_match.setPattern("fill:\\s?(#[0-9a-fA-F]+)");
            match = color_match.match(icon_default);
        }

        if (match.hasMatch()) {
            color_orig = match.captured(1);
        } else {
            color_orig.clear();
        }

        if (filenames.count() > 1) {
            f.setFileName(QVElement::findFilePath(filenames.at(1)));
            if (f.open(QIODevice::ReadOnly)) {
                icon_on = f.readAll();
                f.close();
            }
        }
    }

    if (icon_default.isEmpty()) {
        QFile f(":/icons/onoff.svg");
        if (f.open(QIODevice::ReadOnly)) {
            icon_default = f.readAll();
            f.close();
            color_orig = "#FFFFFF";
        }
    }

    icon_loaded = false;

    color_mode = ColorModeReplace;
    if (!s_color_mode.isEmpty()) {
        s_color_mode = s_color_mode.toLower();
        if (s_color_mode == "stroke") {
            color_mode = ColorModeStroke;
        } else if (s_color_mode == "fill") {
            color_mode = ColorModeFill;
        } else if (s_color_mode == "all") {
            color_mode = ColorModeAll;
        }
    }

    if (!color.isEmpty()) {
        color_default =  QVElement::colorString(color);
    } else {
        color_default = "#ffffff";
    }
    if (!active_color.isEmpty()) {
        color_on =  QVElement::colorString(active_color);
    } else {
        color_on = QVElement::colorString("orange");
    }
}

QSize QVSwitchIcon::defaultSize() const {
    if (icon_loaded || icon_default.isEmpty()) {
        return QVIconWidget::defaultSize();
    }
    QDomDocument d_svg;
    d_svg.setContent(icon_default);
    QDomElement e_svg = d_svg.documentElement();
    if (e_svg.nodeName().toLower() != "svg") {
        return QSize(0,0);
    }
    QRegularExpression re("^([0-9]+).*$");
    QRegularExpressionMatch match_w, match_h;
    match_w = re.match(e_svg.attribute("width"));
    match_h = re.match(e_svg.attribute("height"));
    if (!match_w.hasMatch() || !match_h.hasMatch()) {
        return QSize(0,0);
    }
    return QSize(match_w.captured(1).toInt(),match_h.captured(1).toInt());
}

QSize QVSwitchIcon::sizeHint() const {
    return defaultSize();
}

void QVSwitchIcon::setStatus(bool status) {

    qDebug() << "setStatus" << icon_on.length() << icon_default.length() << color_default << color_on << (void*)this;
    /* Two icons given */
    if (!icon_on.isEmpty()) {
        if (!status) {
            QVIconWidget::load(icon_default);
        } else {
            QVIconWidget::load(icon_on);
        }
        icon_loaded = true;
        return;
    }

    /* Only one icon */
    QString new_color;
    if (status) {
        new_color = color_on;
    } else {
        new_color = color_default;
    }

    switch (color_mode) {
    case ColorModeStroke:
        if (!icon_loaded) {
            load(icon_default);
            icon_loaded = true;
        }
        setPathStyle("stroke",new_color);
        qDebug() << "path to" << new_color;
        break;
    case ColorModeFill:
        if (!icon_loaded) {
            load(icon_default);
            icon_loaded = true;
        }
        setPathStyle("fill",new_color);
        break;
    case ColorModeAll:
        if (!icon_loaded) {
            load(icon_default);
            icon_loaded = true;
        }
        setPathStyle("stroke",new_color);
        setPathStyle("fill",new_color);
        break;
    default:
        qDebug() << "Replacing" << color_orig << new_color;
        if (!color_orig.isEmpty()) {
            load(QString(icon_default).replace(color_orig,new_color).toUtf8());
            icon_loaded = true;
        }
        break;
    }

}

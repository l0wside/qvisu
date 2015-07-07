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

#ifndef QVSWITCHICON_H
#define QVSWITCHICON_H

#include <QtXml>
#include <QByteArray>
#include "qviconwidget.h"

class QVSwitchIcon : public QVIconWidget
{
    Q_OBJECT
public:
    explicit QVSwitchIcon(QDomElement e_item, QWidget *parent);
    explicit QVSwitchIcon(QString icon_file, QString color, QString active_color, QString s_color_mode, QWidget *parent);

    const static int ColorModeReplace = 0;
    const static int ColorModeStroke = 1;
    const static int ColorModeFill = 2;
    const static int ColorModeAll = 3;

    void setColorMode(int);
    QSize defaultSize() const;
    QSize sizeHint() const;

signals:

public slots:
    void setStatus(bool);

private:
    QByteArray icon_default, icon_on;
    bool icon_loaded;
    int color_mode;
    QString color_orig, color_default, color_on;

    void init(QString icon_file, QString color, QString active_color, QString s_color_mode);
};

#endif // QVSWITCHICON_H

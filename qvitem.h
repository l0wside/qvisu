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

#ifndef QVITEM_H
#define QVITEM_H

#include <QtXml>
#include <QString>
#include <QMap>

class QVItem {
public:
    QVItem(QDomElement item_elem, int index = 0);
    QVItem();

    QString getItemName();
    QString getAction();
    QString getAttribute(QString attribute);
    QString toString();
    int getIndex() { return action_index; }

private:
    QString item;
    QString action;
    QMap<QString,QString> attributes;

    int action_index;
};

#endif // QVITEM_H

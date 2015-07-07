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

#include "qvitem.h"

QVItem::QVItem(QDomElement item_elem,int index) {
    item = item_elem.text();
    action = item_elem.attribute("action");
    QDomNamedNodeMap attr_list = item_elem.attributes();
    for (int n=0; n < attr_list.count(); n++) {
        if (attr_list.item(n).toAttr().name() == "action") {
            continue;
        }
        attributes.insert(attr_list.item(n).toAttr().nodeName(),attr_list.item(n).toAttr().nodeValue());
    }
    this->action_index = index;
}

QVItem::QVItem() { }

QString QVItem::getItemName() {
    return item;
}

QString QVItem::getAction() {
    return action;
}

QString QVItem::getAttribute(QString attribute) {
    return attributes.find(attribute).value();
}

QString QVItem::toString() {
    return action + "[" + QString::number(action_index) + "] -> " + item;
}

#ifndef Color_H
#define Color_H

#include <QMetaType>
#include <QVariant>

#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"

namespace lc {

    class Color;

    QDataStream& operator<<(QDataStream&, const Color&);
    QDataStream& operator>>(QDataStream&, Color&);

    class Color : public MetaType {
        public:
            enum Method {
                Invalid = -1,
                ByLayer = 0,
                ByEntity = 1
            };

            Color();
            Color(const int r, const int g, const int b, const int a = 255);

            Color(Method method);

            Color::Method method() const;
            int red() const;
            int green() const;
            int blue() const;
            int alpha() const;

            QColor qColor() const;

            /*
            friend QDataStream& operator<<( QDataStream & stream, const Color & yourObject ) {
                stream << (qint8)yourObject._method << (qint16)yourObject._r << (qint16)yourObject._g << (qint16)yourObject._b << (qint16)yourObject._a;
                return stream;
            }

            friend QDataStream& operator>>( QDataStream & stream, Color & yourObject ) {
                stream >> (qint8&)yourObject._method;
                stream >> (qint16&)yourObject._r;
                stream >> (qint16&)yourObject._g;
                stream >> (qint16&)yourObject._b;
                stream >> (qint16&)yourObject._a;
                return stream;
            }
            */
            operator QVariant() const {
                return QVariant::fromValue(*this);
            }

            bool variantValid() {
                return !(_method == Color::Invalid);
            }

            int metaName() {
                return CONST::COLOR;
            }

        private:
            Method _method;
            int _r;
            int _g;
            int _b;
            int _a;
    };

    /*
    inline uint qHash( const Color &key)
    {
        return qHash(QString("%1_%2_%3_%4").arg(key.red()).arg(key.green()).arg(key.blue()).arg(key.alpha()));
    }*/


}

Q_DECLARE_METATYPE(lc::Color)

#endif // Color_H

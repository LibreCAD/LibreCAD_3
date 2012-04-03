#ifndef LINEWIDTH_H
#define LINEWIDTH_H

#include <QMetaType>
#include <QVariant>
#include <QString>
#include <QHash>

#include "cad/const.h"

#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"

namespace lc {

    class LineWidth;

    QDataStream& operator<<(QDataStream&, const LineWidth&);
    QDataStream& operator>>(QDataStream&, LineWidth&);

    class LineWidth : public MetaType {
        public:
            enum Method {
                Invalid = -1,
                ByLayer = 0,
                ByEntity = 1
            };


            LineWidth();
            LineWidth(const double width);
            LineWidth(LineWidth::Method method);
            virtual ~LineWidth() {}
            Method method() const;
            double width() const;

            /*
                        friend QDataStream& operator<<( QDataStream & stream, const LineWidth & yourObject ) {
                            stream << (qint8)yourObject._method << (double)yourObject._width;
                            return stream;
                        }

                        friend QDataStream& operator>>( QDataStream & stream, LineWidth & yourObject ) {
                            stream >> (qint8&)yourObject._method;
                            stream >>  (double&)yourObject._width;
                            return stream;
                        }

                        bool  operator==(const LineWidth &other) const
                        {
                            // Objects are by entity, the width must be the same
                            if (other._method==LineWidth::ByEntity && this->_method==LineWidth::ByEntity) {
                                return this->_width==other._width;
                            }

                            // If not by entity the methods only must be the same
                            return other._method==this->_method;
                        } */

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }


            bool variantValid() const {
                return !(_method == LineWidth::Invalid);
            }

            int metaName() const {
                return CONST::LINEWIDTH;
            }

        private:
            Method _method;
            double _width;
    };

    /*
    inline uint qHash( const LineWidth &key)
    {
        return qHash(QString("%1_%2").arg(key.width()).arg(key.method()));
    } */


}

Q_DECLARE_METATYPE(lc::LineWidth)



#endif // LINEWIDTH_H

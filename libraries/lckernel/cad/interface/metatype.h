#ifndef METATYPE_H
#define METATYPE_H


/**
  * If a object is a meta object, eg something that can be attached to a entity then this object must extend MetaType
  * and provide a unique meta name
  */
namespace lc {
    class MetaType  {
        public:
            MetaType();
            inline virtual int metaName() = 0;
    };
}
#endif // METATYPE_H

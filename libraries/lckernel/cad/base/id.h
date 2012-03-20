#ifndef ID_H
#define ID_H



#define UNIQUE_CAD_ID unsigned long


/**
  * Class that will provide a unique ID for the lifetime of this object
  * There can only be one ID
  *
  */
namespace lc {
    class ID {
        public:
            ID();
            UNIQUE_CAD_ID id() const;

        private:
            unsigned long _id;
    };
}
#endif // ID_H

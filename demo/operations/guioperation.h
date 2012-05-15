#ifndef OPERATION_H
#define OPERATION_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QGraphicsView>

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"

#include "guioperationfinishedevent.h"


#include "const.h"
/**
  * \brief Interface for all operations
  *
  * A operation is a task execute using document::operateOn(..). All 'creation' operaions will needs to extend this base class
  *
  * \sa OperationManager
  *
  * \author R. van Twisk <librecad@rvt.dds.nl>
  */
class GuiOperation : public QObject {
        Q_OBJECT
    public:
        /*!
          * \brief create the CAD entity with the additional meta data
          */
        virtual std::tr1::shared_ptr<const lc::CADEntity> cadEntity(const QList<std::tr1::shared_ptr<const lc::MetaType> >& metaTypes) const = 0;

        /*!
          * \brief restart this operation
          */
        virtual void restart() = 0;

        /*!
          * \brief OPerationManager will call this function to continue the current operation.
          *
          * It allows for a line to continue and connect start/end points automatically.
          *
          */
        virtual std::tr1::shared_ptr<GuiOperation> next() const = 0 ;
    public:
    signals:

        /*!
          * \brief Signal that a operation needs to emit once the operation is been completed
          */
        void guiOperationFinished(const GuiOperationFinishedEvent&);
};


#endif // OPERATION_H

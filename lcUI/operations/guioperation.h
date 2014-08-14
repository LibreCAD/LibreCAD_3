#pragma once

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QGraphicsView>

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/operations/documentoperation.h"



class GuiOperationFinishedEvent;

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
class GuiOperation;
typedef std::shared_ptr<GuiOperation> GuiOperation_SPtr;
typedef std::shared_ptr<const GuiOperation> GuiOperation_CSPtr;
class GuiOperation : public QObject {
        Q_OBJECT
    public:
        GuiOperation(lc::Document* document) : _document(document) {}
        /*!
          * \brief create the CAD entity with the additional meta data
          */
        virtual lc::operation::DocumentOperation_SPtr operation() const = 0;

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
        virtual GuiOperation_SPtr next() const = 0 ;
    protected:
        lc::Document* document() const {
            return _document;
        }

    public:
    signals:

        /*!
          * \brief Signal that a operation needs to emit once the operation is been completed
          */
        void guiOperationFinished(const GuiOperationFinishedEvent&);
    private:
        lc::Document* _document;
};

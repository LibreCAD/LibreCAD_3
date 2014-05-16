#ifndef OperationManager_H
#define OperationManager_H

#include <QObject>
#include <QStack>
#include "guioperation.h"
#include "cad/document/document.h"

/**
  * \brief OperationManager manages a set of operations that are currently active within the document
  *
  * Ususally one only operation is active and when finnished it will be comitted. However some operations like polyline
  * will have several operations active before the final commit
  *
  */
class OperationManager : QObject {
        Q_OBJECT
    public:
        OperationManager(lc::Document* document);

        /*!
          * \brief cancel all current operations
          *
          */
        void cancel();

        /*!
         * \brief add's a operation to the manager
         *
         *
         */
        void startOperation(GuiOperation_SPtr operation);

        /**
          * \brief call this function to send a cancel signal to the current operation
          *
          * This will instruct the current operation to undo the current operation, however
          * it depend on the operation in question what the prcise action is, wand what makes sense for
          * his context.
          * ususally this function will be called when the user pressed the esc key.
          *
          * \return boolean returns true if there where operations on the stack
          *
          * \author R. van Twisk <librecad@rvt.dds.nl>
          */
        void restart();

        friend class GroupOperations;
    private:

        QStack<GuiOperation_SPtr > activeOperations() const;

    private slots:
        /*!
          * \brief this function is called internally and will commit all operations on the _activeGuiOperations to the document
          *
          * \param GuiOperationFinishedEvent Operation Finnished event
          *
          * \author R. van Twisk <librecad@rvt.dds.nl>
          */
        void on_guioperationFinished_Event(const GuiOperationFinishedEvent&);

    private:
        // Created as a stack of operation because LibreCAD 2.x has that. However, not sure where this is used exactly
        // it might aswell be that later this is going ot be changed to one single Operation that can be active
        QStack<GuiOperation_SPtr > _activeGuiOperations;
        lc::Document* _document;
};

#endif // OperationManager_H

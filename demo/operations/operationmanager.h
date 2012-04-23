#ifndef OperationManager_H
#define OperationManager_H

#include <QObject>
#include <QStack>
#include "operation.h"
#include "cad/document/abstractdocument.h"

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
        OperationManager(lc::AbstractDocument* document);

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
        void startOperation(OperationPtr operation);

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

        QStack<OperationPtr> activeOperations() const;
        void setActiveOperations(QStack<OperationPtr> operations);

    private slots:
        /*!
          * \brief this function is called internally and will commit all operations on the _activeOperations to the document
          *
          * \param OperationFinishedEvent Operation Finnished event
          *
          * \author R. van Twisk <librecad@rvt.dds.nl>
          */
        void on_operationFinished_Event(const OperationFinishedEvent&);

    private:
        QStack<OperationPtr> _activeOperations;
        lc::AbstractDocument* _document;
};

typedef shared_ptr<OperationManager> OperationManagerPtr;

#endif // OperationManager_H

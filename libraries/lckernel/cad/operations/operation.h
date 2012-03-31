#ifndef OPERATIONINTERFACE_H
#define OPERATIONINTERFACE_H

namespace lc {

    class AbstractDocument;

    class Operation {
        public:
            Operation(AbstractDocument* document) {
                _document = document;
            };
            AbstractDocument* document() const {
                return _document;
            };
        private:

            Q_DISABLE_COPY(Operation)

            /**
             * This function get's called when a operation starts and when the document is locked for you
             * so you can do your work
             */
            virtual void process() const = 0;

            /**
             * This function will get called when the process of this operation starts
             */
            virtual void start() const {};
            /**
             * This function will get called when the process of this operation is finnished
             */
            virtual void finnish() const {};

            friend class AbstractDocument;

            AbstractDocument* _document;
    };
}
#endif // OPERATIONINTERFACE_H

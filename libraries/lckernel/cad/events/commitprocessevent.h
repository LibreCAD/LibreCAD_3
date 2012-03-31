#ifndef COMMITPROCESSEVENT_H
#define COMMITPROCESSEVENT_H


namespace lc {

    class Operation;

    /**
     * Event that get's emitted when a operation has been comitted
     */
    class CommitProcessEvent {
        public:
            CommitProcessEvent(Operation* operation) {
                _operation = operation;
            };

            Operation* operation() {
                return _operation;
            }
        private:
            Operation* _operation;
    };
}
#endif // COMMITPROCESSEVENT_H

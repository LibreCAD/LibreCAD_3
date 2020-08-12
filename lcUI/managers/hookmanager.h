#pragma once
#include <QKeyEvent>
#include <QWidget>
#include <tuple>

// The hook manager dispatches the events to only selected targets

namespace lc{
    namespace ui{
        class HookEvent;

        typedef std::function<bool(HookEvent&)> hookCallback;
        /**
         * @brief This class is send as paramater at dispatch
         * 
         */
        class HookEvent {
            friend class HookManager;
            public:
                std::string eventType;
                QKeyEvent* keyEvent;
                QMouseEvent * mouseEvent;
                /**
                 * @brief Clear events
                 * May affect others if not hooked
                 * 
                 */
                void clear();
                /**
                 * @brief Grab the hook
                 * Next time call this handler only
                 */
                void grab();
                /**
                 * @brief Free the hook
                 * 
                 */
                void free();
            private:
                /**
                 * @brief pointer to function of current handler
                 * 
                 */
                hookCallback dispachedTo = nullptr;

                /**
                 * @brief bool so something like e.preventDefault
                 * Pointer to function where it should be dispached
                 * nullptr means all
                 * no _, since used in HookManager
                 */
                hookCallback nextDispatch =nullptr;
        };

        /**
         * @brief Manager to dispach events
         * This should be provided with event from lcadviewer
         * This should be memeber of lcadviewer for now
         */
        class HookManager{
            public:
                /**
                 * @brief called when a key is pressed
                 * 
                 */
                void onKeyEvent(QKeyEvent*);
                /**
                 * @brief called when mouse is clicked or released
                 * 
                 */
                void onMouseEvent(std::string, QMouseEvent*);
                
                /**
                 * @brief Add function to array
                 * 
                 * @param name Name of handler
                 * @param cb Callback for it
                 */
                void append(std::string name, hookCallback cb);
                
                /**
                 * @brief Select state
                 * more like select hook
                 * @param name 
                 */
                void selectState(std::string name);

            private:
                void dispatch();
                HookEvent hookEvent;
                std::map<std::string, hookCallback> _handleVector;
        };
    }
}

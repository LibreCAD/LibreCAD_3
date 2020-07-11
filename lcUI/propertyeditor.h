#include "widgets/guiAPI/dialogwidget.h"

namespace lc
{
    namespace ui
    {
        class PropertyEditor
        {
        public:
            static PropertyEditor* GetPropertyEditor();
        private:
            PropertyEditor();

        private:
            static PropertyEditor* instance;
        };
    }
}

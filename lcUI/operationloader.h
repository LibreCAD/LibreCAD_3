#include <QMainWindow>
#include <kaguya/kaguya.hpp>

#include <set>
#include <map>

namespace lc 
{
    namespace ui 
    {
        /**
         * \brief Lua Operations Loader
         */
        class OperationLoader
        {
        public:
            /**
             * \brief Constructor for OperationLoader
             * \param luaPath string path to lcUI folder
             * \param qmainWindow QMainWindow which will be casted to MainWindow
             * \param luaState lua state
             */
            OperationLoader(const std::string& luaPath, QMainWindow* qmainWindow, kaguya::State& luaState);

            /**
             * \brief Load operations from lua scripts and run init functions
             * \param luaPath string path to lcUI folder
             */
            void loadLuaOperations(const std::string& luaPath);

            /**
             * \brief Initialize each operation on the basis of found properties
             * \param vkey - Operation key for state table
             */
            void initializeOperation(const std::string& vkey);

            /**
             * \brief Add cli command for operation
             * \param vkey - Operation key for state table Eg. "LineOperations"
             * \param opkey - Key for operation property Eg. "command_line"
             */
            void addOperationCommandLine(const std::string& vkey, const std::string& opkey);

            /**
             * \brief Add menu action for operation
             */
            void addOperationMenuAction(const std::string& vkey, const std::string& opkey);

            /**
             * \brief Add toolbar icon for operation
             */
            void addOperationIcon(const std::string& vkey, const std::string& opkey);

            /**
             * \brief Add extra icons in Current operation group of toolbar when operation is run
             */
            void addOperationToolbarOptions(const std::string& vkey, const std::string& opkey);

            /**
             * \brief Add operation to context menu
             */
            void addContextMenuOperations(const std::string& vkey);

        private:
            /**
             * \brief Load lua folder eg, createActions
             * \param folderName - name of the folder
             * \param fileToSkip - skip this file (createOperations.lua in createActions, operations.lua in actions)
             * \param luaPath string path to lcUI folder
             */
            void loadLuaFolder(const std::string folderName, const std::string& fileToSkip, const std::string& luaPath);

            /**
             * \brief Determine which group each operation is in and store in groupElements
             */
            void getSetOfGroupElements();

        private:
            kaguya::State& _L;
            QMainWindow* qmainWindow;

            std::map<std::string, std::set<std::string>> groupElements;
            std::map<std::string, std::set<std::string>> foundProperties;
            std::map<std::string, std::string> groupNames;
        };
    }
}

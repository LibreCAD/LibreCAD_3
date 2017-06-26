#pragma once


#include "block.h"

namespace lc {
    /**
     * @brief Class for storage of components of custom entities
     * Child of Block for compatibility with other CAD software
     */
    class CustomEntityStorage : public Block {
        public:
            CustomEntityStorage(const std::string& pluginName, const std::string& entityName, const geo::Coordinate& base);

            const std::string& pluginName() const;
            const std::string& entityName() const;

            void setParam(const std::string& param, const std::string& value);
            std::string param(const std::string& param) const;

        private:
            std::string _pluginName;
            std::string _entityName;
            std::map<std::string, std::string> _params;
    };

    DECLARE_SHORT_SHARED_PTR(CustomEntityStorage);
}
#pragma once


#include "block.h"

namespace lc {
    namespace meta {
        class CustomEntityStorage; //TODO: rename, as this does not store entities

        DECLARE_SHORT_SHARED_PTR(CustomEntityStorage);

        /**
         * @brief Class for storage of components of custom entities
         * Child of Block for compatibility with other CAD software
         */
        class CustomEntityStorage : public Block {
            public:
                CustomEntityStorage(std::string pluginName,
                                    std::string entityName,
                                    geo::Coordinate base,
                                    std::map<std::string, std::string> params = std::map<std::string, std::string>()
                );

                const std::string& pluginName() const;

                const std::string& entityName() const;

                CustomEntityStorage_CSPtr setParam(const std::string& param, const std::string& value) const;

                std::string param(const std::string& param) const;

                const std::map<std::string, std::string>& params() const;

            private:
                std::string _pluginName;
                std::string _entityName;
                std::map<std::string, std::string> _params;
        };
    }
}
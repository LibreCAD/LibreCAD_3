#include "uitests.h"

storage::Document_SPtr createDocument() {
    auto storageManager = std::make_shared<storage::StorageManagerImpl>();
    return std::make_shared<storage::DocumentImpl>(storageManager);
};

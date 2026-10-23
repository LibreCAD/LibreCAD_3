#pragma once
#include <rhi/qshader.h>

// Internal trusted-asset validation, shared with CPU negative tests. No drawing
// or public API accepts a shader filename or untrusted serialized shader bytes.
namespace lc { namespace rhi { namespace detail {
QShader checkedShader(const QByteArray& bytes, QShader::Stage stage);
QShader resourceShader(const char* resource, QShader::Stage stage);
void initializeResources();
}}}

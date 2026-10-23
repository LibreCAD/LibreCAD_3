#pragma once

#include <QColor>
#include <QImage>
#include <QRectF>
#include <rhi/qrhi.h>
#include <memory>
#include <vector>

// R0-only renderer: not an LcPainter implementation or production geometry API.
class SpikeScene {
public:
    void render(QRhi *rhi, QRhiRenderTarget *target, QRhiCommandBuffer *cb, bool transparent = false);
    int vertexCount() const { return int(vertices.size()); }
    qint64 uploadedBytes() const { return uploadBytes; }
private:
    struct Vertex { float x, y, r, g, b, a, u, v; };
    void buildFixture();
    void quad(float x, float y, float w, float h, QColor color,
              QRectF uv = QRectF(0.5 / 32, 0.5 / 16, 0, 0));
    std::vector<Vertex> vertices;
    QImage atlas;
    QRhi *device = nullptr;
    QRhiRenderPassDescriptor *pass = nullptr;
    int samples = 0;
    qint64 uploadBytes = 0;
    std::unique_ptr<QRhiBuffer> vertexBuffer, uniformBuffer;
    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiSampler> sampler;
    std::unique_ptr<QRhiShaderResourceBindings> bindings;
    std::unique_ptr<QRhiGraphicsPipeline> pipeline;
};

QImage normalizedReadback(const QRhiReadbackResult &result, bool yUp);

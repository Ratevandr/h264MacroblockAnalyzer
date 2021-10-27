#include "opencvimageprovider.hpp"

opencvImageProvider::opencvImageProvider(QObject *parent) : QObject(parent),
    QQuickImageProvider(QQuickImageProvider::Image)
{
    _image = QImage();
}

QImage opencvImageProvider::requestImage(const QString &id, QSize *size, const QSize& requestedSize) {
    Q_UNUSED(id)

    if (size) {
        *size = _image.size();
    }

    if (!requestedSize.isEmpty()) {
        _image = _image.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio);
    }
    return _image;

}

void opencvImageProvider::updateImage(const QImage &image) {
    if (!image.isNull() && _image != image) {
        _image = image;
        emit imageChanged();
    }
}

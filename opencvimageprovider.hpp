#ifndef OPENCVIMAGEPROVIDER_H
#define OPENCVIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>

class opencvImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    opencvImageProvider(QObject *parent = nullptr);

    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize) override;

signals:
    void imageChanged();

public slots:
    void updateImage(const QImage &image);

private:
    QImage _image;
};

#endif // OPENCVIMAGEPROVIDER_H

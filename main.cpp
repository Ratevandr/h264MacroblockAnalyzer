#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "framestream.hpp"
#include "opencvimageprovider.hpp"
#include <memory>
#include <QQmlContext>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    FrameStream frameStream;
    opencvImageProvider *cvImageProv= new opencvImageProvider();



    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("FrameStream", &frameStream);
    engine.rootContext()->setContextProperty("cvImageProv", cvImageProv);
    engine.addImageProvider("live",cvImageProv);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    QObject::connect(&frameStream, &FrameStream::newImage, cvImageProv, &opencvImageProvider::updateImage);

    return app.exec();
}

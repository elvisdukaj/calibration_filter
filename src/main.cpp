#include "thresholdfilter.h"
#include "cannyfilter.h"
#include "calibrationfilter.h"
#include "markerdetectorfilter.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
	
    qmlRegisterType<ThresholdFilter>("com.qubicaamf.vision", 1, 0, "ThresholdFilter");
    qmlRegisterType<CannyFilter>("com.qubicaamf.vision", 1, 0, "CannyFilter");
    qmlRegisterType<CalibrationFilter>("com.qubicaamf.vision", 1, 0, "CalibrationFilter");
    qmlRegisterType<MarkerDetectorFilter>("com.qubicaamf.vision", 1, 0, "MarkerDetectorFilter");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

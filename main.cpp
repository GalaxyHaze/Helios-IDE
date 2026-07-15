#include <QApplication>
#include "editor/core/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Helios");
    app.setOrganizationName("Helios");
    app.setWindowIcon(createHeliosIcon());

    MainWindow mainWin;
    mainWin.show();

    return app.exec();
}

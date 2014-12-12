#include <QApplication>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QtGui>
#include <QTranslator>

#include "mainwindow.h"
#include "pmsettings.h"
#include "logindialog.h"

int main(int argc, char *argv[])
{
QApplication app(argc, argv);


// Load style sheet
QFile styleFile( qApp->applicationDirPath() + "/qss/lhc.qss" );
styleFile.open( QFile::ReadOnly );

// Apply the loaded stylesheet
QString style( styleFile.readAll() );
app.setStyleSheet( style );

// Load settings from different location
QString settingsFilename = qApp->applicationDirPath() + "/settings.xml";
if(argc > 1)
{
    settingsFilename = QString::fromUtf8(argv[1]);
}

qApp->addLibraryPath( qApp->applicationDirPath() + "/plugins");



QTranslator translator;

PMSettings *pmsettings = new PMSettings(settingsFilename);
translator.load("translations/lhc_"+pmsettings->getAttributeSettings("language")+".qm");
delete pmsettings;

app.installTranslator(&translator);

//Inicijuojam DB
QCoreApplication::setOrganizationName("Remdex");
QCoreApplication::setOrganizationDomain("remdex.info");
QCoreApplication::setApplicationName("Live helper chat");

LoginDialog *lgnDialog = new LoginDialog(0,true,&settingsFilename);

if(!lgnDialog->exec())
{
    QTimer::singleShot(250, qApp, SLOT(quit())); 
} 

delete lgnDialog;


MainWindow *w = new MainWindow(settingsFilename);
w->show();
app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
	return app.exec();

}

/*
  Q Light Controller
  main.cpp

  Copyright (C) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QApplication>
#include <QTextStream>
#include <QTranslator>
#include <QMetaType>
#include <QtGlobal>
#include <QVariant>
#include <QLocale>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QHash>
#include <QDir>
#include <QDialog>
#include <QPushButton>
#include <QCloseEvent>
#include <QFileDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QScreen>
#include <QFileInfo>
#include <QFile>
#include <QLabel>
#include <QCheckBox>
#include <QShortcut>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "ui_scelta_directory.h"
#include "qlcconfig.h"
#include "qlci18n.h"
#include "qlcfile.h"
#include "qlcfixturedefcache.h"
#include "qvrcinfo.h"
#include "vcdockarea.h"
#include "vcframe.h"

#if defined(WIN32) || defined(__APPLE__)
  #include "debugbox.h"
#endif

#include "virtualconsole.h"
#include "simpledesk.h"
#include "webaccess.h"
#include "webaccessauth.h"
#include "app.h"
#include "doc.h"

/* Use this namespace for command-line arguments so that we don't pollute
   the global namespace. */
namespace QLCArgs
{
    /**
     * If true, switch to operate mode after ALL initialization is done.
     */
    bool operate = false;

    /**
     * Specifies a workspace file name to load after all initialization
     * has been done, but before switching to operate mode (if applicable)
     */
    QString workspace;

    /** If true, enables kiosk-mode (Operate mode locked, only virtual console) */
    bool kioskMode = false;

    /** If true, opens the application in full screen mode */
    bool fullScreen = false;

    /** If true, adjusts the main window geometry instead of instructing the windowing system to "maximize" */
    bool fullScreenResize = false;

    /** If true, create and run a class to enable a web server for remote controlling */
    bool enableWebAccess = false;

    /** Number of a specific port to use for webaccess */
    int webAccessPort = 0;

    /** If true, the authentication feature of the web interface will be enabled */
    bool enableWebAuth = false;

    /** Path to passwords file for web access basic authentication */
    QString webAccessPasswordFile;

    /** If true, enable a 5% of overscan when in fullscreen mode (Raspberry Only) */
    bool enableOverscan = false;

    /** If true, the application will add extra controls to close windows */
    bool noWindowManager = false;

    /** If true, hides the GUI to 1x1 pixel outside the screen */
    bool noGui = false;

    /** If not null, defines the place for a close button that in virtual console */
    QRect closeButtonRect = QRect();

    /** Debug output level */
    QtMsgType debugLevel = QtCriticalMsg;

    /** Log to file flag */
    bool logToFile = false;

    QFile logFile;

#if defined(WIN32) || defined(__APPLE__)
    /** The debug windows for Windows and OSX */
    DebugBox *dbgBox = NULL;
#endif
}

/**
 * Suppresses debug messages
 */
void qlcMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    QByteArray localMsg = msg.toLocal8Bit();
    if (type >= QLCArgs::debugLevel)
    {
        if (QLCArgs::logToFile == true && QLCArgs::logFile.isOpen())
        {
            QLCArgs::logFile.write(localMsg);
            QLCArgs::logFile.write((char *)"\n");
            QLCArgs::logFile.flush();
        }
        fprintf(stderr, "%s\n", localMsg.constData());
        fflush(stderr);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define endl Qt::endl
#endif

/**
 * Prints the application version
 */
void printVersion()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);
    const QString appName = QCoreApplication::applicationName().isEmpty() ? QString(APPNAME) : QCoreApplication::applicationName();
    const QString appVersion = QCoreApplication::applicationVersion().isEmpty() ? QString(APPVERSION) : QCoreApplication::applicationVersion();

    cout << endl;
    cout << appName << " " << "version " << appVersion << endl;
    cout << "This program is licensed under the terms of the ";
    cout << "Apache 2.0 license." << endl;
    cout << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)" << endl;
    cout << "Copyright (c) Massimo Callegari (massimocallegari@yahoo.it)" << endl;
    cout << endl;
}

/**
 * Prints possible command-line options
 */
void printUsage()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << "Usage:";
    cout << "  qlcplus [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -c or --closebutton <x,y,w,h>\tPlace a close button in virtual console (only when -k is specified)" << endl;
    cout << "  -d or --debug <level>\t\tSet debug output level (0-3, see QtMsgType)" << endl;
    cout << "  -f or --fullscreen <method>\tStart the application in fullscreen mode (method is either 'normal' or 'resize')" << endl;
    cout << "  -g or --log\t\t\tLog debug messages to a file" << endl;
    cout << "  -h or --help\t\t\tPrint this help" << endl;
    cout << "  -k or --kiosk\t\t\tEnable kiosk mode (only virtual console in forced operate mode)" << endl;
    cout << "  -l or --locale <locale>\tForce a locale for translation" << endl;
    cout << "  -m or --nowm\t\t\tInform the application that the system doesn't provide a window manager" << endl;
    cout << "  -n or --nogui\t\t\tStart the application with the GUI hidden (requires --nowm)" << endl;
    cout << "  -o or --open <file>\t\tOpen the specified workspace file" << endl;
    cout << "  -p or --operate\t\tStart in operate mode" << endl;
    cout << "  -v or --version\t\tPrint version information" << endl;
    cout << "  -w or --web\t\t\tEnable remote web access" << endl;
    cout << "  -wp or --web-port <port>\t\tSet the port to use for web access" << endl;
    cout << "  -wa or --web-auth\t\tEnable remote web access with users authentication" << endl;
    cout << "  -a or --web-auth-file <file>\tSpecify a file where to store web access basic authentication credentials" << endl;
    cout << endl;
}

/**
 * Parse command line arguments
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 *
 * @return true to continue with application launch; otherwise false
 */
bool parseArgs()
{
    QStringListIterator it(QCoreApplication::arguments());
    while (it.hasNext() == true)
    {
        QString arg(it.next());

        if ((arg == "-c" || arg == "--closebutton") && it.hasNext() == true)
        {
            QString str(it.next());
            QStringList parts = str.split(",");
            if (parts.size() == 4)
            {
                QRect rect(parts[0].toInt(), parts[1].toInt(),
                           parts[2].toInt(), parts[3].toInt());
                if (rect.isValid() == true)
                    QLCArgs::closeButtonRect = rect;
            }
        }
        else if (arg == "-d" || arg == "--debug")
        {
            if (it.hasNext() == true)
                QLCArgs::debugLevel = QtMsgType(it.peekNext().toInt());
            else
                QLCArgs::debugLevel = QtMsgType(0);
        }
        else if (arg == "-g" || arg == "--log")
        {
            QLCArgs::logToFile = true;
            QString logFilename = QDir::homePath() + QDir::separator() + "QLC+.log";
            QLCArgs::logFile.setFileName(logFilename);
            if (!QLCArgs::logFile.open(QIODevice::Append))
            {
                qWarning() << "Could not open log file!";
                return false;
            }
        }
        else if (arg == "-f" || arg == "--fullscreen")
        {
            QLCArgs::fullScreen = true;
            if (it.hasNext() == true && it.peekNext() == "resize")
                QLCArgs::fullScreenResize = true;
        }
        else if (arg == "-r" || arg == "--overscan")
        {
            QLCArgs::enableOverscan = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            printUsage();
            return false;
        }
        else if (arg == "-k" || arg == "--kiosk")
        {
            QLCArgs::kioskMode = true;
        }
        else if (arg == "-l" || arg == "--locale")
        {
            if (it.hasNext() == true)
                QLCi18n::setDefaultLocale(it.next());
        }
        else if (arg == "-o" || arg == "--open")
        {
            if (it.hasNext() == true)
                QLCArgs::workspace = it.next();
        }
        else if (arg == "-m" || arg == "--nowm")
        {
            QLCArgs::noWindowManager = true;
        }
        else if (arg == "-n" || arg == "--nogui")
        {
            QLCArgs::noGui = true;
        }
        else if (arg == "-p" || arg == "--operate")
        {
            QLCArgs::operate = true;
        }
        else if (arg == "-w" || arg == "--web")
        {
            QLCArgs::enableWebAccess = true;
        }
        else if (arg == "-wp" || arg == "--web-port")
        {
            if (it.hasNext() == true)
                QLCArgs::webAccessPort = it.next().toInt();
        }
        else if (arg == "-wa" || arg == "--web-auth")
        {
            QLCArgs::enableWebAccess = true;
            QLCArgs::enableWebAuth = true;
        }
        else if (arg == "-a" || arg == "--web-auth-file")
        {
            if (it.hasNext())
                QLCArgs::webAccessPasswordFile = it.next();
        }
        else if (arg == "-v" || arg == "--version")
        {
            /* Don't print anything, since version is always
               printed before anything else. Just make the app
               exit by returning false. */
            return false;
        }
    }

    return true;
}

class MandatoryStartupDialog final : public QDialog
{
public:
    explicit MandatoryStartupDialog(QWidget *parent = nullptr)
        : QDialog(parent)
        , m_allowClose(false)
    {
    }

    void allowClose(bool enable)
    {
        m_allowClose = enable;
    }

protected:
    void closeEvent(QCloseEvent *event) override
    {
        if (m_allowClose == false)
        {
            event->ignore();
            return;
        }
        QDialog::closeEvent(event);
    }

    void reject() override
    {
        if (m_allowClose == false)
            return;
        QDialog::reject();
    }

private:
    bool m_allowClose;
};

static bool showStartupOpenWorkspaceDialog(App &app, bool forceDialog = false)
{
    struct StartupSettings
    {
        QString folderPath;
        QString workspacePath;
    };

    auto settingsFilePath = []() -> QString
    {
        QDir appDir(QApplication::applicationDirPath());
        return appDir.filePath("setting.opz");
    };

    auto readStartupSettings = [&settingsFilePath]() -> StartupSettings
    {
        StartupSettings settings;
        QFile file(settingsFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
            return settings;

        QTextStream stream(&file);
        while (stream.atEnd() == false)
        {
            const QString line = stream.readLine().trimmed();
            if (line.startsWith("folder=", Qt::CaseInsensitive))
                settings.folderPath = line.mid(QString("folder=").length()).trimmed();
            else if (line.startsWith("file=", Qt::CaseInsensitive))
                settings.workspacePath = line.mid(QString("file=").length()).trimmed();
        }

        return settings;
    };

    auto saveStartupSettings = [&settingsFilePath](const StartupSettings &settings) -> bool
    {
        QFile file(settingsFilePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) == false)
            return false;

        QTextStream stream(&file);
        stream << "folder=" << settings.folderPath << '\n';
        stream << "file=" << settings.workspacePath << '\n';
        return true;
    };

    auto openWorkspaceAndCloseStartup = [&app](const QString &workspaceFile) -> bool
    {
        if (workspaceFile.isEmpty())
            return false;

        if (QFile::exists(workspaceFile) == false)
            return false;

        app.clearDocument();
        if (app.loadXML(workspaceFile) != QFile::NoError)
            return false;

        app.updateFileOpenMenu(workspaceFile);
        app.slotGoToVirtualConsole();
        if (VirtualConsole::instance() != NULL && VirtualConsole::instance()->dockArea() != NULL)
            VirtualConsole::instance()->dockArea()->hide();
        return true;
    };

    // If a startup workspace was chosen previously, try to open it immediately.
    // This is skipped when the dialog is explicitly requested (e.g. F1).
    StartupSettings startupSettings = readStartupSettings();
    const QString storedStartupWorkspace = startupSettings.workspacePath;
    bool showStoredWorkspaceError = false;
    if (forceDialog == false && storedStartupWorkspace.isEmpty() == false)
    {
        if (openWorkspaceAndCloseStartup(storedStartupWorkspace))
            return true;

        startupSettings.workspacePath.clear();
        saveStartupSettings(startupSettings);
        showStoredWorkspaceError = true;
    }

    auto workspaceFilesFromFolder = [](const QString &folderPath) -> QStringList
    {
        QDir folder(folderPath);
        if (folder.exists() == false)
            return QStringList();

        folder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        folder.setSorting(QDir::Name);
        folder.setNameFilters(QStringList() << "*.qxw" << "*.QXW");

        QStringList files;
        const QFileInfoList entries = folder.entryInfoList();
        for (const QFileInfo &entry : entries)
            files << entry.absoluteFilePath();

        return files;
    };

    MandatoryStartupDialog dialog(&app);
    dialog.setWindowTitle(QObject::tr("Documento Avvio"));
    dialog.setModal(true);
    dialog.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    Ui::SceltaDirectory startupUi;
    startupUi.setupUi(&dialog);

    QPushButton *directoryFilesButton = startupUi.directoryFilesButton;
    QLabel *folderLabel = startupUi.folderLabel;
    QListWidget *filesList = startupUi.filesList;
    QPushButton *openSelectedButton = startupUi.openSelectedButton;
    QCheckBox *alwaysOpenSelectedCheck = startupUi.alwaysOpenSelectedCheck;
    QPushButton *exitButton = startupUi.exitButton;

    alwaysOpenSelectedCheck->setChecked(storedStartupWorkspace.isEmpty() == false);
    openSelectedButton->setDefault(true);
    openSelectedButton->setAutoDefault(true);
    exitButton->setStyleSheet(QStringLiteral("QPushButton { background-color: #c62828; color: white; font-weight: 600; }"));

    QObject::connect(exitButton, &QPushButton::clicked, [&dialog]() {
        dialog.allowClose(true);
        dialog.setResult(QDialog::Rejected);
        dialog.close();
    });

    QString selectedFolder = startupSettings.folderPath;
    bool missingFolderAlertShown = false;
    auto refreshWorkspaceList = [&selectedFolder, &workspaceFilesFromFolder, folderLabel, filesList,
                                 &dialog, &missingFolderAlertShown]() {
        filesList->clear();

        if (selectedFolder.isEmpty())
        {
            folderLabel->setText(QObject::tr("Nessuna cartella selezionata."));
            missingFolderAlertShown = false;
            return;
        }

        QDir selectedDir(selectedFolder);
        if (selectedDir.exists() == false)
        {
            folderLabel->setText(QObject::tr("Cartella non trovata: %1").arg(selectedFolder));
            if (missingFolderAlertShown == false)
            {
                QMessageBox::warning(&dialog,
                                     QObject::tr("Errore"),
                                     QObject::tr("La cartella selezionata non esiste più.\nSeleziona una nuova cartella."));
                missingFolderAlertShown = true;
            }
            return;
        }

        folderLabel->setText(selectedFolder);
        missingFolderAlertShown = false;

        const QStringList workspaceFiles = workspaceFilesFromFolder(selectedFolder);
        for (const QString &workspaceFile : workspaceFiles)
        {
            QListWidgetItem *item = new QListWidgetItem(QFileInfo(workspaceFile).fileName(), filesList);
            item->setData(Qt::UserRole, workspaceFile);
        }
    };

    auto openWorkspaceFromSelection = [&app, &dialog, filesList, alwaysOpenSelectedCheck,
                                       &selectedFolder, &startupSettings, &saveStartupSettings]() {
        QListWidgetItem *selectedItem = filesList->currentItem();
        if (selectedItem == nullptr)
        {
            QMessageBox::information(&dialog,
                                     QObject::tr("Nessun file"),
                                     QObject::tr("Seleziona un file .qxw dall'elenco."));
            return;
        }

        const QString workspaceFile = selectedItem->data(Qt::UserRole).toString();
        if (workspaceFile.isEmpty())
            return;

        app.clearDocument();
        if (app.loadXML(workspaceFile) == QFile::NoError)
        {
            startupSettings.folderPath = selectedFolder;
            if (alwaysOpenSelectedCheck->isChecked())
                startupSettings.workspacePath = workspaceFile;
            else
                startupSettings.workspacePath.clear();

            saveStartupSettings(startupSettings);
            app.updateFileOpenMenu(workspaceFile);
            app.slotGoToVirtualConsole();
            if (VirtualConsole::instance() != NULL && VirtualConsole::instance()->dockArea() != NULL)
                VirtualConsole::instance()->dockArea()->hide();
            dialog.allowClose(true);
            dialog.accept();
            return;
        }

        QMessageBox::warning(&dialog,
                             QObject::tr("Errore"),
                             QObject::tr("Impossibile aprire il file selezionato."));
    };

    QObject::connect(directoryFilesButton, &QPushButton::clicked, [&dialog,
                                                                    &selectedFolder,
                                                                    &startupSettings,
                                                                    &saveStartupSettings,
                                                                    &refreshWorkspaceList,
                                                                    &missingFolderAlertShown]() {
        QString initialFolder = selectedFolder;
        if (initialFolder.isEmpty())
            initialFolder = QDir::homePath();

        const QString newFolder = QFileDialog::getExistingDirectory(
            &dialog,
            QObject::tr("Seleziona cartella workspace"),
            initialFolder,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (newFolder.isEmpty())
            return;

        selectedFolder = newFolder;
        missingFolderAlertShown = false;
        startupSettings.folderPath = selectedFolder;
        // force a new explicit selection from the refreshed list
        startupSettings.workspacePath.clear();
        saveStartupSettings(startupSettings);
        refreshWorkspaceList();
    });

    QObject::connect(openSelectedButton, &QPushButton::clicked, openWorkspaceFromSelection);
    QObject::connect(filesList, &QListWidget::itemDoubleClicked, [&openWorkspaceFromSelection](QListWidgetItem *) {
        openWorkspaceFromSelection();
    });

    // If a folder is already stored, immediately show its .qxw list.
    refreshWorkspaceList();

    dialog.adjustSize();
    QScreen *targetScreen = app.screen();
    if (targetScreen == nullptr)
        targetScreen = QGuiApplication::primaryScreen();

    if (targetScreen != nullptr)
    {
        const QRect screenGeometry = targetScreen->availableGeometry();
        dialog.move(screenGeometry.center() - dialog.rect().center());
    }

    if (showStoredWorkspaceError)
    {
        QMessageBox::warning(&dialog,
                             QObject::tr("Errore"),
                             QObject::tr("Il file di avvio selezionato non esiste più o non è apribile.\nSeleziona una nuova cartella/file."));
    }

    return dialog.exec() == QDialog::Accepted;
}

static void installStartupSettingsMenu(App &app)
{
    QMenuBar *menuBar = app.menuBar();
    menuBar->setStyleSheet(QStringLiteral(
        "QMenuBar::item { font-size: 16px; font-weight: 700; padding: 6px 12px; }"));

    QMenu *settingsMenu = menuBar->addMenu(QObject::tr("Menu"));

    QAction *settingsAction = new QAction(QIcon(":/configure.png"), QStringLiteral("[F1] Settaggio"), &app);
    settingsAction->setShortcut(QKeySequence(Qt::Key_F1));
    settingsAction->setShortcutContext(Qt::ApplicationShortcut);
    QObject::connect(settingsAction, &QAction::triggered, [&app]() {
        showStartupOpenWorkspaceDialog(app, true);
    });
    settingsMenu->addAction(settingsAction);

    settingsMenu->addSeparator();
    QAction *aboutAction = new QAction(QIcon(":/help.png"), QObject::tr("Chi siamo"), &app);
    QObject::connect(aboutAction, &QAction::triggered, &app, &App::slotHelpAbout);
    settingsMenu->addAction(aboutAction);

    QAction *exitAction = new QAction(QIcon(":/exit.png"), QObject::tr("Esci"), &app);
    QObject::connect(exitAction, &QAction::triggered, &app, &QWidget::close);
    settingsMenu->addAction(exitAction);
}

static void ensureKnownFixtureDefinitionAvailable()
{
    const QString fixtureFilename = QStringLiteral("Varytec-Colors-NerveStrobe-HP.qxf");

    QDir userFixturesDir = QLCFixtureDefCache::userDefinitionDirectory();
    const QString userFixturePath = userFixturesDir.absoluteFilePath(fixtureFilename);
    if (QFile::exists(userFixturePath))
        return;

    QStringList sourceCandidates;

    // 1) Installed system fixtures path
    QDir systemFixturesDir = QLCFixtureDefCache::systemDefinitionDirectory();
    sourceCandidates << systemFixturesDir.absoluteFilePath(QStringLiteral("Varytec") + QDir::separator() + fixtureFilename);

    // 2) Source tree / build tree common relative locations
    const QString appDir = QApplication::applicationDirPath();
    sourceCandidates << QDir(appDir).absoluteFilePath("../resources/fixtures/Varytec/" + fixtureFilename);
    sourceCandidates << QDir(appDir).absoluteFilePath("../../resources/fixtures/Varytec/" + fixtureFilename);
    sourceCandidates << QDir::current().absoluteFilePath("resources/fixtures/Varytec/" + fixtureFilename);

    for (const QString &candidate : sourceCandidates)
    {
        if (QFile::exists(candidate) == false)
            continue;

        QFile::copy(candidate, userFixturePath);
        if (QFile::exists(userFixturePath))
            return;
    }
}

/**
 * THE entry point for the application
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 */
int main(int argc, char** argv)
{
    /* Create the Qt core application object */
    QApplication qapp(argc, argv);
    QApplication::setApplicationName(APPNAME);
    QApplication::setApplicationVersion(QVrcInfo::appVersion());

    /* At least MIDI plugin requires this so best to declare it here for everyone */
    qRegisterMetaType<QVariant>("QVariant");

#if defined(__APPLE__) || defined(Q_OS_MAC)
    /* Load plugins from within the bundle ONLY */
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    QLCi18n::init();

    /* Let the world know... */
    printVersion();

    /* Parse command-line arguments */
    if (parseArgs() == false)
        return 0;

    /* Load translation for main application */
    QLCi18n::loadTranslation("qlcplus");

    /* Handle debug messages */
    qInstallMessageHandler(qlcMessageHandler);

    /* Create and initialize the QLC application object */
    App app;

    ensureKnownFixtureDefinitionAvailable();

    if (QLCArgs::enableOverscan == true)
        app.enableOverscan();

    if (QLCArgs::noWindowManager == true)
        QLCFile::setHasWindowManager(false);

    if (QLCArgs::noGui == true)
        app.disableGUI();

    app.startup();
    app.show();
    app.showMaximized();
    installStartupSettingsMenu(app);

    if (QLCArgs::workspace.isEmpty() == false)
    {
        if (app.loadXML(QLCArgs::workspace) == QFile::NoError)
            app.updateFileOpenMenu(QLCArgs::workspace);
    }
    else
    {
        app.slotGoToVirtualConsole();
        app.enableKioskMode();
        if (VirtualConsole::instance() != NULL && VirtualConsole::instance()->dockArea() != NULL)
            VirtualConsole::instance()->dockArea()->hide();

        if (showStartupOpenWorkspaceDialog(app) == false)
            return 0;
    }
    if (QLCArgs::operate == true)
        app.slotModeOperate();
    if (QLCArgs::kioskMode == true)
        app.enableKioskMode();
    if (QLCArgs::fullScreen == true && (app.windowState() & Qt::WindowFullScreen) == 0)
        app.slotControlFullScreen(QLCArgs::fullScreenResize);
    if (QLCArgs::kioskMode == true && QLCArgs::closeButtonRect.isValid() == true)
        app.createKioskCloseButton(QLCArgs::closeButtonRect);

    if (QLCArgs::enableWebAccess == true)
    {
        WebAccess *webAccess = new WebAccess(app.doc(), VirtualConsole::instance(), SimpleDesk::instance(),
                                             QLCArgs::webAccessPort, QLCArgs::enableWebAuth, QLCArgs::webAccessPasswordFile);

        QObject::connect(webAccess, SIGNAL(toggleDocMode()),
                &app, SLOT(slotModeToggle()));
        QObject::connect(webAccess, SIGNAL(loadProject(QString)),
                &app, SLOT(slotLoadDocFromMemory(QString)));
        QObject::connect(webAccess, SIGNAL(storeAutostartProject(QString)),
                &app, SLOT(slotSaveAutostart(QString)));
    }

    return qapp.exec();
}

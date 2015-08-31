#include "window.h"
#include "viewer.h"
#include "scene.h"

#include "ui_polyhedron.h"

#include <CGAL/Qt/debug.h>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextStream>
#include <QUrl>
#include <QFileDialog>
#include <QSettings>
#include <QHeaderView>
#include <QClipboard>
#include <QInputDialog>
#include <QMimeData>

MainWindow::MainWindow (QWidget* parent)
: CGAL::Qt::DemosMainWindow(parent) {
    ui = new Ui::MainWindow;
    ui->setupUi(this);

    // saves some pointers from ui, for latter use.
    m_pViewer = ui->viewer;

    // does not save the state of the viewer
    m_pViewer->setStateFileName(QString::null);

    // accepts drop events
    setAcceptDrops(true);

    // setups scene
    m_pScene = new Scene;
    m_pViewer->setScene(m_pScene);

    // connects actionQuit (Ctrl+Q) and qApp->quit()
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));

    this->addRecentFiles(ui->menuFile, ui->actionQuit);

    connect(this, SIGNAL(openRecentFile(QString)),
            this, SLOT(open(QString)));

    readSettings();
}

MainWindow::~MainWindow () {
    delete m_pScene;
    delete ui;
}

void MainWindow::dragEnterEvent (QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent (QDropEvent *event) {
    std::cout << "drop...";

    Q_FOREACH(QUrl url, event->mimeData()->urls()) {
        QString filename = url.toLocalFile();

        if(!filename.isEmpty()) {
            QTextStream(stderr) << QString("dropEvent(\"%1\")\n").arg(filename);
            open(filename);
        }
    }
    event->acceptProposedAction();
}

void MainWindow::closeEvent (QCloseEvent *event) {
    writeSettings();
    event->accept();
}

void MainWindow::quit() {
    writeSettings();
    close();
}

void MainWindow::readSettings () {
    this->readState("MainWindow", Size|State);
}

void MainWindow::writeSettings () {
    this->writeState("MainWindow");

    std::cerr << "Write setting...done" << std::endl;
}

void MainWindow::updateViewerBBox () {
    const Bbox bbox(-2.0, -2.0, -2.0, 2.0,  2.0,  2.0);

    const double xmin = bbox.xmin();
    const double ymin = bbox.ymin();
    const double zmin = bbox.zmin();
    const double xmax = bbox.xmax();
    const double ymax = bbox.ymax();
    const double zmax = bbox.zmax();

    qglviewer::Vec vec_min(xmin, ymin, zmin);
    qglviewer::Vec vec_max(xmax, ymax, zmax);

    m_pViewer->setSceneBoundingBox(vec_min, vec_max);
    m_pViewer->camera()->showEntireScene();
}

void MainWindow::on_actionView_ball_toggled () {
    m_pScene->toggle_view_ball();
    m_pViewer->update();
}

void MainWindow::on_actionView_edges_toggled () {
    m_pScene->toggle_view_edges();
    m_pViewer->update();
}

void MainWindow::on_actionView_facets_toggled () {
    m_pScene->toggle_view_facets();
    m_pViewer->update();
}

void MainWindow::on_actionView_smooth_toggled () {
    m_pScene->toggle_view_smooth();
    m_pViewer->update();
}

void MainWindow::on_actionView_pointcloud_toggled () {
    m_pScene->toggle_view_pointcloud();
    m_pViewer->update();
}

void MainWindow::on_actionView_vectorfield_toggled () {
    m_pScene->toggle_view_vectorfield();
    m_pViewer->update();
}

void MainWindow::on_actionView_intersections_toggled () {
    m_pScene->toggle_view_intersections();
    m_pViewer->update();
}

void MainWindow::open (QString filename) {
    QFileInfo fileinfo(filename);

    if (fileinfo.isFile() && fileinfo.isReadable()) {
        int index = m_pScene->open(filename);

        if (index >= 0) {
            QSettings settings;
            settings.setValue("open directory",
                              fileinfo.absoluteDir().absolutePath());
            this->addToRecentFiles(filename);

            // update bbox
            updateViewerBBox();
            m_pViewer->update();
        }
    }
}

void MainWindow::on_actionOpen_triggered () {
    QSettings settings;
    QString directory = settings.value("open directory",
                                       QDir::current().dirName()).toString();
    QStringList filenames =
        QFileDialog::getOpenFileNames(this,
                                      tr("Open polyhedral surface or point cloud..."),
                                      directory,
                                      tr("OFF files (*.off)\n"
                                         "XYZ files (*.xyz)\n"
                                         "All files (*)"));
    if (!filenames.isEmpty()) {
        Q_FOREACH (QString filename, filenames) {
            this->open(filename);
        }
    }
}

void MainWindow::on_actionSave_triggered () {
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Point Cloud"),
                                                    QDir::currentPath(),
                                                    tr("Point Clouds (*.xyz)"));
    m_pScene->save(filename);
}

void MainWindow::on_actionSaveNormals_triggered () {
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Point Cloud"),
                                                    QDir::currentPath(),
                                                    tr("Point Clouds (*.xyz)"));

    m_pScene->saveNormals(filename);
}

void MainWindow::on_actionReset_triggered () {
    m_pScene->clear_ball();
    m_pScene->clear_balls();
    m_pScene->clear_pointcloud();
    m_pScene->clear_vectorfield();
    m_pScene->clear_intersections();

    m_pScene->set_visible_ball(false);
    m_pScene->set_visible_edges(true);
    m_pScene->set_visible_facets(false);
    m_pScene->set_visible_pointcloud(true);
    m_pScene->set_visible_vectorfield(true);
    m_pScene->set_visible_intersections(false);

    ui->actionView_ball->setChecked(false);
    ui->actionView_edges->setChecked(true);
    ui->actionView_facets->setChecked(false);
    ui->actionView_pointcloud->setChecked(true);
    ui->actionView_vectorfield->setChecked(true);
    ui->actionView_intersections->setChecked(false);

    m_pViewer->update();
}

void MainWindow::on_actionVectorField_triggered () {
    m_pScene->vector_field();
    m_pViewer->update();
}

void MainWindow::on_actionNsteps_triggered () {
    m_pScene->nsteps();
    m_pViewer->update();
}

void MainWindow::on_actionAddNoise_triggered () {
    m_pScene->add_noise();
    m_pViewer->update();
}

void MainWindow::on_actionEllipsoid_triggered () {
    m_pScene->random_ellipsoid();
    m_pViewer->update();
}

void MainWindow::on_actionChangeRadius_triggered () {
    bool ok = false;
    double r;
    while (!ok) {
        r = QInputDialog::getDouble(NULL, "Balls", "Radius",
                                    0.5, 0, 10, 2, &ok);
    }
    m_pScene->setRadius(r);
}


/****************************************************************************
* MeshLab                                                           o o     *
* An extendible mesh processor                                    o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005, 2006                                          \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
History

$Log$
Revision 1.9  2005/11/18 18:10:28  alemochi
Aggiunto slot cambiare la modalita' di rendering

Revision 1.8  2005/11/18 02:12:04  glvertex
- Added icons to the project file [meshlab.qrc]
- Enabled renderToolbar with icons

Revision 1.7  2005/11/18 00:36:50  davide_portelli
Added View->Toolbar and Windows->Tile and Windows->Cascade

Revision 1.6  2005/11/17 22:15:52  davide_portelli
Added menu View->Tile

Revision 1.5  2005/11/17 20:51:00  davide_portelli
Added Help->About Plugins

Revision 1.4  2005/11/16 23:18:54  cignoni
Added plugins management

Revision 1.3  2005/10/31 17:17:47  cignoni
Sketched the interface  of the odd/even refine function

Revision 1.2  2005/10/22 10:23:28  mariolatronico
on meshlab.pro added a lib dependency , on other files modified #include directive to allow compilation on case sensitive o.s.

Revision 1.1  2005/10/18 10:38:02  cignoni
First rough version. It simply load a mesh.

****************************************************************************/


#include <QtGui>
#include <QToolBar>


#include "interfaces.h"
#include "mainwindow.h"
#include "glarea.h"
#include "plugindialog.h"
				
MainWindow::MainWindow()
{
	workspace = new QWorkspace(this);
	
	setCentralWidget(workspace);
	createActions();
	createMenus();
	createToolBars();

	addToolBar(mainToolBar);
	addToolBar(renderToolBar);

	setWindowTitle(tr("MeshLab v0.1"));

	

	//QTimer::singleShot(500, this, SLOT(aboutPlugins()));

	if(QCoreApplication::instance ()->argc()>1)
		open(QCoreApplication::instance ()->argv()[1]);
	else 
		QTimer::singleShot(500, this, SLOT(open()));

	loadPlugins();
}

void MainWindow::open(QString fileName)
{

	if (fileName.isEmpty()) 
	{
		/*QStringList types << 
		<< "Text files (*.txt)"
		<< "Any files (*)";
		QFileDialog fd = new QFileDialog( this );
		fd->setFilters( types );
		fd->show();*/

		fileName = QFileDialog::getOpenFileName(this,tr("Open File"),"../sample","Mesh files (*.ply *.off *.stl)");
	}
	if (!fileName.isEmpty()) {
		MeshModel *nm= new MeshModel();
		if(!nm->Open(fileName.toAscii())){
			QMessageBox::information(this, tr("Plug & Paint"),
				tr("Cannot load %1.").arg(fileName));

			delete nm;
			return;
		}
		else
		{
			//QMessageBox::information(this, tr("MeshLab"), tr("Opened Mesh of %1. triangles").arg(nm->cm.fn));
			VM.push_back(nm);
			gla=new GLArea(workspace);
			gla->mm=nm;
			gla->setWindowTitle(fileName);   
			workspace->addWindow(gla);
			gla->showMaximized();
			return;
		}
	}
}

bool MainWindow::saveAs()
{
	QString initialPath = QDir::currentPath() + "/untitled.png";

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), initialPath);
	if (fileName.isEmpty()) {
		return false;
	} else {
		//       return paintArea->saveImage(fileName, "png");
		return true;
	}
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Plug & Paint"),
		tr("The <b>Plug & Paint</b> example demonstrates how to write Qt "
		"applications that can be extended through plugins."));
}

void MainWindow::aboutPlugins()
{
	PluginDialog dialog(pluginsDir.path(), pluginFileNames, this);
	dialog.exec();
}

void MainWindow::createActions()
{
	openAct = new QAction(QIcon(":/images/open.png"),tr("&Open..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAsAct = new QAction(QIcon(":/images/save.png"),tr("&Save As..."), this);
	saveAsAct->setShortcut(tr("Ctrl+S"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	viewModePoints	= new QAction(QIcon(":/images/points.png"),tr("&Points"), this);
	viewModeWire		= new QAction(QIcon(":/images/wire.png"),tr("&Wireframe"), this);
	viewModeLines		= new QAction(QIcon(":/images/backlines.png"),tr("&Hidden Lines"), this);
	viewModeFlatLines = new QAction(QIcon(":/images/flatlines.png"),tr("Flat &Lines"), this);
	viewModeFlat		= new QAction(QIcon(":/images/flat.png"),tr("&Flat"), this);
	viewModeSmooth	= new QAction(QIcon(":/images/smooth.png"),tr("&Smooth"), this);

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(tr("&About"), this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	aboutPluginsAct = new QAction(tr("About &Plugins"), this);
	connect(aboutPluginsAct, SIGNAL(triggered()), this, SLOT(aboutPlugins()));

	viewToolbarAct = new QAction (tr("&Toolbar"), this);
	connect(viewToolbarAct, SIGNAL(triggered()), this, SLOT(viewToolbar()));

	windowsTileAct = new QAction(tr("&Tile"), this);
	connect(windowsTileAct, SIGNAL(triggered()), this, SLOT(windowsTile()));

	windowsCascadeAct = new QAction(tr("&Cascade"), this);
	connect(windowsCascadeAct, SIGNAL(triggered()), this, SLOT(windowsCascade()));
	
	connect(viewModeWire, SIGNAL(triggered()), this, SLOT(RenderWire()));
	connect(viewModePoints, SIGNAL(triggered()), this, SLOT(RenderPoint()));
	connect(viewModeFlat, SIGNAL(triggered()), this, SLOT(RenderFlat()));
	connect(viewModeSmooth, SIGNAL(triggered()), this, SLOT(RenderSmooth()));
	connect(viewModeFlatLines, SIGNAL(triggered()), this, SLOT(RenderFlatLine()));
	connect(viewModeLines, SIGNAL(triggered()), this, SLOT(RenderHiddenLines()));

	
	/*connect(windowsCascadeAct, SIGNAL(triggered()), this, SLOT(windowsCascade()));
	connect(windowsCascadeAct, SIGNAL(triggered()), this, SLOT(windowsCascade()));
	connect(windowsCascadeAct, SIGNAL(triggered()), this, SLOT(windowsCascade()));*/

	

}

void MainWindow::createToolBars()
{
	mainToolBar = new QToolBar(this);
	mainToolBar->setIconSize(QSize(32,32));
	mainToolBar->addAction(openAct);
	mainToolBar->addAction(saveAsAct);

	renderToolBar = new QToolBar(this);
	renderToolBar->setIconSize(QSize(32,32));
	renderToolBar->addAction(viewModePoints);
	renderToolBar->addAction(viewModeWire);
	renderToolBar->addAction(viewModeLines);
	renderToolBar->addAction(viewModeFlatLines);
	renderToolBar->addAction(viewModeFlat);
	renderToolBar->addAction(viewModeSmooth);
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	filterMenu = menuBar()->addMenu(tr("&Filter"));

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(viewToolbarAct);

	windowsMenu = menuBar()->addMenu(tr("&Windows"));
	windowsMenu->addAction(windowsTileAct);
	windowsMenu->addAction(windowsCascadeAct);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
	helpMenu->addAction(aboutPluginsAct);
}

void MainWindow::loadPlugins()
{
	pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
	if (pluginsDir.dirName() == "debug" || pluginsDir.dirName() == "release")
		pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	if (pluginsDir.dirName() == "MacOS") {
		pluginsDir.cdUp();
		pluginsDir.cdUp();
		pluginsDir.cdUp();
	}
#endif
	pluginsDir.cd("plugins");

	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if (plugin) {
			MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(plugin);
			if (iFilter)
				addToMenu(plugin, iFilter->filters(), filterMenu, SLOT(applyFilter()));                

			pluginFileNames += fileName;
		}
	}

	// brushMenu->setEnabled(!brushActionGroup->actions().isEmpty());
	// shapesMenu->setEnabled(!shapesMenu->actions().isEmpty());
	filterMenu->setEnabled(!filterMenu->actions().isEmpty());
}

void MainWindow::addToMenu(QObject *plugin, const QStringList &texts,QMenu *menu, const char *member,
													 QActionGroup *actionGroup)
{
	foreach (QString text, texts) {
		QAction *action = new QAction(text, plugin);
		connect(action, SIGNAL(triggered()), this, member);
		menu->addAction(action);

		if (actionGroup) {
			action->setCheckable(true);
			actionGroup->addAction(action);
		}
	}
}

void MainWindow::applyFilter()
{
	QAction *action = qobject_cast<QAction *>(sender());
	MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(action->parent());
	iFilter->applyFilter(action->text(), *(((GLArea *)(workspace->activeWindow()))->mm ), this);
}


void MainWindow::windowsTile(){
	workspace->tile();
}

void MainWindow::windowsCascade(){
	workspace->cascade();
}
void MainWindow::viewToolbar(){
	if(mainToolBar->isVisible()){
		mainToolBar->hide();
	}else{
		mainToolBar->show();
	}
}



void MainWindow::RenderPoint()
{
	gla->SetMode(vcg::GLW::DMPoints);

}

void MainWindow::RenderWire()
{
	gla->SetMode(vcg::GLW::DMWire);

}

void MainWindow::RenderFlat()
{
//	paintArea->SetMode(GLW::CHFace);
	gla->SetMode(vcg::GLW::DMFlat);

}

void MainWindow::RenderSmooth()
{
//	paintArea->SetMode(vcg::GLW::DrawMode::DMWire);
	gla->SetMode(vcg::GLW::DMSmooth);
	
}

void MainWindow::RenderFlatLine()
{
	gla->SetMode(vcg::GLW::DMFlatWire);

}

void MainWindow::RenderHiddenLines()
{
	//gla->SetMode(vcg::GLW::DMHidden);

}
/*
 * satk - SIM application toolkit
 * Copyright © 2011, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Written by - Luc Yriarte <luc.yriarte@linux.intel.com>
 */

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include "stkmainwindow.h"
#include "stkmenumodel.h"
#include "stkdefines.h"
#include "simimageprovider.h"


StkMainWindow::StkMainWindow(StkIf *stkIf, SimIf *simIf,
                             StkAgentService *stkAgentService, QWidget *parent) :
    QMainWindow(parent)
{
    mStkIf = stkIf;
    mSimIf = simIf;
    mStkAgentService = stkAgentService;
    // Create main view from properties
    createMainView();
    // Connect stkIf signals
    connect(mStkIf, SIGNAL(PropertyChanged(QString, QDBusVariant)),
            this, SLOT(stkPropertyChanged(QString, QDBusVariant)));
}


StkMainWindow::~StkMainWindow()
{
    delete mStkProperties;
    // mView is deleted as QMainWindow's centralWidget
}


void StkMainWindow::onEndSession()
{
    mStkAgentService->setExitOnRelease(true);
    close();
}


void StkMainWindow::responseOkWithSelection(int selection)
{
    uchar sel = (uchar)selection;
    QDBusObjectPath stkAgentPath(STK_AGENT_PATH);
    mStkIf->SelectItem(sel,stkAgentPath);
}


void StkMainWindow::stkPropertyChanged(const QString &property, const QDBusVariant &value)
{
    Q_UNUSED(property);
    Q_UNUSED(value);
    // save properties and view for deletion
    StkOfonoProperties *delStkProperties = mStkProperties;
    QQuickView *delView = mView ;
    // reload properties, recreate main view
    createMainView();
    // disconnect old view signals
    QObject *root = delView->rootObject();
    disconnect(root, 0, 0, 0);
    // delete old view and properties
    delete delStkProperties;
    delete delView;
}


void StkMainWindow::createMainView() {
    // Store SimToolkit interface properties
    mStkProperties = new StkOfonoProperties(mStkIf,mSimIf);
    // Create a qml view
    mView = new QQuickView;
    // Register image provider, deleted with the engine
    QQmlEngine * engine = mView->engine();
    engine->addImageProvider(SIM_IMAGE_PROVIDER, new SimImageProvider(mSimIf));

    // Load a qml main menu
    mView->setSource(QUrl("qrc:/StkMenu.qml"));
    // Set it as central widget
    mView->setResizeMode(QQuickView::SizeRootObjectToView);
    QWidget *container = QWidget::createWindowContainer(mView);
    setCentralWidget(container);
    QObject *root = mView->rootObject();

    // Main icon and title
    QObject *icon = root->findChild<QObject*>("icon");
    if (icon)
        icon->setProperty("source", mStkProperties->mainMenuIconUrl());
    QObject *title = root->findChild<QObject*>("title");
    if (title) {
        QString mainMenuTitle = mStkProperties->mainMenuTitle();
        if (!mainMenuTitle.isEmpty())
            title->setProperty("text", mainMenuTitle);
    }

    // SIM Toolkit Menu
    QQmlContext *context = mView->rootContext();
    StkMenuModel * menuModel = new StkMenuModel();
    menuModel->setMenuItems(mStkProperties->mainMenuItems());
    context->setContextProperty("menuModel",menuModel);

    // Hide back button in main view
    QObject *panel = root->findChild<QObject*>("panel");
    if (panel)
        panel->setProperty("showBackButton", false);

    // Connect view signals
    connect(root, SIGNAL(itemSelected(int)), this, SLOT(responseOkWithSelection(int)));
    connect(root, SIGNAL(endSession()), this, SLOT(onEndSession()));
}

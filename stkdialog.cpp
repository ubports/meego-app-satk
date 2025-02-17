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


#include <QVBoxLayout>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include "stkdialog.h"
#include "stkmenumodel.h"


StkDialog::StkDialog(SimImageProvider * imageProvider, const QString &iconUrl,
                     const QString &title, const QString &qmlViewUrl, QWidget *parent) :
    QDialog(parent,Qt::Window)
{
    mImageProvider = imageProvider;
    mIconUrl = iconUrl;
    mTitle = title;
    mQmlViewUrl = qmlViewUrl;
    mMinChars = mMaxChars = mSelection = -1;
    mLoopTone = mHideTyping = mNumeric = false;
    mShowBackButton = mShowEndButton = true;
}


void StkDialog::initView()
{
    // Create QML View as central widget
    mView = new QQuickView;

    // Register image provider, deleted with the engine
    QQmlEngine * engine = mView->engine();
    engine->addImageProvider(SIM_IMAGE_PROVIDER, mImageProvider);
    mView->setSource(QUrl(mQmlViewUrl));
    mView->setResizeMode(QQuickView::SizeRootObjectToView);
    QWidget *container = QWidget::createWindowContainer(mView);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(container);
    setLayout(layout);

    // Find qml objects and connect signals
    QObject *object;
    QObject *root = this->mView->rootObject();

    // Main icon and title
    object = root->findChild<QObject*>("icon");
    if (object)
        object->setProperty("source", mIconUrl);
    object = root->findChild<QObject*>("title");
    if (object && !mTitle.isEmpty())
        object->setProperty("text", mTitle);

    // SIM Toolkit Menu
    QObject * menuView = root->findChild<QObject*>("menuView");
    if (menuView) {
        // set model "menuModel" for ListView "menuView"
        QQmlContext *context = mView->rootContext();
        StkMenuModel * menuModel = new StkMenuModel();
        menuModel->setMenuItems(mMenuItems);
        context->setContextProperty("menuModel",menuModel);
        connect(root, SIGNAL(itemSelected(int)), this, SLOT(responseOkWithSelection(int)));
        if (mSelection != -1)
            menuView->setProperty("currentIndex", mSelection);
    }

    // Text edit / input
    QObject * editText = root->findChild<QObject*>("editText");
    if (editText) {
        editText->setProperty("text",mDefaultText);
        editText->setProperty("isNumeric",mNumeric);
        editText->setProperty("hideTyping",mHideTyping);
        editText->setProperty("minChars",mMinChars);
        editText->setProperty("maxChars",mMaxChars);
        connect(root, SIGNAL(textEntered(QString)), this, SLOT(responseOkWithText(QString)));
    }

    // Audio tone
    QObject * audioTone = root->findChild<QObject*>("audioTone");
    if (audioTone && !mToneSource.isEmpty()) {
        // Default StkPlaySound.qml audioTone loops: SoundEffect.Infinite
        if (!mLoopTone)
            audioTone->setProperty("loops", 1);
        audioTone->setProperty("source", "qrc:/audio/" + mToneSource + ".wav");
    }

    // Browser View
    QObject * browserView = root->findChild<QObject*>("browserView");
    if (browserView && !mUrl.isEmpty())
        browserView->setProperty("url", mUrl);

    // SIM Toolkit Panel
    QObject * panel = root->findChild<QObject*>("panel");
    if (panel) {
        // Default StkPanel.qml showEndButton: true
        if (mShowEndButton) {
            object = panel->findChild<QObject*>("endRect");
            if (object)
                connect(root, SIGNAL(endSession()), this, SLOT(responseEnd()));
        } else {
            panel->setProperty("showEndButton", false);
        }
        // Default StkPanel.qml showBackButton: true
        if (mShowBackButton) {
            object = panel->findChild<QObject*>("backRect");
            if (object)
                connect(root, SIGNAL(goBack()), this, SLOT(responseBack()));
        } else {
            panel->setProperty("showBackButton", false);
        }
    }

    // Yes button
    object = root->findChild<QObject*>("yesRect");
    if (object)
        connect(root, SIGNAL(accepted()), this, SLOT(responseYes()));

    // No button
    object = root->findChild<QObject*>("noRect");
    if (object)
        connect(root, SIGNAL(rejected()), this, SLOT(responseNo()));

    // Ok button
    object = root->findChild<QObject*>("okRect");
    if (object && !menuView && !editText)
        connect(root, SIGNAL(accepted()), this, SLOT(responseOk()));

    // default response: end session
    agentResponse = End;
}



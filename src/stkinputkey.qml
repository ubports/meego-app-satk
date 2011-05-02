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


import Qt 4.7
import MeeGo.Components 0.1

Rectangle {
    id: view
    objectName: "view"
    width: 480
    height: 320

    Theme { id: theme }
    color: theme.dialogBackgroundColor

    signal accepted()
    onAccepted: {
        console.log("Accepted")
        view.textEntered(editText.text)
    }
    signal textEntered(string text)
    onTextEntered: console.log("Text entered: " + text)
    signal goBack()
    onGoBack: console.log("Go back")
    signal endSession()
    onEndSession: console.log("End session")

    Label {
        id: title
        objectName: "title"
        text: qsTr( "SIM Application Toolkit")
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: icon.right
        anchors.leftMargin: 10
        anchors.right: endRect.left
        anchors.rightMargin: 10
        anchors.bottom: editText.top
        anchors.bottomMargin: 10
        wrapMode: Text.WordWrap
        color: theme.dialogTitleFontColor
        font.pixelSize: theme.dialogTitleFontPixelSize
    }

    Image {
        id: icon
        objectName: "icon"
        width: 64
        height: 64
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
    }

    TextEntry {
        id: editText
        objectName: "editText"
        width: 80
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: okRect.top
        anchors.bottomMargin: 10
        textInput.horizontalAlignment: TextInput.AlignHCenter
        textInput.focus: true
        textInput.font.pixelSize: theme.fontPixelSizeLargest
        property bool isNumeric: false
        onTextChanged: {
            if (text.length > 0) {
                text = text.charAt(text.length-1);
                if (isNumeric)
                    if (text.charAt(0)<'0' || text.charAt(0)>'9')
                        text = "";
            }
        }
    }

    Button {
        id: endRect
        objectName: "endRect"
        width: 80
        height: 30
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        text: qsTr( "End")
        onClicked: {
            view.endSession()
        }
    }

    Button {
        id: backRect
        objectName: "backRect"
        width: 80
        height: 30
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        text: qsTr( "Back")
        onClicked: {
            view.goBack()
        }
    }

    Button {
        id: okRect
        objectName: "okRect"
        width: 80
        height: 30
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        text: qsTr( "Ok")
        onClicked: {
            view.accepted()
        }
    }

}




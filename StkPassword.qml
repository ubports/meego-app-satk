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


/*!
    \file StkPassword.qml
    \section StkPassword
    \brief Multiple char password input.

    \subsection Signals
    \li accepted()
    \li textEntered(string text)
    \li goBack()
    \li endSession()

    \subsection Objects
    - \b title : Title bar
      - Properties
        - text: "SIM Application Toolkit"
    - \b icon : Main icon.
      - Properties
        - source
    - \b editText : Multiple char input.
      - Properties
        - text
        - isNumeric: false
        - hideTyping: true
        - minChars: -1
        - maxChars: -1
    - \b formatMsgBox : Format error dialog box.
      - Properties
        - title: "Attention"
        - text: ""
        - acceptButtonText: "Ok"
    - \b okRect : "Ok" button.
      - Signals
        - accepted()
      - Properties
        - text: "Ok"
*/

import Qt 4.7
import MeeGo.Components 0.1
import "qrc:/StkUiCheck.js" as StkUiCheck

Rectangle {
    id: view
    objectName: "view"
    width: stkTheme.viewWidth
    height: stkTheme.viewHeight

    Theme { id: theme }
    StkTheming { id: stkTheme }
    color: stkTheme.viewBackgroundColor

    signal accepted()
    onAccepted: {
        console.log("Accepted");
        console.log("Text len: ", editText.text.length, ", bounds: [", editText.minChars, "..", editText.maxChars, "], numeric: ", editText.isNumeric);
        if (editText.minChars != -1 && editText.text.length < editText.minChars) {
            formatMsgBox.text = qsTr("You entered ") + editText.text.length + qsTr(" characters.") + "\n" +
            qsTr("Please enter at least ") + editText.minChars + qsTr(" characters.");
            formatMsgBox.show();
            return;
        }
        if (editText.maxChars != -1 && editText.text.length > editText.maxChars) {
            formatMsgBox.text = qsTr("You entered ") + editText.text.length + qsTr(" characters.") + "\n" +
            qsTr("Please enter no more than ") + editText.maxChars + qsTr(" characters.");
            formatMsgBox.show();
            return;
        }
        if (editText.isNumeric && !StkUiCheck.checkNumericString(editText.text)) {
            formatMsgBox.text = qsTr("Please enter numbers only");
            formatMsgBox.show();
            return;
        }
        view.textEntered(editText.text)
    }
    signal textEntered(string text)
    onTextEntered: console.log("Text entered: " + text)
    signal goBack()
    onGoBack: console.log("Go back")
    signal endSession()
    onEndSession: console.log("End session")

    StkPanel {
        id: panel
        objectName: "panel"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onGoBack: view.goBack();
        onEndSession: view.endSession();
    }

    Label {
        id: title
        objectName: "title"
        text: "SIM Application Toolkit"
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: icon.right
        anchors.leftMargin: 10
        anchors.right: panel.left
        anchors.rightMargin: 10
        wrapMode: Text.WordWrap
        color: stkTheme.titleFontColor
        font.pixelSize: stkTheme.titleFontPixelSize
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
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: icon.right
        anchors.leftMargin: 10
        anchors.right: panel.left
        anchors.rightMargin: 10
        textInput.horizontalAlignment: TextInput.AlignHCenter
        textInput.focus: true
        textInput.echoMode: TextInput.Password
        property bool isNumeric: false
        property bool hideTyping: true
        property int minChars: -1
        property int maxChars: -1
    }

    Button {
        id: okRect
        objectName: "okRect"
        width: stkTheme.buttonWidth
        height: stkTheme.buttonHeight
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        text: qsTr( "Ok")
        onClicked: {
            view.accepted()
        }
    }

    ModalMessageBox {
        id: formatMsgBox
        showAcceptButton: true
        showCancelButton: false
        fogClickable: false
        acceptButtonText: qsTr( "Ok")
    }

}






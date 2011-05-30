import Qt 4.7
import MeeGo.Components 0.1

Item {
    /* references to the Meego default theme */
    Theme { id: theme }

    /* view properties */
    property int viewWidth: 480
    property int viewHeight: 320
    property string viewBackgroundColor: "#a0a0b0"

    /* title properties */
    property string titleFontColor: theme.dialogTitleFontColor
    property int titleFontPixelSize: theme.dialogTitleFontPixelSize

    /* panel properties */
    property int panelColumnWidth: 90
    property int panelLineHeight: 40
    property string panelBackgroundColor: "#808090"

    /* button properties */
    property int buttonWidth: 80
    property int buttonHeight: 30

    /* menu properties */
    property string menuItemFontColor: theme.contextMenuFontColor
    property int menuItemFontPixelSize: theme.fontPixelSizeMediumLarge
}
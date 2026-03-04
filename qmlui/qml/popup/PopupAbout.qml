/*
  Q Light Controller Plus
  PopupAbout.qml

  Copyright (c) Massimo Callegari

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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: "VRClub Italy"
    standardButtons: Dialog.Close

    contentItem:
        GridLayout
        {
            columnSpacing: UISettings.iconSizeMedium

            Image
            {
                source: "qrc:/vrclubitaly.jpg"
                width: UISettings.iconSizeDefault * 4
                height: UISettings.iconSizeDefault * 3
                sourceSize: Qt.size(width, height)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Text
            {
                color: UISettings.fgMain
                linkColor: "#8AC800"
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                text: "<h3>VRClub Italy<br>" + qlcplus.appVersion + "</h3>\n" +
                      "<b>Versione Visual</b><br>" +
                      "Copyright Ⓒ <b>VRClub Italy</b><br>" +
                      qsTr("Website") + ": <a href='https://www.vrclubitaly.it'>https://www.vrclubitaly.it</a><br>" +
                      "Facebook" + ": <a href='https://www.facebook.com/vrclubitaly/?locale=it_IT'>vrclubitaly</a><br>"
                onLinkActivated: (link) => Qt.openUrlExternally(link)
                Layout.fillWidth: true
                wrapMode: Text.WordWrap

                MouseArea
                {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

        }
}

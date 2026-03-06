/*
  Q Light Controller Plus
  PopupEncrypt.qml

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
import QtQuick.Dialogs

import "."

CustomPopupDialog
{
    id: encryptPopup
    title: qsTr("Cripta file .qxw")
    standardButtons: Dialog.Close
    width: Math.min(mainView.width * 0.6, UISettings.bigItemHeight * 9)

    property string sourceFile: ""
    property string statusMessage: ""
    property bool statusOk: false

    contentItem:
        ColumnLayout
        {
            spacing: 8

            RobotoText
            {
                Layout.fillWidth: true
                label: qsTr("File sorgente (.qxw):")
            }

            RowLayout
            {
                Layout.fillWidth: true
                spacing: 4

                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    color: UISettings.bgControl
                    border.width: 1
                    border.color: UISettings.bgLight

                    RobotoText
                    {
                        anchors.fill: parent
                        leftMargin: 6
                        rightMargin: 6
                        textVAlign: Text.AlignVCenter
                        label: encryptPopup.sourceFile !== "" ? encryptPopup.sourceFile : qsTr("Nessun file selezionato")
                        color: encryptPopup.sourceFile !== "" ? UISettings.fgMain : UISettings.fgMedium
                    }
                }

                Button
                {
                    text: qsTr("Sfoglia...")
                    onClicked: openFileDialog.open()

                    contentItem:
                        Text
                        {
                            text: parent.text
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault
                            color: UISettings.fgMain
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                    background:
                        Rectangle
                        {
                            color: parent.hovered ?
                                       parent.down ? UISettings.highlightPressed : UISettings.highlight
                                     : UISettings.bgLight
                            border.color: UISettings.bgStrong
                            border.width: 2
                        }
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Cripta e Salva")
                enabled: encryptPopup.sourceFile !== ""

                onClicked:
                {
                    encryptPopup.statusMessage = ""
                    saveFileDialog.open()
                }

                contentItem:
                    Text
                    {
                        text: parent.text
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault
                        opacity: parent.enabled ? 1.0 : 0.4
                        color: UISettings.fgMain
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                background:
                    Rectangle
                    {
                        color: parent.hovered ?
                                   parent.down ? UISettings.highlightPressed : UISettings.highlight
                                 : UISettings.bgLight
                        opacity: parent.enabled ? 1.0 : 0.4
                        border.color: UISettings.bgStrong
                        border.width: 2
                    }
            }

            RobotoText
            {
                Layout.fillWidth: true
                visible: encryptPopup.statusMessage !== ""
                label: encryptPopup.statusMessage
                color: encryptPopup.statusOk ? "green" : "red"
                wrapText: true
            }
        }

    FileDialog
    {
        id: openFileDialog
        title: qsTr("Apri file .qxw")
        fileMode: FileDialog.OpenFile
        currentFolder: qlcplus.workingPath
        nameFilters: [ qsTr("QLC+ workspace") + " (*.qxw)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            var path = selectedFile.toString()
            if (path.startsWith("file:"))
                path = Qt.resolvedUrl(selectedFile).toString().replace(/^file:\/\//, "")
            encryptPopup.sourceFile = selectedFile.toString()
            encryptPopup.statusMessage = ""
        }
    }

    FileDialog
    {
        id: saveFileDialog
        title: qsTr("Salva file .igm")
        fileMode: FileDialog.SaveFile
        currentFolder: qlcplus.workingPath
        defaultSuffix: "igm"
        nameFilters: [ qsTr("IGM file") + " (*.igm)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            var result = qlcplus.encryptWorkspace(encryptPopup.sourceFile, selectedFile.toString())
            if (result === "")
            {
                encryptPopup.statusOk = true
                encryptPopup.statusMessage = qsTr("File cifrato salvato con successo.")
            }
            else
            {
                encryptPopup.statusOk = false
                encryptPopup.statusMessage = qsTr("Errore: ") + result
            }
        }
    }
}
